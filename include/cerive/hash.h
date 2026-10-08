#pragma once

#include <stddef.h>

enum : size_t {
	cerive_hash_offset = 2166136261u,
	cerive_hash_prime = 16777619u,
};

_Static_assert(cerive_hash_offset == 2166136261u, "size_t must be at least 32 bits for FNV-1a constants");
_Static_assert(cerive_hash_prime == 16777619u, "size_t must be at least 32 bits for FNV-1a constants");

__attribute__((nonnull(2))) size_t cerive_hash_bytes(size_t hash, void const * data, size_t len);
size_t cerive_hash_mix(size_t hash, size_t value);
