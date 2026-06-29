#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "test_types.h"

#define CHECK(cond) \
	do { \
		if (!(cond)) { \
			++fails; \
			printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); \
		} \
	} while (0)

/*
 * Scalar test types -- one per registered scalar. Each has a scalar field and
 * an int32_t discriminator to verify field ordering.
 */

#define ScalarBool_FIELDS(X) \
	X(bool, val) \
	X(int32_t, id)
CERIVE(ScalarBool, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarChar_FIELDS(X) \
	X(char, val) \
	X(int32_t, id)
CERIVE(ScalarChar, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarI8_FIELDS(X) \
	X(int8_t, val) \
	X(int32_t, id)
CERIVE(ScalarI8, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarI16_FIELDS(X) \
	X(int16_t, val) \
	X(int32_t, id)
CERIVE(ScalarI16, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarI32_FIELDS(X) \
	X(int32_t, val) \
	X(int32_t, id)
CERIVE(ScalarI32, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarI64_FIELDS(X) \
	X(int64_t, val) \
	X(int32_t, id)
CERIVE(ScalarI64, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarU8_FIELDS(X) \
	X(uint8_t, val) \
	X(int32_t, id)
CERIVE(ScalarU8, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarU16_FIELDS(X) \
	X(uint16_t, val) \
	X(int32_t, id)
CERIVE(ScalarU16, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarU32_FIELDS(X) \
	X(uint32_t, val) \
	X(int32_t, id)
CERIVE(ScalarU32, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarU64_FIELDS(X) \
	X(uint64_t, val) \
	X(int32_t, id)
CERIVE(ScalarU64, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarSize_FIELDS(X) \
	X(size_t, val) \
	X(int32_t, id)
CERIVE(ScalarSize, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarFloat_FIELDS(X) \
	X(double, val) \
	X(int32_t, id)
CERIVE(ScalarFloat, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarDouble_FIELDS(X) \
	X(double, val) \
	X(int32_t, id)
CERIVE(ScalarDouble, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarInt_FIELDS(X) \
	X(int, val) \
	X(int32_t, id)
CERIVE(ScalarInt, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarUnsigned_FIELDS(X) \
	X(unsigned, val) \
	X(int32_t, id)
CERIVE(ScalarUnsigned, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarLong_FIELDS(X) \
	X(long, val) \
	X(int32_t, id)
CERIVE(ScalarLong, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarULong_FIELDS(X) \
	X(unsigned_long, val) \
	X(int32_t, id)
CERIVE(ScalarULong, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarLLong_FIELDS(X) \
	X(long_long, val) \
	X(int32_t, id)
CERIVE(ScalarLLong, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

#define ScalarULLong_FIELDS(X) \
	X(unsigned_long_long, val) \
	X(int32_t, id)
CERIVE(ScalarULLong, Struct, Debug, Constructor, Default, PartialEq, Ord, Hash)

/* ---------- test functions ---------- */

static int scalar_bool(void) {
	int fails = 0;

	ScalarBool const s = ScalarBool_new(true, 42);
	CHECK(s.val == true && s.id == 42);

	CHECK(ScalarBool_eq(&s, &(ScalarBool){.val = true, .id = 42}));
	CHECK(!ScalarBool_eq(&s, &(ScalarBool){.val = false, .id = 42}));
	CHECK(!ScalarBool_eq(&s, &(ScalarBool){.val = true, .id = 99}));

	CHECK(ScalarBool_cmp(&s, &(ScalarBool){.val = true, .id = 42}) == cerive_equal);
	CHECK(ScalarBool_cmp(&s, &(ScalarBool){.val = false, .id = 42}) == cerive_greater);
	CHECK(ScalarBool_cmp(&s, &(ScalarBool){.val = true, .id = 43}) == cerive_less);

	CHECK(ScalarBool_hash(&s) == ScalarBool_hash(&(ScalarBool){.val = true, .id = 42}));
	CHECK(ScalarBool_hash(&s) != ScalarBool_hash(&(ScalarBool){.val = false, .id = 42}));

	char buf[64];
	ScalarBool_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarBool {") == buf);

	ScalarBool const z = ScalarBool_default();
	CHECK(z.val == 0 && z.id == 0);

	return fails;
}

static int scalar_char(void) {
	int fails = 0;

	ScalarChar const s = ScalarChar_new('A', 42);
	CHECK(s.val == 'A' && s.id == 42);

	CHECK(ScalarChar_eq(&s, &(ScalarChar){.val = 'A', .id = 42}));
	CHECK(!ScalarChar_eq(&s, &(ScalarChar){.val = 'B', .id = 42}));

	CHECK(ScalarChar_cmp(&s, &(ScalarChar){.val = 'A', .id = 42}) == cerive_equal);
	CHECK(ScalarChar_cmp(&s, &(ScalarChar){.val = 'B', .id = 42}) == cerive_less);

	CHECK(ScalarChar_hash(&s) == ScalarChar_hash(&(ScalarChar){.val = 'A', .id = 42}));
	CHECK(ScalarChar_hash(&s) != ScalarChar_hash(&(ScalarChar){.val = 'B', .id = 42}));

	char buf[64];
	ScalarChar_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarChar {") == buf);

	return fails;
}

static int scalar_i8(void) {
	int fails = 0;

	ScalarI8 const s = ScalarI8_new(-12, 1);
	CHECK(s.val == -12 && s.id == 1);

	CHECK(ScalarI8_eq(&s, &(ScalarI8){.val = -12, .id = 1}));
	CHECK(!ScalarI8_eq(&s, &(ScalarI8){.val = 0, .id = 1}));

	CHECK(ScalarI8_cmp(&s, &(ScalarI8){.val = -12, .id = 1}) == cerive_equal);
	CHECK(ScalarI8_cmp(&s, &(ScalarI8){.val = 0, .id = 1}) == cerive_less);

	CHECK(ScalarI8_hash(&s) == ScalarI8_hash(&(ScalarI8){.val = -12, .id = 1}));
	CHECK(ScalarI8_hash(&s) != ScalarI8_hash(&(ScalarI8){.val = -12, .id = 2}));

	char buf[64];
	ScalarI8_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarI8 {") == buf);

	return fails;
}

static int scalar_i16(void) {
	int fails = 0;

	ScalarI16 const s = ScalarI16_new(-1234, 2);
	CHECK(s.val == -1234 && s.id == 2);

	CHECK(ScalarI16_eq(&s, &(ScalarI16){.val = -1234, .id = 2}));
	CHECK(!ScalarI16_eq(&s, &(ScalarI16){.val = -1234, .id = 9}));

	CHECK(ScalarI16_cmp(&s, &(ScalarI16){.val = -1234, .id = 2}) == cerive_equal);
	CHECK(ScalarI16_cmp(&s, &(ScalarI16){.val = -1234, .id = 1}) == cerive_greater);

	CHECK(ScalarI16_hash(&s) == ScalarI16_hash(&(ScalarI16){.val = -1234, .id = 2}));
	CHECK(ScalarI16_hash(&s) != ScalarI16_hash(&(ScalarI16){.val = 0, .id = 2}));

	char buf[64];
	ScalarI16_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarI16 {") == buf);

	return fails;
}

static int scalar_i32(void) {
	int fails = 0;

	ScalarI32 const s = ScalarI32_new(-123456, 3);
	CHECK(s.val == -123456 && s.id == 3);

	CHECK(ScalarI32_eq(&s, &(ScalarI32){.val = -123456, .id = 3}));
	CHECK(!ScalarI32_eq(&s, &(ScalarI32){.val = -1, .id = 3}));

	CHECK(ScalarI32_cmp(&s, &(ScalarI32){.val = -123456, .id = 3}) == cerive_equal);
	CHECK(ScalarI32_cmp(&s, &(ScalarI32){.val = 0, .id = 3}) == cerive_less);

	CHECK(ScalarI32_hash(&s) == ScalarI32_hash(&(ScalarI32){.val = -123456, .id = 3}));
	CHECK(ScalarI32_hash(&s) != ScalarI32_hash(&(ScalarI32){.val = -123456, .id = 0}));

	char buf[64];
	ScalarI32_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarI32 {") == buf);

	return fails;
}

static int scalar_i64(void) {
	int fails = 0;

	ScalarI64 const s = ScalarI64_new(-123456789012LL, 4);
	CHECK(s.val == -123456789012LL && s.id == 4);

	CHECK(ScalarI64_eq(&s, &(ScalarI64){.val = -123456789012LL, .id = 4}));
	CHECK(!ScalarI64_eq(&s, &(ScalarI64){.val = -123456789012LL, .id = 5}));

	CHECK(ScalarI64_cmp(&s, &(ScalarI64){.val = -123456789012LL, .id = 4}) == cerive_equal);
	CHECK(ScalarI64_cmp(&s, &(ScalarI64){.val = -123456789012LL, .id = 5}) == cerive_less);

	CHECK(ScalarI64_hash(&s) == ScalarI64_hash(&(ScalarI64){.val = -123456789012LL, .id = 4}));
	CHECK(ScalarI64_hash(&s) != ScalarI64_hash(&(ScalarI64){.val = 0, .id = 4}));

	char buf[64];
	ScalarI64_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarI64 {") == buf);

	return fails;
}

static int scalar_u8(void) {
	int fails = 0;

	ScalarU8 const s = ScalarU8_new(200, 5);
	CHECK(s.val == 200 && s.id == 5);

	CHECK(ScalarU8_eq(&s, &(ScalarU8){.val = 200, .id = 5}));
	CHECK(!ScalarU8_eq(&s, &(ScalarU8){.val = 10, .id = 5}));

	CHECK(ScalarU8_cmp(&s, &(ScalarU8){.val = 200, .id = 5}) == cerive_equal);
	CHECK(ScalarU8_cmp(&s, &(ScalarU8){.val = 200, .id = 6}) == cerive_less);

	CHECK(ScalarU8_hash(&s) == ScalarU8_hash(&(ScalarU8){.val = 200, .id = 5}));
	CHECK(ScalarU8_hash(&s) != ScalarU8_hash(&(ScalarU8){.val = 100, .id = 5}));

	char buf[64];
	ScalarU8_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarU8 {") == buf);

	return fails;
}

static int scalar_u16(void) {
	int fails = 0;

	ScalarU16 const s = ScalarU16_new(40000, 6);
	CHECK(s.val == 40000 && s.id == 6);

	CHECK(ScalarU16_eq(&s, &(ScalarU16){.val = 40000, .id = 6}));
	CHECK(!ScalarU16_eq(&s, &(ScalarU16){.val = 40000, .id = 0}));

	CHECK(ScalarU16_cmp(&s, &(ScalarU16){.val = 40000, .id = 6}) == cerive_equal);
	CHECK(ScalarU16_cmp(&s, &(ScalarU16){.val = 40000, .id = 7}) == cerive_less);

	CHECK(ScalarU16_hash(&s) == ScalarU16_hash(&(ScalarU16){.val = 40000, .id = 6}));
	CHECK(ScalarU16_hash(&s) != ScalarU16_hash(&(ScalarU16){.val = 1, .id = 6}));

	char buf[64];
	ScalarU16_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarU16 {") == buf);

	return fails;
}

static int scalar_u32(void) {
	int fails = 0;

	ScalarU32 const s = ScalarU32_new(3000000000U, 7);
	CHECK(s.val == 3000000000U && s.id == 7);

	CHECK(ScalarU32_eq(&s, &(ScalarU32){.val = 3000000000U, .id = 7}));

	CHECK(ScalarU32_cmp(&s, &(ScalarU32){.val = 3000000000U, .id = 7}) == cerive_equal);
	CHECK(ScalarU32_cmp(&s, &(ScalarU32){.val = 3000000000U, .id = 8}) == cerive_less);

	CHECK(ScalarU32_hash(&s) == ScalarU32_hash(&(ScalarU32){.val = 3000000000U, .id = 7}));
	CHECK(ScalarU32_hash(&s) != ScalarU32_hash(&(ScalarU32){.val = 0U, .id = 7}));

	char buf[64];
	ScalarU32_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarU32 {") == buf);

	return fails;
}

static int scalar_u64(void) {
	int fails = 0;

	ScalarU64 const s = ScalarU64_new(500000000000ULL, 8);
	CHECK(s.val == 500000000000ULL && s.id == 8);

	CHECK(ScalarU64_eq(&s, &(ScalarU64){.val = 500000000000ULL, .id = 8}));

	CHECK(ScalarU64_cmp(&s, &(ScalarU64){.val = 500000000000ULL, .id = 8}) == cerive_equal);
	CHECK(ScalarU64_cmp(&s, &(ScalarU64){.val = 500000000000ULL, .id = 0}) == cerive_greater);

	CHECK(ScalarU64_hash(&s) == ScalarU64_hash(&(ScalarU64){.val = 500000000000ULL, .id = 8}));
	CHECK(ScalarU64_hash(&s) != ScalarU64_hash(&(ScalarU64){.val = 0ULL, .id = 8}));

	char buf[64];
	ScalarU64_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarU64 {") == buf);

	return fails;
}

static int scalar_size(void) {
	int fails = 0;

	ScalarSize const s = ScalarSize_new(99, 9);
	CHECK(s.val == 99 && s.id == 9);

	CHECK(ScalarSize_eq(&s, &(ScalarSize){.val = 99, .id = 9}));
	CHECK(!ScalarSize_eq(&s, &(ScalarSize){.val = 99, .id = 10}));

	CHECK(ScalarSize_cmp(&s, &(ScalarSize){.val = 99, .id = 9}) == cerive_equal);
	CHECK(ScalarSize_cmp(&s, &(ScalarSize){.val = 99, .id = 8}) == cerive_greater);

	CHECK(ScalarSize_hash(&s) == ScalarSize_hash(&(ScalarSize){.val = 99, .id = 9}));
	CHECK(ScalarSize_hash(&s) != ScalarSize_hash(&(ScalarSize){.val = 0, .id = 9}));

	char buf[64];
	ScalarSize_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarSize {") == buf);

	return fails;
}

static int scalar_float(void) {
	int fails = 0;

	ScalarFloat const s = ScalarFloat_new(1.5, 10);
	CHECK(s.val == 1.5 && s.id == 10);

	CHECK(ScalarFloat_eq(&s, &(ScalarFloat){.val = 1.5, .id = 10}));
	CHECK(!ScalarFloat_eq(&s, &(ScalarFloat){.val = 1.5, .id = 11}));

	CHECK(ScalarFloat_cmp(&s, &(ScalarFloat){.val = 1.5, .id = 10}) == cerive_equal);
	CHECK(ScalarFloat_cmp(&s, &(ScalarFloat){.val = 1.5, .id = 9}) == cerive_greater);

	CHECK(ScalarFloat_hash(&s) == ScalarFloat_hash(&(ScalarFloat){.val = 1.5, .id = 10}));
	CHECK(ScalarFloat_hash(&s) != ScalarFloat_hash(&(ScalarFloat){.val = 0.0f, .id = 10}));

	char buf[64];
	ScalarFloat_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarFloat {") == buf);

	return fails;
}

static int scalar_double(void) {
	int fails = 0;

	ScalarDouble const s = ScalarDouble_new(2.5, 11);
	CHECK(s.val == 2.5 && s.id == 11);

	CHECK(ScalarDouble_eq(&s, &(ScalarDouble){.val = 2.5, .id = 11}));
	CHECK(!ScalarDouble_eq(&s, &(ScalarDouble){.val = 2.5, .id = 12}));

	CHECK(ScalarDouble_cmp(&s, &(ScalarDouble){.val = 2.5, .id = 11}) == cerive_equal);
	CHECK(ScalarDouble_cmp(&s, &(ScalarDouble){.val = 2.5, .id = 10}) == cerive_greater);

	CHECK(ScalarDouble_hash(&s) == ScalarDouble_hash(&(ScalarDouble){.val = 2.5, .id = 11}));
	CHECK(ScalarDouble_hash(&s) != ScalarDouble_hash(&(ScalarDouble){.val = 0.0, .id = 11}));

	char buf[64];
	ScalarDouble_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarDouble {") == buf);

	return fails;
}

static int scalar_int(void) {
	int fails = 0;

	ScalarInt const s = ScalarInt_new(-99, 12);
	CHECK(s.val == -99 && s.id == 12);

	CHECK(ScalarInt_eq(&s, &(ScalarInt){.val = -99, .id = 12}));
	CHECK(!ScalarInt_eq(&s, &(ScalarInt){.val = -99, .id = 13}));

	CHECK(ScalarInt_cmp(&s, &(ScalarInt){.val = -99, .id = 12}) == cerive_equal);
	CHECK(ScalarInt_cmp(&s, &(ScalarInt){.val = -99, .id = 11}) == cerive_greater);

	CHECK(ScalarInt_hash(&s) == ScalarInt_hash(&(ScalarInt){.val = -99, .id = 12}));
	CHECK(ScalarInt_hash(&s) != ScalarInt_hash(&(ScalarInt){.val = 0, .id = 12}));

	char buf[64];
	ScalarInt_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarInt {") == buf);

	return fails;
}

static int scalar_unsigned(void) {
	int fails = 0;

	ScalarUnsigned const s = ScalarUnsigned_new(42u, 13);
	CHECK(s.val == 42u && s.id == 13);

	CHECK(ScalarUnsigned_eq(&s, &(ScalarUnsigned){.val = 42u, .id = 13}));
	CHECK(!ScalarUnsigned_eq(&s, &(ScalarUnsigned){.val = 42u, .id = 14}));

	CHECK(ScalarUnsigned_cmp(&s, &(ScalarUnsigned){.val = 42u, .id = 13}) == cerive_equal);
	CHECK(ScalarUnsigned_cmp(&s, &(ScalarUnsigned){.val = 42u, .id = 12}) == cerive_greater);

	CHECK(ScalarUnsigned_hash(&s) == ScalarUnsigned_hash(&(ScalarUnsigned){.val = 42u, .id = 13}));
	CHECK(ScalarUnsigned_hash(&s) != ScalarUnsigned_hash(&(ScalarUnsigned){.val = 0u, .id = 13}));

	char buf[64];
	ScalarUnsigned_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarUnsigned {") == buf);

	return fails;
}

static int scalar_long(void) {
	int fails = 0;

	ScalarLong const s = ScalarLong_new(-777L, 14);
	CHECK(s.val == -777L && s.id == 14);

	CHECK(ScalarLong_eq(&s, &(ScalarLong){.val = -777L, .id = 14}));
	CHECK(!ScalarLong_eq(&s, &(ScalarLong){.val = -777L, .id = 99}));

	CHECK(ScalarLong_cmp(&s, &(ScalarLong){.val = -777L, .id = 14}) == cerive_equal);
	CHECK(ScalarLong_cmp(&s, &(ScalarLong){.val = -777L, .id = 15}) == cerive_less);

	CHECK(ScalarLong_hash(&s) == ScalarLong_hash(&(ScalarLong){.val = -777L, .id = 14}));
	CHECK(ScalarLong_hash(&s) != ScalarLong_hash(&(ScalarLong){.val = 0L, .id = 14}));

	char buf[64];
	ScalarLong_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarLong {") == buf);

	return fails;
}

static int scalar_ulong(void) {
	int fails = 0;

	ScalarULong const s = ScalarULong_new(888UL, 15);
	CHECK(s.val == 888UL && s.id == 15);

	CHECK(ScalarULong_eq(&s, &(ScalarULong){.val = 888UL, .id = 15}));
	CHECK(!ScalarULong_eq(&s, &(ScalarULong){.val = 888UL, .id = 16}));

	CHECK(ScalarULong_cmp(&s, &(ScalarULong){.val = 888UL, .id = 15}) == cerive_equal);
	CHECK(ScalarULong_cmp(&s, &(ScalarULong){.val = 888UL, .id = 14}) == cerive_greater);

	CHECK(ScalarULong_hash(&s) == ScalarULong_hash(&(ScalarULong){.val = 888UL, .id = 15}));
	CHECK(ScalarULong_hash(&s) != ScalarULong_hash(&(ScalarULong){.val = 0UL, .id = 15}));

	char buf[64];
	ScalarULong_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarULong {") == buf);

	return fails;
}

static int scalar_llong(void) {
	int fails = 0;

	ScalarLLong const s = ScalarLLong_new(-99999LL, 16);
	CHECK(s.val == -99999LL && s.id == 16);

	CHECK(ScalarLLong_eq(&s, &(ScalarLLong){.val = -99999LL, .id = 16}));
	CHECK(!ScalarLLong_eq(&s, &(ScalarLLong){.val = -99999LL, .id = 17}));

	CHECK(ScalarLLong_cmp(&s, &(ScalarLLong){.val = -99999LL, .id = 16}) == cerive_equal);
	CHECK(ScalarLLong_cmp(&s, &(ScalarLLong){.val = -99999LL, .id = 15}) == cerive_greater);

	CHECK(ScalarLLong_hash(&s) == ScalarLLong_hash(&(ScalarLLong){.val = -99999LL, .id = 16}));
	CHECK(ScalarLLong_hash(&s) != ScalarLLong_hash(&(ScalarLLong){.val = 1LL, .id = 16}));

	char buf[64];
	ScalarLLong_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarLLong {") == buf);

	return fails;
}

static int scalar_ullong(void) {
	int fails = 0;

	ScalarULLong const s = ScalarULLong_new(123456ULL, 17);
	CHECK(s.val == 123456ULL && s.id == 17);

	CHECK(ScalarULLong_eq(&s, &(ScalarULLong){.val = 123456ULL, .id = 17}));
	CHECK(!ScalarULLong_eq(&s, &(ScalarULLong){.val = 123456ULL, .id = 18}));

	CHECK(ScalarULLong_cmp(&s, &(ScalarULLong){.val = 123456ULL, .id = 17}) == cerive_equal);
	CHECK(ScalarULLong_cmp(&s, &(ScalarULLong){.val = 123456ULL, .id = 16}) == cerive_greater);

	CHECK(ScalarULLong_hash(&s) == ScalarULLong_hash(&(ScalarULLong){.val = 123456ULL, .id = 17}));
	CHECK(ScalarULLong_hash(&s) != ScalarULLong_hash(&(ScalarULLong){.val = 0ULL, .id = 17}));

	char buf[64];
	ScalarULLong_debug(&s, buf, sizeof buf);
	CHECK(strstr(buf, "ScalarULLong {") == buf);

	return fails;
}

static int empty_struct(void) {
	int fails = 0;

	Empty const e = (Empty){};
	Empty const e2 = Empty_default();
	CHECK(Empty_eq(&e, &e2));

	char buf[32];
	Empty_debug(&e, buf, sizeof buf);
	CHECK(strstr(buf, "Empty {") == buf);

	CHECK(Empty_cmp(&e, &e) == cerive_equal);
	CHECK(Empty_hash(&e) == Empty_hash(&e2));

	return fails;
}

int main(void) {
	int const fails =
		scalar_bool()
		+ scalar_char()
		+ scalar_i8()
		+ scalar_i16()
		+ scalar_i32()
		+ scalar_i64()
		+ scalar_u8()
		+ scalar_u16()
		+ scalar_u32()
		+ scalar_u64()
		+ scalar_size()
		+ scalar_float()
		+ scalar_double()
		+ scalar_int()
		+ scalar_unsigned()
		+ scalar_long()
		+ scalar_ulong()
		+ scalar_llong()
		+ scalar_ullong()
		+ empty_struct();

	puts(fails == 0 ? "all tests passed" : "FAILURES");
	return fails;
}
