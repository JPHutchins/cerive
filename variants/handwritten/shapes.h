#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "sum.h"

/* Baseline: the same API a careful engineer would write by hand -- offset-cursor
 * Debug, recursive composition, switch-dispatched sum -- to diff the derives
 * against. Structs are not typedef'd (tag constants reuse their names). */

static inline size_t hw_rem(size_t const n, int const off) {
	return (off >= 0 && (size_t) off < n) ? n - (size_t) off : 0;
}
static inline char *hw_at(char *const buf, size_t const n, int const off) {
	return buf + ((off >= 0 && (size_t) off < n) ? (size_t) off : n);
}

struct Point {
	int32_t x;
	int32_t y;
};
static inline int Point_debug(struct Point const *const self, char *const buf, size_t const n) {
	int off = 0;
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "Point { ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "x=%" PRId32 " ", self->x);
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "y=%" PRId32 " ", self->y);
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "}");
	return off;
}
static inline struct Point Point_new(int32_t const x, int32_t const y) { return (struct Point){.x = x, .y = y}; }
static inline struct Point Point_default(void) { return (struct Point){}; }
static inline bool Point_eq(struct Point const *const a, struct Point const *const b) {
	return a->x == b->x && a->y == b->y;
}

struct Line {
	struct Point a;
	struct Point b;
};
static inline int Line_debug(struct Line const *const self, char *const buf, size_t const n) {
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
static inline struct Line Line_new(struct Point const a, struct Point const b) { return (struct Line){.a = a, .b = b}; }
static inline struct Line Line_default(void) { return (struct Line){}; }
static inline bool Line_eq(struct Line const *const a, struct Line const *const b) {
	return Point_eq(&a->a, &b->a) && Point_eq(&a->b, &b->b);
}

struct Frame {
	struct Line edge;
	int32_t id;
};
static inline int Frame_debug(struct Frame const *const self, char *const buf, size_t const n) {
	int off = 0;
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "Frame { ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "edge=");
	off += Line_debug(&self->edge, hw_at(buf, n, off), hw_rem(n, off));
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), " ");
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "id=%" PRId32 " ", self->id);
	off += snprintf(hw_at(buf, n, off), hw_rem(n, off), "}");
	return off;
}
static inline struct Frame Frame_new(struct Line const edge, int32_t const id) { return (struct Frame){.edge = edge, .id = id}; }
static inline struct Frame Frame_default(void) { return (struct Frame){}; }
static inline bool Frame_eq(struct Frame const *const a, struct Frame const *const b) {
	return Line_eq(&a->edge, &b->edge) && a->id == b->id;
}

enum Shape_tag { Point, Line, Frame };
struct Shape {
	union {
		struct Point Point;
		struct Line Line;
		struct Frame Frame;
	};
	enum Shape_tag tag;
};
static inline int Shape_debug(struct Shape const *const self, char *const buf, size_t const n) {
	switch (self->tag) {
		case Point:
			return Point_debug(&self->Point, buf, n);
		case Line:
			return Line_debug(&self->Line, buf, n);
		case Frame:
			return Frame_debug(&self->Frame, buf, n);
	}
	unreachable();
}
static inline bool Shape_eq(struct Shape const *const a, struct Shape const *const b) {
	if (a->tag != b->tag) {
		return false;
	}
	switch (a->tag) {
		case Point:
			return Point_eq(&a->Point, &b->Point);
		case Line:
			return Line_eq(&a->Line, &b->Line);
		case Frame:
			return Frame_eq(&a->Frame, &b->Frame);
	}
	unreachable();
}
