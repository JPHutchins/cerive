#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shapes.h"

/*
 * Stable entry points so each impl's `static inline` API is emitted and its
 * codegen can be diffed across impls: Point (flat), Frame (nested recursion),
 * Shape (tagged-union dispatch).
 */

int study_point_debug(struct Point const *const s, char *const b, size_t const n) { return Point_debug(s, b, n); }
struct Point study_point_new(int32_t const x, int32_t const y) { return Point_new(x, y); }
struct Point study_point_default(void) { return Point_default(); }
bool study_point_eq(struct Point const *const a, struct Point const *const b) { return Point_eq(a, b); }

int study_frame_debug(struct Frame const *const s, char *const b, size_t const n) { return Frame_debug(s, b, n); }
struct Frame study_frame_new(struct Line const e, int32_t const id) { return Frame_new(e, id); }
struct Frame study_frame_default(void) { return Frame_default(); }
bool study_frame_eq(struct Frame const *const a, struct Frame const *const b) { return Frame_eq(a, b); }

int study_shape_debug(struct Shape const *const s, char *const b, size_t const n) { return Shape_debug(s, b, n); }
bool study_shape_eq(struct Shape const *const a, struct Shape const *const b) { return Shape_eq(a, b); }
