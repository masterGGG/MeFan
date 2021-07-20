/**
 *============================================================
 *  @file      factory.hpp
 *  @brief     Factory用于构造同一父类的所有产品。\n
 *  
 *  compiler   gcc4.1.2
 *  platform   Linux
 *
 *  copyright:  TaoMee, Inc. ShangHai CN. All rights reserved.
 *
 *============================================================
 */

#ifndef LIBTAOMEEPP_FACTORY_HPP_
#define LIBTAOMEEPP_FACTORY_HPP_

#include <libtaomee++/utils/noncopyable.hpp>
#include <libtaomee++/utils/obj_registry.hpp>

namespace taomee {

/**
 * @brief Factory用于构造同一父类的所有产品。\n
 *        KeyType：键的类型，一个键可以唯一确定一种产品。\n
 *        AbstractProdType：产品的基类。所有产品必须都要有一个接收“const void*”参数的构造函数，
 *                          例如“AbsProd(const void* data);”。
 */
template <typename KeyType, typename AbstractProdType>
class Factory {
private:
	/**
	 * @brief 产品制造工的抽象父类
	 */
	class AbstractWorker {
	public:
		/**
		 * @brief 虚析构函数
		 */
		virtual ~AbstractWorker()
			{ }
		/**
		 * @brief 抽象的制造产品方法
		 */
		virtual AbstractProdType* create(const void* data = 0) const = 0;
	};

	/**
	 * @brief 产品制造工的模板类，制造ProdType类型的具体产品，ProdType必须继承于AbstractProdType。
	 */
	template <typename ProdType>
	class Worker : public AbstractWorker {
	public:
		~Worker()
			{ }
		/**
		 * @brief 创建产品实例的方法，调用产品的Prod(const void*)构造函数来创建一个具体的产品对象。
		 * @return 成功则返回new出来的产品对象的指针，失败则抛出std::bad_alloc异常。
		 * @note 当产品不再被使用时，必须调用delete将其释放。
		 */
		ProdType* create(const void* data = 0) const
			{ return new ProdType(data); }
	};

public:
	/**
	 * @brief 添加一种产品。\n
	 *        ProdType：产品的类型，必须继承于AbstractProdType。
	 * @param key 产品的键，通过这个键可以唯一确定一种产品。
	 * @param override 是否覆盖键相同的产品，默认是false不覆盖。
	 *                 如果选择不覆盖，则当键已存在时，插入失败，返回false。
	 *                 如果选择覆盖，则当键已存在时，把老的产品删掉，插入新的产品，并且返回true。
	 * @return 成功返回true，失败返回false。
	 */
	template <typename ProdType>
	bool add_product(const KeyType& key, bool override = false)
		{ return m_holder.insert_object(key, new Worker<ProdType>, override); }

	/**
	 * @brief 删除一个键为key的产品
	 * @param key 产品的键
	 */
	void remove_product(const KeyType& key)
		{ m_holder.remove_object(key); }

	/**
	 * @brief 创建一个键为key的产品实例，当产品不再被使用时，必须调用delete将其释放。
	 * @param key 产品的键
	 * @param data 传递给产品的构造函数的参数
	 * @return 如果key对应的产品不存在，则返回0；如果成功创建出产品实例，则返回指向该产品父类的指针；
	 *         如果创建产品失败，则抛出std::bad_alloc异常。
	 */
	AbstractProdType* create(const KeyType& key, const void* data = 0) const;

private:
	/*! 保存所有工人，不同的工人可以制作不同的产品 */
	ObjectRegistry<KeyType, AbstractWorker> m_holder;
};

template <typename KeyType, typename AbstractProdType>
inline AbstractProdType* Factory<KeyType, AbstractProdType>::create(const KeyType& key, const void* data) const
{
	const AbstractWorker* worker = m_holder.get_object(key);
	if (worker) {
		return worker->create(data);
	}
	
	return 0;
}

}

#endif // LIBTAOMEEPP_FACTORY_HPP_
