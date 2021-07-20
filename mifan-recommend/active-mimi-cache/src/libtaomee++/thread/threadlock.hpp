/**
 *============================================================
 *  @file      threadlock.hpp
 *  @brief     定义了线程锁抽象类
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_THREADLOCK_HPP_
#define LIBTAOMEEPP_THREADLOCK_HPP_

#include <ctime>

#include <libtaomee++/utils/noncopyable.hpp>

namespace taomee {

/**
 * @class ThreadLock
 * @brief 线程锁抽象类，定义了线程锁的基本接口。派生于Noncopyable。
 */
class ThreadLock : Noncopyable {
public:
	// dtors
	/**
	 * @brief 抽象析构函数
	 */
	virtual ~ThreadLock() { }
	// Thread Lock Operations
	/**
	 * @brief 上锁。如果已经被锁了，则调用该函数的线程会进入阻塞状态，直到可以上锁为止。
	 * @return 成功上锁则返回true，否则返回false。
	 */
	virtual bool lock() = 0;
	/**
	 * @brief 上锁。如果已经被锁了，则调用该函数的线程会进入阻塞状态，直到可以上锁或者超时为止。
	 * @param timeout time to wait in sec
	 * @return true if lock aquired, otherwise false
	 */
	virtual bool timedlock(time_t timeout) = 0;
	/**
	 * @brief 上锁。无论是否已经被锁了，都立刻返回，不会阻塞。
	 * @return 成功上锁则返回true，否则返回false。
	 */
	virtual bool trylock() = 0;
	/**
	 * @brief 解锁。
	 * @return 成功解锁则返回true，否则返回false。
	 */
	virtual bool unlock() = 0;
	// conversion operators
	operator void*() const // for judging whether the Mutex obj is fine to use
		{ return reinterpret_cast<void*>(!err_); }
protected:
	int err_;
};

} // end of namespace taomee

#endif // LIBTAOMEEPP_THREADLOCK_HPP_
