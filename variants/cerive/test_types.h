#pragma once

#include "shapes.h"

typedef unsigned long unsigned_long;
typedef long long long_long;
typedef unsigned long long unsigned_long_long;

#define Empty_FIELDS(X)
CERIVE(Empty, Struct, Debug, Default, PartialEq, Ord, Hash)

#define TripleP_FIELDS(X) \
	X(*, Point *, ptr) \
	X(int32_t, seq)
CERIVE(TripleP, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

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

#define Many_VARIANTS(X) \
	X(Alpha) \
	X(Beta) \
	X(Gamma) \
	X(Delta) \
	X(Epsilon)
CERIVE_UNION(Many, Debug, PartialEq)
#define Many_new(...) CERIVE_UNION_NEW(Many, __VA_ARGS__)

#define Only_FIELDS(X) \
	X(int32_t, x) \
	X(int32_t, y)
CERIVE(Only, Struct, Debug, Constructor, Default, PartialEq)
#define Single_VARIANTS(X) X(Only)
CERIVE_UNION(Single, Debug, PartialEq)
#define Single_new(...) CERIVE_UNION_NEW(Single, __VA_ARGS__)

#define ShapeWrap_FIELDS(X) \
	X(Shape, inner) \
	X(int32_t, extra)
CERIVE(ShapeWrap, Struct, Debug, Constructor, Default, PartialEq)

#define NodeA_FIELDS(X) \
	X(int32_t, x) \
	X(int32_t, y)
CERIVE(NodeA, Struct, Debug, Constructor, Default, PartialEq)

#define NodeB_FIELDS(X) \
	X(int32_t, a) \
	X(int32_t, b)
CERIVE(NodeB, Struct, Debug, Constructor, Default, PartialEq)

#define NodeC_FIELDS(X) X(int32_t, id)
CERIVE(NodeC, Struct, Debug, Default, PartialEq)

#define Inner_VARIANTS(X) \
	X(NodeA) \
	X(NodeB)
CERIVE_UNION(Inner, Debug, PartialEq)
#define Inner_new(...) CERIVE_UNION_NEW(Inner, __VA_ARGS__)

#define Outer_VARIANTS(X) \
	X(Inner) \
	X(NodeC)
CERIVE_UNION(Outer, Debug, PartialEq)
#define Outer_new(...) CERIVE_UNION_NEW(Outer, __VA_ARGS__)

#define Triple_FIELDS(X) \
	X(int32_t, a) \
	X(int32_t, b) \
	X(int32_t, c)
CERIVE(Triple, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)
