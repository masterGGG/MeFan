/**
 *============================================================
 *  @file      eventpoll.hpp
 *  @brief     基于Linux epoll实现的EventPoll，封装了epoll的全部操作。！！非线程安全！！
 *             libtaomee++/doc/samples目录下有一个使用实例。
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_EVENT_POLL_HPP_
#define LIBTAOMEEPP_EVENT_POLL_HPP_

#include <cassert>
#include <cerrno>
#include <cstring>

#include <queue>
#include <stdexcept>
#include <string>

extern "C" {
#include <sys/epoll.h>
}

#include <libtaomee++/memory/mempool.hpp>
#include <libtaomee++/utils/noncopyable.hpp>

namespace taomee {

/**
  * @brief 基于Linux epoll实现的EventPoll，封装了epoll的全部操作，便于利用epoll编写程序。
  *        凡是Linux epoll支持的fd，都可以添加到EventPoll中。！！非线程安全！！
  *        通过add接口添加fd，然后调用dispatch接口处理触发了事件的fd。
  */
class EventPoll : Noncopyable {
private:	
	class CallbackBase : public MemPool {
	public:
		virtual void execute() = 0;
		virtual ~CallbackBase() { };
	};

	class CallbackV : public CallbackBase {
	public:
		typedef void (*Method)();

	public:
		CallbackV(Method method)
			{ m_func = method; }

		void execute()
			{ m_func(); }
	
	private:
		Method	m_func;
	};

	class Callback0 : public CallbackBase {
	public:
		typedef void (*Method)(int);

	public:
		Callback0(Method method, int fd)
			{ m_func = method; m_fd = fd; }

		void execute()
			{ m_func(m_fd); }
	
	private:
		Method	m_func;
		int		m_fd;
	};

	template < typename P1 >
	class Callback1 : public CallbackBase {
	public:
		typedef void (*Method)(int, P1&);

	public:
		Callback1(Method method, int fd, const P1& p1)
			{ m_func = method; m_fd = fd; m_p1 = p1; }

		void execute()
			{ m_func(m_fd, m_p1); }
	
	private:	
		Method	m_func;
		int		m_fd;
		P1		m_p1;
	};

	template < typename P1, typename P2 >
	class Callback2 : public CallbackBase {
	public:
		typedef void (*Method)(int, P1&, P2&);

	public:
		Callback2(Method method, int fd, const P1& p1, const P2& p2)
			{ m_func = method; m_fd = fd; m_p1 = p1; m_p2 = p2; }

		void execute()
			{ m_func(m_fd, m_p1, m_p2); }
	
	private:	
		Method	m_func;
		int		m_fd;
		P1		m_p1;
		P2		m_p2;
	};

	template < typename P1, typename P2, typename P3 >
	class Callback3 : public CallbackBase {
	public: 
		typedef void (*Method)(int, P1&, P2&, P3&);

	public:
		Callback3(Method method, int fd, const P1& p1, const P2& p2, const P3& p3)
			{ m_func = method; m_fd = fd; m_p1 = p1; m_p2 = p2; m_p3 = p3; }

		void execute()
			{ m_func(m_fd, m_p1, m_p2, m_p3); }

	private:	
		Method	m_func;
		int		m_fd;
		P1		m_p1;
		P2		m_p2;
		P3		m_p3;
	};

	template < typename P1, typename P2, typename P3, typename P4 >
	class Callback4 : public CallbackBase {
	public:
		typedef void (*Method)(int, P1&, P2&, P3&, P4&);

	public:
		Callback4(Method method, int fd, const P1& p1, const P2& p2, const P3& p3, const P4& p4)
			{ m_func = method; m_fd = fd; m_p1 = p1; m_p2 = p2; m_p3 = p3; m_p4 = p4; }
	
		void execute()
			{ m_func(m_fd, m_p1, m_p2, m_p3, m_p4); }
	
	private:	
		Method	m_func;
		int		m_fd;
		P1		m_p1;
		P2		m_p2;
		P3		m_p3;
		P4		m_p4;
	};

	template < typename Class >
	class CallbackPV : public CallbackBase {
	public:
		typedef void (Class::*Method)();

	public:
		CallbackPV(Class* class_instance, Method method)
			{ m_obj  = class_instance; m_func = method; }

		void execute()
			{ (m_obj->*m_func)(); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
	};

	template < typename Class >
	class CallbackP0 : public CallbackBase {
	public:
		typedef void (Class::*Method)(int);

	public:
		CallbackP0(Class* class_instance, Method method, int fd)
			{ m_obj  = class_instance; m_func = method; m_fd = fd; }

		void execute()
			{ (m_obj->*m_func)(m_fd); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
		int		m_fd;
	};

	template < typename Class, typename P1 >
	class CallbackP1 : public CallbackBase {
	public:
		typedef void (Class::*Method)(int, P1&);

	public:
		CallbackP1(Class* class_instance, Method method, int fd, const P1& p1)
			{ m_obj = class_instance; m_func = method; m_fd = fd; m_p1 = p1; }

		void execute()
			{ (m_obj->*m_func)(m_fd, m_p1); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
		int		m_fd;
		P1		m_p1;
	};

	template < typename Class, typename P1, typename P2 >
	class CallbackP2 : public CallbackBase {
	public:
		typedef void (Class::*Method)(int, P1&, P2&);

	public:
		CallbackP2(Class* class_instance, Method method, int fd, const P1& p1, const P2& p2)
			{ m_obj = class_instance; m_func = method; m_fd = fd; m_p1 = p1; m_p2 = p2; }

		void execute()
			{ (m_obj->*m_func)(m_fd, m_p1, m_p2); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
		int		m_fd;
		P1		m_p1;
		P2		m_p2;
	};

	template < class Class, typename P1, typename P2, typename P3 >
	class CallbackP3 : public CallbackBase {
	public: 
		typedef void (Class::*Method)(int, P1&, P2&, P3&);

	public:
		CallbackP3(Class* class_instance, Method method, int fd, const P1& p1, const P2& p2, const P3& p3)
			{ m_obj = class_instance; m_func = method; m_fd = fd; m_p1 = p1; m_p2 = p2; m_p3 = p3; }

		void execute()
			{ (m_obj->*m_func)(m_fd, m_p1, m_p2, m_p3); }

	private:	
		Class*	m_obj;
		Method	m_func;
		int		m_fd;
		P1		m_p1;
		P2		m_p2;
		P3		m_p3;
	};

	template < class Class, typename P1, typename P2, typename P3, typename P4 >
	class CallbackP4 : public CallbackBase {
	public:
		typedef void (Class::*Method)(int, P1&, P2&, P3&, P4&);

	public:
		CallbackP4(Class* class_instance, Method method, int fd, const P1& p1, const P2& p2, const P3& p3, const P4& p4)
			{ m_obj = class_instance; m_func = method; m_fd = fd; m_p1 = p1; m_p2 = p2; m_p3 = p3; m_p4 = p4; }
	
		void execute()
			{ (m_obj->*m_func)(m_fd, m_p1, m_p2, m_p3, m_p4); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
		int		m_fd;
		P1		m_p1;
		P2		m_p2;
		P3		m_p3;
		P4		m_p4;
	};

	struct EventHandler {
	public:
		EventHandler()
			{ in_use = false; }

	public:
		bool in_use;
		bool use_et;
		CallbackBase* in;
		CallbackBase* out;
	};

public:
	/**
	  * @brief EventPoll支持的事件类型
	  */
	enum EventType {
		/*! 同EPOLLIN，当fd可读时，触发该事件。 */
		event_in	= EPOLLIN,
		/*!
		 * 同EPOLLOUT，当fd可写时，触发该事件。注意：一般情况下，不要同时添加event_in和event_out事件，
		 * 否则每次发生event_in事件时，都会同时报告event_out事件。建议只有当fd的内核缓存出现写满的情况后，
		 * 才同时添加event_in和event_out事件（或者event_io）。
		 */
		event_out	= EPOLLOUT,
		/*! 当fd可写或者可写时，触发该事件 */
		event_io	= event_in | event_out,

		// event_err, event_hup, event_rdhup和event_et保留给EventPoll类内使用，用户代码不得使用！
		event_err	= EPOLLERR,
		event_hup	= EPOLLHUP,
		event_rdhup	= EPOLLRDHUP,
		event_et	= EPOLLET
	};

public:
	/**
	 * @brief 默认构造函数
	 * @param maxfd   支持添加到EventPoll中的最大fd（默认是20000）。
	 * @param timeout 供dispatch中调用的epoll_wait使用。假设timeout==100，则表示epoll_wait在100毫秒后，就算得不到
	 *                任何触发了事件的fd也会返回。如果timeout==-1（默认），则epoll_wait得不到fd就不会返回。如果
	 *                timeout==0，则epoll_wait就算得不到任何fd，也会立即返回。
	 *                通常，设置timeout是为了让epoll_wait就算得不到任何fd也能定期返回，从而让add_plugin添加的用户
	 *                代码可以定期得到执行（比如定时器列表，需要定期扫描）。
	 * @throw runtime_error, bad_alloc
	 * @see dispatch, add_plugin
	 */
	EventPoll(int maxfd = 20000, int timeout = -1);

	/**
	 * @brief 析构函数
	 */
	~EventPoll();

	/**
	 * @brief 添加一段用户代码，用于在dispatch中的epoll_wait返回后执行。
	 * @param plugin 用户代码函数指针
	 * @see dispatch
	 */
	void add_plugin(void (*plugin)())
	{
		assert(m_plugin == 0);

		m_plugin = new CallbackV(plugin);
	}

	/**
	 * @brief 添加一段用户代码，用于在dispatch中的epoll_wait返回后执行。
	 * @param plugin 用户代码函数指针
	 * @see dispatch
	 */
	template <typename Class>
	void add_plugin(Class& obj, void (Class::*plugin)())
	{
		assert(m_plugin == 0);

		m_plugin = new CallbackPV<Class>(&obj, plugin);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	void add(int fd, EventType ev_type, void (*incb)(int fd), void (*outcb)(int fd), bool use_et = true)
	{
		CallbackBase* in  = new Callback0(incb, fd);
		CallbackBase* out = new Callback0(outcb, fd);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param in1  回调incb时，传递给incb的参数。in1会按值拷贝到EventPoll中，在incb中可以对in1进行修改。
	 * @param out1 回调outcb时，传递给outcb的参数。out1会按值拷贝到EventPoll中，在outcb中可以对out1进行修改。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename In1, typename Out1>
	void add(int fd, EventType ev_type,
				void (*incb)(int fd, In1&), void (*outcb)(int fd, Out1&),
				const In1& in1 = In1(), const Out1& out1 = Out1(),
				bool use_et = true)
	{
		CallbackBase* in  = new Callback1<In1>(incb, fd, in1);
		CallbackBase* out = new Callback1<Out1>(outcb, fd, out1);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param in1  回调incb时，传递给incb的参数。in1会按值拷贝到EventPoll中，在incb中可以对in1进行修改。
	 * @param in2  回调incb时，传递给incb的参数。in2会按值拷贝到EventPoll中，在incb中可以对in2进行修改。
	 * @param out1 回调outcb时，传递给outcb的参数。out1会按值拷贝到EventPoll中，在outcb中可以对out1进行修改。
	 * @param out2 回调outcb时，传递给outcb的参数。out2会按值拷贝到EventPoll中，在outcb中可以对out2进行修改。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename In1, typename In2, typename Out1, typename Out2>
	void add(int fd, EventType ev_type,
				void (*incb)(int fd, In1&, In2&), void (*outcb)(int fd, Out1&, Out2&),
				const In1& in1 = In1(), const In2& in2 = In2(),
				const Out1& out1 = Out1(), const Out2& out2 = Out2(),
				bool use_et = true)
	{
		CallbackBase* in  = new Callback2<In1, In2>(incb, fd, in1, in2);
		CallbackBase* out = new Callback2<Out1, Out2>(outcb, fd, out1, out2);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param in1  回调incb时，传递给incb的参数。in1会按值拷贝到EventPoll中，在incb中可以对in1进行修改。
	 * @param in2  回调incb时，传递给incb的参数。in2会按值拷贝到EventPoll中，在incb中可以对in2进行修改。
	 * @param in3  回调incb时，传递给incb的参数。in3会按值拷贝到EventPoll中，在incb中可以对in3进行修改。
	 * @param out1 回调outcb时，传递给outcb的参数。out1会按值拷贝到EventPoll中，在outcb中可以对out1进行修改。
	 * @param out2 回调outcb时，传递给outcb的参数。out2会按值拷贝到EventPoll中，在outcb中可以对out2进行修改。
	 * @param out3 回调outcb时，传递给outcb的参数。out3会按值拷贝到EventPoll中，在outcb中可以对out3进行修改。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename In1, typename In2, typename In3, typename Out1, typename Out2, typename Out3>
	void add(int fd, EventType ev_type,
				void (*incb)(int fd, In1&, In2&, In3&), void (*outcb)(int fd, Out1&, Out2&, Out3&),
				const In1& in1 = In1(), const In2& in2 = In2(), const In3& in3 = In3(),
				const Out1& out1 = Out1(), const Out2& out2 = Out2(), const Out3& out3 = Out3(),
				bool use_et = true)
	{
		CallbackBase* in  = new Callback3<In1, In2, In3>(incb, fd, in1, in2, in3);
		CallbackBase* out = new Callback3<Out1, Out2, Out3>(outcb, fd, out1, out2, out3);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param in1  回调incb时，传递给incb的参数。in1会按值拷贝到EventPoll中，在incb中可以对in1进行修改。
	 * @param in2  回调incb时，传递给incb的参数。in2会按值拷贝到EventPoll中，在incb中可以对in2进行修改。
	 * @param in3  回调incb时，传递给incb的参数。in3会按值拷贝到EventPoll中，在incb中可以对in3进行修改。
	 * @param in4  回调incb时，传递给incb的参数。in4会按值拷贝到EventPoll中，在incb中可以对in4进行修改。
	 * @param out1 回调outcb时，传递给outcb的参数。out1会按值拷贝到EventPoll中，在outcb中可以对out1进行修改。
	 * @param out2 回调outcb时，传递给outcb的参数。out2会按值拷贝到EventPoll中，在outcb中可以对out2进行修改。
	 * @param out3 回调outcb时，传递给outcb的参数。out3会按值拷贝到EventPoll中，在outcb中可以对out3进行修改。
	 * @param out4 回调outcb时，传递给outcb的参数。out4会按值拷贝到EventPoll中，在outcb中可以对out4进行修改。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename In1, typename In2, typename In3, typename In4,
				typename Out1, typename Out2, typename Out3, typename Out4>
	void add(int fd, EventType ev_type,
				void (*incb)(int fd, In1&, In2&, In3&, In4&), void (*outcb)(int fd, Out1&, Out2&, Out3&, Out4&),
				const In1& in1 = In1(), const In2& in2 = In2(), const In3& in3 = In3(), const In4& in4 = In4(),
				const Out1& out1 = Out1(), const Out2& out2 = Out2(), const Out3& out3 = Out3(), const Out4& out4 = Out4(),
				bool use_et = true)
	{
		CallbackBase* in  = new Callback4<In1, In2, In3, In4>(incb, fd, in1, in2, in3, in4);
		CallbackBase* out = new Callback4<Out1, Out2, Out3, Out4>(outcb, fd, out1, out2, out3, out4);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param obj  回调函数（incb和outcb）所属类的对象。注意：EventPoll中保存的是obj的地址，如果obj被释放掉，
	 *             必须及时调用remove把该事件也清除掉！
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename Class>
	void add(int fd, EventType ev_type, Class& obj,
				void (Class::*incb)(), void (Class::*outcb)(),
				bool use_et = true)
	{
		CallbackBase* in  = new CallbackPV<Class>(&obj, incb);
		CallbackBase* out = new CallbackPV<Class>(&obj, outcb);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param obj  回调函数（incb和outcb）所属类的对象。注意：EventPoll中保存的是obj的地址，如果obj被释放掉，
	 *             必须及时调用remove把该事件也清除掉！
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename Class>
	void add(int fd, EventType ev_type, Class& obj,
				void (Class::*incb)(int fd), void (Class::*outcb)(int fd),
				bool use_et = true)
	{
		CallbackBase* in  = new CallbackP0<Class>(&obj, incb, fd);
		CallbackBase* out = new CallbackP0<Class>(&obj, outcb, fd);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param obj  回调函数（incb和outcb）所属类的对象。注意：EventPoll中保存的是obj的地址，如果obj被释放掉，
	 *             必须及时调用remove把该事件也清除掉！
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param in1  回调incb时，传递给incb的参数。in1会按值拷贝到EventPoll中，在incb中可以对in1进行修改。
	 * @param out1 回调outcb时，传递给outcb的参数。out1会按值拷贝到EventPoll中，在outcb中可以对out1进行修改。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename Class, typename In1, typename Out1>
	void add(int fd, EventType ev_type, Class& obj,
				void (Class::*incb)(int fd, In1&), void (Class::*outcb)(int fd, Out1&),
				const In1& in1 = In1(),
				const Out1& out1 = Out1(),
				bool use_et = true)
	{
		CallbackBase* in  = new CallbackP1<Class, In1>(&obj, incb, fd, in1);
		CallbackBase* out = new CallbackP1<Class, Out1>(&obj, outcb, fd, out1);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param obj  回调函数（incb和outcb）所属类的对象。注意：EventPoll中保存的是obj的地址，如果obj被释放掉，
	 *             必须及时调用remove把该事件也清除掉！
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param in1  回调incb时，传递给incb的参数。in1会按值拷贝到EventPoll中，在incb中可以对in1进行修改。
	 * @param in2  回调incb时，传递给incb的参数。in2会按值拷贝到EventPoll中，在incb中可以对in2进行修改。
	 * @param out1 回调outcb时，传递给outcb的参数。out1会按值拷贝到EventPoll中，在outcb中可以对out1进行修改。
	 * @param out2 回调outcb时，传递给outcb的参数。out2会按值拷贝到EventPoll中，在outcb中可以对out2进行修改。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename Class, typename In1, typename In2, typename Out1, typename Out2>
	void add(int fd, EventType ev_type, Class& obj,
				void (Class::*incb)(int fd, In1&, In2&), void (Class::*outcb)(int fd, Out1&, Out2&),
				const In1& in1 = In1(), const In2& in2 = In2(),
				const Out1& out1 = Out1(), const Out2& out2 = Out2(),
				bool use_et = true)
	{
		CallbackBase* in  = new CallbackP2<Class, In1, In2>(&obj, fd, incb, in1, in2);
		CallbackBase* out = new CallbackP2<Class, Out1, Out2>(&obj, fd, outcb, out1, out2);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param obj  回调函数（incb和outcb）所属类的对象。注意：EventPoll中保存的是obj的地址，如果obj被释放掉，
	 *             必须及时调用remove把该事件也清除掉！
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param in1  回调incb时，传递给incb的参数。in1会按值拷贝到EventPoll中，在incb中可以对in1进行修改。
	 * @param in2  回调incb时，传递给incb的参数。in2会按值拷贝到EventPoll中，在incb中可以对in2进行修改。
	 * @param in3  回调incb时，传递给incb的参数。in3会按值拷贝到EventPoll中，在incb中可以对in3进行修改。
	 * @param out1 回调outcb时，传递给outcb的参数。out1会按值拷贝到EventPoll中，在outcb中可以对out1进行修改。
	 * @param out2 回调outcb时，传递给outcb的参数。out2会按值拷贝到EventPoll中，在outcb中可以对out2进行修改。
	 * @param out3 回调outcb时，传递给outcb的参数。out3会按值拷贝到EventPoll中，在outcb中可以对out3进行修改。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename Class, typename In1, typename In2, typename In3,
				typename Out1, typename Out2, typename Out3>
	void add(int fd, EventType ev_type, Class& obj,
				void (Class::*incb)(int fd, In1&, In2&, In3&),
				void (Class::*outcb)(int fd, Out1&, Out2&, Out3&),
				const In1& in1 = In1(), const In2& in2 = In2(), const In3& in3 = In3(),
				const Out1& out1 = Out1(), const Out2& out2 = Out2(), const Out3& out3 = Out3(),
				bool use_et = true)
	{
		CallbackBase* in  = new CallbackP3<Class, In1, In2, In3>(&obj, incb, fd, in1, in2, in3);
		CallbackBase* out = new CallbackP3<Class, Out1, Out2, Out3>(&obj, outcb, fd, out1, out2, out3);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 添加一个fd到EventPoll中。凡是Linux epoll支持的fd，都可以添加。
	 * @param fd 凡是Linux epoll支持的fd，都可以添加到EventPoll中
	 * @param ev_type 当fd发生ev_type事件时，调用给定的回调函数
	 * @param obj  回调函数（incb和outcb）所属类的对象。注意：EventPoll中保存的是obj的地址，如果obj被释放掉，
	 *             必须及时调用remove把该事件也清除掉！
	 * @param incb 当fd可读（event_in事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param outcb 当fd可写（event_out事件）时，调用该回调函数。回调函数的第一个参数是触发事件的fd。
	 * @param in1  回调incb时，传递给incb的参数。in1会按值拷贝到EventPoll中，在incb中可以对in1进行修改。
	 * @param in2  回调incb时，传递给incb的参数。in2会按值拷贝到EventPoll中，在incb中可以对in2进行修改。
	 * @param in3  回调incb时，传递给incb的参数。in3会按值拷贝到EventPoll中，在incb中可以对in3进行修改。
	 * @param in4  回调incb时，传递给incb的参数。in4会按值拷贝到EventPoll中，在incb中可以对in4进行修改。
	 * @param out1 回调outcb时，传递给outcb的参数。out1会按值拷贝到EventPoll中，在outcb中可以对out1进行修改。
	 * @param out2 回调outcb时，传递给outcb的参数。out2会按值拷贝到EventPoll中，在outcb中可以对out2进行修改。
	 * @param out3 回调outcb时，传递给outcb的参数。out3会按值拷贝到EventPoll中，在outcb中可以对out3进行修改。
	 * @param out4 回调outcb时，传递给outcb的参数。out4会按值拷贝到EventPoll中，在outcb中可以对out4进行修改。
	 * @param use_et true则使用ET模式，false则使用LT模式。默认是true。如果使用ET模式，当fd中的可读数据
	 *               未能通过一次incb调用来读完时，可以调用report_again_in函数来请求EventPoll随后再次调用incb。
	 * @throw runtime_error, bad_alloc
	 * @see report_again_in, dispatch, modify, remove
	 */
	template <typename Class, typename In1, typename In2, typename In3, typename In4,
				typename Out1, typename Out2, typename Out3, typename Out4>
	void add(int fd, EventType ev_type, Class& obj,
				void (Class::*incb)(int fd, In1&, In2&, In3&, In4&),
				void (Class::*outcb)(int fd, Out1&, Out2&, Out3&, Out4&),
				const In1& in1 = In1(), const In2& in2 = In2(), const In3& in3 = In3(), const In4& in4 = In4(),
				const Out1& out1 = Out1(), const Out2& out2 = Out2(), const Out3& out3 = Out3(), const Out4& out4 = Out4(),
				bool use_et = true)
	{
		CallbackBase* in  = new CallbackP4<Class, In1, In2, In3, In4>(&obj, incb, fd, in1, in2, in3, in4);
		CallbackBase* out = new CallbackP4<Class, Out1, Out2, Out3, Out4>(&obj, outcb, fd, out1, out2, out3, out4);

		add_event(fd, ev_type, in, out, use_et);
	}

	/**
	 * @brief 通过这个函数可以修改fd触发事件时是否调用回调函数。假设调用add的时候，ev_type参数传递的是event_in，
	 *        我们可以通过调用modify函数，并且传递event_out给ev_type，通知EventPoll当fd的event_out事件发生时，
	 *        调用outcb回调函数，而当event_in事件发生时，不做任何响应。我们也可以传递event_io给modify函数，通知
	 *        EventPoll当fd的event_in或者event_out函数发生时，调用incb或outcb。
	 * @param fd 修改这个fd关注的事件
	 * @param ev_type fd被修改成只关注ev_type事件，即当ev_type事件发生时，调用相应的回调函数。
	 * @throw runtime_error
	 * @see add
	 */
	void modify(int fd, EventType ev_type)
	{
		assert((fd < m_maxfd) && (fd > -1) && m_ev_hdlrs[fd].in_use);

		if (epoll_control(EPOLL_CTL_MOD, fd, ev_type, m_ev_hdlrs[fd].use_et) == -1) {
			throw std::runtime_error(std::string("epoll_ctl (EPOLL_CTL_MOD) failed: ") + strerror(errno));
		}
	}

	/**
	 * @brief 从EventPoll中移除对fd的关注。移除后，fd发生任何事件都不会触发调用回调函数。
	 * @param fd 从EventPoll中移除的fd
	 * @see add
	 */
	void remove(int fd)
	{
		assert((fd < m_maxfd) && (fd > -1) && m_ev_hdlrs[fd].in_use);

		EventHandler* evhdlr = &m_ev_hdlrs[fd];
		evhdlr->in_use = false;
		delete evhdlr->in;
		delete evhdlr->out;
		--m_ev_num;

		epoll_control(EPOLL_CTL_DEL, fd, event_in, false);
	}

	/**
	 * @brief 如果使用ET模式，当fd中的可读数据未能通过一次incb调用来读完时，可以调用report_again_in函数
	 *        来请求EventPoll随后再次调用incb。
	 * @param fd 请求EventPoll为该fd再次调用incb
	 * @see add
	 */
	void report_again_in(int fd)
	{
		assert((fd < m_maxfd) && (fd > -1) && m_ev_hdlrs[fd].in_use);

		m_read_evs.push(&m_ev_hdlrs[fd]);
	}

	/**
	 * @brief 调用epoll_wait，等待所有通过add接口添加的fd的事件。当epoll_wait返回时（有可用事件或者超时），
	 *        会先为所有触发了事件的fd调用对应的回调函数，然后为所有“report_again_in”的fd再次调用incb，
	 *        最后调用add_plugin注册的用户代码。
	 * @see add, add_plugin, report_again_in
	 * @throw runtime_error
	 */
	void dispatch();

private:
	void add_event(int fd, EventType ev_type, CallbackBase* in, CallbackBase* out, bool use_et);
	int  epoll_control(int op, int fd, EventType ev_type, bool use_et);
	// dispatch all the ready events reported by epoll
	void dispatch_ready_events(int ev_num);
	// redispatch all the ready but not yet finish processing 'event_in' events
	void redispatch_in_events();

private:
	/*! epoll fd */
	int m_epfd;
	/*! timeout (in millisecond) for epoll_wait */
	int m_timeout;
	/*! number of events added to epoll */
	int m_ev_num;
	/*! maximum fd */
	int m_maxfd;
	/*! hold callbacks of each fd (up to m_maxfd elements) */
	EventHandler* m_ev_hdlrs;
	/*! contains the available events */
	epoll_event*  m_avail_evs;
	/*! contains events to report again for reading */
	std::queue<EventHandler*> m_read_evs;
	/*! extra user code */
	CallbackBase* m_plugin;
};


}

#endif // LIBTAOMEEPP_EVENT_POLL_HPP_
