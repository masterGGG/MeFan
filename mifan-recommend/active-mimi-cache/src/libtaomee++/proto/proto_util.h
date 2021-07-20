/*
 * =====================================================================================
 *
 *       Filename:  proto_util.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/10/2011 02:09:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jim (xcwen), jim@taomee.com
 *        Company:  taomee
 *
 * =====================================================================================
 */
#ifndef PROTO_UTIL_H
#define PROTO_UTIL_H
#include <libtaomee++/proto/proto_base.h> 
extern "C" {
#ifdef DB_SERVER
typedef struct fdsession {
	int			fd;
	uint32_t	id;
	uint16_t	remote_port;
	uint32_t	remote_ip;
} fdsession_t;
int send_pkg_to_client(fdsession_t* fdsess, const void* pkg, const int pkglen);
int net_send(int fd, const void* data, uint32_t len);
#else
#include <async_serv/net_if.h>
#endif


#include <libtaomee/log.h>
}

#ifdef USE_HEX_LOG_PROTO
#include <libtaomee++/utils/strings.hpp>
extern int g_log_send_buf_hex_flag; 
#endif

inline int send_pkg_to_client_ex(fdsession_t* fdsess, const void* pkg, const int pkglen)
{
	int ret= send_pkg_to_client( fdsess, pkg, pkglen);
#ifdef USE_HEX_LOG_PROTO
	if (g_log_send_buf_hex_flag ){
        char outbuf[13000];
        bin2hex(outbuf,(char*)pkg,pkglen,2000);
        DEBUG_LOG("CO[ret=%d][%s]", ret,outbuf );
	}
#endif
	return  ret;

}
inline int net_send_ex(int fd, const void* pkg, const int pkglen)
{
	int ret=net_send( fd, pkg, pkglen);
#ifdef USE_HEX_LOG_PROTO
	if (g_log_send_buf_hex_flag ){
        char outbuf[13000];
        bin2hex(outbuf,(char*)pkg,pkglen,2000);
        DEBUG_LOG("SO[fd=%d][ret=%d][%s]", fd,ret ,outbuf );
	}
#endif
	return  ret;

}

//和send_pkg_to_client 类似, 
//head_buf:头部报文buf  
//:ba:
//返回和send_pkg_to_client 错误一致
inline int send_ba_to_client(fdsession_t*  fdsess ,char *head_buf, byte_array_t *p_ba)
{
	uint32_t head_len=*(uint32_t* )head_buf;
	if (p_ba==NULL) {
		return send_pkg_to_client_ex(fdsess,head_buf,head_len );
	}else{
		static byte_array_t ba;
		ba.init_postion();
		ba.write_buf(head_buf,head_len);
		ba.write_buf(p_ba->get_buf() ,p_ba->get_postion());
		uint32_t total_size=ba.get_postion();
		*(uint32_t*)(ba.get_buf())=total_size;
		return send_pkg_to_client_ex(fdsess,ba.get_buf(),total_size);
	}

}
//和send_pkg_to_client 类似, 
//head_buf:头部报文buf  
//:msg :协议的私有结构
//返回和send_pkg_to_client 错误一致
inline int send_msg_to_client(fdsession_t*  fdsess ,char *head_buf,  Cmessage *msg=NULL, uint32_t other_set_header_position =0, bool  other_set_header_position_is_bigendian=false )
{
	uint32_t head_len=*(uint32_t* )head_buf;
	if (msg==NULL) {
		return send_pkg_to_client_ex(fdsess,head_buf,head_len );
	}else{
		static byte_array_t ba;
		ba.init_postion();
		ba.write_buf(head_buf,head_len);
		msg->write_to_buf(ba);
		uint32_t total_size=ba.get_postion();
		*(uint32_t*)(ba.get_buf())=total_size;
        if (other_set_header_position !=0 ){
			if (other_set_header_position_is_bigendian){
		    	*(uint32_t*)(ba.get_buf()+other_set_header_position)=htonl(total_size-other_set_header_position);
			}else{
		    	*(uint32_t*)(ba.get_buf()+other_set_header_position)=total_size-other_set_header_position;
			}
        }
		return send_pkg_to_client_ex(fdsess,ba.get_buf(),total_size);
	}

}
//和met_send 类似 
//head_buf:头部报文buf  
//:msg :协议的私有结构
//返回和net_send错误一致
inline int net_send_msg(int fd,  char *head_buf,  Cmessage *msg=NULL,uint32_t other_set_header_position =0 , bool  other_set_header_position_is_bigendian=false   )
{

	uint32_t head_len=*(uint32_t* )head_buf;
	if (msg==NULL) {
		return net_send_ex(fd,head_buf,head_len );
	}else{
		static byte_array_t ba;
		ba.init_postion();
		ba.set_is_bigendian(other_set_header_position_is_bigendian);	
		ba.write_buf(head_buf,head_len);
		msg->write_to_buf(ba);
		uint32_t total_size=ba.get_postion();
		*(uint32_t*)(ba.get_buf())=total_size;
		if (other_set_header_position !=0 ){
			if (other_set_header_position_is_bigendian){
		    	*(uint32_t*)(ba.get_buf()+other_set_header_position)=htonl( total_size-other_set_header_position);
			}else{

		    	*(uint32_t*)(ba.get_buf()+other_set_header_position)=total_size-other_set_header_position;
			}
        }

		return net_send_ex(fd,ba.get_buf(),total_size);
	}
}
//和send_pkg_to_client 类似, 
//head_buf:头部报文buf  
//:msg :协议的私有结构
//返回和send_pkg_to_client 错误一致
inline int send_msg_to_client_bigendian(fdsession_t*  fdsess ,char *head_buf,  Cmessage *msg=NULL, uint32_t other_set_header_position =0 )
{
	uint32_t head_len=ntohl(*(uint32_t* )head_buf);
	if (msg==NULL) {
		return send_pkg_to_client_ex(fdsess,head_buf,head_len );
	}else{
		static byte_array_t ba;
		ba.init_postion();
		ba.set_is_bigendian(true);
		ba.write_buf(head_buf,head_len);
		msg->write_to_buf(ba);
		uint32_t total_size=ba.get_postion();
		*(uint32_t*)(ba.get_buf())=htonl(total_size);
        if (other_set_header_position !=0 ){
		    *(uint32_t*)(ba.get_buf()+other_set_header_position)=htonl(total_size-other_set_header_position);
        }
		return send_pkg_to_client_ex(fdsess,ba.get_buf(),total_size);
	}
}

/*
    //简单变量
    byte_array_t ba_w;
    std::vector<uint32_t>     msg_item_list_w;

    msg_item_list_w.push_back(100);
    msg_item_list_w.push_back(200);

    Ctemp_varlist<uint32_t> item_list_w(&(msg_item_list_w));
    item_list_w.write_to_buf(ba_w);
    byte_array_t ba_r(ba_w.get_buf(),ba_w.get_postion());

   //读取数据 
    std::vector<uint32_t>     msg_item_list_r;
    Ctemp_varlist<uint32_t> item_list_r(&(msg_item_list_r));
    item_list_r.read_from_buf(ba_r);

    for (uint32_t i=0;i <item_list_r.p_item_list->size();i++ ){
        KDEBUG_LOG(0,"XX:id:%u" ,
                (*(item_list_r.p_item_list))[i] );

    }
 
 //写数据 自定义类 
   item_t item;
    byte_array_t ba_w;
    std::vector<item_t>     msg_item_list_w;
    item.count=1;
    item.itemid=1000;
    msg_item_list_w.push_back(item);
    item.itemid=2000;
    msg_item_list_w.push_back(item);
    
    Ctemp_varlist<item_t> item_list_w(&(msg_item_list_w));
    item_list_w.write_to_buf(ba_w);
    byte_array_t ba_r(ba_w.get_buf(),ba_w.get_postion());
   
   //读取数据 
    std::vector<item_t>     msg_item_list_r;
    Ctemp_varlist<item_t> item_list_r(&(msg_item_list_r));
    item_list_r.read_from_buf(ba_r);

    for (int i=0;i <item_list_r.p_item_list->size();i++ ){
        KDEBUG_LOG(0,"XX:itemid:%u,cout:%u" ,
        (*(item_list_r.p_item_list))[i].itemid,
        (*(item_list_r.p_item_list))[i].count
                );

    }


*/ 

//用于变长数组的序列化
// _Tp :元素类型
template <class _Tp >
class Ctemp_varlist: public Cmessage {
    public:
        //
        std::vector<_Tp>    * p_item_list;

        Ctemp_varlist( std::vector<_Tp>    *  p_list ){
			this->init(p_list);
		}

        void init(std::vector<_Tp>  *  p_list ){
			this->p_item_list=p_list;
		}

        bool read_from_buf(byte_array_t & ba ){
		    uint32_t item_list_count ;
		    if (!ba.read_uint32( item_list_count )) return false;
		    _Tp item_list_item;
		    this->p_item_list->clear();
		    {for(uint32_t i=0; i<item_list_count;i++){
				if (!ba.read(&item_list_item) ) return false;
		        this->p_item_list->push_back(item_list_item);
		    }}
		    return true;

		}

        bool write_to_buf(byte_array_t & ba ){
		    if (!ba.write_uint32(this->p_item_list->size())) return false;
		    {for(uint32_t i=0; i<this->p_item_list->size() ;i++){
		        if (!ba.write(& (*this->p_item_list)[i])) return false;
		    }}
		    return true;
		}

};
//用于定长数组的序列化
// _Tp :元素类型
template <class _Tp >
class Ctemp_array: public Cmessage {
    public:
        //
		typedef _Tp  T_ITEM  ;
        std::vector<T_ITEM>    * p_item_list;
		uint32_t length; 
		
        Ctemp_array( std::vector<T_ITEM>    *  p_list,uint32_t array_length ){
			this->init(p_list,array_length );
		}

        void init(std::vector<T_ITEM>  *  p_list, uint32_t array_length ){
			this->length=array_length;
			this->p_item_list=p_list;
		}

        bool read_from_buf(byte_array_t & ba ){
		    {for(uint32_t i=0; i<this->length ;i++){
				if (!ba.read(&(*this->p_item_list)[i]) ) return false;
		    }}
		    return true;

		}

        bool write_to_buf(byte_array_t & ba ){
		    {for(uint32_t i=0; i<this->length ;i++){
		        if (!ba.write(& (*this->p_item_list)[i])) return false;
		    }}
		    return true;
		}

};


#endif

