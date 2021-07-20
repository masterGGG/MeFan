/**
 *============================================================
 *  @file      noncopyable.hpp
 *  @brief     Defines a basic class (Noncopyable) to be derived from.
 *             Objects of classes that are derived from Noncopyable will
 *             not be able to be copied or assigned to another object.
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_NONCOPYABLE_HPP_
#define LIBTAOMEEPP_NONCOPYABLE_HPP_

namespace taomee {
/**
 * @class Noncopyable
 * @brief private copy ctor and assignment operator ensure classes derived from it cannot be copied
 */
class Noncopyable {
protected:
	Noncopyable() { }
private:
	Noncopyable(const Noncopyable&);
	Noncopyable& operator=(const Noncopyable&);
};

} // end of namespace taomee

#endif /* LIBTAOMEEPP_NONCOPYABLE_HPP_ */
