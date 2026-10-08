#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "buf.h"
#include "each.h"
#include "new.h"

#define CERIVE_P_union_over(map, T) T##_VARIANTS(map)
#define CERIVE_P_union_tag(variant) variant##_tag,
#define CERIVE_P_union_member(variant) variant variant;
#define CERIVE_P_union_debug_case(variant) \
	case variant##_tag: \
		return variant##_debug(&self->variant, n, buf);
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
#	define CERIVE_UNION_Debug(T) CERIVE_P_debug_stub(T) CERIVE_P_debug_len(T)
#else
#	define CERIVE_UNION_Debug(T) \
	CERIVE_P_debug_signature(T) { \
		switch (self->tag) { \
			CERIVE_P_union_over(CERIVE_P_union_debug_case, T) \
		} \
		unreachable(); \
	} \
	CERIVE_P_debug_len(T)
#endif

#define CERIVE_UNION_PartialEq(T) \
	__attribute__((nonnull(1, 2))) \
	static inline bool T##_eq(T const * const a, T const * const b) { \
		if (a->tag != b->tag) { \
			return false; \
		} \
		switch (a->tag) { \
			CERIVE_P_union_over(CERIVE_P_union_eq_case, T) \
		} \
		unreachable(); \
	}

#define CERIVE_UNION_NEW(T, variant, ...) \
	(T){.tag = variant##_tag, .variant = {__VA_ARGS__}}

#define CERIVE_IS(instance, variant) ((instance).tag == variant##_tag)
