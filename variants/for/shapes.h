#pragma once

#include <stdint.h>

#include "derive_for.h"

#define Point_FIELDS (int32_t, x), (int32_t, y)
DERIVE_STRUCT(Point);
DERIVE_DEBUG(Point)
DERIVE_NEW(Point)
DERIVE_DEFAULT(Point)
DERIVE_PARTIAL_EQ(Point)
DERIVE_ORD(Point)
DERIVE_HASH(Point)

#define Line_FIELDS (Point, a), (Point, b)
DERIVE_STRUCT(Line);
DERIVE_DEBUG(Line)
DERIVE_NEW(Line)
DERIVE_DEFAULT(Line)
DERIVE_PARTIAL_EQ(Line)
DERIVE_ORD(Line)
DERIVE_HASH(Line)

#define Frame_FIELDS (Line, edge), (int32_t, id)
DERIVE_STRUCT(Frame);
DERIVE_DEBUG(Frame)
DERIVE_NEW(Frame)
DERIVE_DEFAULT(Frame)
DERIVE_PARTIAL_EQ(Frame)
DERIVE_ORD(Frame)
DERIVE_HASH(Frame)

#define Span_FIELDS (*, Point, first), (*, Point *, rows), (int32_t, len)
DERIVE_STRUCT(Span);
DERIVE_DEBUG(Span)
DERIVE_NEW(Span)
DERIVE_DEFAULT(Span)
DERIVE_PARTIAL_EQ(Span)
DERIVE_ORD(Span)
DERIVE_HASH(Span)

#define Boxed_FIELDS (Point const, origin), (int32_t, seq)
DERIVE_STRUCT(Boxed);
DERIVE_DEBUG(Boxed)
DERIVE_NEW(Boxed)
DERIVE_DEFAULT(Boxed)
DERIVE_PARTIAL_EQ(Boxed)
DERIVE_ORD(Boxed)
DERIVE_HASH(Boxed)

#define Shape_VARIANTS Point, Line, Frame
DERIVE_UNION(Shape);
DERIVE_UNION_DEBUG(Shape)
DERIVE_UNION_PARTIAL_EQ(Shape)
#define Shape_new(...) UNION_NEW(Shape, __VA_ARGS__)
