/*
 * umsgpack_test.c: MessagePack for MCUs
 * =====================================
 *
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2015-2016 ryochack
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include "umsgpack.h"
#include "minunit/minunit.h"

#define FORMAT_MAX_SIZE 9
struct umsgpack_packer_buf *m_pack = NULL;

static const char m_char_patterns[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()-_=+{}[]{}|;:',.<>/?";
static bool m_is_bigendian = false;

struct {
	const char *pattern;
	const size_t size;
} str_test_data = {
	.pattern = m_char_patterns,
	.size = sizeof(m_char_patterns),
};


void test_setup(void) {
}

void test_teardown(void) {
	umsgpack_free(m_pack);
	m_pack = NULL;
}

static void judge_system_endian(void) {
	uint16_t v = 0x0100;
	m_is_bigendian = (*(uint8_t*)&v);
}

static inline uint16_t _be16(uint16_t v) {
	if (m_is_bigendian) return v;
	const uint8_t *b = (const uint8_t*)&v;
	return ((uint16_t)b[0] << 8) | (uint16_t)b[1];
}

static inline uint32_t _be32(uint32_t v) {
	if (m_is_bigendian) return v;
	const uint8_t *b = (const uint8_t*)&v;
	return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16)
		| ((uint32_t)b[2] << 8) | (uint32_t)b[3];
}

static inline uint64_t _be64(uint64_t v) {
	if (m_is_bigendian) return v;
	const uint8_t *b = (const uint8_t*)&v;
	return ((uint64_t)b[0] << 56) | ((uint64_t)b[1] << 48)
		| ((uint64_t)b[2] << 40) | ((uint64_t)b[3] << 32)
		| ((uint64_t)b[4] << 24) | ((uint64_t)b[5] << 16)
		| ((uint64_t)b[6] << 8) | (uint64_t)b[7];
}

MU_TEST(test_positive_fixint) {
	/* 0x00 - 0x7f */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (uint8_t expects = 0; expects < 0x7f; expects++) {
		mu_check( umsgpack_pack_uint(m_pack, expects) );
		// length
		mu_assert_int_eq(1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, m_pack->data[0]);
		m_pack->pos = 0;
	}
}

MU_TEST(test_fixmap) {
	/* 0x80 - 0x8f */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (int8_t i = 0; i < 0x0f; i++) {
		uint8_t expects = 0x80 | i;
		mu_check( umsgpack_pack_map(m_pack, i) );
		// length
		mu_assert_int_eq(1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, m_pack->data[0]);
		m_pack->pos = 0;
	}
}

MU_TEST(test_fixarray) {
	/* 0x90 - 0x9f (b1001XXXX) */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (int8_t i = 0; i < 0x0f; i++) {
		uint8_t expects = 0x90 | i;
		mu_check( umsgpack_pack_array(m_pack, i) );
		// length
		mu_assert_int_eq(1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, m_pack->data[0]);
		m_pack->pos = 0;
	}
}

MU_TEST(test_fixstr) {
	/* 0xa0 - 0xbf (b101XXXXX) 0~31bytes */
	const size_t str_length = 31;
	const size_t data_size = FORMAT_MAX_SIZE + str_length;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (int8_t i = 0; i <= (int8_t)str_length; i++) {
		uint8_t expects = 0xa0 | i;
		mu_check( umsgpack_pack_str(m_pack, (char*)str_test_data.pattern, i) );
		// length
		mu_assert_int_eq(i+1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, m_pack->data[0]);
		// string
		for (int8_t j = 0; j < i; j++) {
			mu_assert_int_eq(str_test_data.pattern[j], m_pack->data[j+1]);
		}
		m_pack->pos = 0;
	}
}

MU_TEST(test_nil) {
	/* 0xc0 */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	uint8_t expects = 0xc0;
	mu_check( umsgpack_pack_nil(m_pack) );
	// length
	mu_assert_int_eq(1, m_pack->pos);
	// format
	mu_assert_int_eq(expects, (uint8_t)m_pack->data[0]);
}

MU_TEST(test_false) {
	/* 0xc2 */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	uint8_t expects = 0xc2;
	mu_check( umsgpack_pack_bool(m_pack, 0) );
	// length
	mu_assert_int_eq(1, m_pack->pos);
	// format
	mu_assert_int_eq(expects, (uint8_t)m_pack->data[0]);
}

MU_TEST(test_true) {
	/* 0xc3 */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	uint8_t expects = 0xc3;
	mu_check( umsgpack_pack_bool(m_pack, 1) );
	// length
	mu_assert_int_eq(1, m_pack->pos);
	// format
	mu_assert_int_eq(expects, (uint8_t)m_pack->data[0]);
}

MU_TEST(test_bin8) {
	/* 0xc4 + uint8-length + data... */
}

MU_TEST(test_bin16) {
	/* 0xc5 + uint16-length + data... */
}

MU_TEST(test_bin32) {
	/* 0xc6 + uint32-length + data... */
}

MU_TEST(test_ext8) {
	/* 0xc7 + uint8-length + 8bit-type + data... */
}

MU_TEST(test_ext16) {
	/* 0xc8 + uint16-length + 8bit-type + data... */
}

MU_TEST(test_ext32) {
	/* 0xc9 + uint32-length + 8bit-type + data... */
}

MU_TEST(test_float32) {
	/* 0xca + float32-value[BigEndian,IEEE754] */
	const size_t unit_size = sizeof(float);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	const uint8_t foramt = 0xca;
	const float testdata[] = { FLT_MIN, 0.0, FLT_MAX };
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		uint8_t expects = testdata[i];
		const uint32_t *actual;
		mu_check( umsgpack_pack_float(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		actual = (const uint32_t*)&m_pack->data[1];
		mu_assert_double_eq(expects, (float)_be32(*actual));
		m_pack->pos = 0;
	}
}

MU_TEST(test_float64) {
	/* 0xcb + float64-value[BigEndian,IEEE754] */
}

MU_TEST(test_uint8) {
	/* 0xcc + uint8-value(0x80~0xFF) */
	const size_t unit_size = sizeof(uint8_t);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* positive fixint: 0x00-0x7f */
	const uint8_t foramt = 0xcc;
	const uint8_t testdata[] = { 0x80, 0xA0, 0xCC, 0xF0, 0xff };
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		uint8_t expects = testdata[i];
		mu_check( umsgpack_pack_uint(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		mu_assert_int_eq(expects, m_pack->data[1]);
		m_pack->pos = 0;
	}
}

MU_TEST(test_uint16) {
	/* 0xcd + uint16-value[BigEndian](0x0100~0xFFFF) */
	const size_t unit_size = sizeof(uint16_t);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	const uint8_t foramt = 0xcd;
	const uint16_t testdata[] = { 0x0100, 0x1000, 0x2000, 0x4000, 0x8000, 0xffff };
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		uint16_t expects = testdata[i];
		const uint16_t *actual;
		mu_check( umsgpack_pack_uint(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		actual = (const uint16_t*)&m_pack->data[1];
		mu_assert_int_eq(expects, _be16(*actual));
		m_pack->pos = 0;
	}
}

MU_TEST(test_uint32) {
	/* 0xce + uint32-value[BigEndian] */
	const size_t unit_size = sizeof(uint32_t);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	const uint8_t foramt = 0xce;
	const uint32_t testdata[] = { 0x00010000, 0x10000000, 0x20000000, 0x40000000, 0x80000000, 0xffffffff };
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		uint32_t expects = testdata[i];
		const uint32_t *actual;
		mu_check( umsgpack_pack_uint(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		actual = (const uint32_t*)&m_pack->data[1];
		mu_assert_int_eq(expects, _be32(*actual));
		m_pack->pos = 0;
	}
}

MU_TEST(test_uint64) {
	/* 0xcf + uint64-value[BigEndian] */
	const size_t unit_size = sizeof(uint64_t);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	const uint8_t foramt = 0xcf;
	const uint64_t testdata[] = {
		0x0000000100000000,
		0x1000000000000000,
		0x2000000000000000,
		0x4000000000000000,
		0x8000000000000000,
		0xffffffffffffffff,
	};
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		uint64_t expects = testdata[i];
		const uint64_t *actual;
		mu_check( umsgpack_pack_uint64(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		actual = (const uint64_t*)&m_pack->data[1];
		mu_assert_int_eq(expects, _be64(*actual));
		m_pack->pos = 0;
	}
}

MU_TEST(test_int8) {
	/* 0xd0 + int8-value */
	const size_t unit_size = sizeof(int8_t);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* positive fixint: 0x00-0x7f */
	/* negative fixint: 0xe0-0xff */
	const uint8_t foramt = 0xd0;
	const int8_t testdata[] = { 0x80, 0xa0, 0xcc, 0xd0, 0xdf };
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		int8_t expects = testdata[i];
		mu_check( umsgpack_pack_int(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		mu_assert_int_eq(expects, (int8_t)m_pack->data[1]);
		m_pack->pos = 0;
	}
}

MU_TEST(test_int16) {
	/* 0xd1 + int16-value[BigEndian] */
	const size_t unit_size = sizeof(int16_t);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* negative fixint: 0xe0-0xff */
	/* int8: 0x80~0xdf */
	const uint8_t foramt = 0xd1;
	const int16_t testdata[] = { 0x8000, 0x8123, 0xc000, 0xf000, 0xff7f };
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		int16_t expects = testdata[i];
		const uint16_t *actual;
		mu_check( umsgpack_pack_int(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		actual = (const uint16_t*)&m_pack->data[1];
		mu_assert_int_eq(expects, (int16_t)_be16(*actual));
		m_pack->pos = 0;
	}
}

MU_TEST(test_int32) {
	/* 0xd2 + int32-value[BigEndian] */
	const size_t unit_size = sizeof(int32_t);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* negative fixint: 0xe0-0xff */
	/* int8 : 0x80~0xdf */
	/* int16: 0x8000~0xff7f */
	const uint8_t foramt = 0xd2;
	const int32_t testdata[] = { 0x80000000, 0x81234567, 0xc0000000, 0xf0000000, 0xffff7fff };
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		int32_t expects = testdata[i];
		const uint32_t *actual;
		mu_check( umsgpack_pack_int(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		actual = (const uint32_t*)&m_pack->data[1];
		mu_assert_int_eq(expects, (int32_t)_be32(*actual));
		m_pack->pos = 0;
	}
}

MU_TEST(test_int64) {
	/* 0xd3 + int64-value[BigEndian] */
	const size_t unit_size = sizeof(int64_t);
	const size_t data_size = FORMAT_MAX_SIZE + unit_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* negative fixint: 0xe0-0xff */
	/* int8 : 0x80~0xdf */
	/* int16: 0x8000~0xff7f */
	/* int32: 0x80000000~0xffff7fff */
	const uint8_t foramt = 0xd3;
	const int64_t testdata[] = {
		0x8000000000000000,
		0x81234567890abcde,
		0xc000000000000000,
		0xf000000000000000,
		0xffffffff7fffffff,
	};
	const int numof_testdata = sizeof(testdata) / sizeof(testdata[0]);

	for (int i = 0; i < numof_testdata; i++) {
		int64_t expects = testdata[i];
		const uint64_t *actual;
		mu_check( umsgpack_pack_int64(m_pack, expects) );
		// length
		mu_assert_int_eq(1+unit_size, m_pack->pos);
		// format
		mu_assert_int_eq(foramt, m_pack->data[0]);
		// value
		actual = (const uint64_t*)&m_pack->data[1];
		mu_assert_int_eq(expects, (int64_t)_be64(*actual));
		m_pack->pos = 0;
	}
}

MU_TEST(test_fixext1) {
	/* 0xd4 + 8bit-type + 8bit-data */
}

MU_TEST(test_fixext2) {
	/* 0xd5 + 8bit-type + 16bit-data */
}

MU_TEST(test_fixext4) {
	/* 0xd6 + 8bit-type + 32bit-data */
}

MU_TEST(test_fixext8) {
	/* 0xd7 + 8bit-type + 64bit-data */
}

MU_TEST(test_fixext16) {
	/* 0xd8 + 8bit-type + 128bit-data */
}

static void generate_pattern(char *dst, size_t len) {
	size_t remain = len;
	char *p = dst;
	while (remain > 0) {
		size_t copy_size = (remain > str_test_data.size) ? str_test_data.size : remain;
		memcpy(p, str_test_data.pattern, copy_size);
		p += copy_size;
		remain -= copy_size;
	}
}

MU_TEST(test_str8) {
	/* 0xd9 + uint8-lenght + string... */
	const size_t max_data_size = 0xff;
	const size_t data_size = FORMAT_MAX_SIZE + max_data_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* fixstr: 0~31 */
	const uint8_t format = 0xd9;
	const uint8_t str_lengths[] = { 0x20, 0xff };
	const int numof_testdata = sizeof(str_lengths) / sizeof(str_lengths[0]);

	char *ptn = malloc(max_data_size);
	if (!ptn) {
		fprintf(stderr, "%s: failed malloc(%lu). skip test.\n", __func__, max_data_size);
		return;
	}
	generate_pattern(ptn, max_data_size);

	for (int i = 0; i < numof_testdata; i++) {
		uint8_t len = str_lengths[i];
		mu_check( umsgpack_pack_str(m_pack, ptn, len) );
		// length
		mu_assert_int_eq(1+sizeof(uint8_t)+len, m_pack->pos);
		// format
		mu_assert_int_eq(format, m_pack->data[0]);
		mu_assert_int_eq(len, m_pack->data[1]);
		// string
		mu_check(!memcmp(ptn, &m_pack->data[2], len));
		m_pack->pos = 0;
	}

	free(ptn);
}

MU_TEST(test_str16) {
	/* 0xda + uint16-lenght[BigEndian] + string... */
	const size_t max_data_size = 0xffff;
	const size_t data_size = FORMAT_MAX_SIZE + max_data_size;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* fixstr: 0~31 */
	/* str8: 0x20~0xff */
	const uint8_t format = 0xda;
	const uint16_t str_lengths[] = { 0x0100, 0xffff };
	const int numof_testdata = sizeof(str_lengths) / sizeof(str_lengths[0]);

	char *ptn = malloc(max_data_size);
	if (!ptn) {
		fprintf(stderr, "%s: failed malloc(%lu). skip test.\n", __func__, max_data_size);
		return;
	}
	generate_pattern(ptn, max_data_size);

	for (int i = 0; i < numof_testdata; i++) {
		uint16_t len = str_lengths[i];
		const uint16_t *act_len;
		mu_check( umsgpack_pack_str(m_pack, ptn, len) );
		// length
		mu_assert_int_eq(1+sizeof(uint16_t)+len, m_pack->pos);
		// format
		mu_assert_int_eq(format, m_pack->data[0]);
		act_len = (const uint16_t*)&m_pack->data[1];
		mu_assert_int_eq((uint16_t)len, (uint16_t)_be16(*act_len));
		// string
		mu_check(!memcmp(ptn, &m_pack->data[3], len));
		m_pack->pos = 0;
	}

	free(ptn);
}

MU_TEST(test_str32) {
	/* 0xdb + uint32-lenght[BigEndian] + string... */
}

MU_TEST(test_array16) {
	/* 0xdc + uint16-length[BigEndian] + {N objects} */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* fixarray: 0x00-0x0f */
	const uint8_t format = 0xdc;
	const uint16_t data_lengths[] = { 0x0010, 0x8000, 0xffff };
	const int numof_testdata = sizeof(data_lengths) / sizeof(data_lengths[0]);

	for (int i = 0; i < numof_testdata; i++) {
		uint16_t len = data_lengths[i];
		const uint16_t *act_len;
		mu_check( umsgpack_pack_array(m_pack, len) );
		// length
		mu_assert_int_eq(1+sizeof(uint16_t), m_pack->pos);
		// format
		mu_assert_int_eq(format, m_pack->data[0]);
		act_len = (const uint16_t*)&m_pack->data[1];
		mu_assert_int_eq((uint16_t)len, (uint16_t)_be16(*act_len));
		m_pack->pos = 0;
	}
}

MU_TEST(test_array32) {
	/* 0xdd + uint32-lenght[BigEndian] + {N objects} */
}

MU_TEST(test_map16) {
	/* 0xde + uint16-length[BigEndian] + {N*2 objects} */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* fixmap: 0x00-0x0f */
	const uint8_t format = 0xde;
	const uint16_t data_lengths[] = { 0x0010, 0x8000, 0xffff };
	const int numof_testdata = sizeof(data_lengths) / sizeof(data_lengths[0]);

	for (int i = 0; i < numof_testdata; i++) {
		uint16_t len = data_lengths[i];
		const uint16_t *act_len;
		mu_check( umsgpack_pack_map(m_pack, len) );
		// length
		mu_assert_int_eq(1+sizeof(uint16_t), m_pack->pos);
		// format
		mu_assert_int_eq(format, m_pack->data[0]);
		act_len = (const uint16_t*)&m_pack->data[1];
		mu_assert_int_eq((uint16_t)len, (uint16_t)_be16(*act_len));
		m_pack->pos = 0;
	}
}

MU_TEST(test_map32) {
	/* 0xdf + uint32-length[BigEndian] + {N*2 objects} */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	/* fixmap: 0x00-0x0f */
	/* map16 : 0x0010~0xffff */
	const uint8_t format = 0xdf;
	const uint32_t data_lengths[] = { 0x00010000, 0x80000000, 0xffffffff };
	const int numof_testdata = sizeof(data_lengths) / sizeof(data_lengths[0]);

	for (int i = 0; i < numof_testdata; i++) {
		uint32_t len = data_lengths[i];
		const uint32_t *act_len;
		mu_check( umsgpack_pack_map(m_pack, len) );
		// length
		mu_assert_int_eq(1+sizeof(uint32_t), m_pack->pos);
		// format
		mu_assert_int_eq(format, m_pack->data[0]);
		act_len = (const uint32_t*)&m_pack->data[1];
		mu_assert_int_eq((uint32_t)len, (uint32_t)_be32(*act_len));
		m_pack->pos = 0;
	}
}

MU_TEST(test_negative_fixint) {
	/* 0xe0 - 0xff */
	const size_t data_size = FORMAT_MAX_SIZE;
	m_pack = umsgpack_alloc(data_size);
	if (!m_pack) {
		fprintf(stderr, "%s: failed umsgpack_alloc(%lu). skip test.\n", __func__, data_size);
		return;
	}

	for (int8_t expects = -32; expects < 0; expects++) {
		mu_check( umsgpack_pack_int(m_pack, expects) );
		// length
		mu_assert_int_eq(1, m_pack->pos);
		// format
		mu_assert_int_eq(expects, (int8_t)m_pack->data[0]);
		m_pack->pos = 0;
	}
}

MU_TEST_SUITE(test_suite) {
	judge_system_endian();
	MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

	MU_RUN_TEST(test_positive_fixint);
	MU_RUN_TEST(test_fixmap);
	MU_RUN_TEST(test_fixarray);
	MU_RUN_TEST(test_fixstr);
	MU_RUN_TEST(test_nil);
	MU_RUN_TEST(test_false);
	MU_RUN_TEST(test_true);
	MU_RUN_TEST(test_bin8);
	MU_RUN_TEST(test_bin16);
	MU_RUN_TEST(test_bin32);
	MU_RUN_TEST(test_ext8);
	MU_RUN_TEST(test_ext16);
	MU_RUN_TEST(test_ext32);
	MU_RUN_TEST(test_float32);
	MU_RUN_TEST(test_float64);
	MU_RUN_TEST(test_uint8);
	MU_RUN_TEST(test_uint16);
	MU_RUN_TEST(test_uint32);
	MU_RUN_TEST(test_uint64);
	MU_RUN_TEST(test_int8);
	MU_RUN_TEST(test_int16);
	MU_RUN_TEST(test_int32);
	MU_RUN_TEST(test_int64);
	MU_RUN_TEST(test_fixext1);
	MU_RUN_TEST(test_fixext2);
	MU_RUN_TEST(test_fixext4);
	MU_RUN_TEST(test_fixext8);
	MU_RUN_TEST(test_fixext16);
	MU_RUN_TEST(test_str8);
	MU_RUN_TEST(test_str16);
	MU_RUN_TEST(test_str32);
	MU_RUN_TEST(test_array16);
	MU_RUN_TEST(test_array32);
	MU_RUN_TEST(test_map16);
	MU_RUN_TEST(test_map32);
	MU_RUN_TEST(test_negative_fixint);
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return 0;
}
