#pragma once

#include <inttypes.h>
#include <stdint.h>

#include "derive.h"

#define Point_FIELDS (int32_t, x, "%" PRId32), (int32_t, y, "%" PRId32)

DERIVE_STRUCT(Point);
DERIVE_DEBUG(Point)
DERIVE_NEW(Point)
DERIVE_DEFAULT(Point)
DERIVE_EQ(Point)
