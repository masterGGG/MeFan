/**
 *============================================================
 *  @file      threadpool.hpp
 *  @brief     使用模板机制实现的线程池框架。具体例子请参考
 *             thread/thrpooltest.cpp, thread/stress_test.cpp, thread/memthrpool.cpp
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_THREADPOOL_HPP_
#define LIBTAOMEEPP_THREADPOOL_HPP_

#include <queue>

#ifdef USE_BOOST_ANY_
#include <boost/any.hpp>
#endif

#include <libtaomee++/utils/config.hpp>
#include <libtaomee++/utils/noncopyable.hpp>

#include <libtaomee++/thread/thread.hpp>

namespace taomee {

/**
 * @class ThreadPool
 * @brief 线程池，派生于Noncopyable。创建对象后，可以直接判断对象是否合法。例如：\n
 *        ThreadPool<Job> thrpool;\n
 *        if (!thrpool) {\n
 *            // 对象创建不成功
 *            // do something \n
 *        }\n
 *        thrpool.schedule_job(buf);\n
 *        为了使用ThreadPool，必须实现一个类作为ThreadPool的模板参数，并且这个类里面要实现
 *        void run(const DataType data)。如果定义了宏USE_BOOST_ANY_，则DataType为boost: :any；
 *        如果没有定义USE_BOOST_ANY_，则DataType为void*。默认USE_BOOST_ANY_没定义。run()里面
 *        对data进行具体的处理。
 */
template<typename JobPolicy>
class ThreadPool : Noncopyable {
private:
	/**
	 * @brief data type
	 */
#ifdef USE_BOOST_ANY_
	typedef boost::any DataType;
#else
	typedef void* DataType;
#endif

public:
	/**
	 * @brief 构造线程池对象
	 * @param worker_num 线程个数，默认是4
	 */
	ThreadPool(size_t worker_num = 4);
	~ThreadPool();
	/**
	 * @brief get an idle worker to process the incoming data immediately
	 * @data data to be processed. 如果定义了宏USE_BOOST_ANY_，则DataType为boost: :any；
	 *       如果没有定义USE_BOOST_ANY_，则DataType为void*。默认USE_BOOST_ANY_没定义。
	 * @return true if data can be processed immediately, otherwise false
	 */
	bool run_job(const DataType& data = DataType());
	/**
	 * @brief the incoming data will be processed when idle worker available
	 * @data data to be processed. 如果定义了宏USE_BOOST_ANY_，则DataType为boost: :any；
	 *       如果没有定义USE_BOOST_ANY_，则DataType为void*。默认USE_BOOST_ANY_没定义。
	 */
	void schedule_job(const DataType& data = DataType());
	// conversion operators
	operator void*() const // for judging whether the Mutex obj is fine to use
		{ return reinterpret_cast<void*>(!err_); }
private:
	// Forward Declaration
	class Worker;
	// typedefs
	typedef std::queue<Worker*>   WorkerQueue;
	typedef std::queue<DataType>  DataQueue;

	// Worker Queue
	WorkerQueue  worker_q_;
	// Data Queue
	Mutex        data_q_mutex_; // to make Workers idle if no data available
	ThreadCond   data_q_cond_;  // to wake up an idle Worker and process the incoming data
	DataQueue    data_q_;
	//
	bool end_;
	//
	size_t nworkers_idle_;
	int err_;
};

// worker class
template<typename JobPolicy>
class ThreadPool<JobPolicy>::Worker : public Thread {
public:
	Worker(ThreadPool<JobPolicy>* thrpool) : thrpool_(thrpool)
		{ }
	~Worker()
		{ if (joinable()) join(); }
protected:
	void run();
private:
	JobPolicy job_;

	ThreadPool<JobPolicy>* const thrpool_;
};

// implementation of thread pool
#include <libtaomee++/thread/threadpool.tcpp>

} // end namespace taomee

#endif // LIBTAOMEEPP_THREADPOOL_HPP_
