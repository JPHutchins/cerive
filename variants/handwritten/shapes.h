#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <cerive/buf.h>
#include <cerive/hash.h>
#include <cerive/ord.h>
#include <cerive/union.h>

/* Baseline: the same API a careful engineer would write by hand -- offset-cursor
 * Debug, recursive composition, switch-dispatched sum -- to diff the derives
 * against. It uses cerive's shared runtime helpers (cerive_buf_*, cerive_hash_*),
 * exactly as the derived code does, so the comparison isolates the generated
 * logic rather than the primitives. */

typedef struct Point {
	int32_t x;
	int32_t y;
} Point;
static inline int Point_debug(Point const * const self, char * const buf, size_t const n) {
	int off = 0;
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "Point { ");
	off += snprintf(
		cerive_buf_at(buf, n, off),
		cerive_buf_remaining(n, off),
		"x=%" PRId32 " ",
		self->x
	);
	off += snprintf(
		cerive_buf_at(buf, n, off),
		cerive_buf_remaining(n, off),
		"y=%" PRId32 " ",
		self->y
	);
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "}");
	return off;
}
static inline Point Point_new(int32_t const x, int32_t const y) { return (Point) {.x = x, .y = y}; }
static inline Point Point_default(void) { return (Point) {}; }
static inline bool Point_eq(Point const * const a, Point const * const b) {
	return a->x == b->x && a->y == b->y;
}
static inline enum cerive_ordering Point_cmp(Point const * const a, Point const * const b) {
	{
		enum cerive_ordering const o = (a->x > b->x) - (a->x < b->x);
		if (o != cerive_equal) {
			return o;
		}
	}
	{
		enum cerive_ordering const o = (a->y > b->y) - (a->y < b->y);
		if (o != cerive_equal) {
			return o;
		}
	}
	return cerive_equal;
}
static inline size_t Point_hash(Point const * const self) {
	size_t h = cerive_hash_offset;
	h = cerive_hash_bytes(h, &self->x, sizeof self->x);
	h = cerive_hash_bytes(h, &self->y, sizeof self->y);
	return h;
}

typedef struct Line {
	Point a;
	Point b;
} Line;
static inline int Line_debug(Line const * const self, char * const buf, size_t const n) {
	int off = 0;
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "Line { ");
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "a=");
	off += Point_debug(&self->a, cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off));
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), " ");
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "b=");
	off += Point_debug(&self->b, cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off));
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), " ");
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "}");
	return off;
}
static inline Line Line_new(Point const a, Point const b) { return (Line) {.a = a, .b = b}; }
static inline Line Line_default(void) { return (Line) {}; }
static inline bool Line_eq(Line const * const a, Line const * const b) {
	return Point_eq(&a->a, &b->a) && Point_eq(&a->b, &b->b);
}
static inline enum cerive_ordering Line_cmp(Line const * const a, Line const * const b) {
	{
		enum cerive_ordering const o = Point_cmp(&a->a, &b->a);
		if (o != cerive_equal) {
			return o;
		}
	}
	{
		enum cerive_ordering const o = Point_cmp(&a->b, &b->b);
		if (o != cerive_equal) {
			return o;
		}
	}
	return cerive_equal;
}
static inline size_t Line_hash(Line const * const self) {
	size_t h = cerive_hash_offset;
	h = cerive_hash_mix(h, Point_hash(&self->a));
	h = cerive_hash_mix(h, Point_hash(&self->b));
	return h;
}

typedef struct Frame {
	Line edge;
	int32_t id;
} Frame;
static inline int Frame_debug(Frame const * const self, char * const buf, size_t const n) {
	int off = 0;
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "Frame { ");
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "edge=");
	off += Line_debug(&self->edge, cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off));
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), " ");
	off += snprintf(
		cerive_buf_at(buf, n, off),
		cerive_buf_remaining(n, off),
		"id=%" PRId32 " ",
		self->id
	);
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "}");
	return off;
}
static inline Frame Frame_new(Line const edge, int32_t const id) { return (Frame) {.edge = edge, .id = id}; }
static inline Frame Frame_default(void) { return (Frame) {}; }
static inline bool Frame_eq(Frame const * const a, Frame const * const b) {
	return Line_eq(&a->edge, &b->edge) && a->id == b->id;
}
static inline enum cerive_ordering Frame_cmp(Frame const * const a, Frame const * const b) {
	{
		enum cerive_ordering const o = Line_cmp(&a->edge, &b->edge);
		if (o != cerive_equal) {
			return o;
		}
	}
	{
		enum cerive_ordering const o = (a->id > b->id) - (a->id < b->id);
		if (o != cerive_equal) {
			return o;
		}
	}
	return cerive_equal;
}
static inline size_t Frame_hash(Frame const * const self) {
	size_t h = cerive_hash_offset;
	h = cerive_hash_mix(h, Line_hash(&self->edge));
	h = cerive_hash_bytes(h, &self->id, sizeof self->id);
	return h;
}

typedef struct Span {
	Point *first;
	Point **rows;
	int32_t len;
} Span;
static inline int Span_debug(Span const * const self, char * const buf, size_t const n) {
	int off = 0;
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "Span { ");
	off += snprintf(
		cerive_buf_at(buf, n, off),
		cerive_buf_remaining(n, off),
		"first=%p ",
		(void *) self->first
	);
	off += snprintf(
		cerive_buf_at(buf, n, off),
		cerive_buf_remaining(n, off),
		"rows=%p ",
		(void *) self->rows
	);
	off += snprintf(
		cerive_buf_at(buf, n, off),
		cerive_buf_remaining(n, off),
		"len=%" PRId32 " ",
		self->len
	);
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "}");
	return off;
}
static inline Span Span_new(Point *const first, Point **const rows, int32_t const len) {
	return (Span) {.first = first, .rows = rows, .len = len};
}
static inline Span Span_default(void) { return (Span) {}; }
static inline bool Span_eq(Span const * const a, Span const * const b) {
	return a->first == b->first && a->rows == b->rows && a->len == b->len;
}
static inline enum cerive_ordering Span_cmp(Span const * const a, Span const * const b) {
	{
		enum cerive_ordering const o = (a->first > b->first) - (a->first < b->first);
		if (o != cerive_equal) {
			return o;
		}
	}
	{
		enum cerive_ordering const o = (a->rows > b->rows) - (a->rows < b->rows);
		if (o != cerive_equal) {
			return o;
		}
	}
	{
		enum cerive_ordering const o = (a->len > b->len) - (a->len < b->len);
		if (o != cerive_equal) {
			return o;
		}
	}
	return cerive_equal;
}
static inline size_t Span_hash(Span const * const self) {
	size_t h = cerive_hash_offset;
	h = cerive_hash_bytes(h, &self->first, sizeof self->first);
	h = cerive_hash_bytes(h, &self->rows, sizeof self->rows);
	h = cerive_hash_bytes(h, &self->len, sizeof self->len);
	return h;
}

typedef struct Boxed {
	Point const origin;
	int32_t seq;
} Boxed;
static inline int Boxed_debug(Boxed const * const self, char * const buf, size_t const n) {
	int off = 0;
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "Boxed { ");
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "origin=");
	off += Point_debug(&self->origin, cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off));
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), " ");
	off += snprintf(
		cerive_buf_at(buf, n, off),
		cerive_buf_remaining(n, off),
		"seq=%" PRId32 " ",
		self->seq
	);
	off += snprintf(cerive_buf_at(buf, n, off), cerive_buf_remaining(n, off), "}");
	return off;
}
static inline Boxed Boxed_new(Point const origin, int32_t const seq) {
	return (Boxed) {.origin = origin, .seq = seq};
}
static inline Boxed Boxed_default(void) { return (Boxed) {}; }
static inline bool Boxed_eq(Boxed const * const a, Boxed const * const b) {
	return Point_eq(&a->origin, &b->origin) && a->seq == b->seq;
}
static inline enum cerive_ordering Boxed_cmp(Boxed const * const a, Boxed const * const b) {
	{
		enum cerive_ordering const o = Point_cmp(&a->origin, &b->origin);
		if (o != cerive_equal) {
			return o;
		}
	}
	{
		enum cerive_ordering const o = (a->seq > b->seq) - (a->seq < b->seq);
		if (o != cerive_equal) {
			return o;
		}
	}
	return cerive_equal;
}
static inline size_t Boxed_hash(Boxed const * const self) {
	size_t h = cerive_hash_offset;
	h = cerive_hash_mix(h, Point_hash(&self->origin));
	h = cerive_hash_bytes(h, &self->seq, sizeof self->seq);
	return h;
}

enum Shape_tag : uint8_t { Point_tag, Line_tag, Frame_tag };
typedef struct Shape {
	union {
		Point Point;
		Line Line;
		Frame Frame;
	};
	enum Shape_tag tag;
} Shape;
static inline int Shape_debug(Shape const * const self, char * const buf, size_t const n) {
	switch (self->tag) {
		case Point_tag:
			return Point_debug(&self->Point, buf, n);
		case Line_tag:
			return Line_debug(&self->Line, buf, n);
		case Frame_tag:
			return Frame_debug(&self->Frame, buf, n);
	}
	unreachable();
}
static inline bool Shape_eq(Shape const * const a, Shape const * const b) {
	if (a->tag != b->tag) {
		return false;
	}
	switch (a->tag) {
		case Point_tag:
			return Point_eq(&a->Point, &b->Point);
		case Line_tag:
			return Line_eq(&a->Line, &b->Line);
		case Frame_tag:
			return Frame_eq(&a->Frame, &b->Frame);
	}
	unreachable();
}

#define Shape_new(...) CERIVE_UNION_NEW(Shape, __VA_ARGS__)
