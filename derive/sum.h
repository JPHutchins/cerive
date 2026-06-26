#pragma once

#include <stdbool.h>
#include <stddef.h>

/*
 * Generic tagged-union (sum type) helpers. The enum tag constant, the union
 * member, and the member's struct tag all share one spelling -- three different
 * C namespaces -- so a single token both selects a union member and names its
 * struct. Construction and matching are therefore pure token reuse, fully
 * type-checked. (This is why the field structs are not typedef'd: a typedef name
 * would collide with the enum tag constant.)
 *
 * A deriveable sum T is described by `T##_VARIANTS` listing member struct names;
 * each must be a top-level struct that derives the same operation (Debug ->
 * _debug, PartialEq -> _eq). SUM_OVER(mapper, T) is provided by the including
 * framework (derive_for.h / derive_hybrid.h) to iterate that list.
 */

#define SUM_ENUM(variant) variant,
#define SUM_MEMBER(variant) struct variant variant;
#define SUM_DBG(variant) \
	case variant: \
		return variant##_debug(&self->variant, buf, n);
#define SUM_EQ(variant) \
	case variant: \
		return variant##_eq(&a->variant, &b->variant);

#define DERIVE_SUM(T) \
	enum T##_tag { SUM_OVER(SUM_ENUM, T) }; \
	struct T { \
		union { \
			SUM_OVER(SUM_MEMBER, T) \
		}; \
		enum T##_tag tag; \
	}

#define DERIVE_SUM_DEBUG(T) \
	static inline int T##_debug(struct T const *const self, char *const buf, size_t const n) { \
		switch (self->tag) { \
			SUM_OVER(SUM_DBG, T) \
		} \
		unreachable(); \
	}

#define DERIVE_SUM_PARTIAL_EQ(T) \
	static inline bool T##_eq(struct T const *const a, struct T const *const b) { \
		if (a->tag != b->tag) { \
			return false; \
		} \
		switch (a->tag) { \
			SUM_OVER(SUM_EQ, T) \
		} \
		unreachable(); \
	}

/* construct: SUM_NEW(Shape, Point, .x = 1, .y = 2) */
#define SUM_NEW(T, variant, ...) \
	(struct T){.tag = variant, .variant = {__VA_ARGS__}}

/* predicate: SUM_IS(shape, Point) */
#define SUM_IS(sum, variant) ((sum).tag == (variant))

/*
 * match: a type-safe, exhaustive switch binding the correct payload per arm.
 *
 *   SUM_MATCH (shape) {
 *       SUM_CASE (shape, Point, p) { use(p->x); }
 *       SUM_CASE (shape, Frame, f) { use(f->id); }
 *   }
 *
 * `bind` is `struct variant const *`, scoped to the arm and valid once -- a
 * wrong-field access will not compile. The leading break terminates the prior
 * arm, so no manual break is needed.
 */
#define SUM_MATCH(sum) switch ((sum).tag)
#define SUM_CASE(sum, variant, bind) \
	break; \
	case variant: \
		for (struct variant const *const bind = &(sum).variant, *SUM_once_ = bind; SUM_once_; \
			 SUM_once_ = 0)
