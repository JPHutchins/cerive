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
	Point const p = Point_new(1, 2);
	CHECK(p.x == 1 && p.y == 2);
	CHECK(Point_default().x == 0 && Point_default().y == 0);
	CHECK(Point_eq(&p, &(Point){.x = 1, .y = 2}));
	CHECK(!Point_eq(&p, &(Point){.x = 9, .y = 9}));
	char pb[32];
	Point_debug(&p, pb, sizeof pb);
	CHECK(strcmp(pb, "Point { x=1 y=2 }") == 0);

	/* nested: Frame -> Line -> Point */
	Frame const f = Frame_new(Line_new(Point_new(1, 2), Point_new(3, 4)), 7);
	CHECK(f.edge.a.x == 1 && f.edge.b.y == 4 && f.id == 7);
	Frame const z = Frame_default();
	CHECK(z.edge.a.x == 0 && z.id == 0);
	Frame const same = Frame_new(Line_new(Point_new(1, 2), Point_new(3, 4)), 7);
	CHECK(Frame_eq(&f, &same));
	CHECK(!Frame_eq(&f, &z));

	char const *const expect =
		"Frame { edge=Line { a=Point { x=1 y=2 } b=Point { x=3 y=4 } } id=7 }";
	char buf[128];
	int const need = Frame_debug(&f, buf, sizeof buf);
	puts(buf);
	CHECK(strcmp(buf, expect) == 0);
	CHECK(need == (int) strlen(expect));

	/* size query: no buffer, same length */
	CHECK(Frame_debug(&f, NULL, 0) == need);

	/* truncation is safe: NUL-terminated, prefix correct, full length returned */
	char small[10];
	int const need2 = Frame_debug(&f, small, sizeof small);
	CHECK(need2 == need);
	CHECK(strlen(small) == sizeof small - 1);
	CHECK(strncmp(small, expect, sizeof small - 1) == 0);

	puts(fails == 0 ? "all tests passed" : "FAILURES");
	return fails;
}
