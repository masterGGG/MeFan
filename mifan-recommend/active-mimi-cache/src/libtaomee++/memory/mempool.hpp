/**
 *============================================================
 *  @file      mempool.hpp
 *  @brief     内存池基类，继承该类的派生类及其派生类，使用new/delete/new []/delete []动态
 *             分配/释放对象内存时，操作的都是内存池。\n
 *             例如：\n
 *             class Obj : public MemPool; \n
 *             Obj* o = new Obj; \n
 *             delete o; \n
 *             Obj* oo = new Obj[100]; \n
 *             delete [] oo; \n
 *             这里的new/delete都是在操作内存池来分配/释放内存。\n
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

#ifndef LIBTAOMEEPP_MEMPOOL_HPP_
#define LIBTAOMEEPP_MEMPOOL_HPP_

#include <stdexcept>

extern "C" {
#include <glib-2.0/glib.h>
}

namespace taomee {

/**
  * @brief 内存池基类，依赖glib，所以编译时，需要连接glib库以及加上glib.h的路径。
  *        例如：-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include/ -lglib-2.0
  */
class MemPool {
public:
	virtual ~MemPool() { }

public:
	void* operator new(std::size_t n);
	void* operator new(std::size_t n, const std::nothrow_t& nt);
	void  operator delete(void* p, std::size_t n);

	void* operator new[](std::size_t n);	
	void* operator new[](std::size_t n, const std::nothrow_t& nt);
	void  operator delete[](void* p, std::size_t n);
};

inline void*
MemPool::operator new(std::size_t n)
{
	void* p = g_slice_alloc(n);
	if (p) {
		return p;
	}

	throw std::bad_alloc();
}

inline void*
MemPool::operator new(std::size_t n, const std::nothrow_t& nt)
{
	return g_slice_alloc(n);
}

inline void
MemPool::operator delete(void* p, std::size_t n)
{
	g_slice_free1(n, p);
}

inline void*
MemPool::operator new[](std::size_t n)
{
	return operator new(n);
}

inline void*
MemPool::operator new[](std::size_t n, const std::nothrow_t& nt)
{
	return operator new(n, nt);
}

inline void
MemPool::operator delete[](void* p, std::size_t n)
{
	operator delete(p, n);
}

} // end of namespace taomee

#endif // LIBTAOMEEPP_MEMPOOL_HPP_
