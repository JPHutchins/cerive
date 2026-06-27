#pragma once

#include <stdint.h>

/*
 * Total order. <T>_cmp returns the lexicographic comparison of two values, field
 * by field, short-circuiting on the first inequality. Scalar and pointer fields
 * compare with the overflow-safe three-way idiom (yielding -1/0/1, the enum
 * values); record fields recurse into their own _cmp.
 */

enum cerive_ordering : int8_t {
	cerive_less = -1,
	cerive_equal = 0,
	cerive_greater = 1,
};
