#pragma once

#include <stddef.h>

size_t cerive_buf_remaining(size_t const cap, int const off);

char * cerive_buf_at(size_t const cap, char buf[static const cap], int const off);

#define CERIVE_P_debug_signature(T) \
	[[maybe_unused]] __attribute__((nonnull(1))) static inline int T##_debug( \
		[[maybe_unused]] T const * const self, \
		size_t const n, \
		char buf[static const n] \
	)

#define CERIVE_P_debug_stub(T) \
	CERIVE_P_debug_signature(T) { \
		buf[0] = '\0'; \
		return 0; \
	}

#define CERIVE_P_debug_len(T) \
	[[maybe_unused]] __attribute__((nonnull(1))) static inline int T##_debug_len( \
		T const * const self \
	) { \
		char scratch[1]; \
		return T##_debug(self, sizeof scratch, scratch); \
	}
