#pragma once
#ifndef PYYOU_H
#define PYYOU_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

std::size_t error_n = (std::size_t)(-1);

#define PyNotHashable_Check(op)                                                                        \
    PyList_Check(op) || PyObject_TypeCheck(op, &PyDict_Type) || PyIter_Check(op) || PyGen_Check(op) || \
        PyObject_TypeCheck(op, &PyDictItems_Type) || PyObject_TypeCheck(op, &PyDictKeys_Type) ||       \
        PyObject_TypeCheck(op, &PyDictValues_Type) || PyObject_TypeCheck(op, &PySet_Type)
#define PyHashable_Check(op)                                                                                        \
    PyUnicode_Check(op) || PyTuple_Check(op) || PyNumber_Check(op) || PyBytes_Check(op) || PyByteArray_Check(op) || \
        PyBool_Check(op) || op == Py_None

constexpr inline std::size_t PyAny_KIND(PyObject*& o) {
    if(PyUnicode_Check(o)) {
#if PY_MAJOR_VERSION >= 3
        return PyUnicode_KIND(o);

#else
        return 2;

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
    if(PyMapping_Check(o)) {
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

template <typename CharT>
class pyview_t {
   public:
    using value_type = typename CharT;
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
    pyview_t(nullptr_t)
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
        if(size_ == error_n) {
            kind = 8;
            be_hash_clear = true;
            if(PyNumber_Check(py) || PyBool_Check(py) || py == Py_None) {
                size_ = 1;
                data_ = new CharT[1];
                *data_ = (CharT)(PyBool_Check(py) ? (Py_hash_t)py : PyObject_Hash(py));
                is_sequence = false;
                return;
            }
        } else {
            if PyUnicode_Check(o) {
#if PY_MAJOR_VERSION >= 3
                kind = PyUnicode_KIND(o);
                data_ = (CharT*)PyUnicode_DATA(o);
#else
                kind = 2;  // unicode
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

        if(!PySequence_Check(py) || PyRange_Check(py)) {
            py = PySequence_Tuple(py);
            size_ = (std::size_t)PyObject_Length(py);
            be_ref_clear = true;
        }

        be_hash_clear = true;
        canonical = false;
        data_ = new CharT[size_];
        for(std::size_t i = 0; i < size_; i++) {
            PyObject* item = PySequence_ITEM(py, (Py_ssize_t)i);
            if(PyHashable_Check(item)) {
                data_[i] = (CharT)PyObject_Hash(item);
            }else{
                PyObject* tmp = PySequence_Tuple(item);
                data_[i] = (CharT)PyObject_Hash(tmp);
                Py_DECREF(tmp);
            }
            Py_DECREF(item);
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
                *(data_ + size_ - 1) = NULL;
                delete[] data_;
            }
            be_hash_clear = false;
        }
        size_ = error_n;
    }

    inline constexpr std::size_t size() const noexcept { return size_; }
    inline constexpr std::size_t length() const noexcept { return size_; }

    constexpr PyObject* getitem(size_t index) const noexcept {
        return (size() == 0 || is_sequence == false) ? Py_INCREF(py), py
           : (size() > 0 && index < size())          ? PySequence_GetItem(py, (Py_ssize_t)index)
                                                     : NULL;
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
    pyview(nullptr_t)
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
        if(size_ == error_n) {
            kind = 8;
            be_hash_clear = true;
            if(PyNumber_Check(py) || PyBool_Check(py) || py == Py_None) {
                size_ = 1;
                data_64 = new uint64_t[1];
                *data_64 = (uint64_t)PyObject_Hash(py);
                *data_64 = PyBool_Check(py) ? (uint64_t)py : (uint64_t)PyObject_Hash(py);
                is_sequence = false;
                return;
            }
        } else {
            if PyUnicode_Check(o) {
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

        if(!PySequence_Check(py) || PyRange_Check(py)) {
            py = PySequence_Tuple(py);
            size_ = (std::size_t)PyObject_Length(py);
            be_ref_clear = true;
        }

        be_hash_clear = true;
        canonical = false;
        data_64 = new uint64_t[size_];
        for(std::size_t i = 0; i < size_; i++) {
            PyObject* item = PySequence_ITEM(py, (Py_ssize_t)i);
            if(PyHashable_Check(item)) {
                data_64[i] = (uint64_t)PyObject_Hash(item);
            }else{
                PyObject* tmp = PySequence_Tuple(item);
                data_64[i] = (uint64_t)PyObject_Hash(tmp);
                Py_DECREF(tmp);
            }
            Py_DECREF(item);
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
                *(data_64 + size_ - 1) = NULL;
                delete[] data_64;
            } else if(kind == 4 && *(data_32 + size_ - 1)) {
                *(data_32 + size_ - 1) = NULL;
                delete[] data_32;
            } else if(kind == 2 && *(data_16 + size_ - 1)) {
                *(data_16 + size_ - 1) = NULL;
                delete[] data_16;
            } else if(kind == 1 && *(data_8 + size_ - 1)) {
                *(data_8 + size_ - 1) = NULL;
                delete[] data_8;
            }
            be_hash_clear = false;
        }
        size_ = error_n;
    }

    inline constexpr std::size_t size() const noexcept { return size_; }
    inline constexpr std::size_t length() const noexcept { return size_; }

    constexpr PyObject* getitem(size_t index) const noexcept {
        return (size() == 0 || is_sequence == false) ? py
               : (size() > 0 && index < size())      ? PySequence_GetItem(py, (Py_ssize_t)index)
                                                     : NULL;
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

/* Python Interface Method Example */
// #define MODULE_NAME xxxx
// #define MODULE_NAME_S "xxxx"

// // this module description
// #define MODULE_DOCS "xxxx\n"

// #define meth_DESC \
//     "xxxx"        \
//     "\n"

// static PyObject* mech_py(CustomObject* self, PyObject* Py_UNUSED(ignored)) {
//     return PyUnicode_FromString("xxxx");
// }

// #define PY_ADD_METHOD(py_func, c_func, desc) \
//     { py_func, (PyCFunction)c_func, METH_VARARGS, desc }
// #define PY_ADD_METHOD_KWARGS(py_func, c_func, desc) \
//     { py_func, (PyCFunction)c_func, METH_VARARGS | METH_KEYWORDS, desc }

// /* Please extern method define for python */
// /* PyMethodDef Parameter Help
//  * https://docs.python.org/ja/3/c-api/structures.html#c.PyMethodDef
//  */
// static PyMethodDef py_methods[] = {PY_ADD_METHOD("meth", meth_py, meth_DESC), {NULL, NULL, 0, NULL}};

// #if PY_MAJOR_VERSION >= 3
// static struct PyModuleDef py_defmod = {PyModuleDef_HEAD_INIT, MODULE_NAME_S, MODULE_DOCS, 0, py_methods};
// #define PARSE_NAME(mn) PyInit_##mn
// #define PARSE_FUNC(mn) \
//     PyMODINIT_FUNC PARSE_NAME(mn)() { return PyModule_Create(&py_defmod); }

// #else
// #define PARSE_NAME(mn) \
//     init##mn(void) { (void)Py_InitModule3(MODULE_NAME_S, py_methods, MODULE_DOCS); }
// #define PARSE_FUNC(mn) PyMODINIT_FUNC PARSE_NAME(mn)
// #endif

// PARSE_FUNC(MODULE_NAME)

/* Class Example */
// struct CustomObject {
//     PyObject_HEAD PyObject* first; /* first name */
//     PyObject* last;                /* last name */
//     int number;
// };

// static int Custom_traverse(CustomObject* self, visitproc visit, void* arg) {
//     Py_VISIT(self->first);
//     Py_VISIT(self->last);
//     return 0;
// }

// static int Custom_clear(CustomObject* self) {
//     Py_CLEAR(self->first);
//     Py_CLEAR(self->last);
//     return 0;
// }

// static void Custom_dealloc(CustomObject* self) {
//     PyObject_GC_UnTrack(self);
//     Custom_clear(self);
//     Py_TYPE(self)->tp_free((PyObject*)self);
// }

// static PyObject* Custom_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
//     CustomObject* self;
//     if((self = (CustomObject*)type->tp_alloc(type, 0)) != NULL) {
//         self->first = PyUnicode_FromString("");
//         if(self->first == NULL) {
//             Py_DECREF(self);
//             return NULL;
//         }
//         self->last = PyUnicode_FromString("");
//         if(self->last == NULL) {
//             Py_DECREF(self);
//             return NULL;
//         }
//         self->number = 0;
//     }
//     return (PyObject*)self;
// }

// static int Custom_init(CustomObject* self, PyObject* args, PyObject* kwds) {
//     static char* kwlist[] = {"first", "last", "number", NULL};
//     PyObject *first = NULL, *last = NULL, *tmp;

//     if(!PyArg_ParseTupleAndKeywords(args, kwds, "|UUi", kwlist, &first, &last, &self->number))
//         return -1;

//     if(first) {
//         tmp = self->first;
//         Py_INCREF(first);
//         self->first = first;
//         Py_DECREF(tmp);
//     }
//     if(last) {
//         tmp = self->last;
//         Py_INCREF(last);
//         self->last = last;
//         Py_DECREF(tmp);
//     }
//     return 0;
// }

// static PyObject* Custom_name(CustomObject* self, PyObject* Py_UNUSED(ignored)) {
//     return PyUnicode_FromFormat("%S %S", self->first, self->last);
// }

// PyMODINIT_FUNC PyInit_custom4(void) { /* name needs modulename match */
//     static PyModuleDef custommodule = {PyModuleDef_HEAD_INIT};
//     static PyTypeObject CustomType = {PyVarObject_HEAD_INIT(NULL, 0)};
//     PyObject* m;

//     /* /////////////////////////////////////  Please Setting  //////////////////////////////////////////////
//      * [Set Me 1] this module infomation
//      */
//     custommodule.m_name = "custom4";
//     custommodule.m_doc = "Example module that creates an extension type.";

//     /* [Set Me 2] this class, infomation */
//     const char* class_object_name = "Custom"; /* ClassName for python */
//     CustomType.tp_name = "custom4.Custom";    /* module fullpath */
//     CustomType.tp_doc = "Custom objects";     /* display document into python */

//     /* [Set Me 3] this class attributes, infomation */
//     static PyMemberDef Custom_members[] = {
//         /* reference. https://docs.python.org/ja/3/c-api/structures.html#c.PyMemberDef */
//         {"number", T_INT, offsetof(CustomObject, number), 0, "custom number"},
//         {NULL} /* Sentinel */
//     };

//     /* [Set Me 4] this methods infomation */
//     static PyMethodDef Custom_methods[] = {
//         {"name", (PyCFunction)Custom_name, METH_NOARGS, "Return the name, combining the first and last name"},
//         {NULL} /* Sentinel */
//     };

//     /* ///////////////////////////////////////////////////////////////////////////////////////////////////  */

//     CustomType.tp_basicsize = sizeof(CustomObject);
//     CustomType.tp_itemsize = 0;
//     CustomType.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC;
//     CustomType.tp_new = Custom_new;
//     CustomType.tp_init = (initproc)Custom_init;
//     CustomType.tp_dealloc = (destructor)Custom_dealloc;
//     CustomType.tp_traverse = (traverseproc)Custom_traverse;
//     CustomType.tp_clear = (inquiry)Custom_clear;
//     CustomType.tp_members = Custom_members;
//     CustomType.tp_methods = Custom_methods;

//     if(PyType_Ready(&CustomType) < 0)
//         return NULL;

//     custommodule.m_size = -1;
//     m = PyModule_Create(&custommodule);
//     if(m == NULL)
//         return NULL;

//     Py_INCREF(&CustomType);
//     if(PyModule_AddObject(m, class_object_name, (PyObject*)&CustomType) < 0) {
//         Py_DECREF(&CustomType);
//         Py_DECREF(m);
//         return NULL;
//     }
//     return m;
// }

#endif /* !defined(PYYOU_H) */
