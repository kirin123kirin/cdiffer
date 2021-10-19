#pragma once
#ifndef CDIFFER_H
#define CDIFFER_H

#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "pyyou.hpp"

namespace gammy {

#define REPLACEMENT_RATE 60

#define ED_EQUAL 0
#define ED_REPLACE 1
#define ED_INSERT 2
#define ED_DELETE 3
#define ED_LAST 4

#define ARRAY_SIZE 1536

extern PyObject* DIFFTP[2][ED_LAST];

#define ZERO_1 0U
#define ZERO_2 ZERO_1, ZERO_1
#define ZERO_4 ZERO_2, ZERO_2
#define ZERO_8 ZERO_4, ZERO_4
#define ZERO_16 ZERO_8, ZERO_8
#define ZERO_32 ZERO_16, ZERO_16
#define ZERO_64 ZERO_32, ZERO_32
#define ZERO_128 ZERO_64, ZERO_64
#define ZERO_256 ZERO_128, ZERO_128

template <typename T>
struct through_pass_hash {
    T operator()(const int8_t& s) const { return s; }
    T operator()(const int16_t& s) const { return s; }
    T operator()(const int32_t& s) const { return s; }
    T operator()(const int64_t& s) const { return s; }
    T operator()(const uint8_t& s) const { return s; }
    T operator()(const uint16_t& s) const { return s; }
    T operator()(const uint32_t& s) const { return s; }
    T operator()(const uint64_t& s) const { return s; }
    T operator()(const std::string& s) const { return (T)s.data(); }
    T operator()(const pyview& s) const { return (T)s.data_64; }
    T operator()(PyObject*& s) const {
        T hash = T();
        if((hash = (T)PyObject_Hash(s)) == -1) {
            PyObject* item = PySequence_Tuple(s);
            hash = (T)PyObject_Hash(s);
            Py_DECREF(item);
        }
        return hash;
    }
};

template <typename BitTy = uint64_t, std::size_t fraction_size = 131>
struct MappingBlock {
    using value_type = BitTy;
    using size_type = typename std::make_unsigned<BitTy>::type;

    std::array<std::array<value_type, fraction_size>, 2> pair;

    MappingBlock() : pair() {}

    template <typename Tval>
    constexpr value_type const& operator[](const Tval x) const noexcept {
        size_type hash = x % fraction_size;
        value_type vx = (value_type)x;
        while(pair[0][hash] && pair[1][hash] != vx)
            hash = (hash + size_type(1)) % fraction_size;
        // pair[1][hash] = x;
        return pair[0][hash];
    }

    template <typename Tval>
    constexpr value_type& operator[](Tval x) noexcept {
        size_type hash = x % fraction_size;
        value_type vx = (value_type)x;
        while(pair[0][hash] && pair[1][hash] != vx)
            hash = (hash + size_type(1)) % fraction_size;
        pair[1][hash] = vx;
        return pair[0][hash];
    }
};

PyObject* makelist(int dtype, std::size_t x, std::size_t y, PyObject*& a, PyObject*& b, bool swapflag = false) {
    std::size_t len1 = PyAny_Length(a);
    std::size_t len2 = PyAny_Length(b);

    PyObject* list = PyList_New(5);
    Py_INCREF(DIFFTP[swapflag][dtype]);
    PyList_SetItem(list, 0, DIFFTP[swapflag][dtype]);

    if(dtype == ED_INSERT) {
        Py_INCREF(Py_None);
        PyList_SetItem(list, 1L + swapflag, Py_None);
        Py_INCREF(Py_None);
        PyList_SetItem(list, 3L + swapflag, Py_None);
    } else {
        PyList_SetItem(list, 1L + swapflag, PyLong_FromSize_t(x));
        if(len1 == 0 || len1 == error_n || (len1 == 1 && !PySequence_Check(a))) {
            Py_INCREF(a);
            PyList_SetItem(list, 3L + swapflag, a);
        } else {
            PyList_SetItem(list, 3L + swapflag, PySequence_GetItem(a, (Py_ssize_t)x));
        }
    }
    if(dtype == ED_DELETE) {
        Py_INCREF(Py_None);
        PyList_SetItem(list, 2L - swapflag, Py_None);
        Py_INCREF(Py_None);
        PyList_SetItem(list, 4L - swapflag, Py_None);
    } else {
        PyList_SetItem(list, 2L - swapflag, PyLong_FromSize_t(y));
        if(len2 == 0 || len2 == error_n || (len2 == 1 && !PySequence_Check(b))) {
            Py_INCREF(b);
            PyList_SetItem(list, 4L - swapflag, b);
        } else {
            PyList_SetItem(list, 4L - swapflag, PySequence_GetItem(b, (Py_ssize_t)y));
        }
    }
    return list;
}
void makelist(PyObject*& ops,
              int dtype,
              std::size_t x,
              std::size_t y,
              PyObject*& a,
              PyObject*& b,
              bool swapflag = false) {
    PyObject* list = makelist(dtype, x, y, a, b, swapflag);
    if((PyList_Append(ops, list)) == -1) {
        Py_CLEAR(ops);
        Py_CLEAR(list);
        PyErr_Format(PyExc_MemoryError, "Failed while creating result list.");
        return;
    }
    Py_DECREF(list);
}

void complist(PyObject*& ops,
              int dtype,
              std::size_t x,
              std::size_t y,
              PyObject*& a,
              PyObject*& b,
              bool swapflag,
              int startidx,
              PyObject*& condition_value,
              PyObject*& na_value,
              PyObject*& DEL_Flag,
              PyObject*& ADD_Flag) {
    if(swapflag) {
        if(dtype == ED_INSERT)
            dtype = ED_DELETE;
        else if(dtype == ED_DELETE)
            dtype = ED_INSERT;
        return complist(ops, dtype, y, x, b, a, false, startidx, condition_value, na_value, DEL_Flag, ADD_Flag);
    }

    PyObject* ret = NULL;
    PyObject* concat = NULL;
    PyObject* item = NULL;
    PyObject* forcestr = NULL;
    int result = -1;

    PyObject* list = PyList_New(4);
    Py_INCREF(DIFFTP[0][dtype]);
    PyList_SetItem(list, 0, DIFFTP[0][dtype]);

    if(dtype == ED_DELETE) {
        PyList_SetItem(list, 1, PyLong_FromSize_t(x + startidx));
        Py_INCREF(na_value);
        PyList_SetItem(list, 2, na_value);

        item = PySequence_GetItem(a, (Py_ssize_t)x);
        if(DEL_Flag && condition_value) {
            if(item && PyUnicode_Check(item)) {
                concat = PyUnicode_Concat(item, condition_value);
                ret = PyUnicode_Concat(concat, DEL_Flag);
            } else {
                forcestr = PyObject_Str(item ? item : a);
                concat = PyUnicode_Concat(forcestr, condition_value);
                ret = PyUnicode_Concat(concat, DEL_Flag);
            }
        } else {
            ret = item ? item : a;
        }
    } else if(dtype == ED_INSERT) {
        Py_INCREF(na_value);
        PyList_SetItem(list, 1, na_value);
        PyList_SetItem(list, 2, PyLong_FromSize_t(y + startidx));

        item = PySequence_GetItem(b, (Py_ssize_t)y);
        if(ADD_Flag && condition_value) {
            concat = PyUnicode_Concat(ADD_Flag, condition_value);
            if(item && PyUnicode_Check(item)) {
                ret = PyUnicode_Concat(concat, item);
            } else {
                forcestr = PyObject_Str(item ? item : b);
                ret = PyUnicode_Concat(concat, forcestr);
            }
        } else {
            ret = item ? item : b;
        }
    } else if(dtype == ED_REPLACE) {
        PyList_SetItem(list, 1, PyLong_FromSize_t(x + startidx));
        PyList_SetItem(list, 2, PyLong_FromSize_t(y + startidx));

        item = PySequence_GetItem(a, (Py_ssize_t)x);
        if(item && PyUnicode_Check(item)) {
            concat = PyUnicode_Concat(item, condition_value);
        } else {
            forcestr = PyObject_Str(item ? item : a);
            concat = PyUnicode_Concat(forcestr, condition_value);
        }
        Py_XDECREF(item);
        Py_XDECREF(forcestr);
        item = PySequence_GetItem(b, (Py_ssize_t)y);
        if(item && PyUnicode_Check(item)) {
            ret = PyUnicode_Concat(concat, item);
        } else {
            forcestr = PyObject_Str(item ? item : b);
            ret = PyUnicode_Concat(concat, forcestr);
        }
    } else {
        PyList_SetItem(list, 1, PyLong_FromSize_t(x + startidx));
        PyList_SetItem(list, 2, PyLong_FromSize_t(y + startidx));

        ret = PySequence_GetItem(a, (Py_ssize_t)x);
        if(ret == NULL) {
            ret = a;
        }
    }
    PyErr_Clear();

    result = PyList_SetItem(list, 3, ret);

    Py_XDECREF(item);
    Py_XDECREF(forcestr);
    Py_XDECREF(concat);

    if(result == -1 || (PyList_Append(ops, list)) == -1) {
        Py_CLEAR(ops);
        Py_CLEAR(list);
        Py_XDECREF(ret);
        PyErr_Format(PyExc_MemoryError, "Failed while creating result list.");
        return;
    }
    Py_DECREF(list);
}

template <typename CharT>
class Diff_t {
   public:
    CharT a = nullptr;
    CharT b = nullptr;
    std::size_t A = error_n;
    std::size_t B = error_n;
    std::size_t D = error_n;
    std::size_t SIZE = error_n;

    bool swapflag = false;
    bool diffonly = false;
    int rep_rate = REPLACEMENT_RATE;
    bool need_clear_py = false;

    Diff_t()
        : a(nullptr),
          b(nullptr),
          A(error_n),
          B(error_n),
          D(error_n),
          SIZE(SIZE_MAX),
          swapflag(false),
          diffonly(false),
          rep_rate(REPLACEMENT_RATE),
          need_clear_py(false) {}
    Diff_t(std::nullptr_t)
        : a(nullptr),
          b(nullptr),
          A(error_n),
          B(error_n),
          D(error_n),
          SIZE(SIZE_MAX),
          swapflag(false),
          diffonly(false),
          rep_rate(REPLACEMENT_RATE),
          need_clear_py(false) {}

    Diff_t(PyObject* _a, PyObject* _b, bool _need_clear_py = false)
        : a(CharT(_a)), b(CharT(_b)), need_clear_py(_need_clear_py) {
        A = a.size();
        B = b.size();
        swapflag = A > B && _a != Py_None && _b != Py_None;
        if(swapflag) {
            std::swap(A, B);
            std::swap(a, b);
        }
        D = B - A;
        SIZE = (std::size_t)A + B + 1;
    }

    ~Diff_t() {
        if(need_clear_py) {
            Py_XDECREF(a.py);
            Py_XDECREF(b.py);
        }
    }

   public:
    PyObject* difference(bool _diffonly = false, int _rep_rate = REPLACEMENT_RATE) {
        this->diffonly = _diffonly;
        this->rep_rate = _rep_rate;

        if(b.kind == 1) {
            /* for ASCII */
            if(B < 8) {
                std::array<uint8_t, 128> fp = {ZERO_128};
                return core_difference(fp);
            } else if(B < 16) {
                std::array<uint16_t, 128> fp = {ZERO_128};
                return core_difference(fp);
            } else if(B < 32) {
                std::array<uint32_t, 128> fp = {ZERO_128};
                return core_difference(fp);
            } else {
                std::array<uint64_t, 128> fp = {ZERO_128};
                return core_difference(fp);
            }
        }

        if((a.canonical || b.canonical) && (A + B <= 1 || (A == 1 && B == 1))) {
            PyObject* ops = PyList_New(0);
            if(rep_rate < 1) {
                makelist(ops, ED_REPLACE, 0, 0, a.py, b.py, swapflag);
            } else {
                makelist(ops, ED_DELETE, 0, 0, a.py, b.py, swapflag);
                makelist(ops, ED_INSERT, 0, 0, a.py, b.py, swapflag);
            }
            return ops;
        }

        else if(B < 64) {
            if(B < 8) {
                MappingBlock<uint8_t> fp = {};
                fp.pair =
                    std::array<std::array<uint8_t, 131>, 2>{{{ZERO_128, ZERO_2, ZERO_1}, {ZERO_128, ZERO_2, ZERO_1}}};
                return core_difference(fp);
            } else if(B < 16) {
                MappingBlock<uint16_t> fp = {};
                fp.pair =
                    std::array<std::array<uint16_t, 131>, 2>{{{ZERO_128, ZERO_2, ZERO_1}, {ZERO_128, ZERO_2, ZERO_1}}};
                return core_difference(fp);
            } else if(B < 32) {
                MappingBlock<uint32_t, 257> fp = {};
                fp.pair = std::array<std::array<uint32_t, 257>, 2>{{{ZERO_256, ZERO_1}, {ZERO_256, ZERO_1}}};
                return core_difference(fp);
            } else {
                MappingBlock<uint64_t, 521> fp = {};
                fp.pair = std::array<std::array<uint64_t, 521>, 2>{
                    {{ZERO_256, ZERO_256, ZERO_8, ZERO_1}, {ZERO_256, ZERO_256, ZERO_8, ZERO_1}}};
                return core_difference(fp);
            }
        }

        else {
            /* for Big Size ANY data. */
            std::unordered_map<uint64_t, uint64_t, through_pass_hash<uint64_t>> fp = {};
            return core_difference(fp);
        }
    }

    PyObject* compare(bool _diffonly,
                      int _rep_rate,
                      int _startidx,
                      PyObject* _condition_value,
                      PyObject* _na_value,
                      PyObject* _DEL_Flag,
                      PyObject* _ADD_Flag) {
        this->diffonly = _diffonly;
        this->rep_rate = _rep_rate;

        if(b.kind == 1) {
            /* for ASCII */
            if(B < 8) {
                std::array<uint8_t, 128> fp = {ZERO_128};
                return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            } else if(B < 16) {
                std::array<uint16_t, 128> fp = {ZERO_128};
                return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            } else if(B < 32) {
                std::array<uint32_t, 128> fp = {ZERO_128};
                return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            } else {
                std::array<uint64_t, 128> fp = {ZERO_128};
                return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            }
        }

        if((a.canonical || b.canonical) && (A + B <= 1 || (A == 1 && B == 1))) {
            PyObject* ops = PyList_New(0);

            if(rep_rate < 1) {
                complist(ops, ED_REPLACE, 0, 0, a.py, b.py, swapflag, _startidx, _condition_value, _na_value, _DEL_Flag,
                         _ADD_Flag);
            } else {
                complist(ops, ED_DELETE, 0, 0, a.py, b.py, swapflag, _startidx, _condition_value, _na_value, _DEL_Flag,
                         _ADD_Flag);
                complist(ops, ED_INSERT, 0, 0, a.py, b.py, swapflag, _startidx, _condition_value, _na_value, _DEL_Flag,
                         _ADD_Flag);
            }
            return ops;
        }

        else if(B < 64) {
            if(B < 8) {
                MappingBlock<uint8_t> fp = {};
                fp.pair =
                    std::array<std::array<uint8_t, 131>, 2>{{{ZERO_128, ZERO_2, ZERO_1}, {ZERO_128, ZERO_2, ZERO_1}}};
                return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            } else if(B < 16) {
                MappingBlock<uint16_t> fp = {};
                fp.pair =
                    std::array<std::array<uint16_t, 131>, 2>{{{ZERO_128, ZERO_2, ZERO_1}, {ZERO_128, ZERO_2, ZERO_1}}};
                return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            } else if(B < 32) {
                MappingBlock<uint32_t, 257> fp = {};
                fp.pair = std::array<std::array<uint32_t, 257>, 2>{{{ZERO_256, ZERO_1}, {ZERO_256, ZERO_1}}};
                return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            } else {
                MappingBlock<uint64_t, 521> fp = {};
                fp.pair = std::array<std::array<uint64_t, 521>, 2>{
                    {{ZERO_256, ZERO_256, ZERO_8, ZERO_1}, {ZERO_256, ZERO_256, ZERO_8, ZERO_1}}};
                return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            }
        }

        else {
            /* for Big Size ANY data. */
            std::unordered_map<uint64_t, uint64_t, through_pass_hash<uint64_t>> fp = {};
            return core_compare(fp, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
        }
    }

   protected:
    template <typename T>
    void makelist_pyn(PyObject*& ops, T& pyn, int dtype, std::size_t x, std::size_t y) {
        PyObject* list = PyList_New(5);
        Py_INCREF(DIFFTP[swapflag][dtype]);
        PyList_SetItem(list, 0, DIFFTP[swapflag][dtype]);

        if(dtype == ED_INSERT) {
            Py_INCREF(Py_None);
            PyList_SetItem(list, 1L + swapflag, Py_None);
            Py_INCREF(Py_None);
            PyList_SetItem(list, 3L + swapflag, Py_None);
        } else {
            Py_INCREF(pyn[x]);
            PyList_SetItem(list, 1L + swapflag, pyn[x]);
            PyObject* pya = a.getitem(x);
            PyList_SetItem(list, 3L + swapflag, pya);
        }
        if(dtype == ED_DELETE) {
            Py_INCREF(Py_None);
            PyList_SetItem(list, 2L - swapflag, Py_None);
            Py_INCREF(Py_None);
            PyList_SetItem(list, 4L - swapflag, Py_None);
        } else {
            Py_INCREF(pyn[y]);
            PyList_SetItem(list, 2L - swapflag, pyn[y]);
            PyObject* pyb = b.getitem(y);
            PyList_SetItem(list, 4L - swapflag, pyb);
        }

        if((PyList_Append(ops, list)) == -1) {
            Py_CLEAR(ops);
            Py_CLEAR(list);
            PyErr_Format(PyExc_MemoryError, "Failed while creating result list.");
            return;
        }
        Py_DECREF(list);
    }

    template <typename Storage>
    PyObject* core_difference(Storage& fp) {
        std::size_t i = 0, j = 0, x = 0, y = 0, len = 0, sj = 0, mj = 0;
        uint64_t found = 0, adat = 0, trb = 0;
        const std::size_t BITS = std::min(std::size_t(64), (std::size_t)(sizeof(fp[0]) * 8));
        PyObject* ops = PyList_New(0);

        if(a == b) {
            if(!diffonly)
                for(x = 0; x < A; x++)
                    makelist(ops, ED_EQUAL, x, x, a.py, b.py, false);
            return ops;
        }
        if(B == 0) {
            for(x = 0; x < A; x++)
                makelist(ops, ED_DELETE, x, 0, a.py, b.py, swapflag);
            return ops;
        }
        if(A == 0) {
            for(y = 0; y < B; y++)
                makelist(ops, ED_INSERT, 0, y, a.py, b.py, swapflag);
            return ops;
        }
        if(A == 1 && B == 1) {
            if(rep_rate > 0 && ((a.canonical && b.canonical) ||
                                Diff_t<pyview>(a.getitem(0), b.getitem(0), true).similar(rep_rate) * 100 < rep_rate)) {
                makelist(ops, ED_DELETE, x, 0, a.py, b.py, swapflag);
                makelist(ops, ED_INSERT, 0, y, a.py, b.py, swapflag);
            } else {
                makelist(ops, ED_REPLACE, 0, 0, a.py, b.py, swapflag);
            }
            return ops;
        }

        PyObject** pyn = new PyObject*[B];
        if(pyn == NULL) {
            PyErr_NoMemory();
            return NULL;
        }
        for(std::size_t n = 0; n < B; n++) {
            fp[b[n]] |= uint64_t(1) << n % BITS;
            pyn[n] = PyLong_FromSize_t(n);
        }

        for(y = 0, len = BITS < B ? BITS : B; y < len; ++y)
            fp[b[y]] |= 1ULL << (y % BITS);

        x = 0;

        while(i < A && j < B) {
            auto ai = a[i];

            if(ai == b[j]) {
                if(!diffonly)
                    makelist_pyn(ops, pyn, ED_EQUAL, x, j);
            } else {
                adat = fp[ai];
                mj = j % BITS;
                trb = (adat << (BITS - mj + 1)) | (adat >> mj);
                if(x > 0 && (found = trb & (~trb + 1)) != 0) {
                    while(found > 1 && j < B) {
                        found >>= 1;
                        makelist_pyn(ops, pyn, ED_INSERT, x, j);
                        ++j;
                    }
                    if(!diffonly)
                        makelist_pyn(ops, pyn, ED_EQUAL, x, j);
                } else if(i < A) {
                    if(rep_rate > 0 &&
                       ((a.canonical && b.canonical) ||
                        Diff_t<pyview>(a.getitem(x), b.getitem(j), true).similar(rep_rate) * 100 < rep_rate)) {
                        makelist_pyn(ops, pyn, ED_DELETE, x, j);
                        makelist_pyn(ops, pyn, ED_INSERT, x, j);
                    } else {
                        makelist_pyn(ops, pyn, ED_REPLACE, x, j);
                    }

                } else {
                    makelist_pyn(ops, pyn, ED_INSERT, x, j);
                }
            }

            do {
                mj = sj % BITS;
                fp[b[sj]] &= ~(1ULL << mj);
                if(BITS < B) { /* append next sequence data */
                    if(sj + BITS < B - 1)
                        fp[b[sj + BITS]] |= 1ULL << mj;
                    else
                        fp[b[B - 1]] |= 1ULL << mj;
                }
            } while(++sj < j);

            i += 1;
            j += 1;
            x = i < A - 1 ? i : A - 1;
        }

        for(; j < B; ++j)
            makelist_pyn(ops, pyn, ED_INSERT, x, j);
        for(std::size_t n = 0; n < B; n++)
            Py_DECREF(pyn[n]);
        delete[] pyn;
        return ops;
    }
    template <typename Storage>
    PyObject* core_compare(Storage& fp,
                           int startidx,
                           PyObject* condition_value,
                           PyObject* _na_value,
                           PyObject* _DEL_Flag,
                           PyObject* _ADD_Flag) {
        std::size_t i = 0, j = 0, x = 0, y = 0, len = 0, sj = 0, mj = 0;
        uint64_t found = 0, adat = 0, trb = 0;
        const std::size_t BITS = std::min(std::size_t(64), (std::size_t)(sizeof(fp[0]) * 8));
        PyObject* ops = PyList_New(0);

        if(a == b) {
            if(!diffonly) {
                for(x = 0; x < A; x++)
                    complist(ops, ED_EQUAL, x, x, a.py, b.py, false, startidx, condition_value, _na_value, _DEL_Flag,
                             _ADD_Flag);
            }
            return ops;
        }
        if(B == 0) {
            // if(B == 0 || b.py == Py_None) {
            for(x = 0; x < A; x++)
                complist(ops, ED_DELETE, x, 0, a.py, b.py, swapflag, startidx, condition_value, _na_value, _DEL_Flag,
                         _ADD_Flag);
            return ops;
        }
        if(A == 0) {
            // if(A == 0 || a.py == Py_None) {
            for(y = 0; y < B; y++)
                complist(ops, ED_INSERT, 0, y, a.py, b.py, swapflag, startidx, condition_value, _na_value, _DEL_Flag,
                         _ADD_Flag);
            return ops;
        }

        for(std::size_t n = 0; n < B; n++)
            fp[b[n]] |= uint64_t(1) << n % BITS;

        for(y = 0, len = BITS < B ? BITS : B; y < len; ++y)
            fp[b[y]] |= 1ULL << (y % BITS);

        x = 0;

        while(i < A && j < B) {
            auto ai = a[i];

            if(ai == b[j]) {
                if(!diffonly)
                    complist(ops, ED_EQUAL, x, j, a.py, b.py, swapflag, startidx, condition_value, _na_value, _DEL_Flag,
                             _ADD_Flag);
            } else {
                adat = fp[ai];
                mj = j % BITS;
                trb = (adat << (BITS - mj + 1)) | (adat >> mj);
                if(x > 0 && (found = trb & (~trb + 1)) != 0) {
                    while(found > 1 && j < B) {
                        found >>= 1;
                        complist(ops, ED_INSERT, x, j, a.py, b.py, swapflag, startidx, condition_value, _na_value,
                                 _DEL_Flag, _ADD_Flag);
                        ++j;
                    }
                    if(!diffonly && j < B)
                        complist(ops, ED_EQUAL, x, j, a.py, b.py, swapflag, startidx, condition_value, _na_value,
                                 _DEL_Flag, _ADD_Flag);
                } else if(i < A) {
                    if(rep_rate > 0 &&
                       ((a.canonical && b.canonical) ||
                        Diff_t<pyview>(a.getitem(x), b.getitem(j), true).similar(rep_rate) * 100 < rep_rate)) {
                        complist(ops, ED_DELETE, x, j, a.py, b.py, swapflag, startidx, condition_value, _na_value,
                                 _DEL_Flag, _ADD_Flag);
                        complist(ops, ED_INSERT, x, j, a.py, b.py, swapflag, startidx, condition_value, _na_value,
                                 _DEL_Flag, _ADD_Flag);
                    } else {
                        complist(ops, ED_REPLACE, x, j, a.py, b.py, swapflag, startidx, condition_value, _na_value,
                                 _DEL_Flag, _ADD_Flag);
                    }

                } else {
                    complist(ops, ED_INSERT, x, j, a.py, b.py, swapflag, startidx, condition_value, _na_value,
                             _DEL_Flag, _ADD_Flag);
                }
            }

            do {
                mj = sj % BITS;
                fp[b[sj]] &= ~(1ULL << mj);
                if(BITS < B) { /* append next sequence data */
                    if(sj + BITS < B - 1)
                        fp[b[sj + BITS]] |= 1ULL << mj;
                    else
                        fp[b[B - 1]] |= 1ULL << mj;
                }
            } while(++sj < j);

            i += 1;
            j += 1;
            x = i < A - 1 ? i : A - 1;
        }

        for(; j < B; ++j) {
            complist(ops, ED_INSERT, x, j, a.py, b.py, swapflag, startidx, condition_value, _na_value, _DEL_Flag,
                     _ADD_Flag);
        }

        return ops;
    }

   public:
    std::size_t distance(std::size_t max = error_n, bool weight = true) {
        if(a == b)
            return 0;

        if(A == 0)
            return B;

        if(B == 0)
            return A;

        if(A == 1 && B == 1)
            return 1ULL + weight;

        if(b.kind == 1) {
            /* for ASCII */
            if(B < 8) {
                std::array<uint8_t, 128> fp = {ZERO_128};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 16) {
                std::array<uint16_t, 128> fp = {ZERO_128};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 32) {
                std::array<uint32_t, 128> fp = {ZERO_128};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 64) {
                std::array<uint64_t, 128> fp = {ZERO_128};
                return core_distance_bp_simple(fp, max, weight);
            } else {
                std::array<uint64_t, 128> fp = {ZERO_128};
                return core_distance_bp(fp, max, weight);
            }
        }

        else if(B < 64) {
            if(B < 8) {
                MappingBlock<uint8_t> fp = {};
                fp.pair =
                    std::array<std::array<uint8_t, 131>, 2>{{{ZERO_128, ZERO_2, ZERO_1}, {ZERO_128, ZERO_2, ZERO_1}}};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 16) {
                MappingBlock<uint16_t> fp = {};
                fp.pair =
                    std::array<std::array<uint16_t, 131>, 2>{{{ZERO_128, ZERO_2, ZERO_1}, {ZERO_128, ZERO_2, ZERO_1}}};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 32) {
                MappingBlock<uint32_t, 257> fp = {};
                fp.pair = std::array<std::array<uint32_t, 257>, 2>{{{ZERO_256, ZERO_1}, {ZERO_256, ZERO_1}}};
                return core_distance_bp_simple(fp, max, weight);
            } else {
                MappingBlock<uint64_t, 521> fp = {};
                fp.pair = std::array<std::array<uint64_t, 521>, 2>{
                    {{ZERO_256, ZERO_256, ZERO_8, ZERO_1}, {ZERO_256, ZERO_256, ZERO_8, ZERO_1}}};
                return core_distance_bp_simple(fp, max, weight);
            }
        }

        else {
            /* for Big Size ANY data. */
            std::unordered_map<uint64_t, uint64_t, through_pass_hash<uint64_t>> fp = {};
            return core_distance_bp(fp, max, weight);
        }
    }
    double similar(double min = -1.0) { return (double)similar_p((std::size_t)min * 100) / 100.0; }

   protected:
    template <typename Storage>
    std::size_t core_distance_bp(Storage& fp, uint64_t max = INT64_MAX, bool weight = true) {
        /* over 64 charactors
           thank you for
            https://www.slideshare.net/KMC_JP/slide-www
            https://odz.hatenablog.com/entry/20070318/1174200775
            http://handasse.blogspot.com/2009/04/c_29.html
            https://github.com/fujimotos/polyleven/blob/master/doc/myers1999_block.c
            https://stackoverflow.com/questions/65363769/bitparallel-weighted-levenshtein-distance
            https://susisu.hatenablog.com/entry/2017/10/09/134032
         */
        std::size_t dist = A + B, i = 0, j = 0, sj = 0, mj = 0;
        using _Vty = typename std::remove_reference<decltype(fp[0])>::type;
        _Vty found = 0, adat = 0, trb = 0;
        const std::size_t BITS = std::min(std::size_t(64), (std::size_t)(sizeof(_Vty) * 8));

        for(std::size_t y = 0, len = std::min(BITS, B); y < len; ++y)
            fp[b[y]] |= 1ULL << (y % BITS);

        while(i < A && j < B) {
            if(max < dist - (A - i) * 2)
                return error_n - max;

            auto ai = a[i];
            if(ai == b[j]) {
                // ED_EQUAL
                dist -= 2;
            } else {
                adat = fp[ai];
                mj = j % BITS;
                trb = (_Vty)((adat << (BITS - mj + 1)) | (adat >> mj));
                /*  transbit means (*ex : j % BITS)
                 before   8    7    6    5    4*   3    2    1* -----
                 bit                          | adat >> (j % BITS)  |
                                              ----------------      |
                                                             |      |
                 after    3    2    1*   8    7    6    5    4*     |
                 bit                | adat << (BITS - (j % BITS))   |
                                    --------------------------------- */

                if(i > 0 && (found = (_Vty)(trb & (~trb + 1))) != 0) {
                    // ED_INSERT and ED_EQUAL
                    dist -= 2;
                    while(found > 1 && j < B) {
                        ++j;
                        found >>= 1;
                    }
                } else {
                    // ED_REPLACE
                    dist -= (!weight);
                }
            }

            do {
                mj = sj % BITS;
                fp[b[sj]] &= ~(1ULL << mj); /* clear finished data. */
                /* append next sequence data */
                if(sj + BITS < B - 1)
                    fp[b[sj + BITS]] |= 1ULL << mj;
                else
                    fp[b[B - 1]] |= 1ULL << mj;
            } while(++sj < j);

            i += 1;
            j += 1;
        }
        return dist;
    }

    template <typename Storage>
    std::size_t core_distance_bp_simple(Storage& fp, uint64_t max = INT64_MAX, bool weight = true) {
        /* under 64 charactors
         */
        std::size_t dist = A + B, i = 0, j = 0;
        using _Vty = typename std::remove_reference<decltype(fp[0])>::type;
        _Vty found = _Vty(0), trb = _Vty(0);

        for(std::size_t y = 0; y < B; ++y)
            fp[b[y]] |= 1ULL << y;

        while(i < A && j < B) {
            if(max < dist - (A - i) * 2)
                return error_n - max;
            auto ai = a[i];

            if(ai == b[j])
                dist -= 2;
            else if(i > 0 && (trb = (_Vty)(fp[ai] >> j)) != 0) {
                dist -= 2;
                found = (_Vty)(trb & (~trb + 1));
                while(found > 1 && j < B) {
                    ++j;
                    found >>= 1;
                }

            } else if(!weight)
                dist -= 1;

            ++i, ++j;
        }

        return dist;
    }

   public:
    std::size_t similar_p(std::size_t min = error_n) {
        std::size_t L;
        if((L = A + B) > 0) {
            if(min == error_n)
                return 100 - (100 * distance() / L);
            else
                return 100 - (100 * distance(L - (L * min) / 100) / L);
        }
        return 0;
    }
};

class Diff {
   public:
    PyObject* a;
    PyObject* b;
    int kind1;
    int kind2;

    Diff() : a(NULL), b(NULL), kind1(0), kind2(0) {}

    Diff(PyObject* _a, PyObject* _b) : a(_a), b(_b) {
        kind1 = (int)PyAny_KIND(a);
        kind2 = (int)PyAny_KIND(b);
        if(kind1 != kind2)
            kind1 = -kind1;
    }

    std::size_t distance(std::size_t max = error_n, bool weight = true) {
        if(kind1 == 1)
            return Diff_t<pyview_t<uint8_t>>(a, b).distance(max, weight);
        else if(kind1 == 2)
            return Diff_t<pyview_t<uint16_t>>(a, b).distance(max, weight);
        else if(kind1 == 8)
            return Diff_t<pyview_t<uint64_t>>(a, b).distance(max, weight);
        else if(kind1 < 0)
            return PyAny_Length(a, 1) + PyAny_Length(b, 1);
        else
            return Diff_t<pyview_t<uint32_t>>(a, b).distance(max, weight);
    }

    double similar(double min = -1.0) {
        if(kind1 == 1)
            return Diff_t<pyview_t<uint8_t>>(a, b).similar(min);
        else if(kind1 == 2)
            return Diff_t<pyview_t<uint16_t>>(a, b).similar(min);
        else if(kind1 == 8)
            return Diff_t<pyview_t<uint64_t>>(a, b).similar(min);
        else if(kind1 < 0)
            return 0.f;
        else
            return Diff_t<pyview_t<uint32_t>>(a, b).similar(min);
    }

    PyObject* difference(bool _diffonly = false, int _rep_rate = REPLACEMENT_RATE) {
        if(PyObject_RichCompareBool(a, b, Py_EQ)) {
            std::size_t len1 = error_n, i;
            PyObject* ops = PyList_New(0);
            if(_diffonly)
                return ops;
            if(PyMapping_Check(a))
                len1 = (std::size_t)PyObject_Length(a);
            if(len1 == error_n || len1 == 0) {
                makelist(ops, ED_EQUAL, 0, 0, a, b);
                return ops;
            }
            for(i = 0; i < len1; i++)
                makelist(ops, ED_EQUAL, i, i, a, b);
            return ops;
        }

        if(a == Py_None && b != Py_None) {
            std::size_t len2 = PyAny_Length(b);
            if(len2 != error_n) {
                PyObject* ops = PyList_New(0);
                if(len2 > 0) {
                    for(std::size_t i = 0; i < len2; i++)
                        makelist(ops, ED_INSERT, 0, i, a, b);
                } else {
                    makelist(ops, ED_INSERT, 0, 0, a, b);
                }
                return ops;
            }
        }
        if(b == Py_None && a != Py_None) {
            std::size_t len1 = PyAny_Length(a);
            if(len1 != error_n) {
                PyObject* ops = PyList_New(0);
                if(len1 > 0) {
                    for(std::size_t i = 0; i < len1; i++)
                        makelist(ops, ED_DELETE, i, 0, a, b);
                } else {
                    makelist(ops, ED_DELETE, 0, 0, a, b);
                }
                return ops;
            }
        }

        if(kind1 == 1)
            return Diff_t<pyview_t<uint8_t>>(a, b).difference(_diffonly, _rep_rate);
        else if(kind1 == 2)
            return Diff_t<pyview_t<uint16_t>>(a, b).difference(_diffonly, _rep_rate);
        else if(kind1 == 8) {
            return Diff_t<pyview_t<uint64_t>>(a, b).difference(_diffonly, _rep_rate);
        } else if(kind1 < 0) {
            std::size_t len1 = PyAny_Length(a);
            std::size_t len2 = PyAny_Length(b);

            if(len1 + len2 == 0 || (len1 == 1 && len2 == 1)) {
                PyObject* ops = PyList_New(0);
                if(_rep_rate < 1)
                    makelist(ops, ED_REPLACE, 0, 0, a, b);
                else {
                    makelist(ops, ED_DELETE, 0, 0, a, b);
                    makelist(ops, ED_INSERT, 0, 0, a, b);
                }
                return ops;
            }

            if(len1 <= len2)
                return Diff_t<pyview>(a, b).difference(_diffonly, _rep_rate);
            else {
                auto dt = Diff_t<pyview>(b, a);
                dt.swapflag = true;
                return dt.difference(_diffonly, _rep_rate);
            }
        } else if(kind1 == 4)
            return Diff_t<pyview_t<uint32_t>>(a, b).difference(_diffonly, _rep_rate);
        return NULL;
    }

    PyObject* compare(bool _diffonly,
                      int _rep_rate,
                      int _startidx,
                      PyObject* _condition_value,
                      PyObject* _na_value,
                      PyObject* _DEL_Flag,
                      PyObject* _ADD_Flag) {
        if(PyObject_RichCompareBool(a, b, Py_EQ)) {
            std::size_t len1 = error_n, i = 0;
            PyObject* ops = PyList_New(0);
            if(_diffonly)
                return ops;

            if(PyMapping_Check(a))
                len1 = (std::size_t)PyObject_Length(a);

            if(len1 == error_n || len1 == 0) {
                complist(ops, ED_EQUAL, i, i, a, b, false, _startidx, _condition_value, _na_value, _DEL_Flag,
                         _ADD_Flag);
            } else {
                for(i = 0; i < len1; i++)
                    complist(ops, ED_EQUAL, i, i, a, b, false, _startidx, _condition_value, _na_value, _DEL_Flag,
                             _ADD_Flag);
            }
            return ops;
        }

        if(kind1 == 1) {
            return Diff_t<pyview_t<uint8_t>>(a, b).compare(_diffonly, _rep_rate, _startidx, _condition_value, _na_value,
                                                           _DEL_Flag, _ADD_Flag);
        } else if(kind1 == 2) {
            return Diff_t<pyview_t<uint16_t>>(a, b).compare(_diffonly, _rep_rate, _startidx, _condition_value,
                                                            _na_value, _DEL_Flag, _ADD_Flag);
        } else if(a == Py_None) {
            std::size_t len2 = PyAny_Length(b);
            PyObject* ops = PyList_New(0);
            for(std::size_t i = 0, end = len2 ? len2 : 1; i < end; i++)
                complist(ops, ED_INSERT, 0, i, a, b, false, _startidx, _condition_value, _na_value, _DEL_Flag,
                         _ADD_Flag);
            return ops;
        } else if(b == Py_None) {
            std::size_t len1 = PyAny_Length(a);
            PyObject* ops = PyList_New(0);
            for(std::size_t i = 0, end = len1 ? len1 : 1; i < end; i++)
                complist(ops, ED_DELETE, i, 0, a, b, false, _startidx, _condition_value, _na_value, _DEL_Flag,
                         _ADD_Flag);
            return ops;
        } else if(kind1 == 8) {
            return Diff_t<pyview_t<uint64_t>>(a, b).compare(_diffonly, _rep_rate, _startidx, _condition_value,
                                                            _na_value, _DEL_Flag, _ADD_Flag);
        } else if(kind1 < 0) {
            std::size_t len1 = PyAny_Length(a);
            std::size_t len2 = PyAny_Length(b);

            if(len1 + len2 == 0 || (len1 == 1 && len2 == 1)) {
                PyObject* ops = PyList_New(0);

                if(_rep_rate < 1) {
                    complist(ops, ED_REPLACE, 0, 0, a, b, false, _startidx, _condition_value, _na_value, _DEL_Flag,
                             _ADD_Flag);
                } else {
                    complist(ops, ED_DELETE, 0, 0, a, b, false, _startidx, _condition_value, _na_value, _DEL_Flag,
                             _ADD_Flag);
                    complist(ops, ED_INSERT, 0, 0, a, b, false, _startidx, _condition_value, _na_value, _DEL_Flag,
                             _ADD_Flag);
                }
                return ops;
            }

            if(len1 <= len2)
                return Diff_t<pyview>(a, b).compare(_diffonly, _rep_rate, _startidx, _condition_value, _na_value,
                                                    _DEL_Flag, _ADD_Flag);
            else {
                auto dt = Diff_t<pyview>(b, a);
                dt.swapflag = true;
                return dt.compare(_diffonly, _rep_rate, _startidx, _condition_value, _na_value, _DEL_Flag, _ADD_Flag);
            }
        } else if(kind1 == 4) {
            return Diff_t<pyview_t<uint32_t>>(a, b).compare(_diffonly, _rep_rate, _startidx, _condition_value,
                                                            _na_value, _DEL_Flag, _ADD_Flag);
        }
        return NULL;
    }
};

class Compare {
   public:
    PyObject* a;
    PyObject* b;
    PyObject* keya;
    PyObject* keyb;
    bool header;
    bool diffonly;
    int rep_rate;
    int startidx;
    PyObject* condition_value;
    PyObject* na_value;
    PyObject* delete_sign_value = NULL;
    PyObject* insert_sign_value = NULL;

   private:
    int* idxa = NULL;
    int* idxb = NULL;
    Py_ssize_t maxcol = 0;
    bool need_clean_cv = false;
    bool need_clean_nv = false;
    PyObject* DEL_Flag = NULL;
    PyObject* ADD_Flag = NULL;

   public:
    Compare()
        : a(NULL),
          b(NULL),
          keya(NULL),
          keyb(NULL),
          header(true),
          diffonly(false),
          rep_rate(REPLACEMENT_RATE),
          startidx(0),
          condition_value(NULL),
          na_value(NULL),
          delete_sign_value(NULL),
          insert_sign_value(NULL),
          idxa(NULL),
          idxb(NULL),
          maxcol(0),
          need_clean_cv(false),
          need_clean_nv(false),
          DEL_Flag(NULL),
          ADD_Flag(NULL) {}

    Compare(PyObject* args, PyObject* kwargs)
        : a(),
          b(),
          keya(NULL),
          keyb(NULL),
          header(true),
          diffonly(false),
          rep_rate(REPLACEMENT_RATE),
          startidx(0),
          condition_value(NULL),
          na_value(NULL),
          delete_sign_value(NULL),
          insert_sign_value(NULL),
          idxa(NULL),
          idxb(NULL),
          maxcol(0),
          need_clean_cv(false),
          need_clean_nv(false),
          DEL_Flag(NULL),
          ADD_Flag(NULL) {
        const char* kwlist[13] = {"a",
                                  "b",
                                  "keya",
                                  "keyb",
                                  "header",
                                  "diffonly",
                                  "rep_rate",
                                  "startidx",
                                  "condition_value",
                                  "na_value",
                                  "delete_sign_value",
                                  "insert_sign_value",
                                  NULL};

        if(!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|OOiiiiOOOO", (char**)kwlist, &a, &b, &keya, &keyb, &header,
                                        &diffonly, &rep_rate, &startidx, &condition_value, &na_value,
                                        &delete_sign_value, &insert_sign_value))
            return;

        initialize();
    }

    Compare(PyObject* _a,
            PyObject* _b,
            PyObject* _keya,
            PyObject* _keyb,
            bool _header,
            bool _diffonly,
            int _rep_rate,
            int _startidx,
            PyObject* _condition_value,
            PyObject* _na_value,
            PyObject* _delete_sign_value,
            PyObject* _insert_sign_value)
        : a(_a),
          b(_b),
          keya(_keya),
          keyb(_keyb),
          header(_header),
          diffonly(_diffonly),
          rep_rate(_rep_rate),
          startidx(_startidx),
          condition_value(_condition_value),
          na_value(_na_value),
          delete_sign_value(_delete_sign_value),
          insert_sign_value(_insert_sign_value),
          idxa(NULL),
          idxb(NULL),
          maxcol(0),
          need_clean_cv(false),
          need_clean_nv(false),
          DEL_Flag(NULL),
          ADD_Flag(NULL) {
        initialize();
    }

    void initialize() {
        if(keya)
            a = sortWithKey(idxa, a, keya);
        if(keyb)
            b = sortWithKey(idxb, b, keyb);

        if(condition_value == NULL) {
            condition_value = PyUnicode_FromString(" ---> ");
            need_clean_cv = true;
        } else if(!PyUnicode_Check(condition_value)) {
            PyErr_Format(PyExc_AttributeError, "`condition_value` should be unicode string.");
            return;
        }

        if(na_value == NULL) {
            na_value = PyUnicode_FromString("-");
            need_clean_nv = true;
        } else if(!PyUnicode_Check(na_value)) {
            PyErr_Format(PyExc_AttributeError, "`na_value` should be unicode string.");
            return;
        }

        DEL_Flag = delete_sign_value ? delete_sign_value : PyUnicode_FromString("DEL");
        ADD_Flag = insert_sign_value ? insert_sign_value : PyUnicode_FromString("ADD");
    }

    ~Compare() {
        if(idxa)
            PyMem_DEL(idxa);
        if(idxb)
            PyMem_DEL(idxb);
        if(need_clean_cv)
            Py_XDECREF(condition_value);
        if(need_clean_nv)
            Py_DECREF(na_value);
        if(delete_sign_value == NULL)
            Py_XDECREF(DEL_Flag);
        if(insert_sign_value == NULL)
            Py_XDECREF(ADD_Flag);
    }

   private:
    /* thank you for
     * https://stackoverflow.com/questions/51959095/how-to-sort-a-list-with-a-custom-key-using-the-python-c-api */
    PyObject* sortWithKey(int*& indexes, PyObject*& list, PyObject* key) {
        PyObject* argTuple = PyTuple_New(0);
        PyObject* keywords = PyDict_New();
        PyObject* keyString = PyUnicode_FromString("key");
        PyDict_SetItem(keywords, keyString, key);

        Py_ssize_t len = PyObject_Length(list);
        std::unordered_map<uint64_t, int> idict = {};

        for(Py_ssize_t i = 0; i < len; ++i) {
            PyObject* row = PySequence_ITEM(list, i);

            if(PyTuple_Check(row) || PyIter_Check(row) || PyGen_Check(row) || PyRange_Check(row)) {
                PyObject* rerow = PySequence_List(row);
                PySequence_SetItem(list, i, rerow);
                idict[uint64_t(rerow)] = (int)i;
                Py_DECREF(rerow);
            } else {
                idict[uint64_t(row)] = (int)i;
            }
            Py_DECREF(row);
            if(PyErr_Occurred()) {
                return PyErr_Format(PyExc_TypeError, "Can not append index data.");
            }
        }

        PyObject* sortMethod = PyObject_GetAttrString(list, "sort");

        PyObject* result = PyObject_Call(sortMethod, argTuple, keywords);
        if(result == NULL) {
            return PyErr_Format(PyExc_TypeError, "Can not call sort method.");
        }
        Py_DECREF(result);

        indexes = PyMem_New(int, len);

        for(Py_ssize_t i = 0; i < len; ++i) {
            PyObject* row = PySequence_ITEM(list, i);
            indexes[i] = idict[uint64_t(row)];
        }

        Py_DECREF(keyString);
        Py_DECREF(keywords);
        Py_DECREF(argTuple);

        return list;
    }

    PyObject* intercomplist(PyObject*& row) {
        Py_ssize_t rlen, j = 0;
        PyObject *id_a = NULL, *id_b = NULL, *it_a = NULL, *it_b = NULL, *cmp = NULL, *tag = NULL;

        it_a = PySequence_ITEM(row, 3);
        if(!it_a || PyUnicode_Check(it_a) || PyNumber_Check(it_a) || PyBytes_Check(it_a) || PyByteArray_Check(it_a)) {
            Py_CLEAR(it_a);
            return NULL;
        }
        it_b = PySequence_ITEM(row, 4);
        if(!it_b || PyUnicode_Check(it_b) || PyNumber_Check(it_b) || PyBytes_Check(it_b) || PyByteArray_Check(it_b)) {
            Py_DECREF(it_b);
            return NULL;
        }

        PyObject* list = PyList_New(3);

        tag = PySequence_ITEM(row, 0);
        if(tag == NULL)
            return PyErr_Format(PyExc_ValueError, "`Tag name` value Not Found.");

        PyList_SetItem(list, 0, tag);

        if((id_a = PySequence_ITEM(row, 1)) == Py_None) {
            Py_INCREF(na_value);
            PyList_SetItem(list, 1, na_value);
        } else if(keya && idxa) {
            PyList_SetItem(list, 1, PyLong_FromLong(idxa[PyLong_AsSize_t(id_a)] + startidx));
            Py_XDECREF(id_a);
        } else {
            std::size_t ia = PyLong_AsSize_t(id_a) + startidx;
            PyList_SetItem(list, 1, PyLong_FromSize_t(ia));
        }

        if((id_b = PySequence_ITEM(row, 2)) == Py_None) {
            Py_INCREF(na_value);
            PyList_SetItem(list, 2, na_value);
        } else if(keyb && idxb) {
            PyList_SetItem(list, 2, PyLong_FromLong(idxb[PyLong_AsSize_t(id_b)] + startidx));
            Py_XDECREF(id_b);
        } else {
            std::size_t ib = PyLong_AsSize_t(id_b) + startidx;
            PyList_SetItem(list, 2, PyLong_FromSize_t(ib));
        }

        cmp = Diff(it_a, it_b).compare(false, rep_rate, startidx, condition_value, na_value, DEL_Flag, ADD_Flag);
        Py_DECREF(it_a);
        Py_DECREF(it_b);

        if((rlen = PyObject_Length(cmp)) == -1)
            return PyErr_Format(PyExc_ValueError, "Atribute(`a` or `b`) is not a two-dimensional array.");

        for(; j < rlen; j++) {
            PyObject* cols = PySequence_ITEM(cmp, j);
            if(cols == NULL)
                return PyErr_Format(PyExc_ValueError, "Atribute(`a` or `b`) is not a two-dimensional array.");

            PyObject* cell = PySequence_ITEM(cols, 3);
            if(cell == NULL)
                return PyErr_Format(PyExc_ValueError, "Atribute(`a` or `b`) is not a two-dimensional array.");

            PyList_Append(list, cell);
            Py_DECREF(cell);
            Py_DECREF(cols);
        }

        if(maxcol < rlen)
            maxcol = rlen;

        Py_XDECREF(cmp);

        return list;
    }

   public:
    PyObject* _1d(bool is_initialcall = true) {
        if(is_initialcall) {
            Py_INCREF(a);
            Py_INCREF(b);
        }

        PyObject* cmp = Diff(a, b).compare(diffonly, rep_rate, startidx, condition_value, na_value, DEL_Flag, ADD_Flag);

        if(keya || keyb) {
            Py_ssize_t len = PyObject_Length(cmp);
            std::vector<std::pair<std::size_t, PyObject*>> tmp;
            tmp.reserve((std::size_t)len);

            for(Py_ssize_t i = 0; i < len; i++) {
                PyObject* row = PySequence_ITEM(cmp, i);
                PyObject* id_a = PySequence_ITEM(row, 1);
                PyObject* id_b = PySequence_ITEM(row, 2);
                std::size_t ia = id_a == na_value ? INT_MAX : PyLong_AsSize_t(id_a);
                std::size_t ib = id_b == na_value ? INT_MAX : PyLong_AsSize_t(id_b);
                PySequence_SetItem(row, 1, id_a == na_value ? id_a : PyLong_FromLong(idxa[ia] + startidx));
                PySequence_SetItem(row, 2, id_b == na_value ? id_b : PyLong_FromLong(idxb[ib] + startidx));
                tmp.emplace_back(std::min(ia, ib), row);
                Py_XDECREF(id_a);
                Py_XDECREF(id_b);
            }
            std::sort(tmp.begin(), tmp.end());
            for(std::size_t i = 0, count = tmp.size(); i < count; ++i) {
                PyList_SetItem(cmp, (Py_ssize_t)i, tmp[i].second);
            }
        }

        if(header) {
            PyObject* head = Py_BuildValue("[ssss]", "tag", "index_a", "index_b", "data");
            if((PyList_Insert(cmp, 0, head)) == -1)
                return PyErr_Format(PyExc_RuntimeError, "Unknown Error cdiffer.hpp _1d() near");
            Py_DECREF(head);
        }

        return cmp;
    }

    PyObject* _2d(bool is_initialcall = true) {
        Py_ssize_t len, i;
        PyObject* df = Diff(a, b).difference(diffonly, rep_rate);

        if(df == NULL) {
            return PyErr_Format(PyExc_ValueError, NULL);
        }

        if((len = PyObject_Length(df)) == -1) {
            return PyErr_Format(PyExc_RuntimeError, "Unknown Error cdiffer.hpp _2d() head");
        }

        PyObject* ops = PyList_New(len + header);

        int need_ommit = 0;
        if(a == Py_None &&
           !(PyList_Check(b) || PyTuple_Check(b) || PyIter_Check(b) || PyGen_Check(b) || PyRange_Check(b)))
            need_ommit = ED_INSERT;
        else if(!(PyList_Check(a) || PyTuple_Check(a) || PyIter_Check(a) || PyGen_Check(a) || PyRange_Check(a)) &&
                b == Py_None)
            need_ommit = ED_DELETE;

        for(i = 0; i < len; i++) {
            PyObject* row = PySequence_ITEM(df, i);
            if(row == NULL) {
                Py_XDECREF(ops);
                Py_CLEAR(df);
                return PyErr_Format(PyExc_ValueError, "Atribute(`a` or `b`) is not a two-dimensional array.");
            }
            if(need_ommit) {
                PyObject* ctag = PySequence_ITEM(row, 0);
                if(PyObject_RichCompareBool(ctag, DIFFTP[0][need_ommit], Py_NE)) {
                    Py_CLEAR(ctag);
                    Py_CLEAR(ops);
                    Py_CLEAR(df);
                    Py_CLEAR(row);
                    return this->_1d(false);
                }
                Py_XDECREF(ctag);
            }
            PyObject* list = intercomplist(row);
            if(list == NULL) {
                Py_CLEAR(ops);
                Py_CLEAR(df);
                Py_CLEAR(row);
                return this->_1d(false);
            }

            PyList_SET_ITEM(ops, i + header, list);

            Py_XDECREF(row);

            if(PyErr_Occurred() != NULL)
                return PyErr_Format(PyExc_RuntimeError, "Unknown Error cdiffer.hpp _2d() below");
        }

        Py_CLEAR(df);

        if(header) {
            PyObject* head = PyList_New(3 + maxcol);
            PyList_SET_ITEM(head, 0, PyUnicode_FromString("tag"));
            PyList_SET_ITEM(head, 1, PyUnicode_FromString("index_a"));
            PyList_SET_ITEM(head, 2, PyUnicode_FromString("index_b"));
            if(maxcol == 1) {
                PyList_SET_ITEM(head, 3, PyUnicode_FromString("data"));
            } else {
                for(int n = 0; n < maxcol; n++) {
                    char colname[7] = {'C', 'O', 'L', '_', n < 10 ? '0' : char(0x30 + (n / 10)), char(0x30 + (n % 10)),
                                       NULL};
                    PyList_SET_ITEM(head, 3 + n, PyUnicode_FromString((const char*)colname));
                }
            }

            if((PyList_SetItem(ops, 0, head)) == -1)
                return PyErr_Format(PyExc_RuntimeError, "Unknown Error cdiffer.hpp _2d() header");
        }

        return ops;
    }

    PyObject* _3d() {
        Py_ssize_t len, i, j, slen;

        PyObject* la = PyDict_Keys(a);
        PyObject* lb = PyDict_Keys(b);
        if(la == NULL) {
            la = Py_None;
        }
        if(lb == NULL) {
            lb = Py_None;
        }

        PyObject* dfs = Diff(la, lb).difference(diffonly, rep_rate);
        Py_XDECREF(la);
        Py_XDECREF(lb);

        if(dfs == NULL) {
            return PyErr_Format(PyExc_ValueError, "Faiotal Error `Diff.difference` result get.");
        }

        if((len = PyObject_Length(dfs)) == -1) {
            Py_DECREF(dfs);
            return PyErr_Format(PyExc_RuntimeError, "Unknown Error cdiffer.hpp _2d() head");
        }

        PyObject* ops = PyList_New(header == true);

        for(i = 0; i < len; ++i) {
            PyObject *tag, *sa, *sb, *da, *db, *arr, *df, *concat, *content, *row;
            if((arr = PySequence_ITEM(dfs, i)) == NULL) {
                Py_XDECREF(ops);
                Py_XDECREF(dfs);
                return PyErr_Format(PyExc_ValueError, "Cannot get a Dictionary Inner array.");
            }

            tag = PySequence_ITEM(arr, 0);
            sa = PySequence_ITEM(arr, 3);
            sb = PySequence_ITEM(arr, 4);

            bool need_decref_a = true;
            bool need_decref_b = true;
            da = PyDict_GetItemWithError(a, sa);
            db = PyDict_GetItemWithError(b, sb);

            if(da == NULL) {
                da = Py_None;
                Py_INCREF(da);
                need_decref_a = false;
            } else if(PyAny_KIND(da) == 8) {
                if(da == Py_None) {
                    Py_INCREF(da);
                    need_decref_a = false;
                } else if(PyIter_Check(da) || PyGen_Check(da) || PyRange_Check(da)) {
                    da = PySequence_Fast(
                        da, "from `da` iterator");  //@note memory leak when PySequence_List or PySequence_Tuple
                } else if(PyTuple_Check(da)) {
                    if(PyObject_Length(da) == 0)
                        Py_INCREF(da);
                    need_decref_a = false;
                } else if(PyList_Check(da)) {
                    if(PyObject_Length(da) == 0)
                        need_decref_a = false;
                } else if(PyNumber_Check(da)) {
                    Py_INCREF(da);
                    need_decref_a = false;
                }
            } else {
                if(PyObject_Length(da) == 0) {
                    Py_INCREF(da);
                    need_decref_a = false;
                } else {
                    da = Py_BuildValue("[O]", da);
                }
            }

            if(db == NULL) {
                db = Py_None;
                Py_INCREF(db);
                need_decref_b = false;
            } else if(PyAny_KIND(db) == 8) {
                if(db == Py_None) {
                    Py_INCREF(db);
                    need_decref_b = false;
                } else if(PyIter_Check(db) || PyGen_Check(db) || PyRange_Check(db)) {
                    db = PySequence_Fast(
                        db, "from `da` iterator");  //@note memory leak when PySequence_List or PySequence_Tuple
                } else if(PyTuple_Check(db)) {
                    if(PyObject_Length(db) == 0)
                        Py_INCREF(db);
                    need_decref_b = false;
                } else if(PyList_Check(db)) {
                    if(PyObject_Length(db) == 0)
                        need_decref_b = false;
                } else if(PyNumber_Check(db)) {
                    Py_INCREF(db);
                    need_decref_b = false;
                }
            } else {
                if(PyObject_Length(db) == 0) {
                    Py_INCREF(db);
                    need_decref_b = false;
                } else {
                    db = Py_BuildValue("[O]", db);
                }
            }

            Compare cmp(da, db, keya, keyb, false, diffonly, rep_rate, startidx, condition_value, na_value,
                        delete_sign_value, insert_sign_value);

            df = cmp._2d();

            if(need_decref_a)
                Py_DECREF(da);
            if(need_decref_b)
                Py_DECREF(db);

            if(maxcol < cmp.maxcol)
                maxcol = cmp.maxcol;

            if(tag == DIFFTP[0][ED_REPLACE]) {
                concat = PyUnicode_Concat(sa, condition_value);
                content = PyUnicode_Concat(concat, sb);
                Py_XDECREF(concat);
            } else if(tag == DIFFTP[0][ED_INSERT]) {
                content = sb;
            } else {
                content = sa;
            }

            Py_XDECREF(tag);
            Py_XDECREF(sa);
            Py_XDECREF(sb);

            for(j = 0, slen = PyObject_Length(df); j < slen; ++j) {
                if((row = PySequence_ITEM(df, j)) == NULL) {
                    Py_DECREF(ops);
                    Py_DECREF(arr);
                    Py_XDECREF(df);
                    Py_DECREF(dfs);
                    return PyErr_Format(PyExc_ValueError, "Cannot get a Dictionary Inner array.");
                }

                PyList_Insert(row, 0, content);
                PyList_Append(ops, row);

                Py_XDECREF(row);
            }

            Py_DECREF(arr);
            Py_DECREF(df);
        }

        Py_DECREF(dfs);

        if(header) {
            PyObject* head = PyList_New(4 + maxcol);
            PyList_SET_ITEM(head, 0, PyUnicode_FromString("group"));
            PyList_SET_ITEM(head, 1, PyUnicode_FromString("tag"));
            PyList_SET_ITEM(head, 2, PyUnicode_FromString("index_a"));
            PyList_SET_ITEM(head, 3, PyUnicode_FromString("index_b"));
            if(maxcol == 1) {
                PyList_SET_ITEM(head, 4, PyUnicode_FromString("data"));
            } else {
                for(int n = 0; n < maxcol; n++) {
                    char colname[7] = {'C', 'O', 'L', '_', n < 10 ? '0' : char(0x30 + (n / 10)), char(0x30 + (n % 10)),
                                       NULL};
                    PyList_SET_ITEM(head, 4 + n, PyUnicode_FromString((const char*)colname));
                }
            }
            if((PyList_SetItem(ops, 0, head)) == -1)
                return PyErr_Format(PyExc_RuntimeError, "Unknown Error cdiffer.hpp _2d() header");
        }

        return ops;
    }
};

}  // namespace gammy

/*
 * python Interface function
 */
extern "C" PyObject* dist_py(PyObject* self, PyObject* args);
extern "C" PyObject* similar_py(PyObject* self, PyObject* args);
extern "C" PyObject* differ_py(PyObject* self, PyObject* args, PyObject* kwargs);
extern "C" PyObject* compare_py(PyObject* self, PyObject* args, PyObject* kwargs);

#endif /* !defined(CDIFFER_H) */
