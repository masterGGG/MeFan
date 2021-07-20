/**
 *============================================================
 *  @file      udp_pool.hpp
 *  @brief     基于ThreadPool框架实现了UdpPool，便于用多线程处理udp协议发过来的数据。
 *             具体例子请参考 thread/udppooltest.cpp
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */
#ifndef LIBTAOMEEPP_UDP_POOL_HPP_
#define LIBTAOMEEPP_UDP_POOL_HPP_

extern "C" {
#include <arpa/inet.h>
#include <sys/socket.h>
}

#include <cerrno>
#include <cstring>

#include <libtaomee++/utils/noncopyable.hpp>

#include <libtaomee++/thread/threadpool.hpp>

namespace taomee {

/**
 * @class UdpPool
 * @brief 处理以UDP协议发送过来的数据的多线程框架，派生于Noncopyable。要使用这个类的功能，必须实现另外一个类
 *        作为UdpPool的模板参数，并且这个类里面必须实现void run(const void* data)，这个函数负责具体处理UDP
 *        发送过来的数据。\n
 *        创建对象后，可以直接判断对象是否合法。例如：\n\n
 *        UdpPool<Job> upool;\n
 *        if (!upool) {\n
 *            // 对象创建不成功\n
 *            // do something \n
 *        }\n
 *        upool.run();
 */
template <typename JobPolicy>
class UdpPool : Noncopyable {
public:
	/**
	 * @brief 构造UdpPool对象
	 * @param port 监听的UDP端口，默认监听8421。
	 * @param ip   监听的IP，默认监听INADDR_ANY。
	 * @param pkgsize UDP发过来的数据包的最大大小，默认4096字节。
	 * @param nthrs 线程数目，默认开20个。
	 */
	UdpPool(in_port_t port = 8421, const char* ip = 0, const size_t pkgsize = 4096, size_t nthrs = 20);
	~UdpPool() { }
	/**
	 * @brief 启动UdpPool
	 */
	void run();
	// conversion operators
	operator void*() const // for judging whether the UdpPool obj is fine to use
		{ return reinterpret_cast<void*>(!err_); }
private:
	class Job {
	public:
		void run(const void* data)
		{
			job_.run(data);
			delete [] reinterpret_cast<const char*>(data);
		}
	private:
		JobPolicy job_;
	};

	//
	int err_;
	//
	int sockfd_;
	//
	const size_t pkgsize_;
	//
	ThreadPool<Job> thrpool_;	
};

#include "udp_pool.tcpp"

} // end of namespace taomee

#endif // LIBTAOMEEPP_UDP_POOL_HPP_
