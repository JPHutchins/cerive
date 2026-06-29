#pragma once

#include <stddef.h>

/*
 * FNV-1a hash. <T>_hash folds each field into the accumulator the same way Eq
 * compares them -- field by field, so equal values hash equal (hashing the raw
 * struct bytes would fold in indeterminate padding). Scalar and pointer fields
 * hash their own bytes (a pointer by its address, matching by-address Eq); record
 * fields recurse and mix the sub-hash. The two helpers are defined once in
 * src/cerive.c; the constants are needed at the call site by the generated code.
 */

enum : size_t {
	cerive_hash_offset = 2166136261u,
	cerive_hash_prime = 16777619u,
};

_Static_assert(cerive_hash_offset == 2166136261u, "size_t must be at least 32 bits for FNV-1a constants");
_Static_assert(cerive_hash_prime == 16777619u, "size_t must be at least 32 bits for FNV-1a constants");

__attribute__((nonnull(2))) size_t cerive_hash_bytes(size_t hash, void const * data, size_t len);
size_t cerive_hash_mix(size_t hash, size_t value);
