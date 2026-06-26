#pragma once

#include <inttypes.h>
#include <stdint.h>

#include "derive_for.h"

#define Point_FIELDS (SCALAR, int32_t, x, "%" PRId32), (SCALAR, int32_t, y, "%" PRId32)
DERIVE_STRUCT(Point);
DERIVE_DEBUG(Point)
DERIVE_NEW(Point)
DERIVE_DEFAULT(Point)
DERIVE_PARTIAL_EQ(Point)

#define Line_FIELDS (STRUCT, Point, a), (STRUCT, Point, b)
DERIVE_STRUCT(Line);
DERIVE_DEBUG(Line)
DERIVE_NEW(Line)
DERIVE_DEFAULT(Line)
DERIVE_PARTIAL_EQ(Line)

#define Frame_FIELDS (STRUCT, Line, edge), (SCALAR, int32_t, id, "%" PRId32)
DERIVE_STRUCT(Frame);
DERIVE_DEBUG(Frame)
DERIVE_NEW(Frame)
DERIVE_DEFAULT(Frame)
DERIVE_PARTIAL_EQ(Frame)
