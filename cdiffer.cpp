#include "cdiffer.h"

using namespace std;

PyObject* PYDIFF_t[ED_LAST] = {
    PyUnicode_FromString("equal"),    // 0: EQUAL
    PyUnicode_FromString("replace"),  // 1: REPLACE
    PyUnicode_FromString("insert"),   // 2: INSERT
    PyUnicode_FromString("delete")    // 3: DELETE
};

inline static Py_hash_t pyhash(PyObject* py) {
    Py_hash_t hash = PyObject_Hash(py);
    if(hash != -1)
        return hash;
    PyErr_Clear();
    return PyTuple_Type.tp_hash(PySequence_Tuple(py));
}

/*
 * bridge function for `dist_py and similar_py.`.
 */
static size_t dist_op(PyObject* arg1, PyObject* arg2, size_t* lensum) {
    size_t len1, len2, ldist;

#if PY_MAJOR_VERSION >= 3
    if(PyUnicode_Check(arg1) && PyUnicode_Check(arg2) &&
       PyUnicode_KIND(arg1) == PyUnicode_KIND(arg2)) {
#else
    if(PyUnicode_Check(arg1) && PyUnicode_Check(arg2)) {
#endif
        len1 = PyUnicode_GET_LENGTH(arg1);
        len2 = PyUnicode_GET_LENGTH(arg2);

#if PY_MAJOR_VERSION >= 3
        switch(PyUnicode_KIND(arg1)) {
            case 1:
                ldist = distance(len1, PyUnicode_1BYTE_DATA(arg1), len2,
                                 PyUnicode_1BYTE_DATA(arg2));
                break;
            case 2:
                ldist = distance(len1, PyUnicode_2BYTE_DATA(arg1), len2,
                                 PyUnicode_2BYTE_DATA(arg2));
                break;
            case 4:
                ldist = distance(len1, PyUnicode_4BYTE_DATA(arg1), len2,
                                 PyUnicode_4BYTE_DATA(arg2));
                break;
            default:
                ldist = distance(len1, PyUnicode_AsUnicode(arg1), len2,
                                 PyUnicode_AsUnicode(arg2));
                break;
        }
#else
        ldist = distance(len1, PyUnicode_AsUnicode(arg1), len2,
                         PyUnicode_AsUnicode(arg2));
#endif
    } else if(PyBytes_Check(arg1) && PyBytes_Check(arg2)) {
        len1 = PyBytes_GET_SIZE(arg1);
        len2 = PyBytes_GET_SIZE(arg2);
        ldist = distance(len1, PyBytes_AsString(arg1), len2,
                         PyBytes_AsString(arg2));
    } else if(PyByteArray_Check(arg1) && PyByteArray_Check(arg2)) {
        len1 = PyByteArray_GET_SIZE(arg1);
        len2 = PyByteArray_GET_SIZE(arg2);
        ldist = distance(len1, PyByteArray_AsString(arg1), len2,
                         PyByteArray_AsString(arg2));

    } else {
        hasher<uint64_t> seq1(arg1);
        hasher<uint64_t> seq2(arg2);
        len1 = seq1.len;
        len2 = seq2.len;
        if(len1 == 0 && len2 == 0) {
            *lensum = 0;
            return seq1.hash == seq2.hash ? 0 : 1;
        }
        ldist = distance(len1, seq1.hash, len2, seq2.hash);
    }

    *lensum = len1 + len2;

    /* make the inner cycle (i.e. seq2) the longer one */
    if(ldist == error_n) {
        *lensum = -1;
        return error_n;
    }

    return ldist;
}


PyObject* makelist(int tp,
                   size_t aidx,
                   size_t bidx,
                   PyObject* pya,
                   PyObject* pyb) {
    PyObject* list = PyList_New(5);
    Py_INCREF(PYDIFF_t[tp]);
    PyList_SET_ITEM(list, 0, PYDIFF_t[tp]);
    PyList_SET_ITEM(list, 1, PyLong_FromSize_t(aidx));
    PyList_SET_ITEM(list, 2, PyLong_FromSize_t(bidx));
    Py_INCREF(pya);
    PyList_SET_ITEM(list, 3, pya);
    Py_INCREF(pyb);
    PyList_SET_ITEM(list, 4, pyb);
    return list;
}

void makelist(PyObject*& ops,
              int tp,
              size_t aidx,
              size_t bidx,
              PyObject* pya,
              PyObject* pyb) {
    PyObject* list = makelist(tp, aidx, bidx, pya, pyb);
    PyList_Append(ops, list);
    Py_DECREF(list);
}

extern "C" PyObject* dist_py(PyObject* self, PyObject* args) {
    PyObject *arg1, *arg2;
    size_t ldist, lensum;

    if(!PyArg_UnpackTuple(args, (char*)("dist"), 2, 2, &arg1, &arg2))
        return NULL;

    if(PyObject_RichCompareBool(arg1, arg2, Py_EQ))
        return PyLong_FromUnsignedLong(0);

    if((ldist = dist_op(arg1, arg2, &lensum)) == error_n)
        return NULL;

    return PyLong_FromSize_t(ldist);
}

extern "C" PyObject* similar_py(PyObject* self, PyObject* args) {
    PyObject *arg1, *arg2;
    size_t ldist, lensum;

    if(!PyArg_UnpackTuple(args, (char*)("similar"), 2, 2, &arg1, &arg2))
        return NULL;

    if(PyObject_RichCompareBool(arg1, arg2, Py_EQ))
        return PyFloat_FromDouble(1.0);

    if((ldist = dist_op(arg1, arg2, &lensum)) == error_n)
        return NULL;

    if(lensum <= 0)
        return PyFloat_FromDouble(0.0);
    else if(lensum == 2 && ldist == 1)
        return PyFloat_FromDouble((double)0);
    else
        return PyFloat_FromDouble((double)(lensum - ldist) / lensum);
}

/*
 * python Interface differ function
 */
extern "C" PyObject* differ_py(PyObject* self, PyObject* args, PyObject *kwargs) {
    PyObject *arg1, *arg2, *arg3 = NULL, *arg4 = NULL;

    int diffonly = false;
    int rep_rate = REPLACEMENT_RATE;

    const char *kwlist[5] = {"a", "b", "diffonly", "rep_rate", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|ii", (char**)kwlist, 
                                     &arg1, &arg2, &diffonly, &rep_rate))
        return NULL;

    if(PyObject_RichCompareBool(arg1, arg2, Py_EQ)) {
        if(diffonly)
            return PyList_New(0);

        ssize_t len = PyObject_Length(arg1);
        PyErr_Clear();
        PyObject *ret, *list;
        if(len <= 1) {
            ret = PyList_New(0);
            makelist(ret, ED_EQUAL, 0, 0, arg1, arg2);
        } else {
            ret = PyList_New(len);
            for(ssize_t i = 0; i < len; i++) {
                list = makelist(ED_EQUAL, i, i, PySequence_GetItem(arg1, i),
                                PySequence_GetItem(arg2, i));
                PyList_SetItem(ret, i, list);
                Py_DECREF(list);
            }
        }
        return ret;
    }

    if(PyBytes_Check(arg1) && PyBytes_Check(arg2)) {
        hasher<char> seq1(PyBytes_AsString(arg1), arg1);
        hasher<char> seq2(PyBytes_AsString(arg2), arg2);
        return differ_op(seq1, seq2, diffonly, rep_rate);
    } else if(PyByteArray_Check(arg1) && PyByteArray_Check(arg2)) {
        hasher<char> seq1(PyByteArray_AsString(arg1), arg1);
        hasher<char> seq2(PyByteArray_AsString(arg2), arg2);
        return differ_op(seq1, seq2, diffonly, rep_rate);

#if PY_MAJOR_VERSION >= 3
    } else if(PyUnicode_Check(arg1) && PyUnicode_Check(arg2) &&
              PyUnicode_KIND(arg1) == PyUnicode_KIND(arg2)) {
        int kind = PyUnicode_KIND(arg1);
        if(kind == 1) {
            hasher<Py_UCS1> seq1(PyUnicode_1BYTE_DATA(arg1), arg1);
            hasher<Py_UCS1> seq2(PyUnicode_1BYTE_DATA(arg2), arg2);
            return differ_op(seq1, seq2, diffonly, rep_rate);
        } else if(kind == 2) {
            hasher<Py_UCS2> seq1(PyUnicode_2BYTE_DATA(arg1), arg1);
            hasher<Py_UCS2> seq2(PyUnicode_2BYTE_DATA(arg2), arg2);
            return differ_op(seq1, seq2, diffonly, rep_rate);
        } else if(kind == 4) {
            hasher<Py_UCS4> seq1(PyUnicode_4BYTE_DATA(arg1), arg1);
            hasher<Py_UCS4> seq2(PyUnicode_4BYTE_DATA(arg2), arg2);
            return differ_op(seq1, seq2, diffonly, rep_rate);
        } else {
            hasher<Py_UNICODE> seq1(PyUnicode_AsUnicode(arg1), arg1);
            hasher<Py_UNICODE> seq2(PyUnicode_AsUnicode(arg2), arg2);
            return differ_op(seq1, seq2, diffonly, rep_rate);
        }
#else
    } else if(PyUnicode_Check(arg1) && PyUnicode_Check(arg2)) {
        hasher<Py_UNICODE> seq1(PyUnicode_AsUnicode(arg1), arg1);
        hasher<Py_UNICODE> seq2(PyUnicode_AsUnicode(arg2), arg2);
        return differ_op(seq1, seq2, diffonly, rep_rate);
#endif
    } else {
        hasher<uint64_t> seq1(arg1);
        hasher<uint64_t> seq2(arg2);
        return differ_op(seq1, seq2, diffonly, rep_rate, false);
    }
    return NULL;
}

#define MODULE_NAME cdiffer
#define MODULE_NAME_S "cdiffer"

/* {{{ */
// this module description
#define MODULE_DOCS                                                          \
    "A C extension module for fast computation of:\n"                        \
    "- Levenshtein (edit) distance and edit sequence manipulation\n"         \
    "- string similarity\n"                                                  \
    "- approximate median strings, and generally string averaging\n"         \
    "- string sequence and set similarity\n"                                 \
    "\n"                                                                     \
    "Levenshtein has a some overlap with difflib (SequenceMatcher).  It\n"   \
    "supports only strings, not arbitrary sequence types, but on the\n"      \
    "other hand it's much faster.\n"                                         \
    "\n"                                                                     \
    "It supports both normal and Unicode strings, but can't mix them, all\n" \
    "arguments to a function (method) have to be of the same type (or its\n" \
    "subclasses).\n"

#define dist_DESC                                             \
    "Compute absolute Levenshtein distance of two strings.\n" \
    "\n"                                                      \
    "dist(sequence, sequence)\n"                              \
    "\n"                                                      \
    "Examples (it's hard to spell Levenshtein correctly):\n"  \
    "\n"                                                      \
    ">>> dist('coffee', 'cafe')\n"                            \
    "3\n"                                                     \
    ">>> dist(list('coffee'), list('cafe'))\n"                \
    "3\n"                                                     \
    ">>> dist(tuple('coffee'), tuple('cafe'))\n"              \
    "3\n"                                                     \
    ">>> dist(iter('coffee'), iter('cafe'))\n"                \
    "3\n"                                                     \
    ">>> dist(range(4), range(5))\n"                          \
    "1\n"                                                     \
    ">>> dist('coffee', 'xxxxxx')\n"                          \
    "6\n"                                                     \
    ">>> dist('coffee', 'coffee')\n"                          \
    "0\n"                                                     \
    "\n"

#define similar_DESC                                                      \
    "Compute similarity of two strings.\n"                                \
    "\n"                                                                  \
    "similar(sequence, sequence)\n"                                       \
    "\n"                                                                  \
    "The similarity is a number between 0 and 1, it's usually equal or\n" \
    "based on real minimal edit distance.\n"                              \
    "\n"                                                                  \
    "Examples:\n"                                                         \
    "\n"                                                                  \
    ">>> similar('coffee', 'cafe')\n"                                     \
    "0.7\n"                                                               \
    ">>> similar('hoge', 'bar')\n"                                        \
    "0.0\n"                                                               \
    "\n"

#define differ_DESC                                                           \
    "Find sequence of edit operations transforming one string to another.\n"  \
    "\n"                                                                      \
    "differ(source_sequence, destination_sequence, diffonly=False, "          \
    "rep_rate=60)\n"                                                          \
    "\n"                                                                      \
    "The diffonly option refers to whether or not to limit the output "       \
    "results to only those with differences.\n"                               \
    "False by default.\n\n"                                                   \
    "The rep_rate option is an integer between 0 and 100 that specifies the " \
    "percentage of similarity to be replaced.\n"                              \
    "rep_rate = 60 by default.\n\n"                                           \
    "Examples:\n"                                                             \
    "\n"                                                                      \
    ">>> for x in differ('coffee', 'cafe'):\n"                                \
    "...     print(x)\n"                                                      \
    "...\n"                                                                   \
    "['equal',   0, 0,   'c', 'c']\n"                                         \
    "['replace', 1, 1,   'o', 'a']\n"                                         \
    "['equal',   2, 2,   'f', 'f']\n"                                         \
    "['delete',  3, None,'f',None]\n"                                         \
    "['delete',  4, None,'e',None]\n"                                         \
    "['equal',   5, 3,   'e', 'e']\n"                                         \
    ">>> for x in differ('coffee', 'cafe', diffonly=True):\n"                 \
    "...     print(x)\n"                                                      \
    "...\n"                                                                   \
    "['replace', 1, 1,   'o', 'a']\n"                                         \
    "['delete',  3, None,'f',None]\n"                                         \
    "['delete',  4, None,'e',None]\n"                                         \
    "\n"

/* }}} */

#define PY_ADD_METHOD(py_func, c_func, desc) \
    { py_func, (PyCFunction)c_func, METH_VARARGS, desc }
#define PY_ADD_METHOD_KWARGS(py_func, c_func, desc) \
    { py_func, (PyCFunction)c_func, METH_VARARGS | METH_KEYWORDS, desc }

/* Please extern method define for python */
/* PyMethodDef Parameter Help
 * https://docs.python.org/ja/3/c-api/structures.html#c.PyMethodDef
 */
static PyMethodDef py_methods[] = {
    PY_ADD_METHOD("dist", dist_py, dist_DESC),
    PY_ADD_METHOD("similar", similar_py, similar_DESC),
    PY_ADD_METHOD_KWARGS("differ", differ_py, differ_DESC),

    {NULL, NULL, 0, NULL}};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef py_defmod = {PyModuleDef_HEAD_INIT, MODULE_NAME_S,
                                       MODULE_DOCS, 0, py_methods};
#define PARSE_NAME(mn) PyInit_##mn
#define PARSE_FUNC(mn) \
    PyMODINIT_FUNC PARSE_NAME(mn)() { return PyModule_Create(&py_defmod); }


#else
#define PARSE_NAME(mn)                                                \
    init##mn(void) {                                                  \
        (void)Py_InitModule3(MODULE_NAME_S, py_methods, MODULE_DOCS); \
    }
#define PARSE_FUNC(mn) PyMODINIT_FUNC PARSE_NAME(mn)
#endif

PARSE_FUNC(MODULE_NAME)
