#pragma once

#include <stdint.h>

#include "derive_hybrid.h"

#define Point_FIELDS(X) \
	X(int32_t, x) \
	X(int32_t, y)
DERIVE(Point, STRUCT, DEBUG, NEW, DEFAULT, PARTIAL_EQ, ORD, HASH)

#define Line_FIELDS(X) \
	X(Point, a) \
	X(Point, b)
DERIVE(Line, STRUCT, DEBUG, NEW, DEFAULT, PARTIAL_EQ, ORD, HASH)

#define Frame_FIELDS(X) \
	X(Line, edge) \
	X(int32_t, id)
DERIVE(Frame, STRUCT, DEBUG, NEW, DEFAULT, PARTIAL_EQ, ORD, HASH)

#define Span_FIELDS(X) \
	X(*, Point, first) \
	X(*, Point *, rows) \
	X(int32_t, len)
DERIVE(Span, STRUCT, DEBUG, NEW, DEFAULT, PARTIAL_EQ, ORD, HASH)

#define Boxed_FIELDS(X) \
	X(Point const, origin) \
	X(int32_t, seq)
DERIVE(Boxed, STRUCT, DEBUG, NEW, DEFAULT, PARTIAL_EQ, ORD, HASH)

#define Shape_VARIANTS(X) \
	X(Point) \
	X(Line) \
	X(Frame)
DERIVE_UNION(Shape, DEBUG, PARTIAL_EQ)
#define Shape_new(...) UNION_NEW(Shape, __VA_ARGS__)
