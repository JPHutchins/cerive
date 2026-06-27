#pragma once

#include <stddef.h>

/*
 * FNV-1a hash. <T>_hash folds each field into the accumulator the same way Eq
 * compares them -- field by field, so equal values hash equal (hashing the raw
 * struct bytes would fold in indeterminate padding). Scalar and pointer fields
 * hash their own bytes (a pointer by its address, matching by-address Eq); record
 * fields recurse and mix the sub-hash.
 */

enum : size_t {
	cerive_hash_offset = 2166136261u,
	cerive_hash_prime = 16777619u,
};

static inline size_t cerive_hash_bytes(size_t hash, void const *const data, size_t const len) {
	unsigned char const *const bytes = data;
	for (size_t i = 0; i < len; ++i) {
		hash = (hash ^ bytes[i]) * cerive_hash_prime;
	}
	return hash;
}
static inline size_t cerive_hash_mix(size_t const hash, size_t const value) {
	return cerive_hash_bytes(hash, &value, sizeof value);
}
