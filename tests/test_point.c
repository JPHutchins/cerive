#include <stdio.h>
#include <string.h>

#include "point.h"

#define CHECK(cond)                                                  \
	do {                                                             \
		if (!(cond)) {                                               \
			printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond);  \
			++fails;                                                 \
		}                                                            \
	} while (0)

int main(void) {
	int fails = 0;

	Point const d = Point_default();
	CHECK(d.x == 0 && d.y == 0);

	Point const p = Point_new(42, 7);
	CHECK(p.x == 42 && p.y == 7);

	CHECK(Point_eq(&p, &(Point){.x = 42, .y = 7}));
	CHECK(!Point_eq(&p, &d));

	char buf[64];
	Point_debug(&p, buf, sizeof buf);
	CHECK(strcmp(buf, "Point { x=42 y=7 }") == 0);

	puts(fails == 0 ? "all tests passed" : "FAILURES");
	return fails;
}
