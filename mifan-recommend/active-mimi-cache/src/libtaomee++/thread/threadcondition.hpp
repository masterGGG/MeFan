/**
 *============================================================
 *  @file      threadcondition.hpp
 *  @brief     定义了thread condition的操作。更多细节请参考APUE相关章节。
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_THREADCONDITION_HPP_
#define LIBTAOMEEPP_THREADCONDITION_HPP_

extern "C" {
#include <pthread.h>
#include <sys/time.h>
}

#include <libtaomee++/utils/config.hpp>
#include <libtaomee++/utils/noncopyable.hpp>

#include <libtaomee++/thread/mutex.hpp>

namespace taomee {

/**
 * @class ThreadCond
 * @brief thread condition，派生于Noncopyable。创建对象后，可以直接判断对象是否合法。例如：\n
 *        ThreadCond c; \n
 *        if (!c) {\n
 *            // do something \n
 *        }
 */
class ThreadCond : Noncopyable {
public:
	/**
	 * @brief 构造一个的thread condition对象。
	 * @param pshared thread condition是否进程间共享，默认是不共享。
	 */
	ThreadCond(bool pshared = false);
	~ThreadCond();
	// Cond Operations
	/**
	 * @brief atomically unlocks the mutex (as per mutex.unlock()) and waits for the condition
	 *        to be signaled. The thread execution is suspended and does not consume any CPU time
	 *        until the condition is signaled. The mutex must be locked by the calling thread on
	 *        entrance to wait(). Before returning to the calling thread, wait() re-acquires mutex
	 *        (as per mutex.lock()).
	 * @param mutex the mutex to wait
	 * @return true if succeeds, false otherwise
	 */
	bool wait(Mutex& mutex);
	/**
	 * @brief 和wait()类似，但timedwait()只等待timeout时间，而wait()则一直等待。
	 * @param mutex the mutex to wait
	 * @param timeout 超时时间
	 * @return true if succeeds, false otherwise
	 */
	bool timedwait(Mutex& mutex, time_t timeout);
	/**
	 * @brief restarts all the threads that are waiting on the condition. Nothing happens
	 *        if no threads are waiting on the condition.
	 * @return true if succeeds, false otherwise
	 */
	bool broadcast();
	/**
	 * @brief restarts one of the threads that are waiting on the condition. If no threads are
	 *        waiting on the condition, nothing happens. If several threads are waiting on the
	 *        condition, exactly one is restarted, but it is not specified which.
	 * @return true if succeeds, false otherwise
	 */
	bool signal();	
	// conversion operators
	operator void*() const // for judging whether the Mutex obj is fine to use
		{ return reinterpret_cast<void*>(!err_); }
private:
	pthread_cond_t cond_;

	int err_;
};

#ifndef NONINLINE
// implement inline methods
#include <libtaomee++/thread/threadcondition.tcpp>
#endif

} // end of namespace taomee

#endif // LIBTAOMEEPP_THREADCONDITION_HPP_
