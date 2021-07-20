#ifndef PROTO_BASE_H
#define PROTO_BASE_H
#include <stdint.h>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
extern "C"
{
#include <libtaomee/log.h>
}
#if USE_PROTO_READ_FROM_XMLNODE
#include <libxml2/libxml/parser.h>
#endif
//目前默认系统内部是小端
inline void uint32_t_swap(uint32_t &value ){
	value=ntohl(value );
}
inline void int32_t_swap(int32_t &value ){
	value=ntohl(value );
}
inline void uint16_t_swap(uint16_t &value ){
	value=ntohs(value );
}
inline void int16_t_swap(int16_t &value ){
	value=ntohs(value );
}
inline void uint8_t_swap(uint8_t &value ){
	//不调整
}
inline void int8_t_swap(int8_t &value ){
	//不调整
}

inline void uint64_t_swap(uint64_t &value ){
	value=ntohl(value>>32) +(((uint64_t )ntohl( value&0xFFFFFFFF))<<32);
}
inline void int64_t_swap(int64_t &value ){
	value=ntohl(value>>32) +(((uint64_t )ntohl( value&0xFFFFFFFF))<<32);
}

inline void char_swap(char &value ){
	//不调整
}

inline void double_swap(double &value ){
	//不调整
}


#include <string>

class byte_array_t  ;

class Cmessage {
	public :
		//ba 为读取模式
		virtual bool read_from_buf(byte_array_t & ba ){ return true; }
		//ba 为写入模式
		virtual bool write_to_buf(byte_array_t & ba ) { return true; }
		
#if USE_PROTO_READ_FROM_XMLNODE
		virtual bool read_from_xmlnode(xmlNodePtr cur_node ){ return  true;}
#endif
		//得到类型的信息
		virtual const char * _get_type_id() { return "Cmessage"; }

		//对buffer处理
		bool write_to_buf_ex(char * buf,uint32_t buf_size,uint32_t * p_write_len=NULL ); 
		bool read_from_buf_ex(const char * buf,uint32_t buf_size ); 
		std::string write_to_buf_ex(); 
		bool read_from_buf_ex(std::string buf ); 

		//ba为写入模式, is_init_ba_posion:表示是否要从ba的开始处写起:即:清空ba原有数据
		bool write_to_buf_ex( byte_array_t & ba, bool is_init_ba_posion=true ); 

		//ba不分模式,
		//is_init_ba_posion(只对ba是读取模式有效，写入模式不处理): 表示是否要从ba的开始处读起,即:将ba整个数据还原 		 
		bool read_from_buf_ex( byte_array_t & ba, bool is_init_ba_posion=true ); 
	
		virtual void init(){ }
		virtual ~Cmessage(){ }
};


#define GEN_READ_SINGLE_P_VALUE_FUNC( value_type  ) \
		bool read(value_type *p_value ){\
			if ( !this->m_is_read_only ) return false;\
			if(this->m_postion+sizeof(value_type)<=this->m_size) {\
				(*p_value)=*(value_type*)(this->m_buf+this->m_postion );\
				if(this->is_bigendian){value_type##_swap((*p_value)); }\
				this->m_postion+=sizeof(value_type);\
				return true;			\
			}else{\
				return false; \
			}\
		}

#define GEN_WRITE_SINGLE_P_VALUE_FUNC( value_type  ) \
		bool write(const  value_type * p_value){\
			if ( this->m_is_read_only ) return false;\
			if (!this->reset_size(sizeof(value_type) )) return false;\
			*(value_type *)(this->m_buf+this->m_postion )= *p_value;\
			if (this->is_bigendian ){\
				value_type##_swap( *(value_type *)(this->m_buf+this->m_postion ) ); \
			}\
			this->m_postion+= sizeof(value_type) ;\
			return true;\
		}\





#define GEN_READ_SINGLE_VALUE_FUNC( func ,value_type  ) \
		bool func (value_type &value ){\
			if ( !this->m_is_read_only ) return false;\
			if(this->m_postion+sizeof(value_type)<=this->m_size) {\
				value=*(value_type*)(this->m_buf+this->m_postion );\
				if(this->is_bigendian){value_type##_swap(value); }\
				this->m_postion+=sizeof(value_type);\
				return true;			\
			}else{\
				return false; \
			}\
		}

#define GEN_WRITE_SINGLE_VALUE_FUNC( func ,value_type  ) \
		bool func(const  value_type value){\
			if ( this->m_is_read_only ) return false;\
			if (!this->reset_size(sizeof(value) )) return false;\
			*(value_type *)(this->m_buf+this->m_postion )= value;\
			if (this->is_bigendian ){\
				value_type##_swap( *(value_type *)(this->m_buf+this->m_postion ) ); \
			}\
			this->m_postion+= sizeof(value) ;\
			return true;\
		}\

class  byte_array_t {
		char * m_buf;
		uint32_t m_size  ;
		bool 	m_is_read_only; /*是否是用于读取数据*/
		uint32_t m_postion;
		bool   is_bigendian;
				
	public:
		inline const char * get_buf( ){ return this->m_buf; }
		inline uint32_t get_postion( ){ return this->m_postion; }
		inline uint32_t get_bufsize( ){ return this->m_size; }
		inline void init_postion( ){ this->m_postion=0; }
		inline void set_is_bigendian( bool value ){ this->is_bigendian=value; }
		inline bool get_is_read_only(){ return this->m_is_read_only; }

		byte_array_t(const char * buf,int size ){
			this->m_is_read_only=false;
			this->m_buf=NULL;
			this->is_bigendian=false;
			this->init_read_mode(buf,size );
		}
	

		void init_read_mode(const char * buf,int size ){//读取模式
			if(!this->m_is_read_only){
				free(this->m_buf);
			}
			this->m_buf=(char*)buf;
			this->m_size=size;
			this->m_postion=0;
			this->m_is_read_only=true;
		}

		byte_array_t(uint32_t malloc_size=32 ){//写入模式
			this->m_buf=(char*)malloc(malloc_size );
			this->m_size=malloc_size;
			this->m_postion=0;
			this->m_is_read_only=false;
			this->is_bigendian=false;
		}
		//通用，read,write
		GEN_READ_SINGLE_P_VALUE_FUNC(uint32_t )
		GEN_READ_SINGLE_P_VALUE_FUNC(uint16_t )
		GEN_READ_SINGLE_P_VALUE_FUNC(uint8_t )

		GEN_WRITE_SINGLE_P_VALUE_FUNC(uint32_t )
		GEN_WRITE_SINGLE_P_VALUE_FUNC(uint16_t )
		GEN_WRITE_SINGLE_P_VALUE_FUNC(uint8_t )

		GEN_READ_SINGLE_P_VALUE_FUNC(int32_t )
		GEN_READ_SINGLE_P_VALUE_FUNC(int16_t )
		GEN_READ_SINGLE_P_VALUE_FUNC(int8_t )
		GEN_WRITE_SINGLE_P_VALUE_FUNC(int32_t )
		GEN_WRITE_SINGLE_P_VALUE_FUNC(int16_t )
		GEN_WRITE_SINGLE_P_VALUE_FUNC(int8_t )

		GEN_WRITE_SINGLE_P_VALUE_FUNC(int64_t )
		GEN_WRITE_SINGLE_P_VALUE_FUNC(uint64_t )

		GEN_READ_SINGLE_P_VALUE_FUNC(int64_t )
		GEN_READ_SINGLE_P_VALUE_FUNC(uint64_t )

		GEN_READ_SINGLE_P_VALUE_FUNC(char )
		GEN_WRITE_SINGLE_P_VALUE_FUNC(char )

		GEN_READ_SINGLE_P_VALUE_FUNC(double)
		GEN_WRITE_SINGLE_P_VALUE_FUNC(double)



		inline bool write( Cmessage * p_value){
			return p_value->write_to_buf(*this);
		}
		inline bool read( Cmessage * p_value){
			return p_value->read_from_buf(*this);
		}








		//原先的写入函数
		GEN_READ_SINGLE_VALUE_FUNC(read_uint32,uint32_t )
		GEN_READ_SINGLE_VALUE_FUNC(read_uint16,uint16_t )
		GEN_READ_SINGLE_VALUE_FUNC(read_uint8,uint8_t )

		GEN_WRITE_SINGLE_VALUE_FUNC(write_uint32,uint32_t )
		GEN_WRITE_SINGLE_VALUE_FUNC(write_uint16,uint16_t )
		GEN_WRITE_SINGLE_VALUE_FUNC(write_uint8,uint8_t )

		GEN_READ_SINGLE_VALUE_FUNC(read_int32,int32_t )
		GEN_READ_SINGLE_VALUE_FUNC(read_int16,int16_t )
		GEN_READ_SINGLE_VALUE_FUNC(read_int8,int8_t )
		GEN_WRITE_SINGLE_VALUE_FUNC(write_int32,int32_t )
		GEN_WRITE_SINGLE_VALUE_FUNC(write_int16,int16_t )
		GEN_WRITE_SINGLE_VALUE_FUNC(write_int8,int8_t )

		GEN_WRITE_SINGLE_VALUE_FUNC(write_int64,int64_t )
		GEN_WRITE_SINGLE_VALUE_FUNC(write_uint64,uint64_t )

		GEN_READ_SINGLE_VALUE_FUNC(read_int64,int64_t )
		GEN_READ_SINGLE_VALUE_FUNC(read_uint64,uint64_t )


		GEN_READ_SINGLE_VALUE_FUNC(read_char,char )
		GEN_WRITE_SINGLE_VALUE_FUNC(write_char,char )

		GEN_READ_SINGLE_VALUE_FUNC(read_double ,double )
		GEN_WRITE_SINGLE_VALUE_FUNC(write_double,double )
	
		


		bool read_buf(char*  buf, uint32_t size ){
			if(this->m_postion+size<=this->m_size) {
				memcpy(buf,this->m_buf+this->m_postion,size);
				this->m_postion+= size;
				return true;			
			}else{
				return false; 
			}
		}

		bool reset_size(uint32_t add_size ){

			if (this->m_is_read_only) return false;
			if(this->m_postion+add_size>this->m_size) {
				this->m_buf=(char*)realloc(this->m_buf,(this->m_postion+add_size));
				if (!this->m_buf) return false;
				this->m_size=(this->m_postion+add_size);
			}
			return true;
		}

		bool write_buf(const char*  buf, uint32_t size ){
			if (!this->reset_size(size)) return false;
			memcpy(this->m_buf+this->m_postion,buf,size);
			this->m_postion+= size;
			return true;
		}



		bool is_end( ){ return this->m_postion==this->m_size; }
		virtual ~byte_array_t(){
			if( ! this->m_is_read_only ){
				free(this->m_buf);
			}
		}
		
};
#include <iostream>
#include <string>
#include <sstream>
template <class _Tp >  
inline const char * value_to_str(_Tp value ){
	static std::string str;
	std::stringstream strStream;	
	strStream<<value;	
	str=strStream.str();
	return str.c_str();
}

inline const char * value_to_str(uint8_t value ){
	static std::string str;
	std::stringstream strStream;	
	strStream<<int(value);	
	str=strStream.str();
	return str.c_str();
}

inline const char * value_to_str(int8_t value ){
	static std::string str;
	std::stringstream strStream;	
	strStream<<int(value);	
	str=strStream.str();
	return str.c_str();
}



inline std::string Cmessage::write_to_buf_ex(){
	byte_array_t ba;
	ba.init_postion();
	this->write_to_buf(ba);
	return std::string(ba.get_buf(), ba.get_postion() );
}

inline bool Cmessage::write_to_buf_ex(char * buf,uint32_t buf_size,uint32_t * p_write_len ) 
{
	byte_array_t ba;
	ba.init_postion();
	this->write_to_buf(ba);
	if (ba.get_postion()>buf_size ){
		//长度不够
		return false;
	}
	memcpy(buf,ba.get_buf(), ba.get_postion() );
	if (p_write_len!=NULL ){
		*p_write_len=ba.get_postion();
	}
	
	return true;
}

inline bool Cmessage::read_from_buf_ex(std::string buf ) 
{
	byte_array_t ba(buf.c_str(),buf.size() );
	this->init();
 	//失败
    if (!this->read_from_buf(ba)) {
        return false;
    }
    if (!ba.is_end()) {
        return  false;
    }
	return true;
}

inline bool Cmessage::read_from_buf_ex(const char * buf,uint32_t buf_size ) 
{
	byte_array_t ba(buf,buf_size);
	this->init();
 	//失败
    if (!this->read_from_buf(ba)) {
        return false;
    }
    if (!ba.is_end()) {
        return  false;
    }
	return true;
}
//ba 写入模式
inline bool Cmessage::write_to_buf_ex( byte_array_t & ba, bool is_init_ba_posion)
{
	if (is_init_ba_posion ){
		ba.init_postion();
	}
	return this->write_to_buf(ba);
}

inline bool Cmessage::read_from_buf_ex( byte_array_t & ba, bool is_init_ba_posion )
{

	if( ba.get_is_read_only() ){//原来就是读取模式
		if (is_init_ba_posion ){
			ba.init_postion();
		}
		return this->read_from_buf_ex(ba);	
	}else{//原来是写入模式时,不考虑 is_init_ba_posion
		return this->read_from_buf_ex(ba.get_buf(),ba.get_postion());
	}
}
	
	

#define  P_IN dynamic_cast<typeof p_in>(c_in);
#define  P_OUT dynamic_cast<typeof p_out>(c_out);

#endif
