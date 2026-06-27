#pragma once

#include <stdint.h>

#include "derive_hybrid.h"

#define Point_FIELDS(X) X(SCALAR, int32_t, x) X(SCALAR, int32_t, y)
DERIVE_STRUCT(Point);
DERIVE_DEBUG(Point)
DERIVE_NEW(Point)
DERIVE_DEFAULT(Point)
DERIVE_PARTIAL_EQ(Point)
DERIVE_ORD(Point)

#define Line_FIELDS(X) X(STRUCT, Point, a) X(STRUCT, Point, b)
DERIVE_STRUCT(Line);
DERIVE_DEBUG(Line)
DERIVE_NEW(Line)
DERIVE_DEFAULT(Line)
DERIVE_PARTIAL_EQ(Line)
DERIVE_ORD(Line)

#define Frame_FIELDS(X) X(STRUCT, Line, edge) X(SCALAR, int32_t, id)
DERIVE_STRUCT(Frame);
DERIVE_DEBUG(Frame)
DERIVE_NEW(Frame)
DERIVE_DEFAULT(Frame)
DERIVE_PARTIAL_EQ(Frame)
DERIVE_ORD(Frame)

#define Span_FIELDS(X) X(PTR, Point, first) X(PTR, Point *, rows) X(SCALAR, int32_t, len)
DERIVE_STRUCT(Span);
DERIVE_DEBUG(Span)
DERIVE_NEW(Span)
DERIVE_DEFAULT(Span)
DERIVE_PARTIAL_EQ(Span)
DERIVE_ORD(Span)

#define Shape_VARIANTS(X) X(Point) X(Line) X(Frame)
DERIVE_UNION(Shape);
DERIVE_UNION_DEBUG(Shape)
DERIVE_UNION_PARTIAL_EQ(Shape)
#define Shape_new(...) UNION_NEW(Shape, __VA_ARGS__)
