/*
 * =====================================================================================
 *
 *       Filename:  strings.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2010年06月07日 10时20分58秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jim (xcwen), jim@taomee.com
 *        Company:  taomee
 *
 * =====================================================================================
 */

#ifndef  strings_INC
#define  strings_INC
#include <string>
#include <algorithm>
#include <fstream>
#include <stdint.h>
#include <libtaomee++/utils/md5.h>
#include <libtaomee++/utils/tcpip.h>

inline bool isnewline(const char c)
{
    return c == '\r' || c == '\n';
}

// Make sure that we can use Mac, DOS, or Unix style text files on any system
// and they will work, by making sure the definition of whitespace is consistent
inline bool portable_isspace(const char c)
{
    // returns true only on ASCII spaces
    if (static_cast<unsigned char>(c) >= 128)
        return false;
    return isnewline(c) || isspace(c);
}

// Make sure we regard '\r' and '\n' as a space, since Mac, Unix, and DOS
// all consider these differently.
inline bool notspace(const char c)
{
    return !portable_isspace(c);
}

enum { REMOVE_EMPTY = 0x01, /**< REMOVE_EMPTY : remove empty elements. */
	  STRIP_SPACES  = 0x02  /**< STRIP_SPACES : strips leading and trailing blank spaces. */
};
inline std::string &strip(std::string &str)
{
	// If all the string contains is whitespace,
	// then the whitespace may have meaning, so don't strip it
	std::string::iterator it = std::find_if(str.begin(), str.end(), notspace);
	if (it == str.end())
		return str;

	str.erase(str.begin(), it);
	str.erase(std::find_if(str.rbegin(), str.rend(), notspace).base(), str.end());

	return str;
}

#include <vector>
inline std::vector< std::string > split(std::string const &val, char c=',', int flags=REMOVE_EMPTY|STRIP_SPACES)
{
	std::vector< std::string > res;
	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();
	//std::string val1("");
	//printf("===val[%d]===",val1.begin() ==val1.end());

	while (i2 != val.end()) {
		if (*i2 == c) {
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while (i2 != val.end() && *i2 == ' ')
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty()){
		res.push_back(new_val);
	}
	return res;
}

//当dst传入null时将会使用 static buf,最大长度是500
inline char * bin2hex(char * dst, char * src,int len,int max_len=0xFFFF)
{
	static char buf[500*3+1];
	if (dst==NULL){
		max_len=500;
		dst=buf;
	}

	int hex;
	int i;
	int di;
	if (len>max_len) len=max_len;
	for(i=0;i<len;i++){
		hex=((unsigned char)src[i])>>4;
		di=i*3;
		dst[di]=hex<10?'0'+hex:'A'-10 +hex ;
		hex=((unsigned char)src[i])&0x0F;
		dst[di+1]=hex<10?'0'+hex:'A'-10 +hex ;
		dst[di+2]=' ';
	}
	dst[len*3]=0;
	return dst;
}

//dst为NULL时，会使用static char buf[500*2+1]; ,最多500个字节
//   不为：NULL，由使用用户传进来的buf
//is_upper:是否用大写字母:0不是， !=0:是
inline void bin2hex_frm(char * dst, char * src,int len,  int is_upper  )
{
	uint32_t max_len=len; 
	static char buf[500*2+1];
	if (dst==NULL){
		max_len=500;
		dst=buf;
	}
	char ch_a;	
	if (is_upper!=0 ){
		ch_a='A';
	}else{
		ch_a='a';
	}

	int hex;
	int i;
	int di;
	if (len>max_len) len=max_len;
	for(i=0;i<len;i++){
		di=i*2;
		hex=((unsigned char)src[i])>>4;
		dst[di]=hex<10?'0'+hex:ch_a-10 +hex ;
		hex=((unsigned char)src[i])&0x0F;
		dst[di+1]=hex<10?'0'+hex:ch_a-10 +hex ;
	}
	dst[len*2]=0;
}

inline int get_hex(char ch)
{
	if (ch>='0' && ch<='9') return  ch-'0';
	else if (ch>='A' && ch<='F') return  10+ch-'A';
	else if (ch>='a' && ch<='f') return  10+ch-'a';
	else return 0; 
}

inline void hex2bin(char * dst, const char * src,int len)
{
	for(int i=0;i<len;i++){
		dst[i]=(unsigned char)((get_hex(src[i*2])<<4)+get_hex(src[i*2+1]));
	}
}
#include <string.h>
//得到 GCC __DATE__ 的 YYYYMMDD格式
inline const char * GetCompiledDate( const char * _date  )
{
    static char _buf[9] = {0};
    if(_buf[0] == 0)
    {
        static const char * _month[] =
        {
            "Jan",  "Feb",  "Mar",  "Apr",  "May",  "Jun",
            "Jul",  "Aug",  "Sep",  "Oct",  "Nov",  "Dec",
        };
        memcpy(_buf, _date + 7, 4);
        memcpy(_buf+6, _date + 4, 2);
        int month = 0;
        for(int i = 0; i < 12; i++)
        {
            if(memcmp(_month[i], _date, 3) == 0)
            {
                month = i+1;
                break;
            }
        }
        _buf[4] = month / 10 % 10 + '0';
        _buf[5] = month % 10 + '0';
    }
    return _buf;
}


inline std::string file_list_md5( std::vector<std::string> file_name_list)
{
	utils::MD5 md5;
    unsigned char md[16];
    int length,i;
    char buffer[1024];
	md5.reset();
    for (uint32_t i=0;i<file_name_list.size();i++ ) {
        std::ifstream fin(file_name_list[i].c_str(),std::ios::in|std::ios::binary);
        if (fin.is_open()){
            while(!fin.eof()){
                fin.read(buffer, 1024);
                length = fin.gcount();
                if (length > 0) { 
                    md5.update(buffer, length);
                } 
            } 
        }else{
            return "00000000000000000000000000000000";
        }
    } 
	return md5.toString();

}

// version_str::版本信息
//file_name_list:存放每台机器上都一样的文件:bin,so, txt,xml...
//最后会返回程序的版本信息
inline std::string get_version_str ( std::string version_str ,std::vector<std::string> file_name_list,bool is_out_ip =true ,uint32_t ip_type_flag =0x01 )
{
	if  (is_out_ip){
		return std::string("ip:")+get_ip_ex(ip_type_flag )+ std::string("|VERSION:")+version_str+"|compiled:"+GetCompiledDate(__DATE__) 
	   +"-"__TIME__"|md5:"+file_list_md5(file_name_list)	;
	}else{
		return std::string("VERSION:")+version_str+"|compiled:"+GetCompiledDate(__DATE__) 
	   +"-"__TIME__"|md5:"+file_list_md5(file_name_list)	;
	}
}
// version_str::版本信息
//file_name_list:存放每台机器上都一样的文件:bin,so, txt,xml...
//最后会返回程序的版本信息,长度定义在头部
inline const char * get_version_send_buf ( std::string version_str ,std::vector<std::string> file_name_list,bool is_out_ip =true ,uint32_t ip_type_flag =0x01 )
{
	static char version[256];
	memset(version,0,sizeof(version ));
	*((uint32_t*)version)=256;
	strncpy(version+18,
			get_version_str(version_str,file_name_list ,is_out_ip,ip_type_flag ).c_str(),
			256-18);
	return version;
}




#endif
