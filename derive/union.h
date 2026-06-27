#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "new.h"

/*
 * Generic tagged-union helpers. A variant token names three things in three
 * different C namespaces: the union member, the member's struct type, and -- with
 * a `_tag` suffix -- the enum discriminant. The suffix keeps the discriminant
 * from colliding with the struct typedef, so the member types stay typedef'd;
 * callers always pass the bare token and the macros paste `_tag`.
 *
 * A deriveable union T is described by `T##_VARIANTS` listing member type names;
 * each must be a top-level type that derives the same operation (Debug ->
 * _debug, PartialEq -> _eq). UNION_OVER(mapper, T) is provided by the including
 * framework (derive_for.h / derive_hybrid.h) to iterate that list.
 */

#define UNION_ENUM(variant) variant##_tag,
#define UNION_MEMBER(variant) variant variant;
#define UNION_DBG(variant) \
	case variant##_tag: \
		return variant##_debug(&self->variant, buf, n);
#define UNION_EQ(variant) \
	case variant##_tag: \
		return variant##_eq(&a->variant, &b->variant);

#define DERIVE_UNION(T) \
	enum T##_tag : uint8_t { UNION_OVER(UNION_ENUM, T) }; \
	typedef struct T { \
		union { \
			UNION_OVER(UNION_MEMBER, T) \
		}; \
		enum T##_tag tag; \
	} T

#define DERIVE_UNION_DEBUG(T) \
	static inline int T##_debug(T const *const self, char *const buf, size_t const n) { \
		switch (self->tag) { \
			UNION_OVER(UNION_DBG, T) \
		} \
		unreachable(); \
	}

#define DERIVE_UNION_PARTIAL_EQ(T) \
	static inline bool T##_eq(T const *const a, T const *const b) { \
		if (a->tag != b->tag) { \
			return false; \
		} \
		switch (a->tag) { \
			UNION_OVER(UNION_EQ, T) \
		} \
		unreachable(); \
	}

/* construct: declare `#define Shape_new(...) UNION_NEW(Shape, __VA_ARGS__)`,
 * then `Shape_new(Point, .x = 1, .y = 2)` */
#define UNION_NEW(T, variant, ...) \
	(T){.tag = variant##_tag, .variant = {__VA_ARGS__}}

#define UNION_IS(instance, variant) ((instance).tag == variant##_tag)

/*
 * match: MATCH binds a hidden typed pointer to the instance, so each CASE only
 * names its variant and binding; the binding is `variant const *`, scoped to the
 * arm, so a wrong-field access will not compile. A leading break terminates the
 * prior arm -- no manual break needed.
 *
 *   MATCH (shape) {
 *       CASE (Point, p) { use(p->x); }
 *       CASE (Frame, f) { use(f->id); }
 *   }
 */
#define MATCH(instance) \
	for (typeof(instance) *const _match_ = &(instance), *_match_once_ = _match_; _match_once_; \
		 _match_once_ = 0) \
		switch (_match_->tag)
#define CASE(variant, bind) \
	break; \
	case variant##_tag: \
		for (variant const *const bind = &_match_->variant, *_case_once_ = bind; _case_once_; \
			 _case_once_ = 0)
