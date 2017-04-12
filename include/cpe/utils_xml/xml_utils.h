#ifndef CPE_UTILS_XML_H
#define CPE_UTILS_XML_H
#include "cpe/pal/pal_types.h"
#include "cpe/utils/error.h"

#ifdef __cplusplus
extern "C" {
#endif

int cpe_xml_find_attr_int32(int32_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_uint32(uint32_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_int16(int16_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_uint16(uint16_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_int8(int8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_uint8(uint8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_bool(uint8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
int cpe_xml_find_attr_float(float * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);
const char * cpe_xml_find_attr_string(
    char * buff, size_t capacity, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em);

int cpe_xml_read_value_bool(uint8_t * result, const char * data, size_t data_len);
int cpe_xml_read_value_int32(int32_t * result, const char * data, size_t data_len);
int cpe_xml_read_value_uint32(uint32_t * result, const char * data, size_t data_len);    
int cpe_xml_read_value_int16(int16_t * result, const char * data, size_t data_len);
int cpe_xml_read_value_uint16(uint16_t * result, const char * data, size_t data_len);    
int cpe_xml_read_value_int8(int8_t * result, const char * data, size_t data_len);
int cpe_xml_read_value_uint8(uint8_t * result, const char * data, size_t data_len);    
int cpe_xml_read_value_float(float * result, const char * data, size_t data_len);

#ifdef __cplusplus
}
#endif

#endif
