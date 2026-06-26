#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/*
 * Classic X-macro derives: the field list threads an operator `X` through
 * itself -- `#define T##_FIELDS(X) X(type, name, fmt) ...`. Each generator is a
 * single macro, but a bare parameter list hits the trailing-comma wall, so
 * `new` takes a sacrificial leading argument and the type header adds a call
 * shim. Contrast derive.h (FOR_EACH) and derive_hybrid.h.
 */

#define DERIVE_XM_DECL(type, name, fmt) type name;
#define DERIVE_STRUCT(T) typedef struct T {T##_FIELDS(DERIVE_XM_DECL)} T

#define DERIVE_XM_DBG_FMT(type, name, fmt) #name "=" fmt " "
#define DERIVE_XM_DBG_ARG(type, name, fmt) , self->name
#define DERIVE_DEBUG(T) \
	static inline int T##_debug(T const *const self, char *const buf, size_t const n) { \
		return snprintf(buf, n, #T " { " T##_FIELDS(DERIVE_XM_DBG_FMT) "}" \
			T##_FIELDS(DERIVE_XM_DBG_ARG)); \
	}

#define DERIVE_XM_PARAM(type, name, fmt) , type name
#define DERIVE_XM_INIT(type, name, fmt) .name = name,
#define DERIVE_NEW(T) \
	static inline T T##_new_(int derive_pad T##_FIELDS(DERIVE_XM_PARAM)) { \
		(void) derive_pad; \
		return (T){T##_FIELDS(DERIVE_XM_INIT)}; \
	}

#define DERIVE_DEFAULT(T) \
	static inline T T##_default(void) { return (T){}; }

#define DERIVE_XM_EQ(type, name, fmt) &&a->name == b->name
#define DERIVE_EQ(T) \
	static inline bool T##_eq(T const *const a, T const *const b) { \
		return true T##_FIELDS(DERIVE_XM_EQ); \
	}
