#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/*
 * Hybrid derives: the classic operator-threaded `T##_FIELDS(X)` form (one macro
 * per generator, no field-count ceiling), plus a single helper -- DERIVE_DROP1
 * -- that peels the leading comma off a comma-prefixed parameter list, so `new`
 * is a clean function with no sacrificial argument or call shim. The two-layer
 * indirection lets FIELDS expand to commas before DROP1_ splits on them.
 * Contrast derive_xmacro.h (call shim) and derive.h (FOR_EACH recursion).
 */

#define DERIVE_DROP1(...) DERIVE_DROP1_(__VA_ARGS__)
#define DERIVE_DROP1_(first, ...) __VA_ARGS__

#define DERIVE_HY_DECL(type, name, fmt) type name;
#define DERIVE_STRUCT(T) typedef struct T {T##_FIELDS(DERIVE_HY_DECL)} T

#define DERIVE_HY_DBG_FMT(type, name, fmt) #name "=" fmt " "
#define DERIVE_HY_DBG_ARG(type, name, fmt) , self->name
#define DERIVE_DEBUG(T) \
	static inline int T##_debug(T const *const self, char *const buf, size_t const n) { \
		return snprintf(buf, n, #T " { " T##_FIELDS(DERIVE_HY_DBG_FMT) "}" \
			T##_FIELDS(DERIVE_HY_DBG_ARG)); \
	}

#define DERIVE_HY_PARAM(type, name, fmt) , type name
#define DERIVE_HY_INIT(type, name, fmt) .name = name,
#define DERIVE_NEW(T) \
	static inline T T##_new(DERIVE_DROP1(T##_FIELDS(DERIVE_HY_PARAM))) { \
		return (T){T##_FIELDS(DERIVE_HY_INIT)}; \
	}

#define DERIVE_DEFAULT(T) \
	static inline T T##_default(void) { return (T){}; }

#define DERIVE_HY_EQ(type, name, fmt) &&a->name == b->name
#define DERIVE_EQ(T) \
	static inline bool T##_eq(T const *const a, T const *const b) { \
		return true T##_FIELDS(DERIVE_HY_EQ); \
	}
