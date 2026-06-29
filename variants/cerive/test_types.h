#pragma once

#include "shapes.h"

/*
 * Typedefs so multi-word C types can be used as single-token field types with
 * the CERIVE macro system.
 */
typedef unsigned long unsigned_long;
typedef long long long_long;
typedef unsigned long long unsigned_long_long;

/*
 * Empty struct -- degenerate zero-field case.
 */
#define Empty_FIELDS(X) /* nothing */
CERIVE(Empty, Struct, Debug, Default, PartialEq, Ord, Hash)

/*
 * Triple pointer -- a double-pointer to Point stored in a struct, testing
 * pointer-field derives.
 */
#define TripleP_FIELDS(X) \
	X(*, Point *, ptr) \
	X(int32_t, seq)
CERIVE(TripleP, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

/*
 * Variant types for the 5-variant union test.
 */
#define Alpha_FIELDS(X) X(int32_t, val)
CERIVE(Alpha, Struct, Debug, Default, PartialEq)

#define Beta_FIELDS(X) X(double, val)
CERIVE(Beta, Struct, Debug, Default, PartialEq)

#define Gamma_FIELDS(X) X(double, val)
CERIVE(Gamma, Struct, Debug, Default, PartialEq)

#define Delta_FIELDS(X) X(char, val)
CERIVE(Delta, Struct, Debug, Default, PartialEq)

#define Epsilon_FIELDS(X) X(int64_t, val)
CERIVE(Epsilon, Struct, Debug, Default, PartialEq)

/*
 * Many -- a union with 5 variants.
 */
#define Many_VARIANTS(X) \
	X(Alpha) \
	X(Beta) \
	X(Gamma) \
	X(Delta) \
	X(Epsilon)
CERIVE_UNION(Many, Debug, PartialEq)
#define Many_new(...) CERIVE_UNION_NEW(Many, __VA_ARGS__)

/*
 * Single -- a union with exactly 1 variant.
 */
#define Only_FIELDS(X) \
	X(int32_t, x) \
	X(int32_t, y)
CERIVE(Only, Struct, Debug, Constructor, Default, PartialEq)
#define Single_VARIANTS(X) X(Only)
CERIVE_UNION(Single, Debug, PartialEq)
#define Single_new(...) CERIVE_UNION_NEW(Single, __VA_ARGS__)

/*
 * ShapeWrap -- a struct containing a union (Shape) as a field.
 */
#define ShapeWrap_FIELDS(X) \
	X(Shape, inner) \
	X(int32_t, extra)
CERIVE(ShapeWrap, Struct, Debug, Constructor, Default, PartialEq)

/*
 * Triple -- three int32_t fields for ord short-circuit testing.
 */
#define Triple_FIELDS(X) \
	X(int32_t, a) \
	X(int32_t, b) \
	X(int32_t, c)
CERIVE(Triple, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)
