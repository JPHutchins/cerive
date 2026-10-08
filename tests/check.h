#pragma once

#include <stdio.h>

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			++fails; \
			printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
		} \
	} while (0)
