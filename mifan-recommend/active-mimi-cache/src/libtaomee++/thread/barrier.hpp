/**
 *============================================================
 *  @file      barrier.hpp
 *  @brief     封装了pthread_barrier。pthread_barrier用于多线程同步。\n
 *             案例：多个线程进行分布式计算，当算到某个点时，需要等待所有线程都计算完毕才能继续往下执行，\n
 *             这时就可以用Barrier来进行同步等待。具体用法可参考doc/samples/thread下的例子。
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_BARRIER_HPP_
#define LIBTAOMEEPP_BARRIER_HPP_

#include <stdexcept>

extern "C" {
#include <pthread.h>
}

#include <libtaomee++/utils/noncopyable.hpp>

namespace taomee {

/**
 * @brief 封装了pthread_barrier。pthread_barrier用于多线程同步。\n
 *        案例：多个线程进行分布式计算，当算到某个点时，需要等待所有线程都计算完毕才能继续往下执行，\n
 *        这时就可以用Barrier来进行同步等待。具体用法可参考doc/samples/thread下的例子。
 */
class Barrier : Noncopyable {
public:
	// The count argument specifies the number of threads that must call pthread_barrier_wait() before any of them
	// successfully return from the call. The value specified by count must be greater than zero.
	/**
	 * @brief 默认构造函数
	 * @param cnt 指定当调用wait()的线程数达到cnt个后，wait()才会返回。cnt必须不等于0，默认是2。
	 * @param process_shared 设置Barrier是否进程间共享，默认是不共享。\n
	 *        如果系统支持Barrier进程间共享，则当该参数为true时，所有能够访问该Barrier的进程都可以对其进行操作；\n
	 *        如果系统不支持，或者该参数为false（默认值），则Barrier只能被单个进程内的不同线程操作。
	 * @throw runtime_error
	 */
	Barrier(unsigned int cnt = 2, bool process_shared = false);
	// dtor
	~Barrier();

	/**
	 * @brief 用于线程同步。当调用该函数的线程数达到cnt个（构造时指定）时，这些线程才能继续往下执行。
	 * @return 成功返回true，失败返回false。
	 */
	bool wait();

private:
	pthread_barrier_t bar_;
};

inline
Barrier::Barrier(unsigned int cnt, bool process_shared)
{
	pthread_barrierattr_t attr;
	// The pthread_barrierattr_init() function shall initialize a barrier attributes object with the default value for
	// all of the attributes defined by the implementation.
	if (pthread_barrierattr_init(&attr) != 0) {
		throw std::runtime_error("No Enough Memory!");
	}
	if (process_shared) {
		pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
	}

	if (pthread_barrier_init(&bar_, &attr, cnt) != 0) {
		throw std::runtime_error("No Enough Memory Or Invalid Count Specified!");
	}

	pthread_barrierattr_destroy(&attr);
}

inline
Barrier::~Barrier()
{
	pthread_barrier_destroy(&bar_);
}

inline bool
Barrier::wait()
{
	int ret = pthread_barrier_wait(&bar_);
	return ((ret == 0) || (ret == PTHREAD_BARRIER_SERIAL_THREAD));
}

} // end of namespace taomee

#endif // LIBTAOMEEPP_BARRIER_HPP_
