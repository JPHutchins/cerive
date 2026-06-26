#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "point.h"

/*
 * Stable, externally-visible entry points so each variant's `static inline`
 * implementation is actually emitted and its codegen can be compared. Both
 * variants expose the identical Point API, so any difference in the matrix
 * output is attributable to derive-vs-handwritten, not to this wrapper.
 */

int study_debug(Point const *const s, char *const b, size_t const n) {
	return Point_debug(s, b, n);
}

Point study_new(int32_t const x, int32_t const y) {
	return Point_new(x, y);
}

Point study_default(void) {
	return Point_default();
}

bool study_eq(Point const *const a, Point const *const b) {
	return Point_eq(a, b);
}
