/**
 *============================================================
 *  @file      eventable_obj.hpp
 *  @brief     定时事件基类，通过继承EventableObject，就可以利用EventMgr来管理定时事件。\n
 *             请参考：http://10.1.1.5/libtaomee++/samples/event
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_EVENTABLE_OBJECT_HPP_
#define LIBTAOMEEPP_EVENTABLE_OBJECT_HPP_

#include <cassert>
#include <set>

namespace taomee {

// forward declaration
class TimedEvent;

/**
  * @brief 继承了该类的子类，可以通过EventMgr来添加、删除定时事件。\n
  *        注意：当子类对象被释放时，该对象的所有定时事件都会自动被清除。
  */
class EventableObject {
private:
	typedef std::set<TimedEvent*> EventSet;

public:
	EventableObject () {}

	/**
	 * @brief 析构函数，清除对象的所有定时事件
	 */
	virtual ~EventableObject()
	{
		event_remove_event();
	}

private:
	void event_add_event(TimedEvent* ev)
	{
		m_evs.insert(ev);
	}

	void event_remove_event(TimedEvent* ev = 0);

private: // noncopyable
	EventableObject (const EventableObject &);
	EventableObject & operator = (const EventableObject &);

private:
	EventSet	m_evs;

public:
	friend class EventMgr;
};

} // end of namespace taomee

#endif // LIBTAOMEEPP_EVENTABLE_OBJECT_HPP_

