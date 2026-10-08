#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "shapes.h"

int study_point_debug(Point const * const s, size_t const n, char b[static const n]) {
	return Point_debug(s, n, b);
}
int study_point_debug_len(Point const * const s) {
	return Point_debug_len(s);
}
Point study_point_new(int32_t const x, int32_t const y) {
	return Point_new(x, y);
}
Point study_point_new_lit(int32_t const x, int32_t const y) {
	return CERIVE_NEW(Point, .x = x, .y = y);
}
Point study_point_default(void) {
	return Point_default();
}
bool study_point_eq(Point const * const a, Point const * const b) {
	return Point_eq(a, b);
}
enum cerive_ordering study_point_cmp(Point const * const a, Point const * const b) {
	return Point_cmp(a, b);
}
size_t study_point_hash(Point const * const s) {
	return Point_hash(s);
}

int study_frame_debug(Frame const * const s, size_t const n, char b[static const n]) {
	return Frame_debug(s, n, b);
}
int study_frame_debug_len(Frame const * const s) {
	return Frame_debug_len(s);
}
Frame study_frame_new(Line const e, int32_t const id) {
	return Frame_new(e, id);
}
Frame study_frame_new_lit(Line const e, int32_t const id) {
	return CERIVE_NEW(Frame, .edge = e, .id = id);
}
Frame study_frame_default(void) {
	return Frame_default();
}
bool study_frame_eq(Frame const * const a, Frame const * const b) {
	return Frame_eq(a, b);
}
enum cerive_ordering study_frame_cmp(Frame const * const a, Frame const * const b) {
	return Frame_cmp(a, b);
}
size_t study_frame_hash(Frame const * const s) {
	return Frame_hash(s);
}

int study_span_debug(Span const * const s, size_t const n, char b[static const n]) {
	return Span_debug(s, n, b);
}
int study_span_debug_len(Span const * const s) {
	return Span_debug_len(s);
}
Span study_span_new(Point * const first, Point * * const rows, int32_t const len) {
	return Span_new(first, rows, len);
}
bool study_span_eq(Span const * const a, Span const * const b) {
	return Span_eq(a, b);
}
enum cerive_ordering study_span_cmp(Span const * const a, Span const * const b) {
	return Span_cmp(a, b);
}
size_t study_span_hash(Span const * const s) {
	return Span_hash(s);
}

int study_boxed_debug(Boxed const * const s, size_t const n, char b[static const n]) {
	return Boxed_debug(s, n, b);
}
int study_boxed_debug_len(Boxed const * const s) {
	return Boxed_debug_len(s);
}
Boxed study_boxed_new(Point const origin, int32_t const seq) {
	return Boxed_new(origin, seq);
}
bool study_boxed_eq(Boxed const * const a, Boxed const * const b) {
	return Boxed_eq(a, b);
}
enum cerive_ordering study_boxed_cmp(Boxed const * const a, Boxed const * const b) {
	return Boxed_cmp(a, b);
}
size_t study_boxed_hash(Boxed const * const s) {
	return Boxed_hash(s);
}

int study_shape_debug(Shape const * const s, size_t const n, char b[static const n]) {
	return Shape_debug(s, n, b);
}
int study_shape_debug_len(Shape const * const s) {
	return Shape_debug_len(s);
}
bool study_shape_eq(Shape const * const a, Shape const * const b) {
	return Shape_eq(a, b);
}
