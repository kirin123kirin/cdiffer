/* ver.0.1 2021 / 03 / 03 kirin Exp.*/

#ifndef CDIFFER_H
#define CDIFFER_H

#ifndef size_t
#  include <stdlib.h>
#endif

/* A bit dirty. */
#ifndef _STATIC_PY
#  define _STATIC_PY /* */
#endif

//typedef unsigned char bytes_type;
typedef char bytes_type;

/* Edit opration type
 * DON'T CHANGE! used ad arrays indices and the bits are occasionally used
 * as flags */
typedef enum {
	LEV_EDIT_KEEP = 0,
	LEV_EDIT_REPLACE = 1,
	LEV_EDIT_INSERT = 2,
	LEV_EDIT_DELETE = 3,
	LEV_EDIT_LAST  /* sometimes returned when an error occurs */
} LevEditType;

/* Edit operation (atomic).
 * This is the `native' atomic edit operation.  It differs from the difflib
 * one's because it represents a change of one character, not a block.  And
 * we usually don't care about LEV_EDIT_KEEP, though the functions can handle
 * them.  The positions are interpreted as at the left edge of a character.
 */
typedef struct {
	LevEditType type;  /* editing operation type */
	size_t spos;  /* source block position */
	size_t dpos;  /* destination position */
} LevEditOp;

/* Edit operation (difflib-compatible).
 * This is not `native', but conversion functions exist.  These fields exactly
 * correspond to the codeops() tuples fields (and this method is also the
 * source of the silly OpCode name).  Sequences must span over complete
 * strings, subsequences are simply edit sequences with more (or larger)
 * LEV_EDIT_KEEP blocks.
 */
typedef struct {
	LevEditType type;  /* editing operation type */
	size_t sbeg, send;  /* source block begin, end */
	size_t dbeg, dend;  /* destination block begin, end */
} LevOpCode;

static size_t
dist_handler(PyObject* args,
	const char* name,
	size_t xcost,
	size_t* lensum);

_STATIC_PY
size_t
dist_s(size_t len1,
	const bytes_type* string1,
	size_t len2,
	const bytes_type* string2,
	size_t xcost);

_STATIC_PY
size_t
dist_u(size_t len1,
	Py_UCS4* string1,
	size_t len2,
	Py_UCS4* string2,
	size_t xcost);

_STATIC_PY
size_t
dist_o(size_t len1,
	PyObject* string1,
	size_t len2,
	PyObject* string2,
	size_t xcost);


_STATIC_PY
LevEditOp*
differ_op_s(size_t len1,
	const bytes_type* string1,
	size_t len2,
	const bytes_type* string2,
	size_t* n);

_STATIC_PY
LevEditOp*
differ_op_u(size_t len1,
	Py_UCS4* string1,
	size_t len2,
	Py_UCS4* string2,
	size_t* n);

_STATIC_PY
LevEditOp*
differ_op_o(size_t len1,
	PyObject* string1,
	size_t len2,
	PyObject* string2,
	size_t* n);

_STATIC_PY
LevOpCode*
op2opcodes(size_t n,
	LevEditOp* ops,
	size_t* nb,
	size_t len1,
	size_t len2);

/* UNUSED yet */
_STATIC_PY
void
init_rng(unsigned long int seed);

#endif /* not CDIFFER_H */
