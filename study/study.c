#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shapes.h"

/*
 * Stable entry points so each impl's `static inline` API is emitted and its
 * codegen can be diffed across impls: Point (flat), Frame (nested recursion),
 * Shape (tagged-union dispatch).
 */

int study_point_debug(Point const *const s, char *const b, size_t const n) { return Point_debug(s, b, n); }
Point study_point_new(int32_t const x, int32_t const y) { return Point_new(x, y); }
Point study_point_default(void) { return Point_default(); }
bool study_point_eq(Point const *const a, Point const *const b) { return Point_eq(a, b); }
enum ordering study_point_cmp(Point const *const a, Point const *const b) { return Point_cmp(a, b); }
size_t study_point_hash(Point const *const s) { return Point_hash(s); }

int study_frame_debug(Frame const *const s, char *const b, size_t const n) { return Frame_debug(s, b, n); }
Frame study_frame_new(Line const e, int32_t const id) { return Frame_new(e, id); }
Frame study_frame_default(void) { return Frame_default(); }
bool study_frame_eq(Frame const *const a, Frame const *const b) { return Frame_eq(a, b); }
enum ordering study_frame_cmp(Frame const *const a, Frame const *const b) { return Frame_cmp(a, b); }
size_t study_frame_hash(Frame const *const s) { return Frame_hash(s); }

int study_span_debug(Span const *const s, char *const b, size_t const n) { return Span_debug(s, b, n); }
Span study_span_new(Point *const first, Point **const rows, int32_t const len) { return Span_new(first, rows, len); }
bool study_span_eq(Span const *const a, Span const *const b) { return Span_eq(a, b); }
enum ordering study_span_cmp(Span const *const a, Span const *const b) { return Span_cmp(a, b); }
size_t study_span_hash(Span const *const s) { return Span_hash(s); }

int study_shape_debug(Shape const *const s, char *const b, size_t const n) { return Shape_debug(s, b, n); }
bool study_shape_eq(Shape const *const a, Shape const *const b) { return Shape_eq(a, b); }
