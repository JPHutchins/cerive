#pragma once

#include <inttypes.h>
#include <stdint.h>

#include "derive_hybrid.h"

#define Point_FIELDS(X) \
	X(int32_t, x, "%" PRId32) \
	X(int32_t, y, "%" PRId32)

DERIVE_STRUCT(Point);
DERIVE_DEBUG(Point)
DERIVE_NEW(Point)
DERIVE_DEFAULT(Point)
DERIVE_EQ(Point)
