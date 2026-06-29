#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "shapes.h"
#if __has_include("test_types.h")
#include "test_types.h"
#define CERIVE_HAS_EXTRA_TYPES 1
#endif

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
	CHECK(Point_cmp(&p, &(Point){.x = 3, .y = 4}) == cerive_equal);
	CHECK(Point_cmp(&p, &(Point){.x = 3, .y = 5}) == cerive_less);
	CHECK(Point_cmp(&p, &(Point){.x = 3, .y = 3}) == cerive_greater);
	CHECK(Point_cmp(&p, &(Point){.x = 4, .y = 0}) == cerive_less);
	CHECK(Point_cmp(&p, &(Point){.x = 2, .y = 9}) == cerive_greater);

	Frame const f = Frame_new(Line_new(Point_new(1, 2), Point_new(3, 4)), 7);
	CHECK(Frame_cmp(&f, &f) == cerive_equal);
	CHECK(Frame_cmp(&f, &(Frame){.edge = {.a = {1, 2}, .b = {3, 4}}, .id = 8}) == cerive_less);
	CHECK(Frame_cmp(&f, &(Frame){.edge = {.a = {1, 2}, .b = {3, 3}}, .id = 0}) == cerive_greater);

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

	CHECK(Span_cmp(&s, &s) == cerive_equal);
	CHECK(Span_cmp(&s, &(Span){.first = &p0, .rows = rows, .len = 3}) == cerive_less);
	CHECK(Span_cmp(&s, &(Span){.first = &p0, .rows = rows, .len = 1}) == cerive_greater);

	Span const zero = Span_default();
	CHECK(zero.first == NULL && zero.rows == NULL && zero.len == 0);

	char buf[64];
	int const need = Span_debug(&s, buf, sizeof buf);
	CHECK(need == (int) strlen(buf));
	CHECK(strncmp(buf, "Span { first=", 13) == 0);
	CHECK(Span_debug(&s, NULL, 0) == need);

	return fails;
}

static int hashing(void) {
	int fails = 0;

	Point const p = Point_new(3, 4);
	CHECK(Point_hash(&p) == Point_hash(&(Point){.x = 3, .y = 4}));
	CHECK(Point_hash(&p) != Point_hash(&(Point){.x = 4, .y = 3}));

	Frame const f = Frame_new(Line_new(Point_new(1, 2), Point_new(3, 4)), 7);
	CHECK(Frame_hash(&f) == Frame_hash(&(Frame){.edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7}));
	CHECK(Frame_hash(&f) != Frame_hash(&(Frame){.edge = {.a = {1, 2}, .b = {3, 4}}, .id = 8}));

	return fails;
}

static int const_usage(void) {
	int fails = 0;

	Frame const f = CERIVE_NEW(Frame, .edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7);
	Frame const * const pf = &f;
	CHECK(pf->id == 7);

	char buf[128];
	CHECK(Frame_debug(&f, buf, sizeof buf) > 0);
	CHECK(Frame_eq(&f, &f));
	CHECK(Frame_cmp(&f, &f) == cerive_equal);
	CHECK(Frame_hash(&f) == Frame_hash(&f));

	Point const pts[] = {CERIVE_NEW(Point, .x = 1, .y = 2), CERIVE_NEW(Point, .x = 3, .y = 4)};
	CHECK(Point_eq(&pts[0], &(Point const){.x = 1, .y = 2}));

	CHECK(Frame_eq(&f, &(Frame const){.edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7}));

	return fails;
}

static int const_struct_member(void) {
	int fails = 0;

	Boxed const b = Boxed_new((Point){.x = 1, .y = 2}, 7);
	CHECK(b.origin.x == 1 && b.origin.y == 2 && b.seq == 7);

	CHECK(Boxed_eq(&b, &(Boxed){.origin = {1, 2}, .seq = 7}));
	CHECK(!Boxed_eq(&b, &(Boxed){.origin = {1, 2}, .seq = 8}));
	CHECK(Boxed_cmp(&b, &b) == cerive_equal);
	CHECK(Boxed_cmp(&b, &(Boxed){.origin = {1, 2}, .seq = 8}) == cerive_less);
	CHECK(Boxed_hash(&b) == Boxed_hash(&(Boxed){.origin = {1, 2}, .seq = 7}));

	Boxed const z = Boxed_default();
	CHECK(z.origin.x == 0 && z.seq == 0);

	char buf[64];
	Boxed_debug(&b, buf, sizeof buf);
	CHECK(strcmp(buf, "Boxed { origin=Point { x=1 y=2 } seq=7 }") == 0);

	return fails;
}

static int fluent_construct(void) {
	int fails = 0;

	Point const p = CERIVE_NEW(Point, .x = 3, .y = 4);
	CHECK(Point_eq(&p, &(Point){.x = 3, .y = 4}));

	Frame const f = CERIVE_NEW(Frame, .edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7);
	CHECK(Frame_eq(&f, &(Frame){.edge = {.a = {1, 2}, .b = {3, 4}}, .id = 7}));

	Frame const partial = CERIVE_NEW(Frame, .id = 7);
	CHECK(Frame_eq(&partial, &(Frame){.id = 7}));
	CHECK(partial.edge.a.x == 0 && partial.edge.b.y == 0);

	Point a = CERIVE_NEW(Point, .x = 1, .y = 2);
	Point *rows[] = {&a};
	Span const s = CERIVE_NEW(Span, .first = &a, .rows = rows, .len = 1);
	CHECK(s.first == &a && s.rows == rows && s.len == 1);

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
	char const * const expect = "Point { x=42 y=7 }";
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
	Shape const frame = Shape_new(
		Frame,
		.edge = Line_new(Point_new(1, 2), Point_new(3, 4)),
		.id = 7
	);

	CHECK(CERIVE_IS(point, Point) && !CERIVE_IS(point, Frame));
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

#if CERIVE_HAS_EXTRA_TYPES
static int triple_pointer(void) {
	int fails = 0;

	Point p = Point_new(1, 2);
	Point *p_ptr = &p;
	Point **pp = &p_ptr;

	TripleP const t = TripleP_new(pp, 10);
	CHECK(t.ptr == pp && t.seq == 10);
	CHECK((**t.ptr).x == 1);

	CHECK(TripleP_eq(&t, &(TripleP){.ptr = pp, .seq = 10}));
	CHECK(!TripleP_eq(&t, &(TripleP){.ptr = pp, .seq = 99}));

	CHECK(TripleP_cmp(&t, &(TripleP){.ptr = pp, .seq = 10}) == cerive_equal);
	CHECK(TripleP_cmp(&t, &(TripleP){.ptr = pp, .seq = 11}) == cerive_less);
	CHECK(TripleP_cmp(&t, &(TripleP){.ptr = pp, .seq = 9}) == cerive_greater);

	CHECK(TripleP_hash(&t) == TripleP_hash(&(TripleP){.ptr = pp, .seq = 10}));
	CHECK(TripleP_hash(&t) != TripleP_hash(&(TripleP){.ptr = pp, .seq = 0}));

	char buf[64];
	TripleP_debug(&t, buf, sizeof buf);
	CHECK(strstr(buf, "TripleP { ptr=") == buf);

	return fails;
}

static int many_variant_union(void) {
	int fails = 0;

	Many const m0 = Many_new(Alpha, .val = 10);
	Many const m1 = Many_new(Beta, .val = 2.5f);
	Many const m2 = Many_new(Gamma, .val = 3.14);
	Many const m3 = Many_new(Delta, .val = 'X');
	Many const m4 = Many_new(Epsilon, .val = 999);

	CHECK(CERIVE_IS(m0, Alpha) && !CERIVE_IS(m0, Beta));
	CHECK(m0.Alpha.val == 10);
	CHECK(m1.Beta.val == 2.5);
	CHECK(m2.Gamma.val == 3.14);
	CHECK(m3.Delta.val == 'X');
	CHECK(m4.Epsilon.val == 999);

	CHECK(Many_eq(&m0, &Many_new(Alpha, .val = 10)));
	CHECK(!Many_eq(&m0, &Many_new(Alpha, .val = 99)));
	CHECK(!Many_eq(&m0, &m1));

	MATCH (m0) {
		CASE (Alpha, a) { CHECK(a->val == 10); }
		CASE (Beta, b) { CHECK(!"should not reach"); }
		CASE (Gamma, g) { CHECK(!"should not reach"); }
		CASE (Delta, d) { CHECK(!"should not reach"); }
		CASE (Epsilon, e) { CHECK(!"should not reach"); }
	}

	return fails;
}

static int single_variant_union(void) {
	int fails = 0;

	Single const s = Single_new(Only, .x = 7, .y = 8);
	CHECK(CERIVE_IS(s, Only));
	CHECK(s.Only.x == 7 && s.Only.y == 8);

	CHECK(Single_eq(&s, &Single_new(Only, .x = 7, .y = 8)));
	CHECK(!Single_eq(&s, &Single_new(Only, .x = 7, .y = 9)));

	return fails;
}

static int nested_union(void) {
	int fails = 0;

	Shape const point = Shape_new(Point, .x = 1, .y = 2);
	ShapeWrap const w = ShapeWrap_new(point, 42);

	CHECK(w.inner.Point.x == 1 && w.extra == 42);

	CHECK(ShapeWrap_eq(&w, &(ShapeWrap){.inner = point, .extra = 42}));
	CHECK(!ShapeWrap_eq(&w, &(ShapeWrap){.inner = point, .extra = 99}));

	char buf[128];
	ShapeWrap_debug(&w, buf, sizeof buf);
	CHECK(strcmp(buf, "ShapeWrap { inner=Point { x=1 y=2 } extra=42 }") == 0);

	return fails;
}

static int ord_short_circuit_3fields(void) {
	int fails = 0;

	Triple const t = Triple_new(1, 2, 3);

	CHECK(Triple_cmp(&t, &(Triple){.a = 1, .b = 2, .c = 3}) == cerive_equal);
	CHECK(Triple_cmp(&t, &(Triple){.a = 1, .b = 2, .c = 5}) == cerive_less);
	CHECK(Triple_cmp(&t, &(Triple){.a = 1, .b = 2, .c = 0}) == cerive_greater);

	/* Different 'a' should short-circuit before checking 'b' or 'c'. */
	CHECK(Triple_cmp(&t, &(Triple){.a = 0, .b = 9, .c = 9}) == cerive_greater);
	CHECK(Triple_cmp(&t, &(Triple){.a = 2, .b = 0, .c = 0}) == cerive_less);

	/* Same 'a', different 'b' should short-circuit before 'c'. */
	CHECK(Triple_cmp(&t, &(Triple){.a = 1, .b = 1, .c = 9}) == cerive_greater);
	CHECK(Triple_cmp(&t, &(Triple){.a = 1, .b = 3, .c = 0}) == cerive_less);

	return fails;
}

static int debug_exact_buffer(void) {
	int fails = 0;

	Triple const t = (Triple){.a = 100, .b = 200, .c = 300};
	char const * const expect = "Triple { a=100 b=200 c=300 }";
	int const need = (int) strlen(expect);

	CHECK(Triple_debug(&t, NULL, 0) == need);

	/* Buffer of exactly need+1 bytes: fits with NUL terminator. */
	char exact[need + 1];
	CHECK(Triple_debug(&t, exact, sizeof exact) == need);
	CHECK(strcmp(exact, expect) == 0);
	CHECK(exact[need] == '\0');

	return fails;
}

static int match_with_break(void) {
	int fails = 0;

	Shape const shapes[] = {
		Shape_new(Point, .x = 5, .y = 6),
	};

	/* A break inside an inner for-loop inside a CASE body must only break the
	 * inner loop, not the MATCH construct (whose for-loop trick wraps the
	 * switch). */
	int32_t got = -1;
	MATCH (shapes[0]) {
		CASE (Point, p) {
			for (int j = 0; j < 10; ++j) {
				if (j >= 1) {
					break;
				}
			}
			got = p->x;
		}
		CASE (Line, l) {
			got = -2;
		}
		CASE (Frame, f) {
			got = -3;
		}
	}
	CHECK(got == 5);

	return fails;
}
static int nested_union_variant(void) {
	int fails = 0;

	Outer const o = Outer_new(Inner, .NodeA = {.x = 3, .y = 4});
	CHECK(CERIVE_IS(o, Inner));
	CHECK(o.Inner.NodeA.x == 3 && o.Inner.NodeA.y == 4);

	CHECK(Outer_eq(&o, &Outer_new(Inner, .NodeA = {.x = 3, .y = 4})));
	CHECK(!Outer_eq(&o, &Outer_new(Inner, .NodeA = {.x = 9, .y = 9})));
	CHECK(!Outer_eq(&o, &Outer_new(NodeC, .id = 1)));

	/* Construct with the non-union variant (NodeC, a plain struct). */
	Outer const c = Outer_new(NodeC, .id = 5);
	CHECK(CERIVE_IS(c, NodeC));
	CHECK(c.NodeC.id == 5);

	char buf[128];
	Outer_debug(&o, buf, sizeof buf);
	CHECK(strcmp(buf, "NodeA { x=3 y=4 }") == 0);
	Outer_debug(&c, buf, sizeof buf);
	CHECK(strcmp(buf, "NodeC { id=5 }") == 0);

	return fails;
}
static int if_let(void) {
	int fails = 0;

	Shape const point = Shape_new(Point, .x = 7, .y = 8);
	Shape const frame = Shape_new(Frame, .edge = Line_default(), .id = 3);
	int32_t got = -1;

	IF_LET (point, Point, p) {
		got = p->x;
	} else {
		got = -2;
	}
	CHECK(got == 7);

	IF_LET (frame, Frame, f) {
		got = f->id;
	} else {
		got = -2;
	}
	CHECK(got == 3);

	/* else branch taken when variant doesn't match. */
	got = -1;
	IF_LET (point, Frame, f) {
		got = 99;
	} else {
		got = 42;
	}
	CHECK(got == 42);

	return fails;
}
#endif /* CERIVE_HAS_EXTRA_TYPES */

int main(void) {
	int const fails =
		flat_struct()
		+ total_order()
		+ hashing()
		+ fluent_construct()
		+ const_usage()
		+ const_struct_member()
		+ pointer_fields()
		+ nested_composition()
		+ debug_buffer_contract()
		+ union_construct_and_compare()
		+ union_match()
#if CERIVE_HAS_EXTRA_TYPES
		+ triple_pointer()
		+ many_variant_union()
		+ single_variant_union()
		+ nested_union()
		+ ord_short_circuit_3fields()
		+ debug_exact_buffer()
		+ match_with_break()
		+ nested_union_variant()
		+ if_let()
#endif
		;

	puts(fails == 0 ? "all tests passed" : "FAILURES");
	return fails;
}
