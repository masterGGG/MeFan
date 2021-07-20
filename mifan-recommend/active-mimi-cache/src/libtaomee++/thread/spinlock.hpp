/**
 *============================================================
 *  @file      spinlock.hpp
 *  @brief     实现了自旋锁操作。更多细节请参考http://pubs.opengroup.org/onlinepubs/009695399/functions/pthread_spin_destroy.html
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_SPINLOCK_HPP_
#define LIBTAOMEEPP_SPINLOCK_HPP_

extern "C" {
#include <pthread.h>
}

#include <libtaomee++/thread/threadlock.hpp>

namespace taomee {
 
/**
 * @brief 自旋锁，派生于ThreadLock。创建对象后，可以直接判断对象是否合法。例如：\n
 *        SpinLock m; \n
 *        if (!m) {\n
 *            // do something \n
 *        }
 */
class SpinLock : public ThreadLock {
public:
	// ctor
	/**
	 * @brief 构造一个的自旋锁对象。
	 * @param process_shared 自旋锁是否进程间共享，默认是不共享。\n
	 *        如果系统支持自旋锁进程间共享，则当该参数为true时，所有能够访问该自旋锁的进程都可以对其进行操作；\n
	 *        如果系统不支持，或者该参数为false（默认值），则自旋锁只能被单个进程内的不同线程操作。
	 */
	SpinLock(bool process_shared = false);
	// dtor
	~SpinLock();

	// SpinLock Operations
	bool lock();
	/**
	  * @brief 自旋锁不支持timelock操作
	  * @return 总是返回false
	  */
	bool timedlock(time_t timeout);
	bool trylock();
	bool unlock();

private:
	pthread_spinlock_t spin_;
};

inline
SpinLock::SpinLock(bool process_shared)
{
	// The results are undefined if pthread_spin_init() is called specifying an already initialized spin lock.
	// The results are undefined if a spin lock is used without first being initialized.
	// If the pthread_spin_init() function fails, the lock is not initialized and the contents of lock are undefined.
	if (!process_shared) {
		err_ = pthread_spin_init(&spin_, PTHREAD_PROCESS_PRIVATE);
	} else {
		err_ = pthread_spin_init(&spin_, PTHREAD_PROCESS_SHARED);
	}
}

inline
SpinLock::~SpinLock()
{
	unlock();
	// The pthread_spin_destroy() function shall destroy the spin lock and release any resources used by the lock.
	// The results are undefined if pthread_spin_destroy() is called when a thread holds the lock.
	pthread_spin_destroy(&spin_);
}

inline bool
SpinLock::lock()
{
	// The calling thread shall acquire the lock if it is not held by another thread. Otherwise, the thread shall not return
	// from the pthread_spin_lock() call until the lock becomes available. The results are undefined if the calling thread
	// holds the lock at the time the call is made.
	return !pthread_spin_lock(&spin_);
}

inline bool
SpinLock::timedlock(time_t timeout)
{
	return false;
}

inline bool
SpinLock::trylock()
{
	// Lock the spin lock if it is not held by any thread. Otherwise, the function shall fail.
	return !pthread_spin_trylock(&spin_);
}

inline bool
SpinLock::unlock()
{
	// Release the spin lock which was locked via pthread_spin_lock() or pthread_spin_trylock(). The results are undefined
	// if the lock is not held by the calling thread. If there are threads spinning on the lock when pthread_spin_unlock()
	// is called, the lock becomes available and an unspecified spinning thread shall acquire the lock.
	return !pthread_spin_unlock(&spin_);
}

} // end of namespace taomee

#endif /* LIBTAOMEEPP_SPINLOCK_HPP_ */
