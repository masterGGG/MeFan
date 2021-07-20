/**
 *============================================================
 *  @file      def_grid_data.hpp
 *  @brief    Default Grid Data For A* algorithm
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_DEF_GRID_DATA_HPP_
#define LIBTAOMEEPP_DEF_GRID_DATA_HPP_

#include <iostream>

namespace taomee {

/**
 * @brief Default Grid Data For A* algorithm
 */
class DefGridData {
public:
	DefGridData()
		{ m_data = 0; }

	/**
	 * @brief judge if a grid is walkable
	 */
	bool is_walkable() const
		{ return (m_data == 0); }

	/**
	 * @brief set grid walkability
	 * @param walkability 0: walkable, 1: unwalkable
	 */
	void set_walkability(int walkability)
		{ m_data = walkability; }

public:
	friend std::istream& operator >>(std::istream& is, DefGridData& obj)
		{ is >> obj.m_data; return is; }
	friend std::ostream& operator <<(std::ostream& os, const DefGridData& obj)
		{ os << obj.m_data; return os; }

private:
	int	m_data;
};

} // end of namespace taomee

#endif // LIBTAOMEEPP_DEF_GRID_DATA_HPP_
