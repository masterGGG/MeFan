/*
 * =====================================================================================
 *
 *       Filename:  a.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/10/2011 11:55:09 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jim (xcwen), jim@taomee.com
 *        Company:  taomee
 *
 * =====================================================================================
 */
#ifndef  CCMD_MAP_H
#define  CCMD_MAP_H
#include <string.h>
#include <map>
#include <assert.h>
#include <libtaomee++/proto/proto_base.h>
extern "C" {
#include <libtaomee/log.h>
}

/**
 *Ccmdmap:
 * 命令--私有结构
 */

template <class _Tp > 
class Ccmd_map{
	public:
		typedef _Tp  T_ITEM  ;
		std::map< uint32_t ,T_ITEM > cmdmap ;
	private:	
	public:
		typedef typename std::map<uint32_t,T_ITEM >::iterator  iterator;
		Ccmd_map()
		{
		}
		inline void initlist(T_ITEM * p_item_list, int count ){
             
            std::vector<T_ITEM>  item_list( p_item_list, p_item_list+count );
            this->initlist(item_list );
		}

		inline void initlist(std::vector<T_ITEM>& item_list){
			for (int i=0;i<item_list.size();i++ ){
				uint32_t cmdid=item_list[i].cmdid ;
				assert( cmdid!=0 );
                assert(this->cmdmap.find(cmdid)==this->cmdmap.end() );
                
                
				this->cmdmap[cmdid]= item_list[i];
			}
		}


		inline T_ITEM * getitem(uint32_t cmdid){
			if (this->cmdmap.find(cmdid)!=this->cmdmap.end() ){
				return  &this->cmdmap[cmdid];
			}else{
				return  NULL;
			}
		}
};

template <class _Tp > 
struct Ccmd{
	public:
		typedef _Tp  T_Func;
		uint32_t cmdid;
		Cmessage* proto_pri_msg;
		uint32_t  md5_tag; //md5值
		uint32_t  bind_bitmap;
		T_Func   func;
};


#endif  /* CCMD_MAP_H */
