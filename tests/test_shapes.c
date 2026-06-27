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

static int total_order(void) {
	int fails = 0;

	Point const p = Point_new(3, 4);
	CHECK(Point_cmp(&p, &(Point){.x = 3, .y = 4}) == ordering_equal);
	CHECK(Point_cmp(&p, &(Point){.x = 3, .y = 5}) == ordering_less);
	CHECK(Point_cmp(&p, &(Point){.x = 3, .y = 3}) == ordering_greater);
	CHECK(Point_cmp(&p, &(Point){.x = 4, .y = 0}) == ordering_less);
	CHECK(Point_cmp(&p, &(Point){.x = 2, .y = 9}) == ordering_greater);

	Frame const f = Frame_new(Line_new(Point_new(1, 2), Point_new(3, 4)), 7);
	CHECK(Frame_cmp(&f, &f) == ordering_equal);
	CHECK(Frame_cmp(&f, &(Frame){.edge = {.a = {1, 2}, .b = {3, 4}}, .id = 8}) == ordering_less);
	CHECK(Frame_cmp(&f, &(Frame){.edge = {.a = {1, 2}, .b = {3, 3}}, .id = 0}) == ordering_greater);

	return fails;
}

static int pointer_fields(void) {
	int fails = 0;

	Point p0 = Point_new(1, 2);
	Point p1 = Point_new(3, 4);
	Point *rows[] = {&p0, &p1};

	Span const s = Span_new(&p0, rows, 2);
	CHECK(s.first == &p0 && s.rows == rows && s.len == 2);

	CHECK(Span_eq(&s, &(Span){.first = &p0, .rows = rows, .len = 2}));
	CHECK(!Span_eq(&s, &(Span){.first = &p1, .rows = rows, .len = 2}));
	CHECK(!Span_eq(&s, &(Span){.first = &p0, .rows = rows, .len = 9}));

	CHECK(Span_cmp(&s, &s) == ordering_equal);
	CHECK(Span_cmp(&s, &(Span){.first = &p0, .rows = rows, .len = 3}) == ordering_less);
	CHECK(Span_cmp(&s, &(Span){.first = &p0, .rows = rows, .len = 1}) == ordering_greater);

	Span const zero = Span_default();
	CHECK(zero.first == NULL && zero.rows == NULL && zero.len == 0);

	char buf[64];
	int const need = Span_debug(&s, buf, sizeof buf);
	CHECK(need == (int) strlen(buf));
	CHECK(strncmp(buf, "Span { first=", 13) == 0);
	CHECK(Span_debug(&s, NULL, 0) == need);

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

static int union_construct_and_compare(void) {
	int fails = 0;

	Shape const point = Shape_new(Point, .x = 1, .y = 2);
	Shape const frame = Shape_new(Frame, .edge = Line_new(Point_new(1, 2), Point_new(3, 4)), .id = 7);

	CHECK(UNION_IS(point, Point) && !UNION_IS(point, Frame));
	CHECK(point.Point.x == 1 && frame.Frame.id == 7);

	CHECK(Shape_eq(&point, &Shape_new(Point, .x = 1, .y = 2)));
	CHECK(!Shape_eq(&point, &Shape_new(Point, .x = 9, .y = 9)));
	CHECK(!Shape_eq(&point, &frame));

	char buf[128];
	Shape_debug(&point, buf, sizeof buf);
	CHECK(strcmp(buf, "Point { x=1 y=2 }") == 0);
	Shape_debug(&frame, buf, sizeof buf);
	CHECK(strcmp(buf, "Frame { edge=Line { a=Point { x=1 y=2 } b=Point { x=3 y=4 } } id=7 }") == 0);

	return fails;
}

static int union_match(void) {
	int fails = 0;

	Shape const shapes[] = {
		Shape_new(Point, .x = 5, .y = 6),
		Shape_new(Line, .a = Point_new(1, 0), .b = Point_new(0, 0)),
		Shape_new(Frame, .edge = Line_default(), .id = 9),
	};
	int32_t const want[] = {5, 1, 9};

	for (size_t i = 0; i < sizeof shapes / sizeof shapes[0]; ++i) {
		int32_t got = -1;
		MATCH (shapes[i]) {
			CASE (Point, p) { got = p->x; }
			CASE (Line, l) { got = l->a.x; }
			CASE (Frame, f) { got = f->id; }
		}
		CHECK(got == want[i]);
	}

	return fails;
}

int main(void) {
	int const fails =
		flat_struct()
		+ total_order()
		+ pointer_fields()
		+ nested_composition()
		+ debug_buffer_contract()
		+ union_construct_and_compare()
		+ union_match();

	puts(fails == 0 ? "all tests passed" : "FAILURES");
	return fails;
}
