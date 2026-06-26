#include <stdio.h>
#include <string.h>

#include "shapes.h"

#define CHECK(cond)                                                  \
	do {                                                             \
		if (!(cond)) {                                               \
			printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);  \
			++fails;                                                 \
		}                                                            \
	} while (0)

int main(void) {
	int fails = 0;

	/* flat */
	struct Point const p = Point_new(1, 2);
	CHECK(p.x == 1 && p.y == 2);
	CHECK(Point_default().x == 0 && Point_default().y == 0);
	CHECK(Point_eq(&p, &(struct Point){.x = 1, .y = 2}));
	CHECK(!Point_eq(&p, &(struct Point){.x = 9, .y = 9}));
	char pb[32];
	Point_debug(&p, pb, sizeof pb);
	CHECK(strcmp(pb, "Point { x=1 y=2 }") == 0);

	/* nested: Frame -> Line -> Point */
	struct Frame const f = Frame_new(Line_new(Point_new(1, 2), Point_new(3, 4)), 7);
	CHECK(f.edge.a.x == 1 && f.edge.b.y == 4 && f.id == 7);
	struct Frame const z = Frame_default();
	CHECK(z.edge.a.x == 0 && z.id == 0);
	CHECK(Frame_eq(&f, &(struct Frame){.edge = Line_new(Point_new(1, 2), Point_new(3, 4)), .id = 7}));
	CHECK(!Frame_eq(&f, &z));

	char const *const frame_str =
		"Frame { edge=Line { a=Point { x=1 y=2 } b=Point { x=3 y=4 } } id=7 }";
	char buf[128];
	int const need = Frame_debug(&f, buf, sizeof buf);
	puts(buf);
	CHECK(strcmp(buf, frame_str) == 0);
	CHECK(need == (int) strlen(frame_str));
	CHECK(Frame_debug(&f, NULL, 0) == need);

	char small[10];
	int const need2 = Frame_debug(&f, small, sizeof small);
	CHECK(need2 == need);
	CHECK(strlen(small) == sizeof small - 1);
	CHECK(strncmp(small, frame_str, sizeof small - 1) == 0);

	/* tagged union: tag constant == union member == struct name */
	struct Shape const sp = SUM_NEW(Shape, Point, .x = 1, .y = 2);
	CHECK(SUM_IS(sp, Point) && sp.Point.x == 1 && sp.Point.y == 2);
	struct Shape const sf = SUM_NEW(Shape, Frame, .edge = f.edge, .id = f.id);
	CHECK(SUM_IS(sf, Frame) && sf.Frame.id == 7);

	char sb[128];
	Shape_debug(&sp, sb, sizeof sb);
	CHECK(strcmp(sb, "Point { x=1 y=2 }") == 0);
	Shape_debug(&sf, sb, sizeof sb);
	CHECK(strcmp(sb, frame_str) == 0);

	CHECK(Shape_eq(&sp, &SUM_NEW(Shape, Point, .x = 1, .y = 2)));
	CHECK(!Shape_eq(&sp, &sf));

	int matched = 0;
	SUM_MATCH (sp) {
		SUM_CASE (sp, Point, it) { matched = it->x; }
		SUM_CASE (sp, Line, it) { (void) it; matched = -1; }
		SUM_CASE (sp, Frame, it) { (void) it; matched = -1; }
	}
	CHECK(matched == 1);

	puts(fails == 0 ? "all tests passed" : "FAILURES");
	return fails;
}
