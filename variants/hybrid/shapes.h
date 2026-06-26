#pragma once

#include <inttypes.h>
#include <stdint.h>

#include "derive_hybrid.h"

#define Point_FIELDS(X) X(SCALAR, int32_t, x, "%" PRId32) X(SCALAR, int32_t, y, "%" PRId32)
DERIVE_STRUCT(Point);
DERIVE_DEBUG(Point)
DERIVE_NEW(Point)
DERIVE_DEFAULT(Point)
DERIVE_PARTIAL_EQ(Point)

#define Line_FIELDS(X) X(STRUCT, Point, a) X(STRUCT, Point, b)
DERIVE_STRUCT(Line);
DERIVE_DEBUG(Line)
DERIVE_NEW(Line)
DERIVE_DEFAULT(Line)
DERIVE_PARTIAL_EQ(Line)

#define Frame_FIELDS(X) X(STRUCT, Line, edge) X(SCALAR, int32_t, id, "%" PRId32)
DERIVE_STRUCT(Frame);
DERIVE_DEBUG(Frame)
DERIVE_NEW(Frame)
DERIVE_DEFAULT(Frame)
DERIVE_PARTIAL_EQ(Frame)
