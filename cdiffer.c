/*
 * ver.0.1 2021/03/03 kirin Exp.
 * Python extension computing Levenshtein distances, string similarities,
 *
 * Copyright (C) 2021 kirin123kirin <kirin123kirin@gmail.com>
 * 
 * python-Levenshtein:
 * Copyright (C) 2002-2003 David Necas (Yeti) <yeti@physics.muni.cz>.
 *
 * The Taus113 random generator:
 * Copyright (C) 2002 Atakan Gurkan
 * Copyright (C) 1996, 1997, 1998, 1999, 2000 James Theiler, Brian Gough
 * (see below for more)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 */

#ifdef NO_PYTHON
#define _GNU_SOURCE
#include <string.h>
#include <math.h>
  /* for debugging */
#include <stdio.h>
#else /* NO_PYTHON */
#define _LEV_STATIC_PY static
#define lev_wchar Py_UNICODE
#include <Python.h>
#endif /* NO_PYTHON */

#if PY_MAJOR_VERSION >= 3
#define LEV_PYTHON3
#define PyString_Type PyBytes_Type
#define PyString_GET_SIZE PyBytes_GET_SIZE
#define PyString_AS_STRING PyBytes_AS_STRING
#define PyString_Check PyBytes_Check
#define PyString_FromStringAndSize PyBytes_FromStringAndSize
#define PyString_InternFromString PyUnicode_InternFromString
#define PyInt_AS_LONG PyLong_AsLong
#define PyInt_FromLong PyLong_FromLong
#define PyInt_Check PyLong_Check
#define PY_INIT_MOD(module, name, doc, methods) \
        static struct PyModuleDef moduledef = { \
            PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
        module = PyModule_Create(&moduledef);
#define PY_MOD_INIT_FUNC_DEF(name) PyObject* PyInit_##name(void)
#else
#define PY_INIT_MOD(module, name, doc, methods) \
            Py_InitModule3(name, methods, doc);
#define PY_MOD_INIT_FUNC_DEF(name) void init##name(void)
#endif /* PY_MAJOR_VERSION */

#include <assert.h>
#include "cdiffer.h"

  /* FIXME: inline avaliability should be solved in setup.py, somehow, or
   * even better in Python.h, like const is...
   * this should inline at least with gcc and msvc */
#ifndef __GNUC__
#  ifdef _MSC_VER
#    define inline __inline
#  else
#    define inline
#  endif
#  define __attribute__(x)
#endif

#define LEV_EPSILON 1e-14
#define LEV_INFINITY 1e100

   /* Me thinks the second argument of PyArg_UnpackTuple() should be const.
	* Anyway I habitually pass a constant string.
	* A cast to placate the compiler. */
#define PYARGCFIX(x) (char*)(x)

	/* local functions */

#define TAUS113_LCG(n) ((69069UL * n) & 0xffffffffUL)
#define TAUS113_MASK 0xffffffffUL

typedef struct {
	unsigned long int z1, z2, z3, z4;
} taus113_state_t;

static inline unsigned long int
taus113_get(taus113_state_t* state);

static void
taus113_set(taus113_state_t* state,
	unsigned long int s);

#ifndef NO_PYTHON
/* python interface and wrappers */
/* declarations and docstrings */
static PyObject* distance_py(PyObject* self, PyObject* args);
static PyObject* opcodes_py(PyObject* self, PyObject* args);
static PyObject* ratio_py(PyObject* self, PyObject* args);

#define Levenshtein_DESC \
  "A C extension module for fast computation of:\n" \
  "- Levenshtein (edit) distance and edit sequence manipulation\n" \
  "- string similarity\n" \
  "- approximate median strings, and generally string averaging\n" \
  "- string sequence and set similarity\n" \
  "\n" \
  "Levenshtein has a some overlap with difflib (SequenceMatcher).  It\n" \
  "supports only strings, not arbitrary sequence types, but on the\n" \
  "other hand it's much faster.\n" \
  "\n" \
  "It supports both normal and Unicode strings, but can't mix them, all\n" \
  "arguments to a function (method) have to be of the same type (or its\n" \
  "subclasses).\n"

#define distance_DESC \
  "Compute absolute Levenshtein distance of two strings.\n" \
  "\n" \
  "distance(string1, string2)\n" \
  "\n" \
  "Examples (it's hard to spell Levenshtein correctly):\n" \
  "\n" \
  ">>> distance('Levenshtein', 'Lenvinsten')\n" \
  "4\n" \
  ">>> distance('Levenshtein', 'Levensthein')\n" \
  "2\n" \
  ">>> distance('Levenshtein', 'Levenshten')\n" \
  "1\n" \
  ">>> distance('Levenshtein', 'Levenshtein')\n" \
  "0\n" \
  "\n" \
  "Yeah, we've managed it at last.\n"

#define ratio_DESC \
  "Compute similarity of two strings.\n" \
  "\n" \
  "ratio(string1, string2)\n" \
  "\n" \
  "The similarity is a number between 0 and 1, it's usually equal or\n" \
  "somewhat higher than difflib.SequenceMatcher.ratio(), because it's\n" \
  "based on real minimal edit distance.\n" \
  "\n" \
  "Examples:\n" \
  "\n" \
  ">>> ratio('Hello world!', 'Holly grail!')  # doctest: +ELLIPSIS\n" \
  "0.583333...\n" \
  ">>> ratio('Brian', 'Jesus')\n" \
  "0.0\n" \
  "\n" \
  "Really?  I thought there was some similarity.\n"

#define opcodes_DESC \
  "Find sequence of edit operations transforming one string to another.\n" \
  "\n" \
  "opcodes(source_string, destination_string, diffonly=False)\n" \
  "\n" \
  "The result is a list of 5-tuples with the same meaning as in\n" \
  "SequenceMatcher's get_opcodes() output.  But since the algorithms\n" \
  "differ, the actual sequences from Levenshtein and SequenceMatcher\n" \
  "may differ too.\n" \
  "\n" \
  "Examples:\n" \
  "\n" \
  ">>> for x in opcodes('spam', 'park'):\n" \
  "...     print(x)\n" \
  "...\n" \
  "('delete', 0, 1, 0, 0)\n" \
  "('equal', 1, 3, 0, 2)\n" \
  "('insert', 3, 3, 2, 3)\n" \
  "('replace', 3, 4, 3, 4)\n" \
  "\n" \


#define METHODS_ITEM(x) { #x, x##_py, METH_VARARGS, x##_DESC }
static PyMethodDef methods[] = {
  METHODS_ITEM(distance),
  METHODS_ITEM(ratio),
  METHODS_ITEM(opcodes),
  { NULL, NULL, 0, NULL },
};

/* opcode names, these are to be initialized in the init func,
 * indexed by LevEditType values */
struct {
	PyObject* pystring;
	const char* cstring;
	size_t len;
}
static opcode_names[] = {
  { NULL, "equal", 0 },
  { NULL, "replace", 0 },
  { NULL, "insert", 0 },
  { NULL, "delete", 0 },
};
#define N_OPCODE_NAMES ((sizeof(opcode_names)/sizeof(opcode_names[0])))


static long int
levenshtein_common(PyObject* args,
	const char* name,
	size_t xcost,
	size_t* lensum);


static void*
safe_malloc(size_t nmemb, size_t size) {
	/* extra-conservative overflow check */
	if (SIZE_MAX / size <= nmemb) {
		return NULL;
	}
	return malloc(nmemb * size);
}

static void*
safe_malloc_3(size_t nmemb1, size_t nmemb2, size_t size) {
	/* extra-conservative overflow check */
	if (SIZE_MAX / size <= nmemb2) {
		return NULL;
	}
	return safe_malloc(nmemb1, nmemb2 * size);
}

static size_t
get_length_of_anything(PyObject* object)
{
	if (PyInt_Check(object)) {
		long int len = PyInt_AS_LONG(object);
		if (len < 0)
			len = -1;
		return (size_t)len;
	}
	if (PySequence_Check(object))
		return PySequence_Length(object);
	return (size_t)-1;
}

/****************************************************************************
 *
 * Python interface and subroutines
 *
 ****************************************************************************/

static long int
levenshtein_common(PyObject* args, const char* name, size_t xcost,
	size_t* lensum)
{
	PyObject* arg1, * arg2;
	size_t len1, len2;

	if (!PyArg_UnpackTuple(args, PYARGCFIX(name), 2, 2, &arg1, &arg2))
		return -1;

	if (PyObject_RichCompareBool(arg1, arg2, Py_EQ)) {
		*lensum = 0.00000000001;
		return 0;
	}

	if (PyNumber_Check(arg1) && PyNumber_Check(arg2)) {
		*lensum = 2;
		return 2;
	}

	size_t is_iter = 0;
	if (PyGen_Check(arg1) || PyIter_Check(arg1) || PySet_Check(arg1) || PyDict_Check(arg1)) {
		arg1 = PySequence_List(arg1);
		is_iter++;
	}


	if (PyGen_Check(arg2) || PyIter_Check(arg2) || PySet_Check(arg2) || PyDict_Check(arg2)) {
		arg2 = PySequence_List(arg2);
		is_iter++;
	}

	len1 = (size_t)PyObject_Length(arg1);
	len2 = (size_t)PyObject_Length(arg2);

	*lensum = len1 + len2;

	if (len1 == -1 || len2 == -1) {
		PyErr_Format(PyExc_TypeError,
			"Cannot len function. Needs. len(arg1) or len(arg2)");
		
		if (is_iter > 0) {
			Py_XDECREF(arg1);
			Py_XDECREF(arg2);
		}

		return -1;
	}

	if (PyObject_TypeCheck(arg1, &PyString_Type)
		&& PyObject_TypeCheck(arg2, &PyString_Type)) {
		lev_byte* string1, * string2;

		string1 = PyString_AS_STRING(arg1);
		string2 = PyString_AS_STRING(arg2);
		{
			size_t d = lev_edit_distance(len1, string1, len2, string2, xcost);

			if (d == (size_t)(-1)) {
				PyErr_NoMemory();
				return -1;
			}
			return d;
		}
	}
	else if (PyObject_TypeCheck(arg1, &PyUnicode_Type)
		&& PyObject_TypeCheck(arg2, &PyUnicode_Type)) {
		Py_UNICODE* string1, * string2;

		string1 = PyUnicode_AS_UNICODE(arg1);
		string2 = PyUnicode_AS_UNICODE(arg2);
		{
			size_t d = lev_u_edit_distance(len1, string1, len2, string2, xcost);

			if (d == (size_t)(-1)) {
				PyErr_NoMemory();
				return -1;
			}
			return d;
		}
	}

	else if (PySequence_Check(arg1) && PySequence_Check(arg2)) {

		{
			size_t d = lev_o_edit_distance(len1, arg1, len2, arg2, xcost);

			Py_XDECREF(arg1);
			Py_XDECREF(arg2);


			if (d == (size_t)(-1)) {
				PyErr_NoMemory();
				return -1;
			}
			return d;
		}
	}

	else {
		PyErr_Format(PyExc_TypeError,
			"%s expected two Sequence object", name);

		if (is_iter > 0) {
			Py_XDECREF(arg1);
			Py_XDECREF(arg2);
		}

		return -1;
	}
}

static PyObject*
distance_py(PyObject* self, PyObject* args)
{
	size_t lensum;
	long int ldist;

	if ((ldist = levenshtein_common(args, "distance", 0, &lensum)) < 0)
		return NULL;

	return PyInt_FromLong((long)ldist);
}

static PyObject*
ratio_py(PyObject* self, PyObject* args)
{
	size_t lensum;
	long int ldist;

	if ((ldist = levenshtein_common(args, "ratio", 1, &lensum)) < 0)
		return NULL;

	if (lensum == 0)
		return PyFloat_FromDouble(1.0);

	return PyFloat_FromDouble((((double)lensum - (double)ldist) / lensum));
	
}


static PyObject*
opcodes_to_result_list(size_t nb, LevOpCode* bops, 
	size_t len1, size_t len2,
	PyObject* arg1, PyObject* arg2, PyObject* arg3)
{
	PyObject* oplist = PyList_New(0);

	size_t i;

	size_t xcost = 0;
	size_t lensum = len1 + len2;

	for (i = 0; i < nb; i++, bops++) {
		char* tps = opcode_names[bops->type].cstring;
		if (arg3 == NULL || strcmp(tps, "equal") != 0) {

			size_t j = (size_t)bops->sbeg;
			size_t k= (size_t)bops->dbeg;

			for (j, k;
				j < bops->send || k < bops->dend;
				j++, k++)
			{
				PyObject* list = PyList_New(5);
				PyObject* is = opcode_names[bops->type].pystring;
				Py_INCREF(is);

				PyList_SetItem(list, 0, is);

				size_t l;
				for (l = 1; l < 5; l++) {
					Py_INCREF(Py_None); // Fatal Error ni narutame
					PyList_SetItem(list, l, Py_None);
				}

				if (strcmp(tps, "delete") == 0) {
					if (j < len1) {
						PyList_SetItem(list, 1, PyInt_FromLong(j));
						PyList_SetItem(list, 3, PySequence_GetItem(arg1, j));
						xcost++;
					}
				}
				else if (strcmp(tps, "insert") == 0) {
					if (k < len2) {
						PyList_SetItem(list, 2, PyInt_FromLong(k));
						PyList_SetItem(list, 4, PySequence_GetItem(arg2, k));
						xcost++;
					}
				}
				else {
					if (j < len1) {
						PyList_SetItem(list, 1, PyInt_FromLong(j));
						PyList_SetItem(list, 3, PySequence_GetItem(arg1, j));
					}
					if (k < len2) {
						PyList_SetItem(list, 2, PyInt_FromLong(k));
						PyList_SetItem(list, 4, PySequence_GetItem(arg2, k));
					}
					if (strcmp(tps, "replace") == 0)
						xcost++;
				}
				PyList_Append(oplist, list);
				Py_XDECREF(list);  //koreshinaito memory leak
			}
		}
	}
	return oplist;
}

static PyObject*
opcodes_py(PyObject* self, PyObject* args)
{
	PyObject* arg1, * arg2, * arg3 = NULL;

	size_t len1, len2, n, nb;

	if (!PyArg_UnpackTuple(args, PYARGCFIX("opcodes"), 2, 3, &arg1, &arg2, &arg3))
		return NULL;

	if (PyNumber_Check(arg1) && PyNumber_Check(arg2)) {
		PyObject* oplist = PyList_New(1);
		PyObject* list = PyList_New(5);
		Py_INCREF(list);

		PyList_SetItem(list, 0, PyUnicode_FromString("replace"));
		PyList_SetItem(list, 1, PyInt_FromLong(0));
		PyList_SetItem(list, 2, PyInt_FromLong(0));
		Py_XINCREF(arg1);
		PyList_SetItem(list, 3, arg1);
		Py_XINCREF(arg2);
		PyList_SetItem(list, 4, arg2);
		PyList_SetItem(oplist, 0, list);

		Py_DECREF(list);
		return oplist;
	}

	size_t is_iter = 0;
	if (PyGen_Check(arg1) || PyIter_Check(arg1) || PySet_Check(arg1) || PyDict_Check(arg1)) {
		arg1 = PySequence_List(arg1);
		is_iter++;
	}

	if (PyGen_Check(arg2) || PyIter_Check(arg2) || PySet_Check(arg2) || PyDict_Check(arg2)) {
		arg2 = PySequence_List(arg2);
		is_iter++;
	}

	len1 = PyObject_Length(arg1);
	len2 = PyObject_Length(arg2);

	if (len1 == -1 || len2 == -1) {
		PyErr_Format(PyExc_TypeError,
			"Cannot len function. Needs. len(arg1) or len(arg2)");

		if (is_iter > 0) {
			Py_XDECREF(arg1);
			Py_XDECREF(arg2);
		}

		return NULL;
	}


	LevEditOp* ops;
	LevOpCode* bops;

	/* find opcodes: we were called (s1, s2) */
	if (PyObject_TypeCheck(arg1, &PyString_Type)
		&& PyObject_TypeCheck(arg2, &PyString_Type)) {
		lev_byte* string1, * string2;

		string1 = PyString_AS_STRING(arg1);
		string2 = PyString_AS_STRING(arg2);
		ops = lev_editops_find(len1, string1, len2, string2, &n);
	}
	else if (PyObject_TypeCheck(arg1, &PyUnicode_Type)
		&& PyObject_TypeCheck(arg2, &PyUnicode_Type)) {
		Py_UNICODE* string1, * string2;

		string1 = PyUnicode_AS_UNICODE(arg1);
		string2 = PyUnicode_AS_UNICODE(arg2);
		ops = lev_u_editops_find(len1, string1, len2, string2, &n);
	}

	else if (PySequence_Check(arg1) && PySequence_Check(arg2)) {
		ops = lev_o_editops_find(len1, arg1, len2, arg2, &n);
	}
	else {
		PyErr_Format(PyExc_TypeError,
			"expected two Sequence or iter object");
		if (is_iter > 0) {
			Py_XDECREF(arg1);
			Py_XDECREF(arg2);
		}

		return NULL;
	}

	if (!ops && n)
		return PyErr_NoMemory();

	bops = lev_editops_to_opcodes(n, ops, &nb, len1, len2);
	if (!bops && nb)
		return PyErr_NoMemory();

	free(ops);
	
	PyObject* oplist;
	oplist = opcodes_to_result_list(nb, bops, len1, len2, arg1, arg2, arg3);

	//PyObject* ratio = PyFloat_FromDouble((double)(lensum - xcost) / lensum);

	free(bops);

	if (is_iter > 0) {
		Py_XDECREF(arg1);
		Py_XDECREF(arg2);
	}

	return oplist;

}

PY_MOD_INIT_FUNC_DEF(cdiffer)
{
#ifdef LEV_PYTHON3
	PyObject* module;
#endif
	size_t i;

	PY_INIT_MOD(module, "cdiffer", Levenshtein_DESC, methods)
		/* create intern strings for edit operation names */
		if (opcode_names[0].pystring)
			abort();
	for (i = 0; i < N_OPCODE_NAMES; i++) {
#ifdef LEV_PYTHON3
		opcode_names[i].pystring
			= PyUnicode_InternFromString(opcode_names[i].cstring);
#else
		opcode_names[i].pystring
			= PyString_InternFromString(opcode_names[i].cstring);
#endif
		opcode_names[i].len = strlen(opcode_names[i].cstring);
	}
	lev_init_rng(0);
#ifdef LEV_PYTHON3
	return module;
#endif
}
#endif

/****************************************************************************
 *
 * Basic stuff, Levenshtein distance
 *
 ****************************************************************************/
/**
  * lev_edit_distance:
  * @len1: The length of @string1.
  * @string1: A sequence of bytes of length @len1, may contain NUL characters.
  * @len2: The length of @string2.
  * @string2: A sequence of bytes of length @len2, may contain NUL characters.
  * @xcost: If nonzero, the replace operation has weight 2, otherwise all
  *         edit operations have equal weights of 1.
  *
  * Computes Levenshtein edit distance of two strings.
  *
  * Returns: The edit distance.
  **/
_LEV_STATIC_PY size_t
lev_edit_distance(size_t len1, const lev_byte* string1,
	size_t len2, const lev_byte* string2,
	int xcost)
{
	size_t i;
	size_t* row;  /* we only need to keep one row of costs */
	size_t* end;
	size_t half;

	/* strip common prefix */
	while (len1 > 0 && len2 > 0 && *string1 == *string2) {
		len1--;
		len2--;
		string1++;
		string2++;
	}

	/* strip common suffix */
	while (len1 > 0 && len2 > 0 && string1[len1 - 1] == string2[len2 - 1]) {
		len1--;
		len2--;
	}

	/* catch trivial cases */
	if (len1 == 0)
		return len2;
	if (len2 == 0)
		return len1;

	/* make the inner cycle (i.e. string2) the longer one */
	if (len1 > len2) {
		size_t nx = len1;
		const lev_byte* sx = string1;
		len1 = len2;
		len2 = nx;
		string1 = string2;
		string2 = sx;
	}
	/* check len1 == 1 separately */
	if (len1 == 1) {
		if (xcost)
			return len2 + 1 - 2 * (memchr(string2, *string1, len2) != NULL);
		else
			return len2 - (memchr(string2, *string1, len2) != NULL);
	}
	len1++;
	len2++;
	half = len1 >> 1;

	/* initalize first row */
	row = (size_t*)safe_malloc(len2, sizeof(size_t));
	if (!row)
		return (size_t)(-1);
	end = row + len2 - 1;
	for (i = 0; i < len2 - (xcost ? 0 : half); i++)
		row[i] = i;

	/* go through the matrix and compute the costs.  yes, this is an extremely
	 * obfuscated version, but also extremely memory-conservative and relatively
	 * fast.  */
	if (xcost) {
		for (i = 1; i < len1; i++) {
			size_t* p = row + 1;
			const lev_byte char1 = string1[i - 1];
			const lev_byte* char2p = string2;
			size_t D = i;
			size_t x = i;
			while (p <= end) {
				if (char1 == *(char2p++))
					x = --D;
				else
					x++;
				D = *p;
				D++;
				if (x > D)
					x = D;
				*(p++) = x;
			}
		}
	}
	else {
		/* in this case we don't have to scan two corner triangles (of size len1/2)
		 * in the matrix because no best path can go throught them. note this
		 * breaks when len1 == len2 == 2 so the memchr() special case above is
		 * necessary */
		row[0] = len1 - half - 1;
		for (i = 1; i < len1; i++) {
			size_t* p;
			const lev_byte char1 = string1[i - 1];
			const lev_byte* char2p;
			size_t D, x;
			/* skip the upper triangle */
			if (i >= len1 - half) {
				size_t offset = i - (len1 - half);
				size_t c3;

				char2p = string2 + offset;
				p = row + offset;
				c3 = *(p++) + (char1 != *(char2p++));
				x = *p;
				x++;
				D = x;
				if (x > c3)
					x = c3;
				*(p++) = x;
			}
			else {
				p = row + 1;
				char2p = string2;
				D = x = i;
			}
			/* skip the lower triangle */
			if (i <= half + 1)
				end = row + len2 + i - half - 2;
			/* main */
			while (p <= end) {
				size_t c3 = --D + (char1 != *(char2p++));
				x++;
				if (x > c3)
					x = c3;
				D = *p;
				D++;
				if (x > D)
					x = D;
				*(p++) = x;
			}
			/* lower triangle sentinel */
			if (i <= half) {
				size_t c3 = --D + (char1 != *char2p);
				x++;
				if (x > c3)
					x = c3;
				*p = x;
			}
		}
	}
	i = *end;
	free(row);
	return i;
}


/**
 * lev_u_edit_distance:
 * @len1: The length of @string1.
 * @string1: A sequence of Unicode characters of length @len1, may contain NUL
 *           characters.
 * @len2: The length of @string2.
 * @string2: A sequence of Unicode characters of length @len2, may contain NUL
 *           characters.
 * @xcost: If nonzero, the replace operation has weight 2, otherwise all
 *         edit operations have equal weights of 1.
 *
 * Computes Levenshtein edit distance of two Unicode strings.
 *
 * Returns: The edit distance.
 **/
_LEV_STATIC_PY size_t
lev_u_edit_distance(size_t len1, const lev_wchar* string1,
	size_t len2, const lev_wchar* string2,
	int xcost)
{
	size_t i;
	size_t* row;  /* we only need to keep one row of costs */
	size_t* end;
	size_t half;

	/* strip common prefix */
	while (len1 > 0 && len2 > 0 && *string1 == *string2) {
		len1--;
		len2--;
		string1++;
		string2++;
	}

	/* strip common suffix */
	while (len1 > 0 && len2 > 0 && string1[len1 - 1] == string2[len2 - 1]) {
		len1--;
		len2--;
	}

	/* catch trivial cases */
	if (len1 == 0)
		return len2;
	if (len2 == 0)
		return len1;

	/* make the inner cycle (i.e. string2) the longer one */
	if (len1 > len2) {
		size_t nx = len1;
		const lev_wchar* sx = string1;
		len1 = len2;
		len2 = nx;
		string1 = string2;
		string2 = sx;
	}
	/* check len1 == 1 separately */
	if (len1 == 1) {
		lev_wchar z = *string1;
		const lev_wchar* p = string2;
		for (i = len2; i; i--) {
			if (*(p++) == z)
				return len2 - 1;
		}
		return len2 + (xcost != 0);
	}
	len1++;
	len2++;
	half = len1 >> 1;

	/* initalize first row */
	row = (size_t*)safe_malloc(len2, sizeof(size_t));
	if (!row)
		return (size_t)(-1);
	end = row + len2 - 1;
	for (i = 0; i < len2 - (xcost ? 0 : half); i++)
		row[i] = i;

	/* go through the matrix and compute the costs.  yes, this is an extremely
	 * obfuscated version, but also extremely memory-conservative and relatively
	 * fast.  */
	if (xcost) {
		for (i = 1; i < len1; i++) {
			size_t* p = row + 1;
			const lev_wchar char1 = string1[i - 1];
			const lev_wchar* char2p = string2;
			size_t D = i - 1;
			size_t x = i;
			while (p <= end) {
				if (char1 == *(char2p++))
					x = D;
				else
					x++;
				D = *p;
				if (x > D + 1)
					x = D + 1;
				*(p++) = x;
			}
		}
	}
	else {
		/* in this case we don't have to scan two corner triangles (of size len1/2)
		 * in the matrix because no best path can go throught them. note this
		 * breaks when len1 == len2 == 2 so the memchr() special case above is
		 * necessary */
		row[0] = len1 - half - 1;
		for (i = 1; i < len1; i++) {
			size_t* p;
			const lev_wchar char1 = string1[i - 1];
			const lev_wchar* char2p;
			size_t D, x;
			/* skip the upper triangle */
			if (i >= len1 - half) {
				size_t offset = i - (len1 - half);
				
				size_t c3;

				char2p = string2 + offset;

				p = row + offset;
				c3 = *(p++) + (char1 != *(char2p++));
				x = *p;
				x++;
				D = x;
				if (x > c3)
					x = c3;
				*(p++) = x;
			}
			else {
				p = row + 1;
				char2p = string2;
				D = x = i;
			}
			/* skip the lower triangle */
			if (i <= half + 1)
				end = row + len2 + i - half - 2;
			
			/* main */
			while (p <= end) {
				size_t c3 = --D + (char1 != *(char2p++));
				x++;
				if (x > c3)
					x = c3;
				D = *p;
				D++;
				if (x > D)
					x = D;
				*(p++) = x;
			}
			/* lower triangle sentinel */
			if (i <= half) {
				size_t c3 = --D + (char1 != *char2p);
				x++;
				if (x > c3)
					x = c3;
				*p = x;
			}
		}
	}

	i = *end;
	free(row);
	return i;
}

/**
 * lev_o_edit_distance:
 * @len1: The length of @string1.
 * @string1: A sequence of Unicode characters of length @len1, may contain NUL
 *           characters.
 * @len2: The length of @string2.
 * @string2: A sequence of Unicode characters of length @len2, may contain NUL
 *           characters.
 * @xcost: If nonzero, the replace operation has weight 2, otherwise all
 *         edit operations have equal weights of 1.
 *
 * Computes Levenshtein edit distance of two Unicode strings.
 *
 * Returns: The edit distance.
 **/
_LEV_STATIC_PY size_t
lev_o_edit_distance(size_t len1, PyObject *string1,
	size_t len2, PyObject* string2,
	int xcost)
{
	size_t len1o, len2o;
	size_t i;
	size_t* row;  /* we only need to keep one row of costs */
	size_t* end;
	size_t half;
	

	/* strip common prefix */
	len1o = 0;
	size_t offset = 0;
	while (len1 > 0 && len2 > 0 && PyObject_RichCompareBool(PySequence_GetItem(string1, len1o),
		PySequence_GetItem(string2, len1o), Py_EQ)) {
		len1--;
		len2--;
		offset++;
		len1o++;
	}
	len2o = len1o;

	/* strip common suffix */
	size_t suffix = 0;
	while (len1 > 0 && len2 > 0
		&& PyObject_RichCompareBool(PySequence_GetItem(string1, offset + len1 - 1),
			PySequence_GetItem(string2, offset + len2 - 1), Py_EQ)) {
		len1--;
		len2--;
		suffix++;
	}

	/* catch trivial cases */
	if (len1 == 0)
		return len2;
	if (len2 == 0)
		return len1;

	PyObject* char1;
	PyObject* char2p;

	/* make the inner cycle (i.e. string2) the longer one */
	if (len1 > len2) {
		size_t nx = len1;
		len1 = len2;
		len2 = nx;
		//char1 = PySequence_GetSlice(string2, len2o, Py_None);
		//char2p = PySequence_GetSlice(string1, len1o, Py_None);
		char1 = string2;
		char2p = string1;
	}
	else {
		//char1 = PySequence_GetSlice(string1, len1o, Py_None);
		//char2p = PySequence_GetSlice(string2, len2o, Py_None);
		char1 = string1;
		char2p = string2;
	}

	/* check len1 == 1 separately */
	if (len1 == 1) {
		for (i = len2; i; i--) {
			if (PyObject_RichCompareBool(
				PySequence_GetItem(char2p, offset + len2 - i), 
				PySequence_GetItem(char1, offset), 
				Py_EQ))
				return len2 - 1;
		}
		return len2 + (xcost != 0);
	}
	len1++;
	len2++;
	half = len1 >> 1;

	/* initalize first row */
	row = (size_t*)safe_malloc(len2, sizeof(size_t));
	if (!row)
		return (size_t)(-1);
	end = row + len2 - 1;
	for (i = 0; i < len2 - (xcost ? 0 : half); i++)
		row[i] = i;
	
	/* go through the matrix and compute the costs.  yes, this is an extremely
	 * obfuscated version, but also extremely memory-conservative and relatively
	 * fast.  */
	if (xcost) { 
		for (i = 1; i < len1; i++) {
			size_t* p = row + 1;
			size_t D = i - 1;
			size_t x = i;
			size_t cnt = 0;
			while (p <= end) {
				if (PyObject_RichCompareBool(
					PySequence_GetItem(char1, offset + i - 1),
					PySequence_GetItem(char2p, offset + cnt),
					Py_EQ))// == *(char2p++))

					x = D;
				else
					x++;
				D = *p;
				if (x > D + 1)
					x = D + 1;
				*(p++) = x;
				cnt++;
			}
		}
	}
	else {
		/* in this case we don't have to scan two corner triangles (of size len1/2)
		 * in the matrix because no best path can go throught them. note this
		 * breaks when len1 == len2 == 2 so the memchr() special case above is
		 * necessary */
		row[0] = len1 - half - 1;

		size_t offset2 = 0;

		for (i = 1; i < len1; i++) {
			size_t* p;
			//const lev_wchar char1 = string1[i - 1];
			//const lev_wchar* char2p;
			size_t D, x;
			
			size_t cnt = 0;

			/* skip the upper triangle */
			if (i >= len1 - half) {
				offset2 = i - (len1 - half);
				size_t c3;
				
				/*char2p = string2 + offset;*/
				
				p = row + offset2;
				/*c3 = *(p++) + (char1 != *(char2p++));*/
				c3 = *(p++) + (
					PyObject_RichCompareBool(
						PySequence_GetItem(char1, offset + i - 1)
						, PySequence_GetItem(char2p, offset + offset2 + cnt)
						, Py_NE));
				cnt++;
				x = *p;
				x++;
				D = x;
				if (x > c3)
					x = c3;
				*(p++) = x;
			}
			else {
				p = row + 1;
				D = x = i;
			}

			/* skip the lower triangle */
			if (i <= half + 1)
				end = row + len2 + i - half - 2;
			
			/* main */
			
			
			while (p <= end) {
				/* 
				//DEBUG
				printf("%d %d %ls %ls\n", i, *p,
					PyUnicode_AS_UNICODE(PySequence_GetItem(char1, offset + i - 1)),
					PyUnicode_AS_UNICODE(PySequence_GetItem(char2p, offset + offset2 + cnt)
					));
				*/
				size_t c3 = --D + (
					PyObject_RichCompareBool(
						PySequence_GetItem(char1, offset + i - 1)
						,PySequence_GetItem(char2p, offset + offset2 + cnt)
						, Py_NE
					)); //size_t c3 = --D + (char1 != *(char2p++));
				
				cnt++;

				x++;
				if (x > c3)
					x = c3;
				D = *p;
				D++;
				if (x > D)
					x = D;
				*(p++) = x;
			}

			/* lower triangle sentinel */
			if (i <= half) {
				size_t c3 = --D + (
					PyObject_RichCompareBool(
						PySequence_GetItem(char1, i - 1)
						, PySequence_GetItem(char2p, offset + offset2 + cnt)
						, Py_NE
					)); //size_t c3 = --D + (char1 != *char2p);
				
				x++;
				if (x > c3)
					x = c3;
				*p = x;
			}
		}
	}

	i = *end;
	free(row);
	
	//自作のPyObjectは手動で開放してやらないとメモリリークするため参照カウントを減らす
	Py_XDECREF(char1);
	Py_XDECREF(char1);
	Py_XDECREF(char2p);
	Py_XDECREF(char2p);

	return i;
}





/**
 * editops_from_cost_matrix:
 * @len1: The length of @string1.
 * @string1: A string of length @len1, may contain NUL characters.
 * @o1: The offset where the matrix starts from the start of @string1.
 * @len2: The length of @string2.
 * @string2: A string of length @len2, may contain NUL characters.
 * @o2: The offset where the matrix starts from the start of @string2.
 * @matrix: The cost matrix.
 * @n: Where the number of edit operations should be stored.
 *
 * Reconstructs the optimal edit sequence from the cost matrix @matrix.
 *
 * The matrix is freed.
 *
 * Returns: The optimal edit sequence, as a newly allocated array of
 *          elementary edit operations, it length is stored in @n.
 **/
static LevEditOp*
editops_from_cost_matrix(size_t len1, const lev_byte* string1, size_t off1,
	size_t len2, const lev_byte* string2, size_t off2,
	size_t* matrix, size_t* n)
{
	size_t* p;
	size_t i, j, pos;
	LevEditOp* ops;
	int dir = 0;

	pos = *n = matrix[len1 * len2 - 1];
	if (!*n) {
		free(matrix);
		return NULL;
	}
	ops = (LevEditOp*)safe_malloc((*n), sizeof(LevEditOp));
	if (!ops) {
		free(matrix);
		*n = (size_t)(-1);
		return NULL;
	}
	i = len1 - 1;
	j = len2 - 1;
	p = matrix + len1 * len2 - 1;
	while (i || j) {
		/* prefer contiuning in the same direction */
		if (dir < 0 && j && *p == *(p - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_INSERT;
			ops[pos].spos = i + off1;
			ops[pos].dpos = --j + off2;
			p--;
			continue;
		}
		if (dir > 0 && i && *p == *(p - len2) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_DELETE;
			ops[pos].spos = --i + off1;
			ops[pos].dpos = j + off2;
			p -= len2;
			continue;
		}
		if (i && j && *p == *(p - len2 - 1)
			&& string1[i - 1] == string2[j - 1]) {
			/* don't be stupid like difflib, don't store LEV_EDIT_KEEP */
			i--;
			j--;
			p -= len2 + 1;
			dir = 0;
			continue;
		}
		if (i && j && *p == *(p - len2 - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_REPLACE;
			ops[pos].spos = --i + off1;
			ops[pos].dpos = --j + off2;
			p -= len2 + 1;
			dir = 0;
			continue;
		}
		/* we cant't turn directly from -1 to 1, in this case it would be better
		 * to go diagonally, but check it (dir == 0) */
		if (dir == 0 && j && *p == *(p - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_INSERT;
			ops[pos].spos = i + off1;
			ops[pos].dpos = --j + off2;
			p--;
			dir = -1;
			continue;
		}
		if (dir == 0 && i && *p == *(p - len2) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_DELETE;
			ops[pos].spos = --i + off1;
			ops[pos].dpos = j + off2;
			p -= len2;
			dir = 1;
			continue;
		}
		/* coredump right now, later might be too late ;-) */
		assert("lost in the cost matrix" == NULL);
	}
	free(matrix);

	return ops;
}

/**
 * lev_editops_find:
 * @len1: The length of @string1.
 * @string1: A string of length @len1, may contain NUL characters.
 * @len2: The length of @string2.
 * @string2: A string of length @len2, may contain NUL characters.
 * @n: Where the number of edit operations should be stored.
 *
 * Find an optimal edit sequence from @string1 to @string2.
 *
 * When there's more than one optimal sequence, a one is arbitrarily (though
 * deterministically) chosen.
 *
 * Returns: The optimal edit sequence, as a newly allocated array of
 *          elementary edit operations, it length is stored in @n.
 *          It is normalized, i.e., keep operations are not included.
 **/
_LEV_STATIC_PY LevEditOp*
lev_editops_find(size_t len1, const lev_byte* string1,
	size_t len2, const lev_byte* string2,
	size_t* n)
{
	size_t len1o, len2o;
	size_t i;
	size_t* matrix; /* cost matrix */

	/* strip common prefix */
	len1o = 0;
	while (len1 > 0 && len2 > 0 && *string1 == *string2) {
		len1--;
		len2--;
		string1++;
		string2++;
		len1o++;
	}
	len2o = len1o;

	/* strip common suffix */
	while (len1 > 0 && len2 > 0 && string1[len1 - 1] == string2[len2 - 1]) {
		len1--;
		len2--;
	}
	len1++;
	len2++;

	/* initialize first row and column */
	matrix = (size_t*)safe_malloc_3(len1, len2, sizeof(size_t));
	if (!matrix) {
		*n = (size_t)(-1);
		return NULL;
	}
	for (i = 0; i < len2; i++)
		matrix[i] = i;
	for (i = 1; i < len1; i++)
		matrix[len2 * i] = i;

	/* find the costs and fill the matrix */
	for (i = 1; i < len1; i++) {
		size_t* prev = matrix + (i - 1) * len2;
		size_t* p = matrix + i * len2;
		size_t* end = p + len2 - 1;
		const lev_byte char1 = string1[i - 1];
		const lev_byte* char2p = string2;
		size_t x = i;
		p++;
		while (p <= end) {
			size_t c3 = *(prev++) + (char1 != *(char2p++));
			x++;
			if (x > c3)
				x = c3;
			c3 = *prev + 1;
			if (x > c3)
				x = c3;
			*(p++) = x;
		}
	}

	/* find the way back */
	return editops_from_cost_matrix(len1, string1, len1o,
		len2, string2, len2o,
		matrix, n);
}

/**
 * ueditops_from_cost_matrix:
 * @len1: The length of @string1.
 * @string1: A string of length @len1, may contain NUL characters.
 * @o1: The offset where the matrix starts from the start of @string1.
 * @len2: The length of @string2.
 * @string2: A string of length @len2, may contain NUL characters.
 * @o2: The offset where the matrix starts from the start of @string2.
 * @matrix: The cost matrix.
 * @n: Where the number of edit operations should be stored.
 *
 * Reconstructs the optimal edit sequence from the cost matrix @matrix.
 *
 * The matrix is freed.
 *
 * Returns: The optimal edit sequence, as a newly allocated array of
 *          elementary edit operations, it length is stored in @n.
 **/
static LevEditOp*
ueditops_from_cost_matrix(size_t len1, const lev_wchar* string1, size_t o1,
	size_t len2, const lev_wchar* string2, size_t o2,
	size_t* matrix, size_t* n)
{
	size_t* p;
	size_t i, j, pos;
	LevEditOp* ops;
	int dir = 0;
	
	pos = *n = matrix[len1 * len2 - 1];
	
	if (!*n) {
		free(matrix);
		return NULL;
	}
	
	ops = (LevEditOp*)safe_malloc((*n), sizeof(LevEditOp));
	
	if (!ops) {
		free(matrix);
		*n = (size_t)(-1);
		return NULL;
	}
	i = len1 - 1;
	j = len2 - 1;
	p = matrix + len1 * len2 - 1;
	
	while (i || j) {
		/* prefer contiuning in the same direction */
		if (dir < 0 && j && *p == *(p - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_INSERT;
			ops[pos].spos = i + o1;
			ops[pos].dpos = --j + o2;
			p--;
			continue;
		}
		if (dir > 0 && i && *p == *(p - len2) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_DELETE;
			ops[pos].spos = --i + o1;
			ops[pos].dpos = j + o2;
			p -= len2;
			continue;
		}
		
		if (i && j && *p == *(p - len2 - 1)
			&& string1[i - 1] == string2[j - 1]) {
			/* don't be stupid like difflib, don't store LEV_EDIT_KEEP */
			i--;
			j--;
			p -= len2 + 1;
			dir = 0;
			continue;
		}
		if (i && j && *p == *(p - len2 - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_REPLACE;
			ops[pos].spos = --i + o1;
			ops[pos].dpos = --j + o2;
			p -= len2 + 1;
			dir = 0;
			continue;
		}
		/* we cant't turn directly from -1 to 1, in this case it would be better
		 * to go diagonally, but check it (dir == 0) */
		if (dir == 0 && j && *p == *(p - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_INSERT;
			ops[pos].spos = i + o1;
			ops[pos].dpos = --j + o2;
			p--;
			dir = -1;
			continue;
		}
		if (dir == 0 && i && *p == *(p - len2) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_DELETE;
			ops[pos].spos = --i + o1;
			ops[pos].dpos = j + o2;
			p -= len2;
			dir = 1;
			continue;
		}
		/* coredump right now, later might be too late ;-) */
		assert("lost in the cost matrix" == NULL);
	}
	free(matrix);

	return ops;
}


/**
 * lev_u_editops_find:
 * @len1: The length of @string1.
 * @string1: A string of length @len1, may contain NUL characters.
 * @len2: The length of @string2.
 * @string2: A string of length @len2, may contain NUL characters.
 * @n: Where the number of edit operations should be stored.
 *
 * Find an optimal edit sequence from @string1 to @string2.
 *
 * When there's more than one optimal sequence, a one is arbitrarily (though
 * deterministically) chosen.
 *
 * Returns: The optimal edit sequence, as a newly allocated array of
 *          elementary edit operations, it length is stored in @n.
 *          It is normalized, i.e., keep operations are not included.
 **/
 
_LEV_STATIC_PY LevEditOp*
lev_u_editops_find(size_t len1, const lev_wchar* string1,
	size_t len2, const lev_wchar* string2,
	size_t* n)
{
	size_t len1o, len2o;
	size_t i;
	size_t* matrix; /* cost matrix */
	
	/* strip common prefix */
	len1o = 0;
	while (len1 > 0 && len2 > 0 && *string1 == *string2) {
		len1--;
		len2--;
		string1++;
		string2++;
		len1o++;
	}
	len2o = len1o;

	/* strip common suffix */
	while (len1 > 0 && len2 > 0 && string1[len1 - 1] == string2[len2 - 1]) {
		len1--;
		len2--;
	}
	len1++;
	len2++;

	/* initalize first row and column */
	matrix = (size_t*)safe_malloc_3(len1, len2, sizeof(size_t));
	if (!matrix) {
		*n = (size_t)(-1);
		return NULL;
	}
	for (i = 0; i < len2; i++)
		matrix[i] = i;
	for (i = 1; i < len1; i++)
		matrix[len2 * i] = i;


	/* find the costs and fill the matrix */
	for (i = 1; i < len1; i++) {
		size_t* prev = matrix + (i - 1) * len2;
		size_t* p = matrix + i * len2;
		size_t* end = p + len2 - 1;
		const lev_wchar char1 = string1[i - 1];
		const lev_wchar* char2p = string2;
		size_t x = i;
		p++;
		while (p <= end) {
			size_t c3 = *(prev++) + (char1 != *(char2p++));

			x++;
			if (x > c3)
				x = c3;
			c3 = *prev + 1;
			if (x > c3)
				x = c3;
			*(p++) = x;
		}
	}

	/* find the way back */
	return ueditops_from_cost_matrix(len1, string1, len1o,
		len2, string2, len2o,
		matrix, n);
}


_LEV_STATIC_PY LevEditOp*
lev_o_editops_find(size_t len1, PyObject* string1,
	size_t len2, PyObject* string2,
	size_t* n)
{
	size_t len1o, len2o;
	size_t i;
	size_t* matrix; /* cost matrix */

	/* strip common prefix */
	len1o = 0;
	size_t offset = 0;

	while (len1 > 0 && len2 > 0 
		&& PyObject_RichCompareBool(PySequence_GetItem(string1, len1o),
			PySequence_GetItem(string2, len1o), Py_EQ)) {
		len1--;
		len2--;
		//string1++;
		//string2++;
		offset++;
		len1o++;
	}
	len2o = len1o;

	/* strip common suffix */
	size_t suffix = 0;
	while (len1 > 0 && len2 > 0 
		&& PyObject_RichCompareBool(PySequence_GetItem(string1, offset + len1 - 1),
			PySequence_GetItem(string2, offset + len2 - 1), Py_EQ)) {
		len1--;
		len2--;
		suffix++;
	}

	len1++;
	len2++;

	/* initalize first row and column */
	matrix = (size_t*)safe_malloc_3(len1, len2, sizeof(size_t));
	if (!matrix) {
		*n = (size_t)(-1);
		return NULL;
	}
	for (i = 0; i < len2; i++)
		matrix[i] = i;
	for (i = 1; i < len1; i++)
		matrix[len2 * i] = i;

	/* find the costs and fill the matrix */
	for (i = 1; i < len1; i++) {
		size_t* prev = matrix + (i - 1) * len2;
		size_t* p = matrix + i * len2;
		size_t* end = p + len2 - 1;
		
		size_t x = i;
		p++;
		size_t cnt = 0;

		while (p <= end) {

			size_t itemmatch = PyObject_RichCompareBool(
				PySequence_GetItem(string1, offset + i - 1), 
				PySequence_GetItem(string2, offset + cnt), 
				Py_NE);

			if (itemmatch == -1) {
				itemmatch = 1;
			}
			size_t c3 = *(prev++) + itemmatch;
			
			x++;
			if (x > c3)
				x = c3;
			c3 = *prev + 1;
			if (x > c3)
				x = c3;
			*(p++) = x;
			cnt++;
		}
	}
	
	/* find the way back */
	/* ***
	like function "ueditops_from_cost_matrix" for Python Object.
	 * ***
	*/

	size_t* p;

	size_t j, k, pos;
	LevEditOp* ops;
	int dir = 0;

	pos = *n = matrix[len1 * len2 - 1];

	if (!*n) {
		free(matrix);
		return NULL;
	}

	ops = (LevEditOp*)safe_malloc((*n), sizeof(LevEditOp));

	if (!ops) {
		free(matrix);
		*n = (size_t)(-1);
		return NULL;
	}
	j = len1 - 1;
	k = len2 - 1;
	p = matrix + len1 * len2 - 1;

	while (j || k) {
		/* prefer contiuning in the same direction */
		if (dir < 0 && k && *p == *(p - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_INSERT;
			ops[pos].spos = j + len1o;
			ops[pos].dpos = --k + len2o;
			p--;
			continue;
		}
		if (dir > 0 && j && *p == *(p - len2) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_DELETE;
			ops[pos].spos = --j + len1o;
			ops[pos].dpos = k + len2o;
			p -= len2;
			continue;
		}
		if (j && k && *p == *(p - len2 - 1)
			&& PyObject_RichCompareBool(PySequence_GetItem(string1, offset + j - 1),
				PySequence_GetItem(string2, offset + k - 1), Py_EQ)) {

			/* don't be stupid like difflib, don't store LEV_EDIT_KEEP */
			j--;
			k--;
			p -= len2 + 1;
			dir = 0;
			continue;
		}
		if (j && k && *p == *(p - len2 - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_REPLACE;
			ops[pos].spos = --j + len1o;
			ops[pos].dpos = --k + len2o;
			p -= len2 + 1;
			dir = 0;
			continue;
		}
		/* we cant't turn directly from -1 to 1, in this case it would be better
		 * to go diagonally, but check it (dir == 0) */
		if (dir == 0 && k && *p == *(p - 1) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_INSERT;
			ops[pos].spos = j + len1o;
			ops[pos].dpos = --k + len2o;
			p--;
			dir = -1;
			continue;
		}
		if (dir == 0 && j && *p == *(p - len2) + 1) {
			pos--;
			ops[pos].type = LEV_EDIT_DELETE;
			ops[pos].spos = --j + len1o;
			ops[pos].dpos = k + len2o;
			p -= len2;
			dir = 1;
			continue;
		}
		/* coredump right now, later might be too late ;-) */
		assert("lost in the cost matrix" == NULL);
	}
	free(matrix);

	return ops;
}


/**
 * lev_editops_to_opcodes:
 * @n: The size of @ops.
 * @ops: An array of elementary edit operations.
 * @nb: Where the number of difflib block operation codes should be stored.
 * @len1: The length of the source string.
 * @len2: The length of the destination string.
 *
 * Converts elementary edit operations to difflib block operation codes.
 *
 * Note the string lengths are necessary since difflib doesn't allow omitting
 * keep operations.
 *
 * Returns: The converted block operation codes, as a newly allocated array;
 *          its length is stored in @nb.
 **/
_LEV_STATIC_PY LevOpCode*
lev_editops_to_opcodes(size_t n, const LevEditOp* ops, size_t* nb,
	size_t len1, size_t len2)
{
	size_t nbl, i, spos, dpos;
	const LevEditOp* o;
	LevOpCode* bops, * b;
	LevEditType type;

	/* compute the number of blocks */
	nbl = 0;
	o = ops;
	spos = dpos = 0;
	type = LEV_EDIT_KEEP;
	for (i = n; i; ) {
		/* simply pretend there are no keep blocks */
		while (o->type == LEV_EDIT_KEEP && --i)
			o++;
		if (!i)
			break;
		if (spos < o->spos || dpos < o->dpos) {
			nbl++;
			spos = o->spos;
			dpos = o->dpos;
		}
		nbl++;
		type = o->type;
		switch (type) {
		case LEV_EDIT_REPLACE:
			do {
				spos++;
				dpos++;
				i--;
				o++;
			} while (i && o->type == type && spos == o->spos && dpos == o->dpos);
			break;

		case LEV_EDIT_DELETE:
			do {
				spos++;
				i--;
				o++;
			} while (i && o->type == type && spos == o->spos && dpos == o->dpos);
			break;

		case LEV_EDIT_INSERT:
			do {
				dpos++;
				i--;
				o++;
			} while (i && o->type == type && spos == o->spos && dpos == o->dpos);
			break;

		default:
			break;
		}
	}
	if (spos < len1 || dpos < len2)
		nbl++;

	/* convert */
	b = bops = (LevOpCode*)safe_malloc(nbl, sizeof(LevOpCode));
	if (!bops) {
		*nb = (size_t)(-1);
		return NULL;
	}
	o = ops;
	spos = dpos = 0;
	type = LEV_EDIT_KEEP;
	for (i = n; i; ) {
		/* simply pretend there are no keep blocks */
		while (o->type == LEV_EDIT_KEEP && --i)
			o++;
		if (!i)
			break;
		b->sbeg = spos;
		b->dbeg = dpos;
		if (spos < o->spos || dpos < o->dpos) {
			b->type = LEV_EDIT_KEEP;
			spos = b->send = o->spos;
			dpos = b->dend = o->dpos;
			b++;
			b->sbeg = spos;
			b->dbeg = dpos;
		}
		type = o->type;
		switch (type) {
		case LEV_EDIT_REPLACE:
			do {
				spos++;
				dpos++;
				i--;
				o++;
			} while (i && o->type == type && spos == o->spos && dpos == o->dpos);
			break;

		case LEV_EDIT_DELETE:
			do {
				spos++;
				i--;
				o++;
			} while (i && o->type == type && spos == o->spos && dpos == o->dpos);
			break;

		case LEV_EDIT_INSERT:
			do {
				dpos++;
				i--;
				o++;
			} while (i && o->type == type && spos == o->spos && dpos == o->dpos);
			break;

		default:
			break;
		}
		b->type = type;
		b->send = spos;
		b->dend = dpos;
		b++;
	}
	if (spos < len1 || dpos < len2) {
		assert(len1 - spos == len2 - dpos);
		b->type = LEV_EDIT_KEEP;
		b->sbeg = spos;
		b->dbeg = dpos;
		b->send = len1;
		b->dend = len2;
		b++;
	}
	assert((size_t)(b - bops) == nbl);

	*nb = nbl;
	return bops;
}


/* not NO_PYTHON */

/****************************************************************************
 *
 * C (i.e. executive) part
 *
 ****************************************************************************/

 /****************************************************************************
  *
  * Taus113
  *
  ****************************************************************************/

  /*
   * Based on Tausworthe random generator implementation rng/taus113.c
   * from the GNU Scientific Library (http://sources.redhat.com/gsl/)
   * which has the notice
   * Copyright (C) 2002 Atakan Gurkan
   * Based on the file taus.c which has the notice
   * Copyright (C) 1996, 1997, 1998, 1999, 2000 James Theiler, Brian Gough
   */

static inline unsigned long
taus113_get(taus113_state_t* state)
{
	unsigned long b1, b2, b3, b4;

	b1 = ((((state->z1 << 6UL) & TAUS113_MASK) ^ state->z1) >> 13UL);
	state->z1 = ((((state->z1 & 4294967294UL) << 18UL) & TAUS113_MASK) ^ b1);

	b2 = ((((state->z2 << 2UL) & TAUS113_MASK) ^ state->z2) >> 27UL);
	state->z2 = ((((state->z2 & 4294967288UL) << 2UL) & TAUS113_MASK) ^ b2);

	b3 = ((((state->z3 << 13UL) & TAUS113_MASK) ^ state->z3) >> 21UL);
	state->z3 = ((((state->z3 & 4294967280UL) << 7UL) & TAUS113_MASK) ^ b3);

	b4 = ((((state->z4 << 3UL) & TAUS113_MASK) ^ state->z4) >> 12UL);
	state->z4 = ((((state->z4 & 4294967168UL) << 13UL) & TAUS113_MASK) ^ b4);

	return (state->z1 ^ state->z2 ^ state->z3 ^ state->z4);

}

static void
taus113_set(taus113_state_t* state,
	unsigned long int s)
{
	if (!s)
		s = 1UL;                    /* default seed is 1 */

	state->z1 = TAUS113_LCG(s);
	if (state->z1 < 2UL)
		state->z1 += 2UL;

	state->z2 = TAUS113_LCG(state->z1);
	if (state->z2 < 8UL)
		state->z2 += 8UL;

	state->z3 = TAUS113_LCG(state->z2);
	if (state->z3 < 16UL)
		state->z3 += 16UL;

	state->z4 = TAUS113_LCG(state->z3);
	if (state->z4 < 128UL)
		state->z4 += 128UL;

	/* Calling RNG ten times to satify recurrence condition */
	taus113_get(state);
	taus113_get(state);
	taus113_get(state);
	taus113_get(state);
	taus113_get(state);
	taus113_get(state);
	taus113_get(state);
	taus113_get(state);
	taus113_get(state);
	taus113_get(state);
}


/**
 * lev_init_rng:
 * @seed: A seed.  If zero, a default value (currently 1) is used instead.
 *
 * Initializes the random generator used by some Levenshtein functions.
 *
 * This does NOT happen automatically when these functions are used.
 **/
_LEV_STATIC_PY void
lev_init_rng(unsigned long int seed)
{
	static taus113_state_t state;

	taus113_set(&state, seed);
}
