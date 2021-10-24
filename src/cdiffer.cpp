#define PY_SSIZE_T_CLEAN
#include "cdiffer.hpp"
// #include <Python.h>

PyObject* gammy::DIFFTP[2][ED_LAST] = {{
                                           PyUnicode_FromString("equal"),    // 0: EQUAL
                                           PyUnicode_FromString("replace"),  // 1: REPLACE
                                           PyUnicode_FromString("insert"),   // 2: INSERT
                                           PyUnicode_FromString("delete")    // 3: DELETE
                                       },
                                       {
                                           PyUnicode_FromString("equal"),    // 0: EQUAL
                                           PyUnicode_FromString("replace"),  // 1: REPLACE
                                           PyUnicode_FromString("delete"),   // 2: DELETE
                                           PyUnicode_FromString("insert")    // 3: INSERT
                                       }};

/*
 * python Interface function
 */
PyObject* dist_py(PyObject* self, PyObject* args) {
    PyObject *arg1, *arg2;
    size_t ldist;

    if(!PyArg_UnpackTuple(args, (char*)("dist"), 2, 2, &arg1, &arg2))
        return NULL;

    if(PyObject_RichCompareBool(arg1, arg2, Py_EQ))
        return PyLong_FromUnsignedLong(0);
    if(PyAny_Length(arg1) == 1 && PyAny_Length(arg2) == 1)
        return PyLong_FromSize_t(2ULL);

    ldist = gammy::Diff(arg1, arg2).distance();

    if(ldist == error_n)
        return NULL;
    return PyLong_FromSize_t(ldist);
}

PyObject* similar_py(PyObject* self, PyObject* args) {
    PyObject *arg1, *arg2;
    double lsim;

    if(!PyArg_UnpackTuple(args, (char*)("similar"), 2, 2, &arg1, &arg2))
        return NULL;

    if(PyObject_RichCompareBool(arg1, arg2, Py_EQ))
        return PyFloat_FromDouble(1.0);

    lsim = gammy::Diff(arg1, arg2).similar();

    return PyFloat_FromDouble(lsim);
}

PyObject* differ_py(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject *arg1, *arg2;
    int diffonly = false;
    int rep_rate = REPLACEMENT_RATE;

    const char* kwlist[5] = {"a", "b", "diffonly", "rep_rate", NULL};

    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "OO|ii", (char**)kwlist, &arg1, &arg2, &diffonly, &rep_rate))
        return NULL;

    return gammy::Diff(arg1, arg2).difference((bool)diffonly, rep_rate);
}

PyObject* compare_py(PyObject* self, PyObject* args, PyObject* kwargs) {
    PyObject *a, *b;

    if(!PyArg_UnpackTuple(args, (char*)("compare"), 2, 2, &a, &b))
        return NULL;

    if((a == Py_None || b == Py_None) ||
       ((PyUnicode_Check(a) || PyBool_Check(a) || PyNumber_Check(a) || PyBytes_Check(a) || PyByteArray_Check(a)) &&
        (PyUnicode_Check(b) || PyBool_Check(b) || PyNumber_Check(b) || PyBytes_Check(b) || PyByteArray_Check(b)))) {
        return gammy::Compare(args, kwargs)._1d();

    } else if(PyDict_Check(a) && PyDict_Check(b)) {
        return gammy::Compare(args, kwargs)._3d();

    } else {
        return gammy::Compare(args, kwargs)._2d();
    }
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
    "4\n"                                                     \
    ">>> dist(list('coffee'), list('cafe'))\n"                \
    "4\n"                                                     \
    ">>> dist(tuple('coffee'), tuple('cafe'))\n"              \
    "4\n"                                                     \
    ">>> dist(iter('coffee'), iter('cafe'))\n"                \
    "4\n"                                                     \
    ">>> dist(range(4), range(5))\n"                          \
    "1\n"                                                     \
    ">>> dist('coffee', 'xxxxxx')\n"                          \
    "12\n"                                                    \
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
    "0.6\n"                                                               \
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
    "['delete',  1, None,'o',None]\n"                                         \
    "['insert',  None, 1,None,'a']\n"                                         \
    "['equal',   2, 2,   'f', 'f']\n"                                         \
    "['delete',  3, None,'f',None]\n"                                         \
    "['delete',  4, None,'e',None]\n"                                         \
    "['equal',   5, 3,   'e', 'e']\n"                                         \
    ">>> for x in differ('coffee', 'cafe', diffonly=True):\n"                 \
    "...     print(x)\n"                                                      \
    "...\n"                                                                   \
    "['delete',  1, None,'o',None]\n"                                         \
    "['insert',  None, 1,None,'a']\n"                                         \
    "['delete',  3, None,'f',None]\n"                                         \
    "['delete',  4, None,'e',None]\n"                                         \
    "\n"                                                                      \
    ">>> for x in differ('coffee', 'cafe', rep_rate = 0):\n"                  \
    "...     print(x)\n"                                                      \
    "...\n"                                                                   \
    "['equal',   0, 0,   'c', 'c']\n"                                         \
    "['replace', 1, 1,   'o', 'a']\n"                                         \
    "['equal',   2, 2,   'f', 'f']\n"                                         \
    "['delete',  3, None,'f',None]\n"                                         \
    "['delete',  4, None,'e',None]\n"                                         \
    "['equal',   5, 3,   'e', 'e']\n"                                         \
    ">>> for x in differ('coffee', 'cafe', diffonly=True, rep_rate = 0):\n"   \
    "...     print(x)\n"                                                      \
    "...\n"                                                                   \
    "['replace', 1, 1,   'o', 'a']\n"                                         \
    "['delete',  3, None,'f',None]\n"                                         \
    "['delete',  4, None,'e',None]\n"                                         \
    "\n"

#define compare_DESC                                                                                           \
    "This Function is compare and prety printing 2 sequence data.\n"                                           \
    "\n"                                                                                                       \
    "# Parameters :\n"                                                                                         \
    "    arg1 -> iterable : left comare target data.\n"                                                             \
    "    arg2 -> iterable : right comare target data.\n"                                                            \
    "    keya -> callable one argument function : Using sort and compare with key about `a` object.\n"         \
    "    keyb -> callable one argument function : Using sort and compare with key about `a` object.\n"         \
    "    header -> bool : output data with header(True) or without header(False). <default True>\n"            \
    "    diffonly -> bool : output data with equal data(False) or without equal data(True). <default False>\n" \
    "    rep_rate -> int: Threshold to be considered as replacement.(-1 ~ 100). -1: allways replacement.\n"    \
    "    startidx -> int: output record index starting number. <default `0`>\n"                                \
    "    condition_value -> str : Conjunctions for comparison.\n"                                              \
    "    na_value -> str: if not found data when filled value.\n"                                              \
    "    delete_sign_value -> str: if deleted data when adding sign value.\n"                                  \
    "    insert_sign_value ->  str: if insert data when adding sign value.\n"                                  \
    "\n"                                                                                                       \
    "# Return : Lists of List\n"                                                                               \
    "    1st column -> matching rate (0 ~ 100).\n"                                                             \
    "    2nd column -> matching tagname (unicode string).\n"                                                   \
    "    3rd over   -> compare data.\n"                                                                        \
    "\n"                                                                                                       \
    "# Example\n"                                                                                              \
    "    >>> from cdiffer import compare\n"                                                                    \
    "    ... compare('coffee', 'cafe')\n"                                                                      \
    "    [[60, 'insert', 'c', 'a', 'f', 'e'],\n"                                                               \
    "     [60, 'delete', 'c', 'o', 'f', 'f', 'e', 'e']]\n"                                                     \
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
static PyMethodDef py_methods[] = {PY_ADD_METHOD("dist", dist_py, dist_DESC),
                                   PY_ADD_METHOD("similar", similar_py, similar_DESC),
                                   PY_ADD_METHOD_KWARGS("differ", differ_py, differ_DESC),
                                   PY_ADD_METHOD_KWARGS("compare", compare_py, compare_DESC),

                                   {NULL, NULL, 0, NULL}};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef py_defmod = {PyModuleDef_HEAD_INIT, MODULE_NAME_S, MODULE_DOCS, 0, py_methods};
#define PARSE_NAME(mn) PyInit_##mn
#define PARSE_FUNC(mn) \
    PyMODINIT_FUNC PARSE_NAME(mn)() { return PyModule_Create(&py_defmod); }

#else
#define PARSE_NAME(mn) \
    init##mn(void) { (void)Py_InitModule3(MODULE_NAME_S, py_methods, MODULE_DOCS); }
#define PARSE_FUNC(mn) PyMODINIT_FUNC PARSE_NAME(mn)
#endif

PARSE_FUNC(MODULE_NAME)
