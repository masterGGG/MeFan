
#ifndef  PROTO_CLIENT_H
#define  PROTO_CLIENT_H

//db clinet 接口类
#include  <libtaomee++/utils/tcpip.h>
class Cproto_base : Ctcp {
	private:
		uint32_t  proto_header_len_;
		byte_array_t ba_in_;
		byte_array_t ba_out_;
	public:
		Cproto_base(const char * ipport, uint32_t send_recv_timeout=1, uint32_t reconnect_interval=10 ):Ctcp(ipport,send_recv_timeout,reconnect_interval ) {
			this->proto_header_len_=18;
		}

	 	virtual	int pkg_header( uint32_t userid,uint16_t cmdid ) {
			this->ba_in_.write_uint32((uint32_t)0);//暂不设置长度
			this->ba_in_.write_uint32(uint32_t(0)); //seq
			this->ba_in_.write_uint16(cmdid);
			this->ba_in_.write_uint32(uint32_t(0));//ret
			this->ba_in_.write_uint32(userid);
		}

	 	virtual	int get_pkg_len(char * buf,int buf_len ){
			if (buf_len>=4) return *((uint32_t *)buf );
			else return 0;
		}

	 	virtual	int set_pkg_header_len( uint32_t len ){
			*((uint32_t *)this->ba_in_.get_buf())=len;
		}

	 	virtual	int get_result( char * buf, int buf_len ){
			if ( buf_len< this->proto_header_len_)  return -1;
			return *((uint32_t*)( buf+10 ));//ret

		}

		inline int deal_proto_cmd(uint32_t userid,uint16_t cmdid,Cmessage *c_in, Cmessage *c_out  )
		{
			char * out_buf =NULL ;
			int out_buf_size=0;
			this->ba_in_.init_postion();
			this->pkg_header(userid,cmdid );
			if (c_in !=NULL  ){
				this->ba_in_.write(c_in );
			}
			this->set_pkg_header_len(this->ba_in_.get_postion() );
			int ret;
			ret=this->do_net_io(this->ba_in_.get_buf(),this->ba_in_.get_postion(),
				&out_buf,&out_buf_size );
			if ( ret==1003) return ret;
			
			ret=this->get_result(out_buf,out_buf_size );
			if ( ret!=0) {//-1 or >0
				free(out_buf);
				return ret;
			}
			this->ba_out_.init_read_mode(out_buf+this->proto_header_len_,
					out_buf_size-this->proto_header_len_  );

			bool unpack_ret;
			if (c_out==NULL){
				if (out_buf_size=this->proto_header_len_ ){
					return 0;
				}else{
					return 1010;
				}
			}
			unpack_ret=this->ba_out_.read( c_out );

			free(out_buf);
 			if (unpack_ret){
				return 0;
			}else{
				return 1010;
			}
		}


};


#endif  /*PROTO_CLIENT_H*/

