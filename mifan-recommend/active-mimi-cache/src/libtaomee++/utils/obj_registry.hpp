/**
 *============================================================
 *  @file      obj_registry.hpp
 *  @brief     用于保存所有同类型的对象指针
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_OBJECT_REGISTRY_HPP_
#define LIBTAOMEEPP_OBJECT_REGISTRY_HPP_

#include <map>

#include <libtaomee++/utils/noncopyable.hpp>

namespace taomee {

/**
 * @brief ObjectRegistry用于保存所有同类型的对象指针
 */
template<typename KeyType, typename ObjType>
class ObjectRegistry : public Noncopyable {
private:
	/*! 保存对象指针的容器 */
	typedef std::map<KeyType, ObjType*> RegistryMapType;

public:
	/*! size type */
	typedef typename RegistryMapType::size_type size_type;

public:
	/**
	 * @brief 析构函数，调用delete删除所有注册进来的对象
	 */
	~ObjectRegistry();

	/**
	 * @brief 注册一个对象
	 * @param key 对象的键
	 * @param obj new出来的对象
	 * @param override 是否覆盖键相同的对象，默认是false不覆盖。
	 *                 如果选择不覆盖，则当键已存在时，插入失败，返回false。
	 *                 如果选择覆盖，则当键已存在时，把老的对象delete掉，插入新的对象，并且返回true。
	 * @return 成功返回true，失败返回false。
	 */
	bool insert_object(const KeyType& key, ObjType* obj, bool override = false);

	/**
	 * @brief 删除一个已注册对象
	 * @param key 对象的键
	 */
	void remove_object(const KeyType& key);

	/**
	 * @brief 得到一个已注册对象
	 * @param key 对象的键
	 * @return 返回和key对应的对象的指针。如果找不到，则返回0。
	 */
	const ObjType* get_object(const KeyType& key) const;

	/**
	 * @brief 如果键为key的对象存在，则返回true，反之，则返回false
	 * @param key 对象的键
	 */
    bool has_object(const KeyType& key) const
    	{ return (m_reg_objs.find(key) != m_reg_objs.end()); }

	/**
	 * @brief ObjectRegistry中注册的对象个数
	 * @return 返回已注册对象个数
	 */
	size_type size() const
		{ return m_reg_objs.size(); }

private:
	/*! Registered Objects */
    RegistryMapType m_reg_objs;
};

template<typename KeyType, typename ObjType>
ObjectRegistry<KeyType, ObjType>::
~ObjectRegistry()
{
    for(typename RegistryMapType::iterator iter = m_reg_objs.begin(); iter != m_reg_objs.end(); ++iter) {
        delete iter->second;
    }
}

template<typename KeyType, typename ObjType>
bool ObjectRegistry<KeyType, ObjType>::
insert_object(const KeyType& key, ObjType* obj, bool override)
{
	typename RegistryMapType::iterator iter = m_reg_objs.find(key);
	if (iter != m_reg_objs.end()) {
		if (!override) {
			return false;
		}
		delete iter->second;
	}

	m_reg_objs[key] = obj;
	return true;
}

template<typename KeyType, typename ObjType>
void ObjectRegistry<KeyType, ObjType>::
remove_object(const KeyType& key)
{
	typename RegistryMapType::iterator iter = m_reg_objs.find(key);
	if (iter != m_reg_objs.end()) {
		delete iter->second;
		m_reg_objs.erase(iter);
	}
}

template<typename KeyType, typename ObjType>
const ObjType* ObjectRegistry<KeyType, ObjType>::
get_object(const KeyType& key) const
{
	typename RegistryMapType::const_iterator iter = m_reg_objs.find(key);
	return ((iter != m_reg_objs.end()) ? iter->second : 0);
}

};

#endif // LIBTAOMEEPP_OBJECT_REGISTRY_HPP_

