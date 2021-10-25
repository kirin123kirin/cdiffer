#pragma once
#ifndef PYYOU_H
#define PYYOU_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <limits>

std::size_t error_n = (std::size_t)(-1);

#define PyNotHashable_Check(op)                                                                        \
    PyList_Check(op) || PyObject_TypeCheck(op, &PyDict_Type) || PyIter_Check(op) || PyGen_Check(op) || \
        PyObject_TypeCheck(op, &PyDictItems_Type) || PyObject_TypeCheck(op, &PyDictKeys_Type) ||       \
        PyObject_TypeCheck(op, &PyDictValues_Type) || PyObject_TypeCheck(op, &PySet_Type)
#define PyHashable_Check(op)                                                                                        \
    PyUnicode_Check(op) || PyTuple_Check(op) || PyNumber_Check(op) || PyBytes_Check(op) || PyByteArray_Check(op) || \
        PyBool_Check(op) || op == Py_None

inline std::size_t PyAny_KIND(PyObject*& o) {
    if(PyUnicode_Check(o)) {
#if PY_MAJOR_VERSION >= 3
        return PyUnicode_KIND(o);

#else
        return 4;

#endif

    } else if(PyBytes_Check(o) || PyByteArray_Check(o)) {
        return 1;
    } else {
        return 8;
    }
}

/*
  deal iter_level : default 0
    * 0 -> return `(std::size_t)-1`, Do nothing to the iterator (The return value is set to `-1`)
    * 1 -> return `object size`, Run the iterator. And return the length of the object.(But the iterator will be
  consumed.)
    * 2 -> return `object size`, It's a destructive feature, so be careful. Instead of consuming the iterator, save the
  tuple and replace it with the original object.

*/
inline std::size_t PyAny_Length(PyObject*& o, int deal_iter_level = 0) {
#if PY_MAJOR_VERSION >= 3
    if(PyMapping_Check(o)) {
#else
    if(PyMapping_Check(o) || PySequence_Check(o)) {
#endif
        return (std::size_t)PyObject_Length(o);
    } else {
        if(PyNumber_Check(o) || PyBool_Check(o))
            return 1;
        if(o == Py_None)
            return 1;

        std::size_t len = error_n;
        if(deal_iter_level > 0) {
            PyObject* item = PySequence_Tuple(o);
            len = (std::size_t)PyObject_Length(item);
            if(deal_iter_level == 2)
                std::swap(o, item);
            Py_DECREF(item);
        }
        return len;
    }
}

template <class T>
struct PyMallocator {
    typedef T value_type;

    PyMallocator() = default;
    template <class U>
    constexpr PyMallocator(const PyMallocator<U>&) noexcept {}

    [[nodiscard]] T* allocate(std::size_t n) {
        if(n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();
        if(auto p = PyMem_New(T, n)) {
            // report(p, n);
            return p;
        }
        throw std::bad_alloc();
    }

    void deallocate(T* p, std::size_t n) noexcept {
        PyMem_Del(p);
        ;
    }

    bool operator==(const PyMallocator<T>&) { return true; }

    bool operator!=(const PyMallocator<T>&) { return false; }

    //    private:
    //     void report(T* p, std::size_t n, bool alloc = true) const {
    //         std::cout << (alloc ? "Alloc: " : "Dealloc: ") << sizeof(T) * n << " bytes at " << std::hex <<
    //         std::showbase
    //                   << reinterpret_cast<void*>(p) << std::dec << '\n';
    //     }
};

template <typename CharT>
class pyview_t {
   public:
    using value_type = CharT;
    using size_type = std::size_t;

    PyObject* py = NULL;
    std::size_t kind = 0;
    CharT* data_;
    bool canonical = true;

   protected:
    std::size_t size_ = error_n;
    bool be_hash_clear = false;
    bool be_ref_clear = false;
    bool is_sequence = true;
    bool auto_close = true;

   public:
    pyview_t()
        : py(NULL),
          kind(0),
          data_(nullptr),
          size_(error_n),
          be_hash_clear(false),
          be_ref_clear(false),
          is_sequence(true),
          auto_close(true) {}
    pyview_t(std::nullptr_t)
        : py(NULL),
          kind(0),
          data_(nullptr),
          size_(error_n),
          be_hash_clear(false),
          be_ref_clear(false),
          is_sequence(true),
          auto_close(true) {}

    pyview_t(PyObject*& o, bool _auto_close = true) : py(o), auto_close(_auto_close) {
        size_ = PyAny_Length(o);
        open();
    }
    pyview_t(PyObject*& o, std::size_t len, bool _auto_close = true) : py(o), size_(len), auto_close(_auto_close) {
        open();
    }

    const void open() {
        PyObject* o = py;
        if(PyNumber_Check(py) || PyBool_Check(py) || py == Py_None) {
            size_ = 1;
            kind = 8;
            data_ = new CharT[1];
            if(data_ == NULL) {
                PyErr_NoMemory();
                return;
            }
            be_hash_clear = true;
            *data_ = (CharT)(PyBool_Check(py) ? (uint64_t)py : PyObject_Hash(py));
            is_sequence = false;
            return;
        } else {
            if(PyUnicode_Check(o)) {
#if PY_MAJOR_VERSION >= 3
                kind = PyUnicode_KIND(o);
                data_ = (CharT*)PyUnicode_DATA(o);
#else
                kind = 4;  // unicode
                data_ = (CharT*)PyUnicode_AsUnicode(py);
#endif
                return;
            }
            if(PyBytes_Check(o)) {
                kind = 1;  // byte
                data_ = (CharT*)PyBytes_AsString(py);
                return;
            }
            if(PyByteArray_Check(o)) {
                kind = 1;  // byte
                data_ = (CharT*)PyByteArray_AsString(py);
                return;
            }
        }
        kind = 8;
        if(size_ == 0)
            return;

        if(size_ == error_n || !PySequence_Check(py) || PyRange_Check(py)) {
            py = PySequence_Tuple(py);
            size_ = (std::size_t)PyObject_Length(py);
            be_ref_clear = true;
        }

        data_ = new CharT[size_];
        
        if(data_ == NULL) {
            PyErr_NoMemory();
            return;
        }

        CharT errval = CharT(-1);
        
        std::fill(data_, data_ + size_, errval);
        be_hash_clear = true;
        canonical = false;


        for(std::size_t i = 0; i < size_; i++) {
            PyObject* item = PySequence_ITEM(py, (Py_ssize_t)i);
            if(PyHashable_Check(item)) {
                data_[i] = (CharT)PyObject_Hash(item);
            } else {
                PyObject* tmp = PySequence_Tuple(item);
                if (tmp == NULL){
                    data_[i] = errval;
                    Py_DECREF(item);
                    continue;
                }
                data_[i] = (CharT)PyObject_Hash(tmp);
                if((PySequence_SetItem(py, (Py_ssize_t)i, tmp)) == -1) {
                    PyErr_Format(PyExc_ReferenceError, "Unknown panic, pyyou.hpp pyview_t class.");
                    return;
                }
                Py_DECREF(tmp);
            }
            Py_DECREF(item);

            //@todo nandoka crash shita
            if(data_[i] == errval) {
                PyErr_Format(PyExc_ReferenceError, "Cannot Hash data. Force Stop");
                return;
            }
        }
    }

    ~pyview_t() {
        if(auto_close && size_ != error_n)
            close();
    }

    void close() {
        if(be_ref_clear) {
            Py_CLEAR(py);
            be_ref_clear = false;
        }
        if(be_hash_clear && size_ != error_n) {
            if(*(data_ + size_ - 1)) {
                *(data_ + size_ - 1) = 0;
                delete[] data_;
            }
            be_hash_clear = false;
        }
        size_ = error_n;
    }

    inline constexpr std::size_t size() const noexcept { return size_; }
    inline constexpr std::size_t length() const noexcept { return size_; }

    constexpr PyObject* getitem(size_t index) const noexcept {
        if(size() == 0 || is_sequence == false) {
            Py_INCREF(py);
            return py;
        } else if(size() > 0 && index < size()) {
            return PySequence_GetItem(py, (Py_ssize_t)index);
        } else {
            return PyErr_Format(PyExc_IndexError, "Bad Index value.");
        }
    }

    constexpr CharT const* data() const noexcept { return data_; }
    constexpr CharT*& data() noexcept { return data_; }

    template <typename T>
    constexpr CharT const& operator[](T pos) const noexcept {
        return data_[pos];
    }
    template <typename T>
    CharT& operator[](T pos) noexcept {
        return data_[pos];
    }

    pyview_t(const pyview_t<CharT>& other) {
        this->kind = other.kind;
        this->is_sequence = other.is_sequence;
        this->canonical = other.canonical;
        this->size_ = other.size_;
        this->py = other.py;

        this->data_ = other.data_;
        if(this->size_ == 0)
            this->be_hash_clear = false;
    }

    pyview_t<CharT>& operator=(const pyview_t<CharT>& other) noexcept {
        if(this == &other)
            return *this;

        this->kind = other.kind;
        this->is_sequence = other.is_sequence;
        this->canonical = other.canonical;
        this->size_ = other.size_;
        // if (this->be_ref_clear)
        //     Py_DECREF(this->py);
        this->py = other.py;

        this->data_ = other.data_;
        if(this->size_ == 0)
            this->be_hash_clear = false;

        return *this;
    }

    bool operator==(PyObject*& rpy) { return (bool)PyObject_RichCompareBool(this->py, rpy, Py_EQ); }
    bool operator!=(PyObject*& rpy) { return (bool)PyObject_RichCompareBool(this->py, rpy, Py_NE); }
    constexpr bool operator==(const pyview_t<CharT>& rhs) const noexcept { return this->data() == rhs.data(); }
    constexpr bool operator!=(const pyview_t<CharT>& rhs) const noexcept { return this->data() != rhs.data(); }
    constexpr bool operator<(const pyview_t<CharT>& rhs) const noexcept { return this->data() < rhs.data(); }
    constexpr bool operator<=(const pyview_t<CharT>& rhs) const noexcept { return this->data() <= rhs.data(); }
    constexpr bool operator>(const pyview_t<CharT>& rhs) const noexcept { return this->data() > rhs.data(); }
    constexpr bool operator>=(const pyview_t<CharT>& rhs) const noexcept { return this->data() >= rhs.data(); }

    pyview_t<CharT>& operator++() {
        ++data_;
        return *this;
    }
    pyview_t<CharT>& operator++(int) {
        data_++;
        return *this;
    }

    pyview_t<CharT>& operator--() {
        --data_;
        return *this;
    }
    pyview_t<CharT>& operator--(int) {
        data_--;
        return *this;
    }

    constexpr CharT* begin() noexcept { return data_; }
    constexpr CharT* end() noexcept { return data_ + size_; }
    constexpr CharT const* cbegin() noexcept { return begin(); }
    constexpr CharT const* cend() noexcept { return end(); }

    std::reverse_iterator<CharT const*> rbegin() noexcept { return std::reverse_iterator<CharT const*>(end()); }
    std::reverse_iterator<CharT const*> rend() noexcept { return std::reverse_iterator<CharT const*>(begin()); }
    std::reverse_iterator<CharT const*> crbegin() noexcept { return rbegin(); }
    std::reverse_iterator<CharT const*> crend() noexcept { return rend(); }
};

class pyview {
   public:
    using value_type = uint64_t;
    using size_type = std::size_t;

    PyObject* py = NULL;
    std::size_t kind = 0;
    union {
        uint8_t* data_8;
        uint16_t* data_16;
        uint32_t* data_32;
        uint64_t* data_64 = nullptr;
    };
    bool canonical = true;

   protected:
    std::size_t size_ = error_n;
    bool be_hash_clear = false;
    bool be_ref_clear = false;
    bool is_sequence = true;
    bool auto_close = true;

   public:
    pyview()
        : py(NULL),
          kind(0),
          data_64(nullptr),
          size_(error_n),
          be_hash_clear(false),
          be_ref_clear(false),
          is_sequence(true),
          auto_close(true) {}
    pyview(std::nullptr_t)
        : py(NULL),
          kind(0),
          data_64(nullptr),
          size_(error_n),
          be_hash_clear(false),
          be_ref_clear(false),
          is_sequence(true),
          auto_close(true) {}

    pyview(PyObject*& o, bool _auto_close = true) : py(o), auto_close(_auto_close) {
        size_ = PyAny_Length(o);
        open();
    }
    pyview(PyObject*& o, std::size_t len, bool _auto_close = true) : py(o), size_(len), auto_close(_auto_close) {
        open();
    }

    const void open() {
        PyObject* o = py;
        if(PyNumber_Check(py) || PyBool_Check(py) || py == Py_None) {
            size_ = 1;
            kind = 8;
            data_64 = new uint64_t[1];
            if(data_64 == NULL) {
                PyErr_NoMemory();
                return;
            }
            *data_64 = (uint64_t)PyObject_Hash(py);
            *data_64 = PyBool_Check(py) ? (uint64_t)py : (uint64_t)PyObject_Hash(py);
            be_hash_clear = true;
            is_sequence = false;
            return;
        } else if(size_ != error_n) {
            if(PyUnicode_Check(o)) {
#if PY_MAJOR_VERSION >= 3
                kind = PyUnicode_KIND(o);
                data_32 = (uint32_t*)PyUnicode_DATA(o);
#else
                kind = 2;  // unicode
                data_16 = (uint16_t*)PyUnicode_AsUnicode(py);
#endif
                return;
            }
            if(PyBytes_Check(o)) {
                kind = 1;  // byte
                data_8 = (uint8_t*)PyBytes_AsString(py);
                return;
            }
            if(PyByteArray_Check(o)) {
                kind = 1;  // byte
                data_8 = (uint8_t*)PyByteArray_AsString(py);
                return;
            }
        }
        kind = 8;
        if(size_ == 0)
            return;

        if(size_ == error_n || !PySequence_Check(py) || PyRange_Check(py)) {
            py = PySequence_Tuple(py);
            size_ = (std::size_t)PyObject_Length(py);
            be_ref_clear = true;
        }

        data_64 = new uint64_t[size_];

        if(data_64 == NULL) {
            PyErr_NoMemory();
            return;
        }

        uint64_t errval = uint64_t(-1);

        std::fill(data_64, data_64 + size_, errval);
        be_hash_clear = true;
        canonical = false;

        for(std::size_t i = 0; i < size_; i++) {
            PyObject* item = PySequence_ITEM(py, (Py_ssize_t)i);
            if(PyHashable_Check(item)) {
                data_64[i] = (uint64_t)PyObject_Hash(item);
            } else {
                PyObject* tmp = PySequence_Tuple(item);
                if (tmp == NULL){
                    data_64[i] = errval;
                    Py_DECREF(item);
                    continue;
                }

                data_64[i] = (uint64_t)PyObject_Hash(tmp);
                if((PySequence_SetItem(py, (Py_ssize_t)i, tmp)) == -1) {
                    PyErr_Format(PyExc_ReferenceError, "Unknown panic, pyyou.hpp pyview_t class.");
                    return;
                }
                Py_DECREF(tmp);
            }
            Py_DECREF(item);
            //@todo nandoka crash shita
            if(data_64[i] == errval) {
                PyErr_Format(PyExc_ReferenceError, "Cannot Hash data. Force Stop");
                return;
            }
        }
    }

    ~pyview() {
        if(auto_close && size_ != error_n)
            close();
    }
    void close() {
        if(be_ref_clear) {
            Py_CLEAR(py);
            be_ref_clear = false;
        }
        if(be_hash_clear && size_ != error_n) {
            if(kind == 8 && *(data_64 + size_ - 1)) {
                *(data_64 + size_ - 1) = 0;
                delete[] data_64;
            } else if(kind == 4 && *(data_32 + size_ - 1)) {
                *(data_32 + size_ - 1) = 0;
                delete[] data_32;
            } else if(kind == 2 && *(data_16 + size_ - 1)) {
                *(data_16 + size_ - 1) = 0;
                delete[] data_16;
            } else if(kind == 1 && *(data_8 + size_ - 1)) {
                *(data_8 + size_ - 1) = 0;
                delete[] data_8;
            }
            be_hash_clear = false;
        }
        size_ = error_n;
    }

    inline constexpr std::size_t size() const noexcept { return size_; }
    inline constexpr std::size_t length() const noexcept { return size_; }

    constexpr PyObject* getitem(size_t index) const noexcept {
        if(size() == 0 || is_sequence == false) {
            return py;
        } else if(size() > 0 && index < size()) {
            return PySequence_GetItem(py, (Py_ssize_t)index);
        } else {
            return PyErr_Format(PyExc_IndexError, "Bad Index value.");
        }
    }
    constexpr uint64_t const* data() const noexcept { return data_64; }
    constexpr uint64_t*& data() noexcept { return data_64; }

    template <typename T>
    constexpr uint64_t const operator[](T pos) const noexcept {
        return (kind == 1 ? data_8[pos] : kind == 2 ? data_16[pos] : kind == 8 ? data_64[pos] : data_32[pos]);
    }
    template <typename T>
    uint64_t operator[](T pos) noexcept {
        return (kind == 1 ? data_8[pos] : kind == 2 ? data_16[pos] : kind == 8 ? data_64[pos] : data_32[pos]);
    }

    pyview(const pyview& other) {
        this->kind = other.kind;
        this->is_sequence = other.is_sequence;
        this->canonical = other.canonical;
        this->size_ = other.size_;
        this->py = other.py;
        if(kind == 1) {
            this->data_8 = other.data_8;
        } else if(kind == 2) {
            this->data_16 = other.data_16;
        } else if(kind == 4) {
            this->data_32 = other.data_32;
        } else if(kind == 8) {
            this->data_64 = other.data_64;
        }
        if(this->size_ == 0)
            this->be_hash_clear = false;
    }

    pyview& operator=(const pyview& other) noexcept {
        if(this == &other)
            return *this;

        this->kind = other.kind;
        this->is_sequence = other.is_sequence;
        this->canonical = other.canonical;
        this->size_ = other.size_;
        // if (this->be_ref_clear)
        //     Py_DECREF(this->py);
        this->py = other.py;

        if(kind == 1) {
            this->data_8 = other.data_8;
        } else if(kind == 2) {
            this->data_16 = other.data_16;
        } else if(kind == 4) {
            this->data_32 = other.data_32;
        } else if(kind == 8) {
            this->data_64 = other.data_64;
        }

        if(this->size_ == 0)
            this->be_hash_clear = false;

        return *this;
    }

    pyview& operator++() {
        if(kind == 1)
            ++data_8;
        else if(kind == 2)
            ++data_16;
        else if(kind == 8)
            ++data_64;
        else
            ++data_32;
        return *this;
    }
    pyview& operator++(int) {
        if(kind == 1)
            data_8++;
        else if(kind == 2)
            data_16++;
        else if(kind == 8)
            data_64++;
        else
            data_32++;
        return *this;
    }

    pyview& operator--() {
        if(kind == 1)
            --data_8;
        else if(kind == 2)
            --data_16;
        else if(kind == 8)
            --data_64;
        else
            --data_32;
        return *this;
    }
    pyview& operator--(int) {
        if(kind == 1)
            data_8--;
        else if(kind == 2)
            data_16--;
        else if(kind == 8)
            data_64--;
        else
            data_32--;
        return *this;
    }

    bool operator==(PyObject*& rpy) { return (bool)PyObject_RichCompareBool(this->py, rpy, Py_EQ); }
    bool operator!=(PyObject*& rpy) { return (bool)PyObject_RichCompareBool(this->py, rpy, Py_NE); }
    constexpr bool operator==(const pyview& rhs) const noexcept { return this->data_64 == rhs.data_64; }
    constexpr bool operator!=(const pyview& rhs) const noexcept { return this->data_64 != rhs.data_64; }
    constexpr bool operator<(const pyview& rhs) const noexcept {
        return (kind == 1   ? this->data_8 < rhs.data_8
                : kind == 2 ? this->data_16 < rhs.data_16
                : kind == 8 ? this->data_64 < rhs.data_64
                            : this->data_32 < rhs.data_32);
    }
    constexpr bool operator<=(const pyview& rhs) const noexcept {
        return (kind == 1   ? this->data_8 <= rhs.data_8
                : kind == 2 ? this->data_16 <= rhs.data_16
                : kind == 8 ? this->data_64 <= rhs.data_64
                            : this->data_32 <= rhs.data_32);
    }
    constexpr bool operator>(const pyview& rhs) const noexcept {
        return (kind == 1   ? this->data_8 > rhs.data_8
                : kind == 2 ? this->data_16 > rhs.data_16
                : kind == 8 ? this->data_64 > rhs.data_64
                            : this->data_32 > rhs.data_32);
    }
    constexpr bool operator>=(const pyview& rhs) const noexcept {
        return (kind == 1   ? this->data_8 >= rhs.data_8
                : kind == 2 ? this->data_16 >= rhs.data_16
                : kind == 8 ? this->data_64 >= rhs.data_64
                            : this->data_32 >= rhs.data_32);
    }

    constexpr uint64_t* begin() noexcept { return data_64; }
    constexpr uint64_t* end() noexcept {
        return (kind == 1   ? (uint64_t*)(data_8 + size_)
                : kind == 2 ? (uint64_t*)(data_16 + size_)
                : kind == 8 ? (data_64 + size_)
                            : (uint64_t*)(data_32 + size_));
    }
    constexpr uint64_t const* cbegin() noexcept { return begin(); }
    constexpr uint64_t const* cend() noexcept { return end(); }

    std::reverse_iterator<uint64_t const*> rbegin() noexcept { return std::reverse_iterator<uint64_t const*>(end()); }
    std::reverse_iterator<uint64_t const*> rend() noexcept { return std::reverse_iterator<uint64_t const*>(begin()); }
    std::reverse_iterator<uint64_t const*> crbegin() noexcept { return rbegin(); }
    std::reverse_iterator<uint64_t const*> crend() noexcept { return rend(); }
};

#endif /* !defined(PYYOU_H) */
