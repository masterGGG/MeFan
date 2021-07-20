/**
 *============================================================
 *  @file      rwlock.hpp
 *  @brief     实现了读写锁操作。更多细节请参考APUE相关章节。
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_RWLOCK_HPP_
#define LIBTAOMEEPP_RWLOCK_HPP_

extern "C" {
#include <pthread.h>
#include <sys/time.h>
}

#include <libtaomee++/utils/config.hpp>
#include <libtaomee++/utils/noncopyable.hpp>

#include <libtaomee++/thread/threadlock.hpp>

namespace taomee {

/**
 * @class Rwlock
 * @brief 读写锁，派生于ThreadLock。创建对象后，可以直接判断对象是否合法。例如：\n
 *        Rwlock rw; \n
 *        if (!rw) {\n
 *            // do something \n
 *        }
 */
class Rwlock : public ThreadLock {
public:
	// ctors
	/**
	 * @brief 构造一个的读写锁对象。
	 * @param pshared 读写锁是否进程间共享，默认是不共享。
	 */
	Rwlock(bool pshared = false);
	// dtors
	~Rwlock();
	// Rwlock Operations
	bool lock();
	bool timedlock(time_t timeout);
	bool trylock();
	bool unlock();	
	// Extended Operations
	/**
	 * @brief 类似lock()，但rdlock()只上读锁，而lock()上读写锁。
	 * @return 成功则返回true，失败则返回false。
	 * @see lock
	 */
	bool rdlock();
	/**
	 * @brief 类似timedlock()，但timedrdlock()只上读锁，而timedlock()上读写锁。
	 * @param timeout 超时时间
	 * @return 成功则返回true，失败则返回false。
	 * @see timedlock
	 */
	bool timedrdlock(time_t timeout);
	/**
	 * @brief 类似trylock()，但tryrdlock()只上读锁，而trylock()上读写锁。
	 * @return 成功则返回true，失败则返回false。
	 * @see trylock
	 */
	bool tryrdlock();
protected:
	bool destroy();
private:
	pthread_rwlock_t rwlock_;
};

#ifndef NONINLINE
// implement inline methods
#include <libtaomee++/thread/rwlock.tcpp>
#endif

} // end namespace taomee

#endif // LIBTAOMEEPP_RWLOCK_HPP_
