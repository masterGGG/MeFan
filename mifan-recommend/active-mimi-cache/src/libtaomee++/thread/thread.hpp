/**
 *============================================================
 *  @file      thread.hpp
 *  @brief     实现了线程的基本操作。更多细节请参考APUE相关章节。
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_THREAD_HPP_
#define LIBTAOMEEPP_THREAD_HPP_

#include <cassert>

extern "C" {
#include <pthread.h>
#include <signal.h>
}

#include <libtaomee++/utils/config.hpp>
#include <libtaomee++/utils/noncopyable.hpp>

#include <libtaomee++/thread/mutex.hpp>
#include <libtaomee++/thread/threadcondition.hpp>

namespace taomee {

/**
 * @class Thread
 * @brief 线程抽象类，派生于Noncopyable。Thread是个抽象类，要使用这个类的功能，必须定义该类的派生类，
 *        并且派生类要实现run()函数。\n
 *        创建对象后，可以直接判断对象是否合法。例如：\n
 *        class Worker : public Thread { //... };\n
 *        Worker w; \n
 *        if (!w) {\n
 *            // 对象创建不成功
 *            // do something \n
 *        }\n
 *        w.start();
 */
class Thread : Noncopyable {
public:
	// typedefs
	/**
	 * @brief 线程ID类型
	 */
	typedef pthread_t thread_id_type;
	// enums
	/**
	 * @brief 线程调度粒度类型
	 */
	enum scope_type {
		/*! 系统级别调度 */
		system_scope	= PTHREAD_SCOPE_SYSTEM,  // system scheduling contention scope
		/*! 进程级别调度 */
		process_scope	= PTHREAD_SCOPE_PROCESS  // process scheduling contention scope
	};
	// ctor
	/**
	 * @brief 构造线程对象
	 * @param detached 创建的线程是否处于detach状态，默认是“否”。
	 * @param scope 创建的线程以何种粒度进行调度，默认是系统级别。
	 */
	Thread(bool detached = false, scope_type scope = system_scope);
	// dtor
	virtual ~Thread();
	//
	/**
	 * @brief get the thread creation stack attributes stackaddr and stacksize
	 * @return true if succeeds, otherwise false
	 */
	bool get_stack(void*& stackaddr, size_t& stacksize) const;
	/**
	 * @brief set the thread creation stack attributes stackaddr and stacksize
	 * @return true if succeeds, otherwise false
	 */
	bool set_stack(void* stackaddr, size_t stacksize);
	//
	/**
	 * @brief get the thread creation stacksize attribute
	 * @return true if succeeds, otherwise false
	 */
	bool get_stacksize(size_t& stacksize) const;
	/**
	 * @brief set the thread creation stacksize attribute
	 * @return true if succeeds, otherwise false
	 */
	bool set_stacksize(size_t stacksize);

	/**
	 * @brief get the thread guardsize attribute
	 * @return true if succeeds, otherwise false
	 */
	bool get_guardsize(size_t& guardsize) const;
	/**
	 * @brief set the thread guardsize attribute
	 * @return true if succeeds, otherwise false
	 */
	bool set_guardsize(size_t guardsize);

	/**
	 * @brief Start a thread. Only one thread is running for a given instance.
	 *        start() always returns false if a thread for the given instance is running. 
	 * @return true if succeeds, otherwise false
	 */
	bool start();                 // start a thread
	/**
	 * @brief Join a thread. Block until the thread is ended. By calling join,
	 *        we automatically place the thread in the detached state so that
	 *        its resources can be recovered.
	 * @return true if succeeds, otherwise false
	 */
	bool join();                  // join a thread
	/**
	 * @brief detach a thread
	 * @return true if succeeds, otherwise false
	 */
	bool detach();              // detach a thread
	/**
	 * @brief cancel a running thread.\n
	 *        You cannot use cancellation unless all the code that a thread might be
	 *        running when it is cancelled knows how to handle being cancelled and
	 *        sets appropriate modes and handlers. You must not ever cancel a thread
	 *        that is or might be running C++ standard library code unless you know
	 *        that your C++ standard library is cancellation safe.\n
	 *        A thread can elect to ignore or otherwise control how it is canceled.
	 *        cancel *doesn't wait* for the thread to terminate. It merely makes the request.
	 * @return true if succeeds, otherwise false
	 */
	bool cancel();              // cancel the running thread
	/**
	 * @brief Send a signal to a thread. We can pass a signo value of 0 to check for existence of the thread
	 * @return true if succeeds, otherwise false
	 */
	bool kill(int signo = 0);	 // send a signal to a thread

	/**
	 * @brief  Check if the thread is joinable
	 * @return true if it is, otherwise false
	 */
	bool joinable() const;       // if thread is joinable
	/**
	 * @brief  Check if the thread is running
	 * @return true if it is, otherwise false
	 */
	bool running() const;       // if thread is running

	/**
	 * @brief  get thread id of the running thread
	 * @return ID of the thread
	 */
	const thread_id_type& tid() const;  // return thread ID
	/**
	 * @brief  compare if the given thread id 'id' is equal to the ID of the running thread
	 * @return true if equal, false otherwise
	 */
	bool equal(const thread_id_type& id) const; // compare thread ID

	// conversion operators
	operator void*() const // for judging whether the Mutex obj is fine to use
		{ return reinterpret_cast<void*>(!err_); }

	// static methods
	/**
	 * @brief get the level of concurrency
	 * @return level of concurrency
	 */
	static int  get_concurrency();
	/**
	 * @brief set the level of concurrency
	 * @return true if succeeds, otherwise false
	 */
	static bool set_concurrency(int level);
	/**
	 * @brief Examine and change blocked signals.\n
	 *        If there are any pending unblocked signals after the call, at least one of those signals shall be
	 * 		  delivered before the call returns. It is not possible to block those signals which cannot be ignored.
	 * 		  If any of the SIGFPE, SIGILL, SIGSEGV, or SIGBUS signals are generated while they are blocked,
	 * 		  the result is undefined, unless the signal was generated by the kill function, the sigqueue function,
	 * 		  or the raise function. If sigmask_op fails, the thread's signal mask shall not be changed.	
	 * @param how indicates the way in which the set is changed\n
	 *               SIG_BLOCK - resulting set shall be the union of the current set and the signal set pointed to by set\n
	 *               SIG_SETMASK - resulting set shall be the signal set pointed to by set\n
	 *               SIG_UNBLOCK - resulting set shall be the intersection of the current set and the complement
	 *                             of the signal set pointed to by set
	 * @param set if it's a null pointer, the value of the argument how is not significant and the thread's signal mask
	 *        shall be unchanged; thus the call can be used to enquire about currently blocked signals
	 * @param old_set if it's not a null pointer, the previous mask shall be stored in it
	 * @return true if succeeded, otherwise false
	 */
	static bool sigmask_op(int how, const sigset_t* set, sigset_t* old_set = 0);
	/**
	 * @brief Register fork handlers.\n
	 *        We can call at_fork multiple times to install more than one set of fork handlers.
	 * 	      When multiple fork handlers are used, the order in which the handlers are called differs.
	 * 		  The after_fork and after_fork_child fork handlers are called in the order in which they
	 * 		  were registered, whereas the b4_fork fork handlers are called in the opposite order from
	 * 		  which they were registered.	
	 * @return: true if succeeds, otherwise false
	 */
	static bool at_fork(void (*b4_fork)() = 0, void (*after_fork)() = 0, void (*after_fork_child)() = 0);
protected:
	// enums
	/**
	 * @brief cancel线程的方法
	 */
	enum cancel_type {
		/*! cancel the calling thread as soon as the cancellation request is received */
		deferred_cancel	= PTHREAD_CANCEL_DEFERRED,
		/*! keep the cancellation request pending until the next cancellation point */
		asyn_cancel		= PTHREAD_CANCEL_ASYNCHRONOUS
	};
	/**
	 * @brief Virtual function to be implementated by a derived class.
	 *        The implementation of this function should carry the actual work that
	 *        is to be done by the newly created thread.
	 *        Always avoid using C++ libs to implement run(), especially cout/cerr!
	 */
	virtual void run() = 0;
	/**
	 * @brief exit the thread. rval_ptr is returned to join
	 */
	void exit(void* rval_ptr) { pthread_exit(rval_ptr); }

	// static methods
	/**
	 * @brief atomically set the calling thread's cancelability state and return the previous state.\n
	 *        When the state is set to PTHREAD_CANCEL_DISABLE, a call to cancel will not kill the
	 * 	      thread. Instead, the cancellation request remains pending for the thread. When the state is enabled
	 * 		  again, the thread will act on any pending cancellation requests at the next cancellation point.
	 * @param cancelable 线程是否可以取消，默认是可以。
	 * @param old_cancelable 原来的状态将会被保存到old_cancelable里。
	 * @return: true if succeeds, otherwise false
	 */
	static bool set_cancelability(bool cancelable = true, bool* old_cancelable = 0);
	/**
	 * @brief set the calling thread's cancelability type to the indicated type and return the previous type
	 * @param type 取消类型，默认是deferred_cancel。
	 * @param old_type 原来的取消类型。
	 * @return: true if succeeds, otherwise false
	 */
	static bool set_canceltype(cancel_type type = deferred_cancel, cancel_type* old_type = 0);
	/**
	 * @brief create a cancellation point in the calling thread\n
	 *        When calling test_cancel, if a cancellation request is pending and if cancellation has not
	 * 	      been disabled, the thread will be canceled. When cancellation is disabled, however, calls to
	 * 		  test_cancel have no effect.
	 */
	static void test_cancel();
private:
	static void* call_run(void* t);      // used by pthread_create
	static void  reset_running(void* t); // set running status of `t` to false
	// private fields
	thread_id_type tid_; // thread ID
	pthread_attr_t attr_;

	int  err_;
	bool detached_; // true if thread is detached, otherwise false
	bool joinable_; // true if thread is joinable, otherwise false
	bool running_;  // true if thread is running, otherwise false
	// the two fields below are for synchronization of thread startup
	Mutex       mutex_;
	ThreadCond  cond_;
};

#ifndef NONINLINE
// implement inline methods
#include <libtaomee++/thread/thread.tcpp>
#endif

} // end namespace taomee

#endif // LIBTAOMEEPP_THREAD_HPP_
