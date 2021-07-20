/**
 *============================================================
 *  @file      bitmanip.hpp
 *  @brief     位操作函数
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_BITMANIP_HPP_
#define LIBTAOMEEPP_BITMANIP_HPP_

#include <cassert>
#include <climits>
#include <cstddef>

extern "C" {
#include <stdint.h>
}

namespace taomee {

#if 0
/**
 * @brief 计算`val`中值为1的位的个数。
 * @param uint8_t val
 * @return int, `val`中值为1的位的个数。
 */
static inline int
nbits_on8(uint8_t val)
{
	val = ((val & 0xAA) >> 1) + (val & 0x55);
	val = ((val & 0xCC) >> 2) + (val & 0x33);
	val = ((val & 0xF0) >> 4) + (val & 0x0F);

	return val;
}

// 计算`val_`里面值为1的位的个数。
#define NBITS_ON(val_, nloops_, cnt_) \
		do { \
			int i = 0; \
			for (; i != (nloops_); ++i) { \
				(cnt_) += nbits_on8( *(((uint8_t*)&(val_)) + i) ); \
			} \
		} while (0)

/**
 * @brief 计算`val`中值为1的位的个数。
 * @param uint16_t val
 * @return int, `val`中值为1的位的个数。
 */
static inline int
nbits_on16(uint16_t val)
{
	int cnt = 0;
	NBITS_ON(val, 2, cnt);

	return cnt;
}

/**
 * @brief 计算`val`中值为1的位的个数。
 * @param uint32_t val
 * @return int, `val`中值为1的位的个数。
 */
static inline int
nbits_on32(uint32_t val)
{
	int cnt = 0;
	NBITS_ON(val, 4, cnt);

	return cnt;
}

/**
 * @brief 计算`val`中值为1的位的个数。
 * @param uint64_t val
 * @return int, `val`中值为1的位的个数。
 */
static inline int
nbits_on64(uint64_t val)
{
	int cnt = 0;
	NBITS_ON(val, 8, cnt);

	return cnt;
}
#endif

static uint64_t bit64_table[64] = {
	1ull, 1ull << 1, 1ull << 2, 1ull << 3, 1ull << 4, 1ull << 5,
	1ull << 6, 1ull << 7, 1ull << 8, 1ull << 9, 1ull << 10, 1ull << 11,
	1ull << 12, 1ull << 13, 1ull << 14, 1ull << 15,	1ull << 16, 1ull << 17,
	1ull << 18, 1ull << 19, 1ull << 20, 1ull << 21, 1ull << 22, 1ull << 23,
	1ull << 24, 1ull << 25, 1ull << 26, 1ull << 27, 1ull << 28, 1ull << 29,
	1ull << 30, 1ull << 31, 1ull << 32, 1ull << 33, 1ull << 34, 1ull << 35,
	1ull << 36, 1ull << 37, 1ull << 38, 1ull << 39, 1ull << 40, 1ull << 41,
	1ull << 42, 1ull << 43, 1ull << 44, 1ull << 45, 1ull << 46, 1ull << 47,
	1ull << 48, 1ull << 49,	1ull << 50, 1ull << 51, 1ull << 52, 1ull << 53,
	1ull << 54, 1ull << 55,	1ull << 56, 1ull << 57, 1ull << 58, 1ull << 59,
	1ull << 60, 1ull << 61, 1ull << 62, 1ull << 63
};

static uint64_t bit64_not_table[64] = {
	~(1ull), ~(1ull << 1), ~(1ull << 2), ~(1ull << 3), ~(1ull << 4), ~(1ull << 5),
	~(1ull << 6), ~(1ull << 7), ~(1ull << 8), ~(1ull << 9), ~(1ull << 10), ~(1ull << 11),
	~(1ull << 12), ~(1ull << 13), ~(1ull << 14), ~(1ull << 15), ~(1ull << 16), ~(1ull << 17),
	~(1ull << 18), ~(1ull << 19), ~(1ull << 20), ~(1ull << 21), ~(1ull << 22), ~(1ull << 23),
	~(1ull << 24), ~(1ull << 25), ~(1ull << 26), ~(1ull << 27), ~(1ull << 28), ~(1ull << 29),
	~(1ull << 30), ~(1ull << 31), ~(1ull << 32), ~(1ull << 33), ~(1ull << 34), ~(1ull << 35),
	~(1ull << 36), ~(1ull << 37), ~(1ull << 38), ~(1ull << 39), ~(1ull << 40), ~(1ull << 41),
	~(1ull << 42), ~(1ull << 43), ~(1ull << 44), ~(1ull << 45), ~(1ull << 46), ~(1ull << 47),
	~(1ull << 48), ~(1ull << 49), ~(1ull << 50), ~(1ull << 51), ~(1ull << 52), ~(1ull << 53),
	~(1ull << 54), ~(1ull << 55), ~(1ull << 56), ~(1ull << 57), ~(1ull << 58), ~(1ull << 59),
	~(1ull << 60), ~(1ull << 61), ~(1ull << 62), ~(1ull << 63)
};

/**
 * @brief 把数组arr的第pos位设为1。\n
 *        pos      -= 1;\n
 *        size_t i  = pos / (sizeof(T) * CHAR_BIT);\n
 *        arr[i]   |= (static_cast<T>(1) << (pos % (sizeof(T) * CHAR_BIT)));
 * @param arr
 * @param pos
 */
template <typename T, size_t N>
void set_bit_on(T (&arr)[N], size_t pos)
{
	assert((pos > 0) && (pos <= sizeof(arr) * CHAR_BIT));

	pos 	 -= 1;
	size_t i  = pos / (sizeof(T) * CHAR_BIT);
	arr[i]	 |= bit64_table[pos % (sizeof(T) * CHAR_BIT)];
}

/**
 * @brief 把val的第pos位设为1。
 * @param val
 * @param pos
 * @return 第pos位被设为1后的值。
 */
template <typename T>
T set_bit_on(T val, size_t pos)
{
	assert((pos > 0) && (pos <= sizeof(T) * CHAR_BIT));

	return (val | bit64_table[pos - 1]);
}

/**
 * @brief 把数组arr的第pos位设为0。\n
 *        pos      -= 1;\n
 *        size_t i  = pos / (sizeof(T) * CHAR_BIT);\n
 *        arr[i]   &= ~(static_cast<T>(1) << (pos % (sizeof(T) * CHAR_BIT)));
 * @param arr
 * @param pos
 */
template <typename T, size_t N>
void set_bit_off(T (&arr)[N], size_t pos)
{
	assert((pos > 0) && (pos <= sizeof(arr) * CHAR_BIT));

	pos 	 -= 1;
	size_t i  = pos / (sizeof(T) * CHAR_BIT);
	arr[i]	 &= bit64_not_table[pos % (sizeof(T) * CHAR_BIT)];
}

/**
 * @brief 把val的第pos位设为0。
 * @param val
 * @param pos
 * @return 第pos位被设为0后的值。
 */
template <typename T>
T set_bit_off(T val, size_t pos)
{
	assert((pos > 0) && (pos <= sizeof(T) * CHAR_BIT));

	return (val & bit64_not_table[pos - 1]);
}

/**
 * @brief 检测arr的第pos位是否为1。\n
 *        pos      -= 1;\n
 *        size_t i  = pos / (sizeof(T) * CHAR_BIT);\n
 *        return (arr[i] & (static_cast<T>(1) << (pos % (sizeof(T) * CHAR_BIT))));
 * @param arr
 * @param pos
 * @return true or false
 */
template <typename T, size_t N>
bool test_bit_on(T (&arr)[N], size_t pos)
{
	assert((pos > 0) && (pos <= sizeof(arr) * CHAR_BIT));

	pos 	 -= 1;
	size_t i  = pos / (sizeof(T) * CHAR_BIT);
	return (arr[i] & bit64_table[pos % (sizeof(T) * CHAR_BIT)]);
}


/**
 * @brief 检测val的第pos位是否为1。
 * @param val
 * @param pos
 * @return true or false
 */
template <typename T>
bool test_bit_on(T val, size_t pos)
{
	assert((pos > 0) && (pos <= sizeof(T) * CHAR_BIT));

	return (val & bit64_table[pos - 1]);
}

}

#endif // LIBTAOMEEPP_BITMANIP_HPP_
