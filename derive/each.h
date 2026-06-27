#pragma once

/*
 * Generic comma-list fan-out. DERIVE_OVER(P, T, a, b, ...) expands to
 * P##_a(T) P##_b(T) ... -- one application of a `P`-prefixed generator per
 * listed token. Bounded count-and-unroll (up to 12), so no FOR_EACH/EXPAND
 * recursion. Used by both DERIVE (P = DERIVE) and DERIVE_UNION (P = DERIVE_UNION).
 */

#define EACH_CAT(a, b) EACH_CAT_(a, b)
#define EACH_CAT_(a, b) a##b
#define EACH_COUNT(...) EACH_COUNT_(__VA_ARGS__, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define EACH_COUNT_(a, b, c, d, e, f, g, h, i, j, k, l, n, ...) n

#define DERIVE_OVER(P, T, ...) __VA_OPT__(EACH_CAT(EACH_, EACH_COUNT(__VA_ARGS__))(P, T, __VA_ARGS__))
#define EACH_1(P, T, a) P##_##a(T)
#define EACH_2(P, T, a, ...) P##_##a(T) EACH_1(P, T, __VA_ARGS__)
#define EACH_3(P, T, a, ...) P##_##a(T) EACH_2(P, T, __VA_ARGS__)
#define EACH_4(P, T, a, ...) P##_##a(T) EACH_3(P, T, __VA_ARGS__)
#define EACH_5(P, T, a, ...) P##_##a(T) EACH_4(P, T, __VA_ARGS__)
#define EACH_6(P, T, a, ...) P##_##a(T) EACH_5(P, T, __VA_ARGS__)
#define EACH_7(P, T, a, ...) P##_##a(T) EACH_6(P, T, __VA_ARGS__)
#define EACH_8(P, T, a, ...) P##_##a(T) EACH_7(P, T, __VA_ARGS__)
#define EACH_9(P, T, a, ...) P##_##a(T) EACH_8(P, T, __VA_ARGS__)
#define EACH_10(P, T, a, ...) P##_##a(T) EACH_9(P, T, __VA_ARGS__)
#define EACH_11(P, T, a, ...) P##_##a(T) EACH_10(P, T, __VA_ARGS__)
#define EACH_12(P, T, a, ...) P##_##a(T) EACH_11(P, T, __VA_ARGS__)
