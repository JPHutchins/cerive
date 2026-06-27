#pragma once

#include <stddef.h>

/*
 * FNV-1a hash. T_hash folds each field into the accumulator the same way Eq
 * compares them -- field by field, so equal values hash equal (hashing the raw
 * struct bytes would fold in indeterminate padding). SCALAR and PTR fields hash
 * their own bytes (a PTR by its address, matching by-address Eq); STRUCT fields
 * recurse and mix the sub-hash.
 */

enum : size_t {
	hash_offset = 2166136261u,
	hash_prime = 16777619u,
};

static inline size_t hash_bytes(size_t h, void const *const p, size_t const n) {
	unsigned char const *const bytes = p;
	for (size_t i = 0; i < n; ++i) {
		h = (h ^ bytes[i]) * hash_prime;
	}
	return h;
}
static inline size_t hash_mix(size_t const h, size_t const v) {
	return hash_bytes(h, &v, sizeof v);
}
