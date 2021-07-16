#pragma once
#ifndef CDIFFER_H
#define CDIFFER_H

#define PY_SSIZE_T_CLEAN
#include <array>
#include <unordered_map>
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
    T operator()(const PyObject*& s) const {
        T hash;
        if((hash = (T)PyObject_Hash(s)) == -1) {
            PyObject* item = PySequence_Tuple(s);
            hash = (T)PyObject_Hash(s);
            Py_DECREF(item);
        }
        return hash;
    }
};

template <typename BitTy = uint64_t, std::size_t fraction_size = 83>
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
        if(len1 <= 1) {
            Py_INCREF(a);
            PyList_SetItem(list, 3L + swapflag, a);
        } else
            PyList_SetItem(list, 3L + swapflag, PySequence_GetItem(a, (Py_ssize_t)x));
    }
    if(dtype == ED_DELETE) {
        Py_INCREF(Py_None);
        PyList_SetItem(list, 2L - swapflag, Py_None);
        Py_INCREF(Py_None);
        PyList_SetItem(list, 4L - swapflag, Py_None);
    } else {
        PyList_SetItem(list, 2L - swapflag, PyLong_FromSize_t(y));
        if(len2 <= 1) {
            Py_INCREF(b);
            PyList_SetItem(list, 4L - swapflag, b);
        } else
            PyList_SetItem(list, 4L - swapflag, PySequence_GetItem(b, (Py_ssize_t)y));
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

    Diff_t()
        : a(nullptr),
          b(nullptr),
          A(error_n),
          B(error_n),
          D(error_n),
          SIZE(SIZE_MAX),
          swapflag(false),
          diffonly(false),
          rep_rate(REPLACEMENT_RATE) {}
    Diff_t(nullptr_t) { Diff_t::Diff_t(); }

    Diff_t(PyObject* _a, PyObject* _b) : a(CharT(_a)), b(CharT(_b)) {
        A = a.size();
        B = b.size();
        swapflag = a.kind == b.kind && A > B;
        if(swapflag) {
            std::swap(A, B);
            std::swap(a, b);
        }
        D = B - A;
        SIZE = (std::size_t)A + B + 1;
    }

    ~Diff_t() {}

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

        if(A < 2 && B < 2) {
            PyObject* ops = PyList_New(0);

            if(rep_rate < 1) {
                makelist(ops, ED_REPLACE, 0, 0, a.py, b.py);
            } else {
                makelist(ops, ED_DELETE, 0, 0, a.py, b.py);
                makelist(ops, ED_INSERT, 0, 0, a.py, b.py);
            }
            return ops;
        }

        else if(B < 256) {
            if(B < 8) {
                MappingBlock<uint8_t> fp = {};
                fp.pair = std::array<std::array<uint8_t, 83>, 2>{
                    {{ZERO_64, ZERO_16, ZERO_2, ZERO_1}, {ZERO_64, ZERO_16, ZERO_2, ZERO_1}}};
                return core_difference(fp);
            } else if(B < 16) {
                MappingBlock<uint16_t> fp = {};
                fp.pair = std::array<std::array<uint16_t, 83>, 2>{
                    {{ZERO_64, ZERO_16, ZERO_2, ZERO_1}, {ZERO_64, ZERO_16, ZERO_2, ZERO_1}}};
                return core_difference(fp);
            } else if(B < 32) {
                MappingBlock<uint32_t> fp = {};
                fp.pair = std::array<std::array<uint32_t, 83>, 2>{
                    {{ZERO_64, ZERO_16, ZERO_2, ZERO_1}, {ZERO_64, ZERO_16, ZERO_2, ZERO_1}}};
                return core_difference(fp);
            } else if(B < 64) {
                MappingBlock<uint64_t, 127> fp = {};
                fp.pair = std::array<std::array<uint64_t, 127>, 2>{
                    {{ZERO_64, ZERO_32, ZERO_16, ZERO_8, ZERO_4, ZERO_2, ZERO_1},
                     {ZERO_64, ZERO_32, ZERO_16, ZERO_8, ZERO_4, ZERO_2, ZERO_1}}};
                return core_difference(fp);
            } else if(B < 128) {
                MappingBlock<uint64_t, 211> fp = {};
                fp.pair = std::array<std::array<uint64_t, 211>, 2>{
                    {{ZERO_128, ZERO_64, ZERO_16, ZERO_2, ZERO_1}, {ZERO_128, ZERO_64, ZERO_16, ZERO_2, ZERO_1}}};
                return core_difference(fp);
            } else if(B < 192) {
                MappingBlock<uint64_t, 313> fp = {};
                fp.pair = std::array<std::array<uint64_t, 313>, 2>{
                    {{ZERO_256, ZERO_32, ZERO_16, ZERO_8, ZERO_1}, {ZERO_256, ZERO_32, ZERO_16, ZERO_8, ZERO_1}}};
                return core_difference(fp);
            } else {
                MappingBlock<uint64_t, 599> fp = {};
                fp.pair = std::array<std::array<uint64_t, 599>, 2>{
                    {{ZERO_256, ZERO_256, ZERO_64, ZERO_16, ZERO_4, ZERO_2, ZERO_1},
                     {ZERO_256, ZERO_256, ZERO_64, ZERO_16, ZERO_4, ZERO_2, ZERO_1}}};
                return core_difference(fp);
            }
        }

        else {
            /* for Big Size ANY data. */
            std::unordered_map<uint64_t, uint64_t, through_pass_hash<uint64_t>> fp = {};
            return core_difference(fp);
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
            PyList_SetItem(list, 3L + swapflag, a.getitem(x));
        }
        if(dtype == ED_DELETE) {
            Py_INCREF(Py_None);
            PyList_SetItem(list, 2L - swapflag, Py_None);
            Py_INCREF(Py_None);
            PyList_SetItem(list, 4L - swapflag, Py_None);
        } else {
            Py_INCREF(pyn[y]);
            PyList_SetItem(list, 2L - swapflag, pyn[y]);
            PyList_SetItem(list, 4L - swapflag, b.getitem(y));
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
            if(rep_rate > 0 &&
               ((a.canonical && b.canonical) || Diff_t<pyview>(a.getitem(0), b.getitem(0)).similar(rep_rate) * 100 < rep_rate)) {
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

            if(!diffonly && ai == b[j]) {
                makelist_pyn(ops, pyn, ED_EQUAL, x, j);
            } else {
                adat = fp[ai];
                mj = j % BITS;
                trb = (adat << (BITS - mj + 1)) | (adat >> mj);
                if((found = trb & (~trb + 1)) != 0) {
                    while(found > 1) {
                        found >>= 1;
                        makelist_pyn(ops, pyn, ED_INSERT, x, j);
                        ++j;
                    }
                    if(!diffonly)
                        makelist_pyn(ops, pyn, ED_EQUAL, x, j);
                } else if(i < A) {
                    if(rep_rate > 0 && ((a.canonical && b.canonical) ||
                       Diff_t<pyview>(a.getitem(x), b.getitem(j)).similar(rep_rate) * 100 < rep_rate)) {
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

        else if(B < 256) {
            if(B < 8) {
                MappingBlock<uint8_t> fp = {};
                fp.pair = std::array<std::array<uint8_t, 83>, 2>{
                    {{ZERO_64, ZERO_16, ZERO_2, ZERO_1}, {ZERO_64, ZERO_16, ZERO_2, ZERO_1}}};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 16) {
                MappingBlock<uint16_t> fp = {};
                fp.pair = std::array<std::array<uint16_t, 83>, 2>{
                    {{ZERO_64, ZERO_16, ZERO_2, ZERO_1}, {ZERO_64, ZERO_16, ZERO_2, ZERO_1}}};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 32) {
                MappingBlock<uint32_t> fp = {};
                fp.pair = std::array<std::array<uint32_t, 83>, 2>{
                    {{ZERO_64, ZERO_16, ZERO_2, ZERO_1}, {ZERO_64, ZERO_16, ZERO_2, ZERO_1}}};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 64) {
                MappingBlock<uint64_t, 127> fp = {};
                fp.pair = std::array<std::array<uint64_t, 127>, 2>{
                    {{ZERO_64, ZERO_32, ZERO_16, ZERO_8, ZERO_4, ZERO_2, ZERO_1},
                     {ZERO_64, ZERO_32, ZERO_16, ZERO_8, ZERO_4, ZERO_2, ZERO_1}}};
                return core_distance_bp_simple(fp, max, weight);
            } else if(B < 128) {
                MappingBlock<uint64_t, 211> fp = {};
                fp.pair = std::array<std::array<uint64_t, 211>, 2>{
                    {{ZERO_128, ZERO_64, ZERO_16, ZERO_2, ZERO_1}, {ZERO_128, ZERO_64, ZERO_16, ZERO_2, ZERO_1}}};
                return core_distance_bp(fp, max, weight);
            } else if(B < 192) {
                MappingBlock<uint64_t, 313> fp = {};
                fp.pair = std::array<std::array<uint64_t, 313>, 2>{
                    {{ZERO_256, ZERO_32, ZERO_16, ZERO_8, ZERO_1}, {ZERO_256, ZERO_32, ZERO_16, ZERO_8, ZERO_1}}};
                return core_distance_bp(fp, max, weight);
            } else {
                MappingBlock<uint64_t, 599> fp = {};
                fp.pair = std::array<std::array<uint64_t, 599>, 2>{
                    {{ZERO_256, ZERO_256, ZERO_64, ZERO_16, ZERO_4, ZERO_2, ZERO_1},
                     {ZERO_256, ZERO_256, ZERO_64, ZERO_16, ZERO_4, ZERO_2, ZERO_1}}};
                return core_distance_bp(fp, max, weight);
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

                if((found = (_Vty)(trb & (~trb + 1))) != 0) {
                    // ED_INSERT and ED_EQUAL
                    dist -= 2;
                    while(found > 1) {
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
            else if((trb = (_Vty)(fp[ai] >> j)) != 0) {
                dist -= 2;
                found = (_Vty)(trb & (~trb + 1));
                while(found > 1) {
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
        else if(kind1 == 8) {
            if(PyAny_Length(a) == 1 && PyAny_Length(b) == 1)
                return 1ULL + weight;
            else
                return Diff_t<pyview_t<uint64_t>>(a, b).distance(max, weight);
        } else if(kind1 < 0)
            return PyAny_Length(a, 1) + PyAny_Length(b, 1);
        else
            return Diff_t<pyview_t<uint32_t>>(a, b).distance(max, weight);
    }

    double similar(double min = -1.0) {
        if(kind1 == 1)
            return Diff_t<pyview_t<uint8_t>>(a, b).similar(min);
        else if(kind1 == 2)
            return Diff_t<pyview_t<uint16_t>>(a, b).similar(min);
        else if(kind1 == 8) {
            if(PyAny_Length(a) == 1 && PyAny_Length(b) == 1)
                return 0.f;
            else
                return Diff_t<pyview_t<uint64_t>>(a, b).similar(min);
        } else if(kind1 < 0)
            return 0.f;
        else
            return Diff_t<pyview_t<uint32_t>>(a, b).similar(min);
    }

    PyObject* difference(bool _diffonly = false, int _rep_rate = REPLACEMENT_RATE) {
        if(kind1 == 1)
            return Diff_t<pyview_t<uint8_t>>(a, b).difference(_diffonly, _rep_rate);
        else if(kind1 == 2)
            return Diff_t<pyview_t<uint16_t>>(a, b).difference(_diffonly, _rep_rate);
        else if(kind1 == 8) {
            if(PyAny_Length(a) == 1 && PyAny_Length(b) == 1) {
                PyObject* ops = PyList_New(0);
                if(_rep_rate < 1)
                    makelist(ops, ED_REPLACE, 0, 0, a, b);
                else {
                    makelist(ops, ED_DELETE, 0, 0, a, b);
                    makelist(ops, ED_INSERT, 0, 0, a, b);
                }
                return ops;
            } else
                return Diff_t<pyview_t<uint64_t>>(a, b).difference(_diffonly, _rep_rate);
        } else if(kind1 < 0) {
            return Diff_t<pyview>(a, b).difference(_diffonly, _rep_rate);
        } else if(kind1 == 4)
            return Diff_t<pyview_t<uint32_t>>(a, b).difference(_diffonly, _rep_rate);
        return NULL;
    }
};
}  // namespace gammy

/*
 * python Interface function
 */
extern "C" PyObject* dist_py(PyObject* self, PyObject* args);
extern "C" PyObject* similar_py(PyObject* self, PyObject* args);
extern "C" PyObject* differ_py(PyObject* self, PyObject* args, PyObject* kwargs);

#endif /* !defined(CDIFFER_H) */
