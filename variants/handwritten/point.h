#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Point {
	int32_t x;
	int32_t y;
} Point;

static inline int Point_debug(Point const *const self, char *const buf, size_t const n) {
	return snprintf(buf, n, "Point { x=%" PRId32 " y=%" PRId32 " }", self->x, self->y);
}

static inline Point Point_new(int32_t const x, int32_t const y) {
	return (Point){.x = x, .y = y};
}

static inline Point Point_default(void) {
	return (Point){};
}

static inline bool Point_eq(Point const *const a, Point const *const b) {
	return a->x == b->x && a->y == b->y;
}
