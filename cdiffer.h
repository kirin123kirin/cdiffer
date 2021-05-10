#pragma once
#define PY_SSIZE_T_CLEAN
#include <Python.h>

using namespace std;

#if PY_MAJOR_VERSION == 2
typedef Py_ssize_t Py_hash_t;
#define PyUnicode_GET_LENGTH PyUnicode_GET_SIZE
#define PyLong_FromSize_t PyInt_FromSize_t
#define PyLong_FromUnsignedLong PyInt_FromSize_t
#define PyLong_AsLong PyInt_AsLong
#endif

/* Internal Common definition */
size_t error_n = (size_t)(-1);
#define REPLACEMENT_RATE 60

/* Edit type */
#define ED_EQUAL 0
#define ED_REPLACE 1
#define ED_INSERT 2
#define ED_DELETE 3
#define ED_LAST 4

extern PyObject* PYDIFF_t[ED_LAST];

inline static Py_hash_t pyhash(PyObject* py);

template <typename T>
struct hasher {
   private:
    bool be_hash_clear = false;
    bool be_ref_clear = false;

   public:
    PyObject* py;
    T* hash;
    Py_ssize_t len;

    hasher() {
        hash = NULL;
        py = NULL;
        len = -1;
    };

    hasher(T* _hash, PyObject* o) {
        hash = _hash;
        py = o;
        if((len = PyObject_Length(o)) == -1) {
            PyErr_Clear();
            if(o == Py_None)
                len = 0;
            else if(PyNumber_Check(o) || PyBool_Check(o))
                len = 1;
        }
    };

    hasher(PyObject* o) {
        hash = NULL;
        py = o;
        len = -1;

        if((len = PyObject_Length(o)) == -1) {
            PyErr_Clear();

            if(PyNumber_Check(o) || PyBool_Check(o) || o == Py_None) {
                len = 1;
                hash = new T[len];
                hash[0] = pyhash(py);
                be_hash_clear = true;
                return;
            }
            // for iter, generator
            else {
                py = PySequence_Tuple(o);
                be_ref_clear = true;
                be_hash_clear = true;
                if((len = PyObject_Length(py)) == 0) {
                    hash = new T[1];
                    hash[0] = pyhash(py);
                } else {
                    hash = new T[len];
                    for(Py_ssize_t i = 0; i < len; i++) {
                        hash[i] = pyhash(PySequence_ITEM(py, i));
                    }
                }
                return;
            }
        }

        if(PyDict_Check(o) || PyAnySet_Check(o) ||
           PyObject_TypeCheck(o, &PyDictKeys_Type) ||
           PyObject_TypeCheck(o, &PyDictValues_Type) ||
           PyObject_TypeCheck(o, &PyDictItems_Type)) {
            py = PySequence_Tuple(o);
            be_ref_clear = true;
        }

        if(len == 0) {
            hash = new T[1];
            hash[0] = pyhash(py);
            be_hash_clear = true;
            return;
        }

        // for range object. and other...
        if(PySequence_Check(py)) {
            hash = new T[len];
            be_hash_clear = true;
            for(Py_ssize_t i = 0; i < len; i++) {
                hash[i] = pyhash(PySequence_ITEM(py, i));
            }
            return;
        } else {
            hash = new T[len];
            hash[0] = pyhash(py);
            be_hash_clear = true;
            return;
        }

        PyErr_Format(PyExc_TypeError, "Unknown Data Type. `%s`",
                     py->ob_type->tp_name);
        return;
    }

    ~hasher() {
        if(be_hash_clear && len >= 0)
            this->clear();
    }

    void clear() {
        if(be_ref_clear) {
            Py_CLEAR(py);
            be_ref_clear = false;
        }
        py = NULL;
        if(hash && len >= 0)
            delete[] hash;
        hash = NULL;
        len = -1;
    }

    T operator[](Py_ssize_t index) {
        if(len < index + 1)
            throw "Index is out of bounds";
        return hash[index];
    }
};

template <typename T1, typename T2>
size_t distance(size_t len1, T1* seq1, size_t len2, T2* seq2) {
    size_t i;
    size_t* row; /* we only need to keep one row of costs */
    size_t* end;
    size_t half;

    /* strip common prefix */
    while(len1 > 0 && len2 > 0 && *seq1 == *seq2) {
        len1--;
        len2--;
        seq1++;
        seq2++;
    }

    /* strip common suffix */
    while(len1 > 0 && len2 > 0 && seq1[len1 - 1] == seq2[len2 - 1]) {
        len1--;
        len2--;
    }

    /* catch trivial cases */
    if(len1 == 0)
        return len2;
    if(len2 == 0)
        return len1;

    /* check len1 == 1 separately */
    if(len1 == 1) {
        T1 z = *seq1;
        const T2* p = seq2;
        for(i = len2; i; i--) {
            if(*(p++) == z)
                return len2 - 1;
        }
        return len2;
    }
    len1++;
    len2++;
    half = len1 >> 1;

    /* initalize first row */
    row = new size_t[len2];
    if(!row)
        return (size_t)(-1);
    end = row + len2 - 1;

    for(i = 0; i < len2 - half; i++)
        row[i] = i;

    /* in this case we don't have to scan two corner triangles (of size len1/2)
     * in the matrix because no best path can go throught them. note this
     * breaks when len1 == len2 == 2 so the memchr() special case above is
     * necessary */
    row[0] = len1 - half - 1;
    for(i = 1; i < len1; i++) {
        size_t* p;
        const T1 child1 = seq1[i - 1];
        const T2* child2;
        size_t D, x;
        /* skip the upper triangle */
        if(i >= len1 - half) {
            size_t offset = i - (len1 - half);
            size_t c3;

            child2 = seq2 + offset;
            p = row + offset;
            c3 = *(p++) + (child1 != *(child2++));
            x = *p;
            x++;
            D = x;
            if(x > c3)
                x = c3;
            *(p++) = x;
        } else {
            p = row + 1;
            child2 = seq2;
            D = x = i;
        }
        /* skip the lower triangle */
        if(i <= half + 1)
            end = row + len2 + i - half - 2;
        /* main */
        while(p <= end) {
            size_t c3 = --D + (child1 != *(child2++));
            x++;
            if(x > c3)
                x = c3;
            D = *p;
            D++;
            if(x > D)
                x = D;
            *(p++) = x;
        }
        /* lower triangle sentinel */
        if(i <= half) {
            size_t c3 = --D + (child1 != *child2);
            x++;
            if(x > c3)
                x = c3;
            *p = x;
        }
    }

    i = *end;
    delete[] row;
    return i;
}

/*
 * bridge function for `dist_py and similar_py.`.
 */
static size_t dist_op(PyObject* arg1, PyObject* arg2, size_t* lensum);

extern "C" PyObject* dist_py(PyObject* self, PyObject* args);
extern "C" PyObject* similar_py(PyObject* self, PyObject* args);
extern "C" PyObject* differ_py(PyObject* self,
                               PyObject* args,
                               PyObject* kwargs);

template <typename T = uint64_t>
void makelist(PyObject*& ops,
              int tp,
              size_t aidx,
              const hasher<T>& hash1,
              size_t bidx,
              const hasher<T>& hash2,
              bool swapflag) {
    PyObject* list = PyList_New(5);

    if(swapflag) {
        if(tp == ED_DELETE)
            tp = ED_INSERT;
        else if(tp == ED_INSERT)
            tp = ED_DELETE;
    }
    Py_INCREF(PYDIFF_t[tp]);
    PyList_SET_ITEM(list, 0, PYDIFF_t[tp]);
    if(aidx == error_n) {
        Py_INCREF(Py_None);
        PyList_SET_ITEM(list, 1 + swapflag, Py_None);
        Py_INCREF(Py_None);
        PyList_SET_ITEM(list, 3 + swapflag, Py_None);
    } else {
        PyList_SET_ITEM(list, 1 + swapflag, PyLong_FromSize_t(aidx));
        if(hash1.len < 0) {
            Py_INCREF(Py_None);
            PyList_SET_ITEM(list, 3 + swapflag, Py_None);
        } else if(PySequence_Check(hash1.py)) {
            PyList_SET_ITEM(list, 3 + swapflag,
                            PySequence_GetItem(hash1.py, aidx));
        } else {
            Py_INCREF(hash1.py);
            PyList_SET_ITEM(list, 3 + swapflag, hash1.py);
        }
    }
    if(bidx == error_n) {
        Py_INCREF(Py_None);
        PyList_SET_ITEM(list, 2 - swapflag, Py_None);
        Py_INCREF(Py_None);
        PyList_SET_ITEM(list, 4 - swapflag, Py_None);
    } else {
        PyList_SET_ITEM(list, 2 - swapflag, PyLong_FromSize_t(bidx));
        if(hash2.len < 0) {
            Py_INCREF(Py_None);
            PyList_SET_ITEM(list, 4 - swapflag, Py_None);
        } else if(PySequence_Check(hash2.py)) {
            PyList_SET_ITEM(list, 4 - swapflag,
                            PySequence_GetItem(hash2.py, bidx));
        } else {
            Py_INCREF(hash2.py);
            PyList_SET_ITEM(list, 4 - swapflag, hash2.py);
        }
    }
    PyList_Append(ops, list);
    Py_DECREF(list);
}

PyObject* makelist(int tp,
                   size_t aidx,
                   size_t bidx,
                   PyObject* pya,
                   PyObject* pyb);

void makelist(PyObject*& ops,
              int tp,
              size_t aidx,
              size_t bidx,
              PyObject* pya,
              PyObject* pyb);

/* diffonly true version */
template <typename T>
static PyObject* difference(const hasher<T>& hash1,
                            const hasher<T>& hash2,
                            size_t rep_rate = REPLACEMENT_RATE,
                            bool swapflag = false) {
    size_t len1 = hash1.len;
    size_t len2 = hash2.len;
    const T* seq1 = hash1.hash;
    const T* seq2 = hash2.hash;
    size_t o1, o2;
    size_t i;
    size_t* matrix; /* cost matrix */
    size_t n;

    if(len1 == 0 && len2 == 0) {
        PyErr_Format(PyExc_ValueError, "Cannot diff Empty data.");
        return NULL;
    }

    /* strip common prefix */
    o1 = 0;
    while(len1 > 0 && len2 > 0 && *seq1 == *seq2) {
        len1--;
        len2--;
        seq1++;
        seq2++;
        o1++;
    }
    o2 = o1;

    if(len1 == 0 && len2 == 0) {
        return PyList_New(0);
    }

    /* strip common suffix */
    while(len1 > 0 && len2 > 0 && seq1[len1 - 1] == seq2[len2 - 1]) {
        len1--;
        len2--;
    }

    len1++;
    len2++;

    /* initalize first row and column */
    matrix = new size_t[len1 * len2];
    if(!matrix) {
        n = (size_t)(-1);
        PyErr_Format(PyExc_MemoryError, "Cannot keep memory.");
        return NULL;
    }
    for(i = 0; i < len2; i++)
        matrix[i] = i;
    for(i = 1; i < len1; i++)
        matrix[len2 * i] = i;

    /* find the costs and fill the matrix */
    for(i = 1; i < len1; i++) {
        size_t* prev = matrix + (i - 1) * len2;
        size_t* P = matrix + i * len2;
        size_t* end = P + len2 - 1;
        const T child1 = seq1[i - 1];
        const T* child2 = seq2;
        size_t x = i;
        P++;
        while(P <= end) {
            size_t c3 = *(prev++) + (child1 != *(child2++));
            x++;
            if(x > c3)
                x = c3;
            c3 = *prev + 1;
            if(x > c3)
                x = c3;
            *(P++) = x;
        }
    }

    /* find the way back */
    size_t* p;
    size_t j, k, dist;
    int dir = 0;

    PyObject* ops = PyList_New(0);
    if(!ops) {
        n = (size_t)(-1);
        return NULL;
    }

    n = matrix[len1 * len2 - 1];
    if(!n) {
        delete[] matrix;
        return NULL;
    }

    j = len1 - 1;
    k = len2 - 1;
    p = matrix + len1 * len2 - 1;

    while(j || k) {
        /* prefer contiuning in the same direction */
        if(dir < 0 && k && *p == *(p - 1) + 1) {
            makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
            p--;
            continue;
        }
        if(dir > 0 && j && *p == *(p - len2) + 1) {
            makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            p -= len2;
            continue;
        }
        if(j && k && *p == *(p - len2 - 1) && seq1[j - 1] == seq2[k - 1]) {
            /* don't be stupid like difflib, don't store ED_KEEP */
            j--;
            k--;
            p -= len2 + 1;
            dir = 0;
            continue;
        }
        if(j && k && *p == *(p - len2 - 1) + 1) {
            if(rep_rate == 0) {
                makelist(ops, ED_REPLACE, --j + o1, hash1, --k + o2, hash2,
                         swapflag);
            } else {
                PyObject* pya = PySequence_GetItem(hash1.py, --j + o1);
                PyObject* pyb = PySequence_GetItem(hash2.py, --k + o2);
                ssize_t clen1 = PyObject_Length(pya);
                ssize_t clen2 = PyObject_Length(pyb);
                PyErr_Clear();
                if((clen1 <= 0 || clen2 <= 0) || (clen1 == 1 && clen2 == 1)) {
                    makelist(ops, ED_INSERT, -1, hash1, k + o2, hash2,
                             swapflag);
                    n++;
                    makelist(ops, ED_DELETE, j + o1, hash1, -1, hash2,
                             swapflag);
                } else {
                    size_t lensum;
                    dist = dist_op(pya, pyb, &lensum);
                    if(lensum > 0 &&
                       (100 * (lensum - dist) / lensum) >= rep_rate) {
                        makelist(ops, ED_REPLACE, j + o1, hash1, k + o2, hash2,
                                 swapflag);
                    } else {
                        makelist(ops, ED_INSERT, -1, hash1, k + o2, hash2,
                                 swapflag);
                        n++;
                        makelist(ops, ED_DELETE, j + o1, hash1, -1, hash2,
                                 swapflag);
                    }
                }
            }
            p -= len2 + 1;
            dir = 0;
            continue;
        }
        /* we cant't turn directly from -1 to 1, in this case it would be better
         * to go diagonally, but check it (dir == 0) */
        if(dir == 0 && k && *p == *(p - 1) + 1) {
            makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
            p--;
            dir = -1;
            continue;
        }
        if(dir == 0 && j && *p == *(p - len2) + 1) {
            makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            p -= len2;
            dir = 1;
            continue;
        }
        /* coredump right now, later might be too late ;-) */
        assert("lost in the cost matrix" == NULL);
    }
    delete[] matrix;
    PyList_Reverse(ops);
    return ops;
}

/* diffonly false version */
template <typename T>
static PyObject* difference_with_equal(const hasher<T>& hash1,
                                       const hasher<T>& hash2,
                                       size_t rep_rate = REPLACEMENT_RATE,
                                       bool swapflag = false) {
    size_t len1 = hash1.len;
    size_t len2 = hash2.len;
    const T* seq1 = hash1.hash;
    const T* seq2 = hash2.hash;
    size_t o1, o2;
    size_t i;
    size_t* matrix; /* cost matrix */
    size_t n;

    if(len1 == 0 && len2 == 0) {
        PyErr_Format(PyExc_ValueError, "Cannot diff Empty data.");
        return NULL;
    }

    /* strip common prefix */
    o1 = 0;
    while(len1 > 0 && len2 > 0 && *seq1 == *seq2) {
        len1--;
        len2--;
        seq1++;
        seq2++;
        o1++;
    }
    o2 = o1;

    PyObject* ops = PyList_New(0);
    if(!ops) {
        n = (size_t)(-1);
        return NULL;
    }

    if(len1 == 0 && len2 == 0) {
        for(size_t i = 0; i < o1; i++) {
            makelist(ops, ED_EQUAL, i, hash1, i, hash2, swapflag);
        }
        return ops;
    }

    /* strip common suffix */
    while(len1 > 0 && len2 > 0 && seq1[len1 - 1] == seq2[len2 - 1]) {
        makelist(ops, ED_EQUAL, --len1 + o1, hash1, --len2 + o2, hash2, swapflag);
    }

    len1++;
    len2++;

    /* initalize first row and column */
    matrix = new size_t[len1 * len2];
    if(!matrix) {
        n = (size_t)(-1);
        PyErr_Format(PyExc_MemoryError, "Cannot keep memory.");
        return NULL;
    }
    for(i = 0; i < len2; i++)
        matrix[i] = i;
    for(i = 1; i < len1; i++)
        matrix[len2 * i] = i;

    /* find the costs and fill the matrix */
    for(i = 1; i < len1; i++) {
        size_t* prev = matrix + (i - 1) * len2;
        size_t* P = matrix + i * len2;
        size_t* end = P + len2 - 1;
        const T child1 = seq1[i - 1];
        const T* child2 = seq2;
        size_t x = i;
        P++;
        while(P <= end) {
            size_t c3 = *(prev++) + (child1 != *(child2++));
            x++;
            if(x > c3)
                x = c3;
            c3 = *prev + 1;
            if(x > c3)
                x = c3;
            *(P++) = x;
        }
    }

    /* find the way back */
    size_t* p;
    size_t j, k, dist;
    int dir = 0;

    n = matrix[len1 * len2 - 1];
    if(!n) {
        delete[] matrix;
        return NULL;
    }

    j = len1 - 1;
    k = len2 - 1;
    p = matrix + len1 * len2 - 1;

    while(j || k) {
        /* prefer contiuning in the same direction */
        if(dir < 0 && k && *p == *(p - 1) + 1) {
            makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
            p--;
            continue;
        }
        if(dir > 0 && j && *p == *(p - len2) + 1) {
            makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            p -= len2;
            continue;
        }
        if(j && k && *p == *(p - len2 - 1) && seq1[j - 1] == seq2[k - 1]) {
            /* don't be stupid like difflib, don't store ED_KEEP */
            makelist(ops, ED_EQUAL, --j + o1, hash1, --k + o2, hash2, swapflag);
            p -= len2 + 1;
            dir = 0;
            continue;
        }
        if(j && k && *p == *(p - len2 - 1) + 1) {
            if(rep_rate == 0) {
                makelist(ops, ED_REPLACE, --j + o1, hash1, --k + o2, hash2,
                         swapflag);
            } else {
                PyObject* pya = PySequence_GetItem(hash1.py, --j + o1);
                PyObject* pyb = PySequence_GetItem(hash2.py, --k + o2);
                ssize_t clen1 = PyObject_Length(pya);
                ssize_t clen2 = PyObject_Length(pyb);
                PyErr_Clear();
                if((clen1 <= 0 || clen2 <= 0) || (clen1 == 1 && clen2 == 1)) {
                    makelist(ops, ED_INSERT, -1, hash1, k + o2, hash2,
                             swapflag);
                    n++;
                    makelist(ops, ED_DELETE, j + o1, hash1, -1, hash2,
                             swapflag);
                } else {
                    size_t lensum;
                    dist = dist_op(pya, pyb, &lensum);
                    if(lensum > 0 &&
                       (100 * (lensum - dist) / lensum) >= rep_rate) {
                        makelist(ops, ED_REPLACE, j + o1, hash1, k + o2, hash2,
                                 swapflag);
                    } else {
                        makelist(ops, ED_INSERT, -1, hash1, k + o2, hash2,
                                 swapflag);
                        n++;
                        makelist(ops, ED_DELETE, j + o1, hash1, -1, hash2,
                                 swapflag);
                    }
                }
            }
            p -= len2 + 1;
            dir = 0;
            continue;
        }
        /* we cant't turn directly from -1 to 1, in this case it would be better
         * to go diagonally, but check it (dir == 0) */
        if(dir == 0 && k && *p == *(p - 1) + 1) {
            makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
            p--;
            dir = -1;
            continue;
        }
        if(dir == 0 && j && *p == *(p - len2) + 1) {
            makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            p -= len2;
            dir = 1;
            continue;
        }
        /* coredump right now, later might be too late ;-) */
        assert("lost in the cost matrix" == NULL);
    }
    delete[] matrix;

    for(ssize_t i = o1 - 1; i >= 0; i--) {
        makelist(ops, ED_EQUAL, i, hash1, i, hash2, swapflag);
    }

    PyList_Reverse(ops);
    return ops;
}

/* diffonly true version for bytes, unicode string parsed */
template <typename T>
static PyObject* difference_parsed(const hasher<T>& hash1,
                                   const hasher<T>& hash2,
                                   size_t rep_rate = REPLACEMENT_RATE,
                                   bool swapflag = false) {
    size_t len1 = hash1.len;
    size_t len2 = hash2.len;
    const T* seq1 = hash1.hash;
    const T* seq2 = hash2.hash;
    size_t o1, o2;
    size_t i;
    size_t* matrix; /* cost matrix */
    size_t n;

    if(len1 == 0 && len2 == 0) {
        PyErr_Format(PyExc_ValueError, "Cannot diff Empty data.");
        return NULL;
    }

    /* strip common prefix */
    o1 = 0;
    while(len1 > 0 && len2 > 0 && *seq1 == *seq2) {
        len1--;
        len2--;
        seq1++;
        seq2++;
        o1++;
    }
    o2 = o1;

    if(len1 == 0 && len2 == 0) {
        return PyList_New(0);
    }

    /* strip common suffix */
    while(len1 > 0 && len2 > 0 && seq1[len1 - 1] == seq2[len2 - 1]) {
        len1--;
        len2--;
    }

    len1++;
    len2++;

    /* initalize first row and column */
    matrix = new size_t[len1 * len2];
    if(!matrix) {
        n = (size_t)(-1);
        PyErr_Format(PyExc_MemoryError, "Cannot keep memory.");
        return NULL;
    }
    for(i = 0; i < len2; i++)
        matrix[i] = i;
    for(i = 1; i < len1; i++)
        matrix[len2 * i] = i;

    /* find the costs and fill the matrix */
    for(i = 1; i < len1; i++) {
        size_t* prev = matrix + (i - 1) * len2;
        size_t* P = matrix + i * len2;
        size_t* end = P + len2 - 1;
        const T child1 = seq1[i - 1];
        const T* child2 = seq2;
        size_t x = i;
        P++;
        while(P <= end) {
            size_t c3 = *(prev++) + (child1 != *(child2++));
            x++;
            if(x > c3)
                x = c3;
            c3 = *prev + 1;
            if(x > c3)
                x = c3;
            *(P++) = x;
        }
    }

    /* find the way back */
    size_t* p;
    size_t j, k;
    int dir = 0;

    PyObject* ops = PyList_New(0);
    if(!ops) {
        n = (size_t)(-1);
        return NULL;
    }

    n = matrix[len1 * len2 - 1];
    if(!n) {
        delete[] matrix;
        return NULL;
    }

    j = len1 - 1;
    k = len2 - 1;
    p = matrix + len1 * len2 - 1;

    while(j || k) {
        /* prefer contiuning in the same direction */
        if(dir < 0 && k && *p == *(p - 1) + 1) {
            makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
            p--;
            continue;
        }
        if(dir > 0 && j && *p == *(p - len2) + 1) {
            makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            p -= len2;
            continue;
        }
        if(j && k && *p == *(p - len2 - 1) && seq1[j - 1] == seq2[k - 1]) {
            /* don't be stupid like difflib, don't store ED_KEEP */
            j--;
            k--;
            p -= len2 + 1;
            dir = 0;
            continue;
        }
        if(j && k && *p == *(p - len2 - 1) + 1) {
            if(rep_rate == 0) {
                makelist(ops, ED_REPLACE, --j + o1, hash1, --k + o2, hash2,
                         swapflag);
            } else {
                makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
                n++;
                makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            }
            p -= len2 + 1;
            dir = 0;
            continue;
        }
        /* we cant't turn directly from -1 to 1, in this case it would be better
         * to go diagonally, but check it (dir == 0) */
        if(dir == 0 && k && *p == *(p - 1) + 1) {
            makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
            p--;
            dir = -1;
            continue;
        }
        if(dir == 0 && j && *p == *(p - len2) + 1) {
            makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            p -= len2;
            dir = 1;
            continue;
        }
        /* coredump right now, later might be too late ;-) */
        assert("lost in the cost matrix" == NULL);
    }
    delete[] matrix;
    PyList_Reverse(ops);
    return ops;
}

/* diffonly false version for bytes, unicode string parsed */
template <typename T>
static PyObject* difference_with_equal_parsed(
    const hasher<T>& hash1,
    const hasher<T>& hash2,
    size_t rep_rate = REPLACEMENT_RATE,
    bool swapflag = false) {
    size_t len1 = hash1.len;
    size_t len2 = hash2.len;
    const T* seq1 = hash1.hash;
    const T* seq2 = hash2.hash;
    size_t o1, o2;
    size_t i;
    size_t* matrix; /* cost matrix */
    size_t n;

    if(len1 == 0 && len2 == 0) {
        PyErr_Format(PyExc_ValueError, "Cannot diff Empty data.");
        return NULL;
    }

    /* strip common prefix */
    o1 = 0;
    while(len1 > 0 && len2 > 0 && *seq1 == *seq2) {
        len1--;
        len2--;
        seq1++;
        seq2++;
        o1++;
    }
    o2 = o1;

    PyObject* ops = PyList_New(0);
    if(!ops) {
        n = (size_t)(-1);
        return NULL;
    }

    if(len1 == 0 && len2 == 0) {
        for(size_t i = 0; i < o1; i++) {
            makelist(ops, ED_EQUAL, i, hash1, i, hash2, swapflag);
        }
        return ops;
    }

    /* strip common suffix */
    while(len1 > 0 && len2 > 0 && seq1[len1 - 1] == seq2[len2 - 1]) {
        makelist(ops, ED_EQUAL, --len1 + o1, hash1, --len2 + o2, hash2, swapflag);
    }

    len1++;
    len2++;

    /* initalize first row and column */
    matrix = new size_t[len1 * len2];
    if(!matrix) {
        n = (size_t)(-1);
        return NULL;
    }
    for(i = 0; i < len2; i++)
        matrix[i] = i;
    for(i = 1; i < len1; i++)
        matrix[len2 * i] = i;

    /* find the costs and fill the matrix */
    for(i = 1; i < len1; i++) {
        size_t* prev = matrix + (i - 1) * len2;
        size_t* P = matrix + i * len2;
        size_t* end = P + len2 - 1;
        const T child1 = seq1[i - 1];
        const T* child2 = seq2;
        size_t x = i;
        P++;
        while(P <= end) {
            size_t c3 = *(prev++) + (child1 != *(child2++));
            x++;
            if(x > c3)
                x = c3;
            c3 = *prev + 1;
            if(x > c3)
                x = c3;
            *(P++) = x;
        }
    }

    /* find the way back */
    size_t* p;
    size_t j, k;
    int dir = 0;

    n = matrix[len1 * len2 - 1];
    if(!n) {
        delete[] matrix;
        PyErr_Format(PyExc_MemoryError, "Cannot keep memory.");
        return NULL;
    }

    j = len1 - 1;
    k = len2 - 1;
    p = matrix + len1 * len2 - 1;

    while(j || k) {
        /* prefer contiuning in the same direction */
        if(dir < 0 && k && *p == *(p - 1) + 1) {
            makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
            p--;
            continue;
        }
        if(dir > 0 && j && *p == *(p - len2) + 1) {
            makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            p -= len2;
            continue;
        }
        if(j && k && *p == *(p - len2 - 1) && seq1[j - 1] == seq2[k - 1]) {
            /* don't be stupid like difflib, don't store ED_KEEP */
            makelist(ops, ED_EQUAL, --j + o1, hash1, --k + o2, hash2, swapflag);
            p -= len2 + 1;
            dir = 0;
            continue;
        }
        if(j && k && *p == *(p - len2 - 1) + 1) {
            if(rep_rate == 0) {
                makelist(ops, ED_REPLACE, --j + o1, hash1, --k + o2, hash2,
                         swapflag);
            } else {
                makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
                n++;
                makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            }
            p -= len2 + 1;
            dir = 0;
            continue;
        }
        /* we cant't turn directly from -1 to 1, in this case it would be better
         * to go diagonally, but check it (dir == 0) */
        if(dir == 0 && k && *p == *(p - 1) + 1) {
            makelist(ops, ED_INSERT, -1, hash1, --k + o2, hash2, swapflag);
            p--;
            dir = -1;
            continue;
        }
        if(dir == 0 && j && *p == *(p - len2) + 1) {
            makelist(ops, ED_DELETE, --j + o1, hash1, -1, hash2, swapflag);
            p -= len2;
            dir = 1;
            continue;
        }
        /* coredump right now, later might be too late ;-) */
        assert("lost in the cost matrix" == NULL);
    }
    delete[] matrix;

    for(ssize_t i = o1 - 1; i >= 0; i--) {
        makelist(ops, ED_EQUAL, i, hash1, i, hash2, swapflag);
    }

    PyList_Reverse(ops);
    return ops;
}

template <typename T>
inline PyObject* differ_op(hasher<T>& seq1,
                           hasher<T>& seq2,
                           bool diffonly,
                           int rep_rate,
                           bool parsed = true) {
    PyObject* ops;
    if(seq1.len < seq2.len) {
        if(diffonly) {
            if(parsed)
                ops = difference_parsed(seq1, seq2, rep_rate, false);
            else
                ops = difference(seq1, seq2, rep_rate, false);
        } else {
            if(parsed)
                ops = difference_with_equal_parsed(seq1, seq2, rep_rate, false);
            else
                ops = difference_with_equal(seq1, seq2, rep_rate, false);
        }
    } else {
        if(diffonly) {
            if(parsed)
                ops = difference_parsed(seq2, seq1, rep_rate, true);
            else
                ops = difference(seq2, seq1, rep_rate, true);

        } else {
            if(parsed)
                ops = difference_with_equal_parsed(seq2, seq1, rep_rate, true);
            else
                ops = difference_with_equal(seq2, seq1, rep_rate, true);
        }
    }
    return ops;
}
