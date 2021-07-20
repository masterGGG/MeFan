/**
 *============================================================
 *  @file      timeval.hpp
 *  @brief     封装了timeval、gettimeofday
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_TIMEVAL_HPP_
#define LIBTAOMEEPP_TIMEVAL_HPP_

extern "C" {
#include <stdint.h>

#include <sys/time.h>
}

namespace taomee {

/**
  * @brief timeval的封装
  */
class TimeVal {
public:
	/**
	  * @brief 构造一个TimeVal对象，里面保存当前时间，精确度同timeval
	  */
	TimeVal()
	{
		update();
	}

	/**
	  * @brief 使用tv构造TimeVal。
	  * @param tv
	  */
	explicit TimeVal(const timeval& tv)
	{
		m_tv = tv;
	}

	/**
	  * @brief 添加ms毫秒到TimeVal
	  * @param ms 毫秒
	  */
	void ms_timeadd(uint32_t ms)
	{
		m_tv.tv_sec  += ms / 1000;
		m_tv.tv_usec += (ms % 1000) * 1000;
		if (m_tv.tv_usec > 999999) {
			m_tv.tv_sec  += m_tv.tv_usec / 1000000;
			m_tv.tv_usec  = m_tv.tv_usec % 1000000;
		}
	}

	/**
	  * @brief 更新TimeVal对象到当前时间，精确度同timeval
	  */
	void update()
	{
		gettimeofday(&m_tv, 0);
	}

public:
	/**
	  * @brief operator =
	  */
	const TimeVal& operator =(const timeval& tv)
	{
		m_tv = tv;
		return *this;
	}

private:
	timeval m_tv;

public:
	/**
	  * @brief 重载小于运算符
	  */
	friend bool operator <(const TimeVal& t1, const TimeVal& t2);
	/**
	  * @brief operator =
	  */
	friend bool operator ==(const TimeVal& t1, const TimeVal& t2);
	/**
	  * @brief operator <=
	  */
	friend bool operator <=(const TimeVal& t1, const TimeVal& t2);
};

inline bool operator <(const TimeVal& t1, const TimeVal& t2)
{
	return (t1.m_tv.tv_sec < t2.m_tv.tv_sec) || (t1.m_tv.tv_sec == t2.m_tv.tv_sec && t1.m_tv.tv_usec < t2.m_tv.tv_usec);
}

inline bool operator ==(const TimeVal& t1, const TimeVal& t2)
{
	return (t1.m_tv.tv_sec == t2.m_tv.tv_sec && t1.m_tv.tv_usec == t2.m_tv.tv_usec);
}

inline bool operator <=(const TimeVal& t1, const TimeVal& t2)
{
	return (t1 < t2) || (t1 == t2);
}

}

#endif // LIBTAOMEEPP_TIMEVAL_HPP_
