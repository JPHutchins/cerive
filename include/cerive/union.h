#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "each.h"
#include "new.h"

#ifndef CERIVE_P_assert
#	ifdef CERIVE_NO_ASSERT
#		define CERIVE_P_assert(ptr) ((void) 0)
#	else
#		define CERIVE_P_assert(ptr) \
	do { \
		_Pragma("GCC diagnostic push") \
		_Pragma("GCC diagnostic ignored \"-Wnonnull-compare\"") \
		if ((uintptr_t)(ptr) == 0) { \
			__builtin_trap(); \
		} \
		_Pragma("GCC diagnostic pop") \
	} while (0)
#	endif
#endif

/*
 * Tagged unions. A variant token names three things in three different C
 * namespaces: the union member, the member's struct type, and -- with a `_tag`
 * suffix -- the enum discriminant. The suffix keeps the discriminant from
 * colliding with the struct typedef, so member types stay typedef'd; callers
 * always pass the bare token and the macros paste `_tag`.
 *
 * A union T is described by `T##_VARIANTS(X)` listing member type names.
 * CERIVE_UNION(T, traits...) generates the enum + struct, then fans out the named
 * traits like CERIVE does -- CERIVE_UNION(Shape, Debug, PartialEq).
 */

#define CERIVE_P_union_over(map, T) T##_VARIANTS(map)
#define CERIVE_P_union_tag(variant) variant##_tag,
#define CERIVE_P_union_member(variant) variant variant;
#define CERIVE_P_union_debug_case(variant) \
	case variant##_tag: \
		return variant##_debug(&self->variant, buf, n);
#define CERIVE_P_union_eq_case(variant) \
	case variant##_tag: \
		return variant##_eq(&a->variant, &b->variant);

#define CERIVE_P_union_def(T) \
	enum T##_tag : uint8_t { CERIVE_P_union_over(CERIVE_P_union_tag, T) }; \
	typedef struct T { \
		union { \
			CERIVE_P_union_over(CERIVE_P_union_member, T) \
		}; \
		enum T##_tag tag; \
	} T;
#define CERIVE_UNION(T, ...) CERIVE_P_union_def(T) CERIVE_P_over(CERIVE_UNION, T, __VA_ARGS__)

#ifdef CERIVE_NO_DEBUG
#	define CERIVE_UNION_Debug(T) \
	__attribute__((nonnull(1))) \
	static inline int T##_debug(T const * const self, char * const buf, size_t const n) { \
		(void) self; \
		(void) buf; \
		(void) n; \
		return 0; \
	}
#else
#	define CERIVE_UNION_Debug(T) \
	__attribute__((nonnull(1))) \
	static inline int T##_debug(T const * const self, char * const buf, size_t const n) { \
		CERIVE_P_assert(self); \
		switch (self->tag) { \
			CERIVE_P_union_over(CERIVE_P_union_debug_case, T) \
		} \
		unreachable(); \
	}
#endif

#define CERIVE_UNION_PartialEq(T) \
	__attribute__((nonnull(1, 2))) \
	static inline bool T##_eq(T const * const a, T const * const b) { \
		CERIVE_P_assert(a); \
		CERIVE_P_assert(b); \
		if (a->tag != b->tag) { \
			return false; \
		} \
		switch (a->tag) { \
			CERIVE_P_union_over(CERIVE_P_union_eq_case, T) \
		} \
		unreachable(); \
	}

/* construct: declare `#define Shape_new(...) CERIVE_UNION_NEW(Shape, __VA_ARGS__)`,
 * then `Shape_new(Point, .x = 1, .y = 2)`
 *
 * The manual #define is REQUIRED because the C preprocessor cannot produce a
 * #define directive from macro expansion (C23 6.10.3.4). There is no way for
 * CERIVE_UNION itself to generate the T_new shorthand -- the two-line pattern
 * above is the intended API. */
#define CERIVE_UNION_NEW(T, variant, ...) \
	(T){.tag = variant##_tag, .variant = {__VA_ARGS__}}

#define CERIVE_IS(instance, variant) ((instance).tag == variant##_tag)
