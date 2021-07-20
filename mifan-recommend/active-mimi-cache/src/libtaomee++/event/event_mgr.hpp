/**
 *============================================================
 *  @file      event_mgr.hpp
 *  @brief     事件管理类。使用它可以管理继承自EventableObject的子类的定时事件。\n
 *             请参考：http://10.1.1.5/libtaomee++/samples/event
 *             注意：依赖glib，所以编译时，需要连接glib库以及加上glib.h的路径。\n
 *             例如：-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include/ -lglib-2.0
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_EVENT_MANAGER_HPP_
#define LIBTAOMEEPP_EVENT_MANAGER_HPP_

#include <map>

#include <libtaomee++/memory/mempool.hpp>
#include <libtaomee++/time/timeval.hpp>

#include <libtaomee++/event/eventable_obj.hpp>

namespace taomee {

/**
  * @brief 定时事件。EventMgr: :add_event会返回TimedEvent*指针，可以通过这个指针删除/修改某个事件。
  */
class TimedEvent : public MemPool {
private:	
	class CallbackBase : public MemPool {
	public:
		virtual int execute() = 0;
		virtual ~CallbackBase() { };
	};

	template < typename Class >
	class CallbackP0 : public CallbackBase {
	public:
		typedef int (Class::*Method)();

	public:
		CallbackP0(Class* class_instance, Method method)
			{ m_obj  = class_instance; m_func = method; }

		int execute()
			{ return (m_obj->*m_func)(); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
	};

	template < typename Class, typename P1 >
	class CallbackP1 : public CallbackBase {
	public:
		typedef int (Class::*Method)(P1&);

	public:
		CallbackP1(Class* class_instance, Method method, const P1& p1)
			{ m_obj = class_instance; m_func = method; m_p1 = p1; }

		int execute()
			{ return (m_obj->*m_func)(m_p1); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
		P1		m_p1;
	};

	template < typename Class, typename P1, typename P2 >
	class CallbackP2 : public CallbackBase {
	public:
		typedef int (Class::*Method)(P1&, P2&);

	public:
		CallbackP2(Class* class_instance, Method method, const P1& p1, const P2& p2)
			{ m_obj = class_instance; m_func = method; m_p1 = p1; m_p2 = p2; }

		int execute()
			{ return (m_obj->*m_func)(m_p1, m_p2); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
		P1		m_p1;
		P2		m_p2;
	};

	template < class Class, typename P1, typename P2, typename P3 >
	class CallbackP3 : public CallbackBase {
	public:	
		typedef int (Class::*Method)(P1&, P2&, P3&);

	public:
		CallbackP3(Class* class_instance, Method method, const P1& p1, const P2& p2, const P3& p3)
			{ m_obj = class_instance; m_func = method; m_p1 = p1; m_p2 = p2; m_p3 = p3; }

		int execute()
			{ return (m_obj->*m_func)(m_p1, m_p2, m_p3); }

	private:	
		Class*	m_obj;
		Method	m_func;
		P1		m_p1;
		P2		m_p2;
		P3		m_p3;
	};

	template < class Class, typename P1, typename P2, typename P3, typename P4 >
	class CallbackP4 : public CallbackBase {
	public:
		typedef int (Class::*Method)(P1&, P2&, P3&, P4&);

	public:
		CallbackP4(Class* class_instance, Method method, const P1& p1, const P2& p2, const P3& p3, const P4& p4)
			{ m_obj = class_instance; m_func = method; m_p1 = p1; m_p2 = p2; m_p3 = p3; m_p4 = p4; }
	
		int execute()
			{ return (m_obj->*m_func)(m_p1, m_p2, m_p3, m_p4); }
	
	private:	
		Class*	m_obj;
		Method	m_func;
		P1		m_p1;
		P2		m_p2;
		P3		m_p3;
		P4		m_p4;
	};

private:
	TimedEvent(EventableObject* ev_owner, CallbackBase* cb, uint32_t interval, int repeat)
		{ m_ev_owner = ev_owner; m_cb = cb; m_interval = interval; m_repeat_times = repeat; }

	~TimedEvent()
		{ delete m_cb; }

	void deactivate()
		{ m_ev_owner = 0; }

	bool is_active() const
		{ return m_ev_owner; }

private:
	EventableObject*	m_ev_owner;
	CallbackBase*		m_cb;
	/*! in milliseconds */
	uint32_t			m_interval;
	int					m_repeat_times;
	TimeVal				m_expired_tv;
	// use intrusive list later if performance is bad

public:
	friend class EventMgr;
	friend class EventableObject;
};

/**
  * @brief 事件管理器。使用它可以管理继承自EventableObject的子类的定时事件。
  */
class EventMgr {
private:
	typedef std::multimap<TimeVal, TimedEvent*> EventMap;

public:
	/**
	 * @brief 添加一件定时事件
	 * @param obj 事件拥有者
	 * @param method 事件发生时调用的回调函数。该回调函数返回0则一切正常，如果返回非0，则事件管理器会立刻把该定时事件删除。
	 * @param first_expired_tv 事件第一次发生的时间
	 * @param interval 事件第一次发生后，再次发生的间隔时间（单位是毫秒），配合repeats使用。如果不需要重复，则不用传递任何参数。
	 * @param repeats 事件重复次数，配合interval使用。如果希望事件第一次发生后，还要再重复发生一次，则传递参数1；
	 *                如果希望定时事件一直不断重复发生，则传递-1；如果不需要重复，则不用传递任何参数。
	 * @return 刚刚添加的定时事件指针。
	 */
	template <typename Class>
	TimedEvent* add_event(Class& obj, int (Class::*method)(), const timeval& first_expired_tv, uint32_t interval = 0, int repeats = 0)
	{
		TimedEvent::CallbackBase* cb = new TimedEvent::CallbackP0<Class>(&obj, method);

		return do_add_event(&obj, cb, first_expired_tv, interval, repeats);
	}

	/**
	 * @brief 添加一件定时事件
	 * @param obj 事件拥有者
	 * @param method 事件发生时调用的回调函数。该回调函数返回0则一切正常，如果返回非0，则事件管理器会立刻把该定时事件删除。
	 * @param p1 传递给回调函数method的参数
	 * @param first_expired_tv 事件第一次发生的时间
	 * @param interval 事件第一次发生后，再次发生的间隔时间（单位是毫秒），配合repeats使用。如果不需要重复，则不用传递任何参数。
	 * @param repeats 事件重复次数，配合interval使用。如果希望事件第一次发生后，还要再重复发生一次，则传递参数1；
	 *                如果希望定时事件一直不断重复发生，则传递-1；如果不需要重复，则不用传递任何参数。
	 * @return 刚刚添加的定时事件指针。
	 */
	template <typename Class, typename P1>
	TimedEvent* add_event(Class& obj, int (Class::*method)(P1&), const P1& p1, const timeval& first_expired_tv, uint32_t interval = 0, int repeats = 0)
	{
		TimedEvent::CallbackBase* cb = new TimedEvent::CallbackP1<Class, P1>(&obj, method, p1);

		return do_add_event(&obj, cb, first_expired_tv, interval, repeats);
	}

	/**
	 * @brief 添加一件定时事件
	 * @param obj 事件拥有者
	 * @param method 事件发生时调用的回调函数。该回调函数返回0则一切正常，如果返回非0，则事件管理器会立刻把该定时事件删除。
	 * @param p1 传递给回调函数method的参数
	 * @param p2 传递给回调函数method的参数
	 * @param first_expired_tv 事件第一次发生的时间
	 * @param interval 事件第一次发生后，再次发生的间隔时间（单位是毫秒），配合repeats使用。如果不需要重复，则不用传递任何参数。
	 * @param repeats 事件重复次数，配合interval使用。如果希望事件第一次发生后，还要再重复发生一次，则传递参数1；
	 *                如果希望定时事件一直不断重复发生，则传递-1；如果不需要重复，则不用传递任何参数。
	 * @return 刚刚添加的定时事件指针。
	 */
	template <typename Class, typename P1, typename P2>
	TimedEvent* add_event(Class& obj, int (Class::*method)(P1&, P2&), const P1& p1, const P2& p2, const timeval& first_expired_tv, uint32_t interval = 0, int repeats = 0)
	{
		TimedEvent::CallbackBase* cb = new TimedEvent::CallbackP2<Class, P1, P2>(&obj, method, p1, p2);

		return do_add_event(&obj, cb, first_expired_tv, interval, repeats);
	}

	/**
	 * @brief 添加一件定时事件
	 * @param obj 事件拥有者
	 * @param method 事件发生时调用的回调函数。该回调函数返回0则一切正常，如果返回非0，则事件管理器会立刻把该定时事件删除。
	 * @param p1 传递给回调函数method的参数
	 * @param p2 传递给回调函数method的参数
	 * @param p3 传递给回调函数method的参数
	 * @param first_expired_tv 事件第一次发生的时间
	 * @param interval 事件第一次发生后，再次发生的间隔时间（单位是毫秒），配合repeats使用。如果不需要重复，则不用传递任何参数。
	 * @param repeats 事件重复次数，配合interval使用。如果希望事件第一次发生后，还要再重复发生一次，则传递参数1；
	 *                如果希望定时事件一直不断重复发生，则传递-1；如果不需要重复，则不用传递任何参数。
	 * @return 刚刚添加的定时事件指针。
	 */
	template <typename Class, typename P1, typename P2, typename P3>
	TimedEvent* add_event(Class& obj, int (Class::*method)(P1&, P2&, P3&), const P1& p1, const P2& p2, const P3& p3, const timeval& first_expired_tv, uint32_t interval = 0, int repeats = 0)
	{
		TimedEvent::CallbackBase* cb = new TimedEvent::CallbackP3<Class, P1, P2, P3>(&obj, method, p1, p2, p3);

		return do_add_event(&obj, cb, first_expired_tv, interval, repeats);
	}

	/**
	 * @brief 添加一件定时事件
	 * @param obj 事件拥有者
	 * @param method 事件发生时调用的回调函数。该回调函数返回0则一切正常，如果返回非0，则事件管理器会立刻把该定时事件删除。
	 * @param p1 传递给回调函数method的参数
	 * @param p2 传递给回调函数method的参数
	 * @param p3 传递给回调函数method的参数
	 * @param p4 传递给回调函数method的参数
	 * @param first_expired_tv 事件第一次发生的时间
	 * @param interval 事件第一次发生后，再次发生的间隔时间（单位是毫秒），配合repeats使用。如果不需要重复，则不用传递任何参数。
	 * @param repeats 事件重复次数，配合interval使用。如果希望事件第一次发生后，还要再重复发生一次，则传递参数1；
	 *                如果希望定时事件一直不断重复发生，则传递-1；如果不需要重复，则不用传递任何参数。
	 * @return 刚刚添加的定时事件指针。
	 */
	template <typename Class, typename P1, typename P2, typename P3, typename P4>
	TimedEvent* add_event(Class& obj, int (Class::*method)(P1&, P2&, P3&, P4&), const P1& p1, const P2& p2, const P3& p3, const P4& p4, const timeval& first_expired_tv, uint32_t interval = 0, int repeats = 0)
	{
		TimedEvent::CallbackBase* cb = new TimedEvent::CallbackP4<Class, P1, P2, P3, P4>(&obj, method, p1, p2, p3, p4);

		return do_add_event(&obj, cb, first_expired_tv, interval, repeats);
	}

	/**
	 * @brief 删除一个定时事件
	 * @param ev 需要删除的定时事件
	 */
	void remove_event(TimedEvent* ev)
	{
		ev->m_ev_owner->event_remove_event(ev);
	}

	/**
	 * @brief 删除obj的所有定时事件
	 * @param obj obj的所有定时事件将被删除
	 */
	void remove_events(EventableObject& obj)
	{
		obj.event_remove_event();
	}

	/**
	 * @brief 修改定时事件ev的发生时间。注意，在ev发生时的回调函数中调用该函数是不起作用的！
	 * @param ev 需要修改发生时间的定时事件
	 * @param tv 发生时间
	 */
	void modify_expired_tv(TimedEvent* ev, const timeval& tv)
	{
		assert(m_allevents.count(ev->m_expired_tv));

		m_allevents.erase(ev->m_expired_tv);

		ev->m_expired_tv = tv;
		// add this event to the global event list, updating will all be done later on...
		m_allevents.insert(std::make_pair(ev->m_expired_tv, ev));
	}

	/**
	 * @brief 修改定时事件发生的间隔时间，单位是毫秒
	 * @param ev 定时事件
	 * @param interval 间隔时间，单位是毫秒
	 */
	void modify_interval(TimedEvent* ev, uint32_t interval)
	{
		ev->m_interval = interval;
	}

	/**
	 * @brief 处理所有定时事件。该函数必须被定期调用，否则所有的定时事件都不会发生。
	 *        定期调用的时间间隔精度决定了定时事件发生的时间精度。
	 */
	void process_events();

private:
	TimedEvent* do_add_event(EventableObject* obj, TimedEvent::CallbackBase* cb, const timeval& first_expired_tv, uint32_t interval, int repeats)
	{
		// create a timed event
		TimedEvent* event = new TimedEvent(obj, cb, interval, repeats);
		event->m_expired_tv = first_expired_tv;
		// add this event to the global event list, updating will all be done later on...
		m_allevents.insert(std::make_pair(event->m_expired_tv, event));
		// add this event to the object's list
		obj->event_add_event(event);

		return event;
	}

private:
	EventMap	m_allevents;
};

} // end of namespace taomee

#endif // LIBTAOMEEPP_EVENT_MANAGER_HPP_

