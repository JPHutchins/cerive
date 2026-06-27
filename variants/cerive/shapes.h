#pragma once

#include <stdint.h>

#include <cerive/cerive.h>

#define Point_FIELDS(X) \
	X(int32_t, x) \
	X(int32_t, y)
CERIVE(Point, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define Line_FIELDS(X) \
	X(Point, a) \
	X(Point, b)
CERIVE(Line, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define Frame_FIELDS(X) \
	X(Line, edge) \
	X(int32_t, id)
CERIVE(Frame, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define Span_FIELDS(X) \
	X(*, Point, first) \
	X(*, Point *, rows) \
	X(int32_t, len)
CERIVE(Span, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define Boxed_FIELDS(X) \
	X(Point const, origin) \
	X(int32_t, seq)
CERIVE(Boxed, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define Shape_VARIANTS(X) \
	X(Point) \
	X(Line) \
	X(Frame)
CERIVE_UNION(Shape, Debug, PartialEq)
#define Shape_new(...) CERIVE_UNION_NEW(Shape, __VA_ARGS__)
