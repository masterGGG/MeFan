/**
 *============================================================
 *  @file      utilities.hpp
 *  @brief    some useful utilities
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_UTILITIES_HPP_
#define LIBTAOMEEPP_UTILITIES_HPP_

#include <string>

namespace taomee {

/**
 * @brief check if the 'filename' extension is the same as 'ext'
 * @param filename filename to check
 * @param ext filename extension (.exe, .ttf, .ttc ...)
 * @return true if 'ext' is a filename extension of 'filename', false otherwise
 */
inline bool
check_filename_ext(const std::string& filename, const std::string& ext)
{
	std::string::size_type loc = filename.rfind(ext);

	return ((loc != std::string::npos) && ((filename.size() - loc) == ext.size()));
}

/**
 * @brief get current version number of libtaomee++
 * @return current version number of libtaomee++
 */
inline const char*
libtaomeepp_get_version()
{
	return "0.7.2";
}

}

#endif // LIBTAOMEEPP_UTILITIES_HPP_

