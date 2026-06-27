#pragma once

#include <stdint.h>

/*
 * Total order. T_cmp returns the lexicographic comparison of two values, field
 * by field, short-circuiting on the first inequality. SCALAR fields compare with
 * the overflow-safe three-way idiom (yielding -1/0/1, the enum values); STRUCT
 * fields recurse into their own _cmp.
 */

enum ordering : int8_t {
	ordering_less = -1,
	ordering_equal = 0,
	ordering_greater = 1,
};
