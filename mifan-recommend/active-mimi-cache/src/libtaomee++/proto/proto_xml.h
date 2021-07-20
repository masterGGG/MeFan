/**
 * ============================================================
 * @file   proto_xml.h
 * @author jim (jim@taomee.com)
 * @date   2013-01-11 16:05:56
 *
 * Last-Updated: 2013-02-28 18:17:39
 *           By: jim
 *     Update #: 108
 *
 * 
 * @brief  处理xml 读取到 Cmessage 中
 * 
 * ============================================================
 */
#ifndef _PROTO_XML_H_
#define _PROTO_XML_H_
#include <libtaomee++/proto/proto_base.h>
#include <libxml2/libxml/parser.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C"
{
#endif
#include <libtaomee/log.h>
#ifdef __cplusplus
}
#endif


#include <vector>
#include <sstream>

#include <algorithm>
#include <functional>
inline bool str2vector_check_set_space( char ch  ){
	return ch==',' || ch == ';';
}
template <typename  Type >
inline void str2vector (const char * src,std::vector< Type > & v  ){
	std::string str(src);
    std::replace_if(str.begin(),str.end(), str2vector_check_set_space ,' ');
	std::stringstream ss(str );
	Type value;
	while( ss>>value ){
		v.push_back(value);
	}
}

inline bool xml_read_attr_string(char * dsc, uint32_t dsc_size ,xmlNodePtr  node,const char * field_name, bool is_required,const char * def_value ){
	xmlChar *szAttr = xmlGetProp (node, BAD_CAST field_name );
	if (szAttr){
		bool ret=false;
		if (strlen((const char *)szAttr )<dsc_size){
			strcpy(dsc,(const char *) szAttr );
			ret=true;
		}else{
			DEBUG_LOG("XMLERR field=[%s] len=%u max=%u",field_name, 
				strlen((const  char*  )szAttr) , dsc_size);
		}
		xmlFree(szAttr);  		/* 释放空间 */
		return ret;
	}else{
		if (is_required) {
			DEBUG_LOG("XMLERR field=[%s] is_required",field_name );
		    return false;
		}else{
			if (strlen((const char *)def_value )<dsc_size){
				strcpy(dsc,(const char *) def_value );
				return true;
			}else{
				return false;
			}
		}
	}
}
/** 
 * 
 * 
 * @param value 
 * @param node 
 * @param field_name 
 * @param is_required 是否是必填的
 * @param def_value 不填时默认值
 * @param min_value 最小值
 * @param max_value 最大值
 * 
 * @return  读取是否有效
 */
inline bool  xml_read_attr_double(double &value, xmlNodePtr node ,const char * field_name, bool is_required,double def_value,  double min_value, double max_value  )
{
	xmlChar *szAttr = xmlGetProp (node, BAD_CAST field_name );
	double read_value;
	if (szAttr){
		read_value=atof((const char *) szAttr );
		xmlFree(szAttr);  		/* 释放空间 */
		if(read_value >=min_value && read_value<=max_value ){
			value=read_value;
			return true;
		}else{
			DEBUG_LOG("XMLERR field=[%s] v=%0.0f  min= %0.0f max= %0.0f ",
					 field_name,read_value,min_value,max_value
					);
			return false;
		}
	}else{
		if (is_required) {
			DEBUG_LOG("XMLERR field=[%s] is_required",field_name );
		    return false;
		}else{
			value=def_value;
			return true;
		}
	}
}
#define  GEN_XML_READ_ATTR_INTEGER(type)  \
inline bool  xml_read_attr_##type(type##_t &value, xmlNodePtr  node ,const char * field_name, bool is_required,type##_t def_value,  type##_t min_value, type##_t max_value  ){ \
	double v;															\
	bool ret= xml_read_attr_double (v,node,field_name,is_required,def_value,min_value,max_value );\
	value=v;															\
	return ret;															\
}

GEN_XML_READ_ATTR_INTEGER(uint8)
GEN_XML_READ_ATTR_INTEGER(uint16)
GEN_XML_READ_ATTR_INTEGER(uint32)
GEN_XML_READ_ATTR_INTEGER(uint64)
GEN_XML_READ_ATTR_INTEGER(int8)
GEN_XML_READ_ATTR_INTEGER(int16)
GEN_XML_READ_ATTR_INTEGER(int32)
GEN_XML_READ_ATTR_INTEGER(int64)

inline bool message_read_from_xml_file( Cmessage & msg ,const char * file_name )
{
	xmlDocPtr     pdoc = NULL;
	xmlNodePtr    proot = NULL;
	// 打开 xml 文档
	//XML_PARSE_NOBLANKS 去除空白
	pdoc = xmlReadFile (file_name, "UTF-8", XML_PARSE_NOBLANKS);

	if (pdoc == NULL) {
		DEBUG_LOG("XML ERR :xmlReadFile  err file_name= %s",file_name);
		return false;
	}

	// 获取 xml 文档对象的根节对象
	proot = xmlDocGetRootElement (pdoc);

	if (proot == NULL) {
		DEBUG_LOG("XML ERR : xmlDocGetRootElement err file_name= %s",file_name);
		return false;
	}
	bool ret=msg.read_from_xmlnode(proot);
	/* 关闭和清理 */
	xmlFreeDoc (pdoc);
	xmlCleanupParser ();
	return ret;
	
}

#endif /* _PROTO_XML_H_ */
