/**
 *============================================================
 *  @file     item_manager.hpp
 *  @brief    用于解析Item表的公共部分。公共部分的格式必须统一，详见items.xml\n
 *            使用方法：请参阅conf_parser/item.cpp和items.xml\n
 *            g++ item.cpp -I/usr/include/libxml2 -lxml2 -ltaomee++
 * 
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */
#ifndef LIBTAOMEEPP_ITEM_MANAGER_HPP_
#define LIBTAOMEEPP_ITEM_MANAGER_HPP_

#include <string>

#include <libtaomee++/bitmanip/bitmanip.hpp>
#include <libtaomee++/conf_parser/xmlparser.hpp>

extern "C" {
#include <stdint.h>
}

namespace taomee {

/**
 * @brief 解析item配置文件出错时抛出的异常。继承自XmlParseError
 */
class ItemParseError : public XmlParseError {
public:
	explicit ItemParseError(const std::string& errmsg) : XmlParseError(errmsg)
		{ }
};

/**
 * @brief 保存Item的公共数据
 */
class Item {
public:
	/**
	 * @brief 默认构造函数，初始化Item内部成员
	 */
	Item() { init(); }

	/**
	 * @brief 返回Item ID
	 * @return item ID
	 */
	uint32_t id() const { return id_; }
	/**
	 * @brief 返回Item类别 
	 * @return item类别
	 */
	uint32_t category() const { return cat_; }
	/**
	 * @brief 返回Item在DB中的类别（有些物品，在online分类不同，但db都是保存在同一张表）。
	 * @return item在DB中的类别
	 */
	uint32_t db_category() const { return dbcat_; }
	/**
	 * @brief 返回用户最多能同时拥有的该Item的数量。某些物品，用户同时只能拥有1件，有些却可以N件。
	 * @return 用户最多能同时拥有的该Item的数量
	 */
	uint32_t max() const { return max_; }
	/**
	 * @brief 返回该Item的购买价格
	 * @return item的购买价格
	 */
	uint32_t price() const { return price_; }
	/**
	 * @brief 返回该Item的售卖价格
	 * @return item的售卖价格
	 */
	uint32_t sellprice() const { return sellprice_; }	
	/**
	 * @brief 返回该Item是否全员可买
	 * @return true or false
	 */
	bool is_item_all_buyable() const { return test_bit_on(flag_, item_all_buyable); }
	/**
	 * @brief 返回该Item是否全员可卖
	 * @return true or false
	 */
	bool is_item_all_sellable() const { return test_bit_on(flag_, item_all_sellable); }
	/**
	 * @brief 返回该Item是否vip可买
	 * @return true or false
	 */
	bool is_item_vip_buyable() const { return test_bit_on(flag_, item_vip_buyable); }
	/**
	 * @brief 返回该Item是否vip可卖
	 * @return true or false
	 */
	bool is_item_vip_sellable() const { return test_bit_on(flag_, item_vip_sellable); }
	/**
	 * @brief 返回该Item是否只有vip可用
	 * @return true or false
	 */
	bool is_item_vip_only() const { return test_bit_on(flag_, item_vip_only); }

protected:
	/**
	 * @brief 初始化Item内部成员
	 */
	void init() { memset(this, 0, sizeof(*this)); }

private:
	enum {
		item_all_buyable   = 1,
		item_all_sellable  = 2,
		item_vip_buyable   = 3,
		item_vip_sellable  = 4,
		item_vip_only	   = 5
	};

	/**
	 * @brief 设置item ID
	 * @param id item ID
	 */
	void set_id(uint32_t id) { id_ = id; }
	/**
	 * @brief 设置item类别
	 * @param cat item 类别
	 */
	void set_category(uint32_t cat) { cat_ = cat; }
	/**
	 * @brief 设置item在db中的类别
	 * @param dbcat item在db中的类别
	 */
	void set_db_category(uint32_t dbcat) { dbcat_ = dbcat; }
	/**
	 * @brief 设置用户能够拥有的该item的最大个数
	 * @param max 用户能够拥有的该item的最大个数
	 */
	void set_max(uint32_t max) { max_ = max; }
	/**
	 * @brief 设置item的购买价格 
	 * @param price item购买价格
	 */
	void set_price(uint32_t price) { price_ = price; }
	/**
	 * @brief 设置item的售卖价格
	 * @param sellprice item的售卖价格
	 */
	void set_sellprice(uint32_t sellprice) { sellprice_ = sellprice; }
	/**
	 * @brief 设置item是否全员可买
	 */
	void set_item_all_buyable() { flag_ = set_bit_on(flag_, item_all_buyable); }
	/**
	 * @brief 设置item是否全员可卖
	 */
	void set_item_all_sellable() { flag_ = set_bit_on(flag_, item_all_sellable); }
	/**
	 * @brief 设置item是否vip可买
	 */
	void set_item_vip_buyable() { flag_ = set_bit_on(flag_, item_vip_buyable); }
	/**
	 * @brief 设置item是否vip可卖
	 */
	void set_item_vip_sellable() { flag_ = set_bit_on(flag_, item_vip_sellable); }
	/**
	 * @brief 设置item是否只有vip可用
	 */
	void set_item_vip_only()	{ flag_ = set_bit_on(flag_, item_vip_only); }

	/* item id */
	uint32_t	id_;
	/* flag: tradability, viptradability, viponly */
	uint8_t		flag_;
	/* category of an item */
	uint8_t		cat_;
	/* category id for db */
	uint8_t		dbcat_;
	/* max number of an item a player can own */
	uint32_t	max_;
	/* coins needed to buy an item */
	uint32_t	price_;
	/* coins received after selling an item */
	uint32_t	sellprice_;

	/**
	 * @brief 必须把ItemManager声明为Item类的友元类，因为ItemManager里面要用到Item里的私有函数。
	 */
	template <typename ITEM, int MAX_CAT, int MAX_ELEM>
	friend class ItemManager;
};

/**
 * @brief 解析item配置文件，管理item。\n
 *        ITEM：可用Item或者继承自Item的派生类。\n
 *        MAX_CAT：指定ItemManager最大支持的item种类数目。\n
 *        MAX_ELEM：指定ItemManager每个item类别下最大支持的item个数。
 */
template <typename ITEM, int MAX_CAT, int MAX_ELEM>
class ItemManager {
public:
	/**
	 * @brief 构造函数，指定需要解析的item配置文件
	 */
	ItemManager(const char* file) : filename_(file) { }

	/**
	 * @brief 返回item_id指定的Item的指针
	 * @param item_id
	 * @return 如果找到item_id对应的物品，则返回指向该ITEM的指针，
	 *         如果找不到，则放回0。
	 */
	const ITEM* get_item(uint32_t item_id) const;
	/**
	 * @brief 解析item配置文件
	 */
	void load_items() { reload_items(); }
	/**
	 * @brief 重新解析item配置文件
	 */
	void reload_items();
private:
	enum {
		/* base item id */
		item_base_item_id  = 100000
	};

	/**
	 * @brief 初始化items_
	 */
	void init_all_items();
	/**
	 * @brief 解析一条item配置
	 */
	void load_an_item(xmlNodePtr cur, uint32_t cat_id, uint32_t dbcat, uint32_t max);
	/**
	 * @brief 解析非公共的item配置。如果item配置表中有扩展的配置，
	 *        可以通过继承ItemManager，并且覆盖该函数来实现。
	 * @return 如果解析配置文件出错,则返回-1；成功则返回0。
	 */
	virtual int load_item_extended_info(ITEM* item, xmlNodePtr cur) { return 0; }

	ITEM items_[MAX_CAT][MAX_ELEM];
	std::string filename_;
};

template <typename ITEM, int MAX_CAT, int MAX_ELEM>
inline void
ItemManager<ITEM, MAX_CAT, MAX_ELEM>::init_all_items()
{
	int i, j;
	for (i = 0; i != MAX_CAT; ++i) {
		for (j = 0; j != MAX_ELEM; ++j) {
			items_[i][j].init();
		}
	}
}

template <typename ITEM, int MAX_CAT, int MAX_ELEM>
inline const ITEM*
ItemManager<ITEM, MAX_CAT, MAX_ELEM>::get_item(uint32_t item_id) const
{
	uint32_t idx1 = ((item_id - 1) / item_base_item_id);
	uint32_t idx2 = ((item_id - 1) % item_base_item_id);

	if ((idx1 < MAX_CAT) && (idx2 < MAX_ELEM) && (items_[idx1][idx2].id() == item_id)) {
		return &(items_[idx1][idx2]);
	} else {
		return 0;
	}
}

template <typename ITEM, int MAX_CAT, int MAX_ELEM>
void
ItemManager<ITEM, MAX_CAT, MAX_ELEM>::reload_items()
{
	// xmlDocPtr doc = xmlParseFile(filename_.c_str());
	xmlDocPtr doc = xmlReadFile(filename_.c_str(), 0, XML_PARSE_NOBLANKS);
	if (!doc) {
		throw ItemParseError(std::string("failed to parse item file '") + filename_ + "'");
	}

	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if (!cur) {
		xmlFreeDoc(doc);
		throw ItemParseError(std::string("xmlDocGetRootElement error when loading item file '") + filename_ + "'");
	}

	init_all_items();

	// load items from an xml file
	uint32_t id, max, dbcatid;
	cur = cur->xmlChildrenNode; 
	while (cur) {
		if (!xmlStrcmp(cur->name, reinterpret_cast<const xmlChar*>("Cat"))) {
			// Category ID
			get_xml_prop(id, cur, "ID");
			get_xml_prop(dbcatid, cur, "DbCatID");
			get_xml_prop(max, cur, "Max");
			if (id >= MAX_CAT) {
				std::ostringstream oss;
				oss << "invalid category id: " << id;
				throw ItemParseError(oss.str());
			}
			// load configs for each item
			load_an_item(cur->xmlChildrenNode, id, dbcatid, max);
		}
		cur = cur->next;
	}

	xmlFreeDoc(doc);
}

template <typename ITEM, int MAX_CAT, int MAX_ELEM>
void
ItemManager<ITEM, MAX_CAT, MAX_ELEM>::load_an_item(xmlNodePtr cur, uint32_t cat_id, uint32_t dbcat, uint32_t max)
{
	uint32_t id, idx, flag;
	uint32_t base_id = (cat_id * item_base_item_id) + 1;
	ITEM*    items   = items_[cat_id];
	while (cur) {
		if (!xmlStrcmp(cur->name, (const xmlChar*)"Item")) {
			get_xml_prop(id, cur, "ID");

			idx = id - base_id;
			if ((id < base_id) || ((idx) >= MAX_ELEM) || items[idx].id()) {
				std::ostringstream oss;
				oss << "invalid/duplicated item: id=" << id << " catid=" << cat_id;
				throw ItemParseError(oss.str());
			}

			ITEM* item = &(items[idx]);

			item->set_id(id);
			item->set_category(cat_id);
			item->set_db_category(dbcat);

			uint32_t tmp;
			get_xml_prop_def(tmp, cur, "Max", max);
			item->set_max(tmp);
			get_xml_prop_def(tmp, cur, "Price", 0);
			item->set_price(tmp);
			get_xml_prop_def(tmp, cur, "SellPrice", 0);
			item->set_sellprice(tmp);
			// decode flag
			get_xml_prop_def(flag, cur, "Tradability", 3);
			if (test_bit_on(flag, 1)) {
				item->set_item_all_buyable();
			}
			if (test_bit_on(flag, 2)) {
				item->set_item_all_sellable();
			}
			get_xml_prop_def(flag, cur, "VipTradability", 3);
			if (test_bit_on(flag, 1)) {
				item->set_item_vip_buyable();
			}
			if (test_bit_on(flag, 2)) {
				item->set_item_vip_sellable();
			}
			get_xml_prop_def(flag, cur, "VipOnly", 0);
			if (flag) {
				item->set_item_vip_only();
			}

			// load extended info of an item
			if (load_item_extended_info(item, cur) != 0) {
				std::ostringstream oss;
				oss << "error occur while loading extended item info: id=" << id;
				throw ItemParseError(oss.str());
			}
		}
	
		cur = cur->next;
	}
}

} // end of namespace taomee

#endif // LIBTAOMEEPP_ITEM_MANAGER_HPP_

