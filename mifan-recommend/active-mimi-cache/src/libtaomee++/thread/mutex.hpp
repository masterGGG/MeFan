/**
 *============================================================
 *  @file      mutex.hpp
 *  @brief     实现了互斥锁操作。更多细节请参考APUE相关章节。
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_MUTEX_HPP_
#define LIBTAOMEEPP_MUTEX_HPP_

extern "C" {
#include <pthread.h>
#include <sys/time.h>
}

#include <libtaomee++/utils/config.hpp>
#include <libtaomee++/utils/noncopyable.hpp>

#include <libtaomee++/thread/threadlock.hpp>

namespace taomee {
 
/**
 * @class Mutex
 * @brief 互斥锁，派生于ThreadLock。创建对象后，可以直接判断对象是否合法。例如：\n
 *        Mutex m; \n
 *        if (!m) {\n
 *            // do something \n
 *        }
 */
class Mutex : public ThreadLock {
	friend class ThreadCond;
public:
	/**
	 * @brief 互斥锁类型
	 */
	enum MutexType {
		/*!
		 * Not detect deadlock. Attempting to unlock a mutex locked by a different thread results in
		 * undefined behavior. Attempting to unlock an unlocked mutex results in undefined behavior.
		 */
		normal_mutex	= PTHREAD_MUTEX_NORMAL,
		/*!
		 * Provides error checking. A thread attempting to relock this mutex without first unlocking it shall 
		 * return with an error. A thread attempting to unlock a mutex which another thread has locked
		 * shall return with an error. A thread attempting to unlock an unlocked mutex shall return with an error. 
		 */
		error_chk_mutex	= PTHREAD_MUTEX_ERRORCHECK,
		/*!
		 * A thread attempting to relock this mutex without first unlocking it shall succeed in locking the mutex.
		 * Multiple locks of this mutex shall require the same number of unlocks. A thread attempting to unlock
		 * a mutex which another thread has locked shall return with an error. A thread attempting to unlock an
		 * unlocked mutex shall return with an error.
		 */
		recursive_mutex	= PTHREAD_MUTEX_RECURSIVE,
		/*!
		 * An implementation may map this mutex to one of the other mutex types.
		 */
		default_mutex	= PTHREAD_MUTEX_DEFAULT
	};
	// ctors
	/**
	 * @brief 构造一个默认属性的互斥锁对象。
	 */
	Mutex();
	/**
	 * @brief 构造一个的互斥锁对象。
	 * @param type 互斥锁的类型
	 * @param process_shared 互斥锁是否进程间共享，默认是不共享。
	 */
	Mutex(MutexType type, bool process_shared = false);
	// dtor
	~Mutex();
	// Mutex Operations
	bool lock();
	bool timedlock(time_t timeout);
	bool trylock();
	bool unlock();
protected:
	bool destroy();
private:
	pthread_mutex_t mutex_;
};

#ifndef NONINLINE
// implement inline methods
#include <libtaomee++/thread/mutex.tcpp>
#endif

} // end of namespace taomee

#endif /* LIBTAOMEEPP_MUTEX_HPP_ */
