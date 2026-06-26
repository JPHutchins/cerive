#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shapes.h"

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			++fails; \
			printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
		} \
	} while (0)

static int flat_struct(void) {
	int fails = 0;

	Point const p = Point_new(3, 4);
	CHECK(p.x == 3 && p.y == 4);

	CHECK(Point_default().x == 0 && Point_default().y == 0);

	CHECK(Point_eq(&p, &(Point){.x = 3, .y = 4}));
	CHECK(!Point_eq(&p, &(Point){.x = 3, .y = 5}));

	char buf[32];
	Point_debug(&p, buf, sizeof buf);
	CHECK(strcmp(buf, "Point { x=3 y=4 }") == 0);

	return fails;
}

static int nested_composition(void) {
	int fails = 0;

	Frame const f = Frame_new(Line_new(Point_new(1, 2), Point_new(3, 4)), 7);
	CHECK(f.edge.a.x == 1 && f.edge.b.y == 4 && f.id == 7);

	Frame const same = Frame_new(Line_new(Point_new(1, 2), Point_new(3, 4)), 7);
	CHECK(Frame_eq(&f, &same));
	CHECK(!Frame_eq(&f, &(Frame){}));

	Frame const zero = Frame_default();
	CHECK(zero.edge.a.x == 0 && zero.edge.b.y == 0 && zero.id == 0);

	char buf[128];
	Frame_debug(&f, buf, sizeof buf);
	CHECK(strcmp(buf, "Frame { edge=Line { a=Point { x=1 y=2 } b=Point { x=3 y=4 } } id=7 }") == 0);

	return fails;
}

static int debug_buffer_contract(void) {
	int fails = 0;

	Point const p = Point_new(42, 7);
	char const *const expect = "Point { x=42 y=7 }";
	int const need = (int) strlen(expect);

	CHECK(Point_debug(&p, NULL, 0) == need);

	char full[32];
	CHECK(Point_debug(&p, full, sizeof full) == need);
	CHECK(strcmp(full, expect) == 0);

	char truncated[8];
	CHECK(Point_debug(&p, truncated, sizeof truncated) == need);
	CHECK(strlen(truncated) == sizeof truncated - 1);
	CHECK(strncmp(truncated, expect, sizeof truncated - 1) == 0);

	return fails;
}

static int sum_construct_and_compare(void) {
	int fails = 0;

	Shape const point = SUM_NEW(Shape, Point, .x = 1, .y = 2);
	Shape const frame = SUM_NEW(Shape, Frame, .edge = Line_new(Point_new(1, 2), Point_new(3, 4)), .id = 7);

	CHECK(SUM_IS(point, Point) && !SUM_IS(point, Frame));
	CHECK(point.Point.x == 1 && frame.Frame.id == 7);

	CHECK(Shape_eq(&point, &SUM_NEW(Shape, Point, .x = 1, .y = 2)));
	CHECK(!Shape_eq(&point, &SUM_NEW(Shape, Point, .x = 9, .y = 9)));
	CHECK(!Shape_eq(&point, &frame));

	char buf[128];
	Shape_debug(&point, buf, sizeof buf);
	CHECK(strcmp(buf, "Point { x=1 y=2 }") == 0);
	Shape_debug(&frame, buf, sizeof buf);
	CHECK(strcmp(buf, "Frame { edge=Line { a=Point { x=1 y=2 } b=Point { x=3 y=4 } } id=7 }") == 0);

	return fails;
}

static int sum_match(void) {
	int fails = 0;

	Shape const shapes[] = {
		SUM_NEW(Shape, Point, .x = 5, .y = 6),
		SUM_NEW(Shape, Line, .a = Point_new(1, 0), .b = Point_new(0, 0)),
		SUM_NEW(Shape, Frame, .edge = Line_default(), .id = 9),
	};
	int32_t const want[] = {5, 1, 9};

	for (size_t i = 0; i < sizeof shapes / sizeof shapes[0]; ++i) {
		int32_t got = -1;
		SUM_MATCH (shapes[i]) {
			SUM_CASE (shapes[i], Point, p) { got = p->x; }
			SUM_CASE (shapes[i], Line, l) { got = l->a.x; }
			SUM_CASE (shapes[i], Frame, f) { got = f->id; }
		}
		CHECK(got == want[i]);
	}

	return fails;
}

int main(void) {
	int const fails =
		flat_struct()
		+ nested_composition()
		+ debug_buffer_contract()
		+ sum_construct_and_compare()
		+ sum_match();

	puts(fails == 0 ? "all tests passed" : "FAILURES");
	return fails;
}
