/**
 *============================================================
 *  @file      ini_parser.hpp
 *  @brief     用于解析配置文件。配置文件的格式如下： \n
 *				 [logger]  // 配置区名称(Sector Name) \n
 *				 LOG_DIR		= ../log    # 键值对（Key-Value Pair） \n
 *				 LOG_MAX_SIZE	= 10000000  // 支持 // 和 # 两种注释方式 \n
 *				 [misc]    #  配置区名称 \n
 *				 WORKER_PROC_NUM	= 10
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_INI_PARSER_HPP_
#define LIBTAOMEEPP_INI_PARSER_HPP_

#include <cstdlib>

#include <map>
#include <stdexcept>
#include <string>

/**
 * @namespace taomee
 * @brief 淘米C++公共库使用的名字空间，所有淘米公司开发的C++库都放在该名字空间下。
 */
namespace taomee {

typedef std::map<std::string, std::string> StrMap;
typedef StrMap::iterator StrMapIterator;

const char* const midstring = "_____][_____";

/**
 * @class IniParseError
 * @brief 解析配置文件出错时抛出的异常。继承自std: :logic_error
 */
class IniParseError : public std::logic_error {
public:
	explicit IniParseError(const std::string& errmsg) : std::logic_error(errmsg)
		{ }
};

/**
 * @class IniParser
 * @brief 配置文件解析器。
 */
class IniParser {
public:
	/**
	 * @brief 构造函数。
	 * @param pathname 需要解析的文件的路径名。如果忽略不填，则暂时不设置待解析文件的路径名。
	 */
	explicit IniParser(const std::string& pathname = "") : ini_path_(pathname)
		{ }

	/**
	 * @brief 如果构造对象时没有设置待解析文件的路径名，则可调用该函数来设置。如果已经设置过了，则抛出异常。
	 * @param pathname 需要解析的文件的路径名。
	 * @throws IniParseError
	 */
	void set_pathname(const std::string& pathname);

	/**
	 * @brief 配置文件解析函数。必须先调用该函数，才能调用read/read_int来获取配置项的值。
	 * @throws IniParseError
	 * @see read, read_int
	 * @return void
	 */
	void parse();

	/**
	 * @brief 读取配置项的值。必须先调用parse函数，才能调用该函数来获取配置项的值。
	 * @param sect 配置区名称。
	 * @param key  键名称。
	 * @throws IniParseError
	 * @see read_int, parse
	 * @return sect配置区里键为key的配置项的值。
	 */
	std::string read(const char* sect, const char* key);

	/**
	 * @brief 读取配置项的值。必须先调用parse函数，才能调用该函数来获取配置项的值。
	 * @param sect 配置区名称。
	 * @param key  键名称。
	 * @throws IniParseError
	 * @see read, parse
	 * @return sect配置区里键为key的配置项的值。
	 */
	int read_int(const char* sect, const char* key);
private:
	std::string  ini_path_;
	StrMap       ini_map_;
};

inline void IniParser::
set_pathname(const std::string& pathname)
{
	if (!ini_path_.empty()) {
		throw IniParseError("Pathname is Set Already!");
	}

	ini_path_ = pathname;
}

inline std::string IniParser::
read(const char* sect, const char* key)
{
	std::string mapkey = sect;
	mapkey += midstring;
	mapkey += key;

	StrMapIterator it = ini_map_.find(mapkey);
	if (it != ini_map_.end()) {
		return it->second;
	}

	throw IniParseError(std::string("Cannot Find Key '") + key + "' At Sector [" + sect + "].");
}

inline int IniParser::
read_int(const char* sect, const char* key)
{
	return atoi(read(sect, key).c_str());
}


} // end of namespace taomee

#endif // LIBTAOMEEPP_INI_PARSER_HPP_

