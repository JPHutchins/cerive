#pragma once

#include <stdbool.h>
#include <stddef.h>

/*
 * Generic tagged-union (sum type) helpers. A variant token names three things in
 * three different C namespaces: the union member, the member's struct type, and
 * -- with a `_tag` suffix -- the enum discriminant. The suffix keeps the
 * discriminant from colliding with the struct typedef, so the field structs stay
 * typedef'd; callers always pass the bare token and the macros paste `_tag`.
 *
 * A deriveable sum T is described by `T##_VARIANTS` listing member type names;
 * each must be a top-level type that derives the same operation (Debug ->
 * _debug, PartialEq -> _eq). SUM_OVER(mapper, T) is provided by the including
 * framework (derive_for.h / derive_hybrid.h) to iterate that list.
 */

#define SUM_ENUM(variant) variant##_tag,
#define SUM_MEMBER(variant) variant variant;
#define SUM_DBG(variant) \
	case variant##_tag: \
		return variant##_debug(&self->variant, buf, n);
#define SUM_EQ(variant) \
	case variant##_tag: \
		return variant##_eq(&a->variant, &b->variant);

#define DERIVE_SUM(T) \
	enum T##_tag { SUM_OVER(SUM_ENUM, T) }; \
	typedef struct T { \
		union { \
			SUM_OVER(SUM_MEMBER, T) \
		}; \
		enum T##_tag tag; \
	} T

#define DERIVE_SUM_DEBUG(T) \
	static inline int T##_debug(T const *const self, char *const buf, size_t const n) { \
		switch (self->tag) { \
			SUM_OVER(SUM_DBG, T) \
		} \
		unreachable(); \
	}

#define DERIVE_SUM_PARTIAL_EQ(T) \
	static inline bool T##_eq(T const *const a, T const *const b) { \
		if (a->tag != b->tag) { \
			return false; \
		} \
		switch (a->tag) { \
			SUM_OVER(SUM_EQ, T) \
		} \
		unreachable(); \
	}

#define SUM_NEW(T, variant, ...) \
	(T){.tag = variant##_tag, .variant = {__VA_ARGS__}}

#define SUM_IS(sum, variant) ((sum).tag == variant##_tag)

#define SUM_MATCH(sum) switch ((sum).tag)
#define SUM_CASE(sum, variant, bind) \
	break; \
	case variant##_tag: \
		for (variant const *const bind = &(sum).variant, *SUM_once_ = bind; SUM_once_; SUM_once_ = 0)
