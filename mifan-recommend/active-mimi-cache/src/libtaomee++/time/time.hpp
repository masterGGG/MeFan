/**
 *============================================================
 *  @file      time.hpp
 *  @brief     时间相关的操作。
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_TIME_HPP_
#define LIBTAOMEEPP_TIME_HPP_

extern "C" {
#include <sys/time.h>
}

#include <cmath>

namespace taomee {

/**
 * @brief 比较tv1和tv2是否相等。
 * @param tv1
 * @param tv2
 * @return 相等：0；tv1 > tv2：正数；tv2 > tv1：负数。
 */
inline suseconds_t timecmp(const timeval& tv1, const timeval& tv2)
{
	if (tv1.tv_sec > tv2.tv_sec) {
		return 1;
	} else if (tv1.tv_sec == tv2.tv_sec) {
		return (tv1.tv_usec - tv2.tv_usec);
	} else {
		return -1;
	}
}

/**
 * @brief 把tv增加tmplus秒。
 * @param tv
 * @param tmplus 可以精确到微秒。例如：10.0000101。
 * @return void
 */
inline void timeadd(timeval& tv, double tmplus)
{
	if ( tmplus > 1.0 ) {
		tv.tv_sec  += static_cast<time_t>(tmplus);
		tmplus     -= static_cast<time_t>(tmplus);
	}
	tv.tv_usec += lround(tmplus * 1000000.0);
	if (tv.tv_usec > 999999) {
		tv.tv_sec  += tv.tv_usec / 1000000;
		tv.tv_usec  = tv.tv_usec % 1000000;
	}
}

/**
 * @brief 计算tv1和tv2的时间差。
 * @param tv1
 * @param tv2
 * @return tv1和tv2的时间差，精确到微秒。例如：2.0121001。
 */
inline double timediff(const timeval& tv1, const timeval& tv2)
{
	return tv1.tv_sec - tv2.tv_sec + (tv1.tv_usec - tv2.tv_usec)/1000000.0;
}

/**
 * @brief 计算tv1和tv2的时间差。
 * @param tv1
 * @param tv2
 * @return tv1和tv2的时间差，精确到毫秒。例如：200毫秒。
 */
inline int timediff2(const timeval& tv1, const timeval& tv2)
{
	return (tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec)/1000;
}

} // namespace taomee

#endif // LIBTAOMEEPP_TIME_HPP_
