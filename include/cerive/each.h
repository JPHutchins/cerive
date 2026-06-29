#pragma once

/*
 * Generic comma-list fan-out. CERIVE_P_over(prefix, T, a, b, ...) expands to
 * prefix##_a(T) prefix##_b(T) ... -- one application of a `prefix`-named
 * generator per listed token. Bounded count-and-unroll (up to 12), so no
 * FOR_EACH/EXPAND recursion is needed. CERIVE (prefix = CERIVE) and CERIVE_UNION
 * (prefix = CERIVE_UNION) both build on it.
 */

#define CERIVE_P_cat(a, b) CERIVE_P_cat_(a, b)
#define CERIVE_P_cat_(a, b) a##b

#define CERIVE_P_count(...) CERIVE_P_count_(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define CERIVE_P_count_(a, b, c, d, e, f, g, h, i, j, k, l, n, ...) n

#define CERIVE_P_over(prefix, T, ...) __VA_OPT__( \
	CERIVE_P_cat(CERIVE_P_over_, CERIVE_P_count(__VA_ARGS__))(prefix, T, __VA_ARGS__) \
)
#define CERIVE_P_over_1(prefix, T, a) prefix##_##a(T)
#define CERIVE_P_over_2(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_1(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_3(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_2(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_4(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_3(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_5(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_4(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_6(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_5(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_7(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_6(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_8(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_7(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_9(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_8(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_10(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_9(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_11(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_10(prefix, T, __VA_ARGS__)
#define CERIVE_P_over_12(prefix, T, a, ...) prefix##_##a(T) CERIVE_P_over_11(prefix, T, __VA_ARGS__)
