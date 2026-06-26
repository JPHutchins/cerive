#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Baseline: the same API and the same offset-cursor Debug a careful engineer
 * would write by hand, so the derived variants can be diffed against it. */

static inline size_t hw_rem(size_t const n, int const off) {
	return (off >= 0 && (size_t) off < n) ? n - (size_t) off : 0;
}
static inline char *hw_at(char *const buf, size_t const n, int const off) {
	return buf + ((off >= 0 && (size_t) off < n) ? (size_t) off : n);
}

typedef struct Point {
	int32_t x;
	int32_t y;
} Point;
static inline int Point_debug(Point const *const self, char *const buf, size_t const n) {
	int off = 0;
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "Point { ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "x=%" PRId32 " ", self->x);
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "y=%" PRId32 " ", self->y);
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "}");
	return off;
}
static inline Point Point_new(int32_t const x, int32_t const y) { return (Point){.x = x, .y = y}; }
static inline Point Point_default(void) { return (Point){}; }
static inline bool Point_eq(Point const *const a, Point const *const b) {
	return a->x == b->x && a->y == b->y;
}

typedef struct Line {
	Point a;
	Point b;
} Line;
static inline int Line_debug(Line const *const self, char *const buf, size_t const n) {
	int off = 0;
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "Line { ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "a=");
	off += Point_debug(&self->a, hw_at(buf, n, off), hw_rem(n, off));
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), " ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "b=");
	off += Point_debug(&self->b, hw_at(buf, n, off), hw_rem(n, off));
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), " ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "}");
	return off;
}
static inline Line Line_new(Point const a, Point const b) { return (Line){.a = a, .b = b}; }
static inline Line Line_default(void) { return (Line){}; }
static inline bool Line_eq(Line const *const a, Line const *const b) {
	return Point_eq(&a->a, &b->a) && Point_eq(&a->b, &b->b);
}

typedef struct Frame {
	Line edge;
	int32_t id;
} Frame;
static inline int Frame_debug(Frame const *const self, char *const buf, size_t const n) {
	int off = 0;
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "Frame { ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "edge=");
	off += Line_debug(&self->edge, hw_at(buf, n, off), hw_rem(n, off));
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), " ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "id=%" PRId32 " ", self->id);
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "}");
	return off;
}
static inline Frame Frame_new(Line const edge, int32_t const id) { return (Frame){.edge = edge, .id = id}; }
static inline Frame Frame_default(void) { return (Frame){}; }
static inline bool Frame_eq(Frame const *const a, Frame const *const b) {
	return Line_eq(&a->edge, &b->edge) && a->id == b->id;
}
