#include <errno.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/file.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils_xml/xml_utils.h"

int cpe_xml_find_attr_int32(int32_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    int index, indexAttribute;
    
    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];
        const char *valueBegin;
        const char *valueEnd;
        char buf[64];
        size_t len;
        
        if (strcmp(localname, attr_name) != 0) continue;

        valueBegin = ((const char * *)attributes)[index+3];
        valueEnd = ((const char * *)attributes)[index+4];
        len = valueEnd - valueBegin;
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;
        memcpy(buf, valueBegin, len);
        buf[len] = 0;

        if (sscanf(buf, FMT_INT32_T, result) != 1) {
            CPE_ERROR(em, "read attr %s from value %s fail!", attr_name, buf);
            return -1; 
        }

        return 0;
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return -1;
}

int cpe_xml_find_attr_uint32(uint32_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    int index, indexAttribute;
    
    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];
        const char *valueBegin;
        const char *valueEnd;
        char buf[64];
        size_t len;
        
        if (strcmp(localname, attr_name) != 0) continue;

        valueBegin = ((const char * *)attributes)[index+3];
        valueEnd = ((const char * *)attributes)[index+4];
        len = valueEnd - valueBegin;
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;
        memcpy(buf, valueBegin, len);
        buf[len] = 0;

        if (sscanf(buf, FMT_UINT32_T, result) != 1) {
            CPE_ERROR(em, "read attr %s from value %s fail!", attr_name, buf);
            return -1; 
        }

        return 0;
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return -1;
}

int cpe_xml_find_attr_int16(int16_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    int32_t tmp;
    if (cpe_xml_find_attr_int32(&tmp, attr_name, nb_attributes, attributes, em) != 0) return -1;

    *result = tmp;
    
    return 0;
}

int cpe_xml_find_attr_uint16(uint16_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    uint32_t tmp;
    if (cpe_xml_find_attr_uint32(&tmp, attr_name, nb_attributes, attributes, em) != 0) return -1;

    *result = tmp;
    
    return 0;
}

int cpe_xml_find_attr_int8(int8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    int32_t tmp;
    if (cpe_xml_find_attr_int32(&tmp, attr_name, nb_attributes, attributes, em) != 0) return -1;

    *result = tmp;
    
    return 0;
}

int cpe_xml_find_attr_uint8(uint8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    uint32_t tmp;
    if (cpe_xml_find_attr_uint32(&tmp, attr_name, nb_attributes, attributes, em) != 0) return -1;

    *result = tmp;
    
    return 0;
}

int cpe_xml_find_attr_float(float * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    int index, indexAttribute;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];
        const char *valueBegin;
        const char *valueEnd;
        char * endptr;

        if (strcmp(localname, attr_name) != 0) continue;

        valueBegin = ((const char * *)attributes)[index+3];
        valueEnd = ((const char * *)attributes)[index+4];

        *result = strtof(((const char * *)attributes)[index+3], &endptr);
        if (endptr != valueEnd) {
            char buf[64];
            size_t len = valueEnd - valueBegin;
            if (len >= sizeof(buf)) len = sizeof(buf) - 1;
            memcpy(buf, valueBegin, len);
            buf[len] = 0;
            CPE_ERROR(em, "read attr %s from value %s fail!", attr_name, buf);
            return -1; 
        }

        return 0;
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return -1;
}

int cpe_xml_find_attr_bool(uint8_t * result, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em) {
    char _f[] = "False";
    char _t[] = "True";
    int index, indexAttribute;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];
        const char *valueBegin;
        const char *valueEnd;
        size_t data_len;

        if (strcmp(localname, attr_name) != 0) continue;

        valueBegin = ((const char * *)attributes)[index+3];
        valueEnd = ((const char * *)attributes)[index+4];
        data_len = valueEnd - valueBegin;

        if (data_len + 1 == sizeof(_f) && memcmp(valueBegin, _f, data_len) == 0) {
            *result = 0;
            return 0;
        }
        else if (data_len + 1 == sizeof(_t) && memcmp(valueBegin, _t, data_len) == 0) {
            *result = 1;
            return 0;
        }
        else {
            char buf[64];
            if (data_len >= sizeof(buf)) data_len = sizeof(buf) - 1;
            memcpy(buf, valueBegin, data_len);
            buf[data_len] = 0;
            CPE_ERROR(em, "read attr %s from value %s fail!", attr_name, buf);
            return -1; 
        }
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return -1;
}

int cpe_xml_read_value_bool(uint8_t * result, const char * data, size_t data_len) {
    char _f[] = "False";
    char _t[] = "True";

    if (data_len + 1 == sizeof(_f) && memcmp(data, _f, data_len) == 0) {
        *result = 0;
        return 0;
    }
    else if (data_len + 1 == sizeof(_t) && memcmp(data, _t, data_len) == 0) {
        *result = 1;
        return 0;
    }
    else {
        return -1;
    }
}

int cpe_xml_read_value_int32(int32_t * result, const char * data, size_t data_len) {
    if (sscanf(data, FMT_INT32_T, result) != 1) {
        return -1;
    }
    return 0;
}

int cpe_xml_read_value_uint32(uint32_t * result, const char * data, size_t data_len) {
    if (sscanf(data, FMT_UINT32_T, result) != 1) {
        return -1;
    }
    return 0;
}

int cpe_xml_read_value_int16(int16_t * result, const char * data, size_t data_len) {
	int32_t tmp;
	if (cpe_xml_read_value_int32(&tmp, data, data_len) != 0) return -1;

	*result = tmp;

	return 0;
}

int cpe_xml_read_value_uint16(uint16_t * result, const char * data, size_t data_len) {
	uint32_t tmp;
	if (cpe_xml_read_value_uint32(&tmp, data, data_len) != 0) return -1;

	*result = tmp;

	return 0;
}

int cpe_xml_read_value_int8(int8_t * result, const char * data, size_t data_len) {
    int32_t tmp;
    if (cpe_xml_read_value_int32(&tmp, data, data_len) != 0) return -1;

    *result = tmp;
    
    return 0;
}

int cpe_xml_read_value_uint8(uint8_t * result, const char * data, size_t data_len) {
    uint32_t tmp;
    if (cpe_xml_read_value_uint32(&tmp, data, data_len) != 0) return -1;

    *result = tmp;
    
    return 0;
}

int cpe_xml_read_value_float(float * result, const char * data, size_t data_len) {
    char * endptr;
    *result = strtof(data, &endptr);
    if (endptr - data != data_len) return -1;
    return 0;
}

const char * cpe_xml_find_attr_string(
    char * buff, size_t capacity, const char * attr_name, int nb_attributes, const void * attributes, error_monitor_t em)
{
    int index, indexAttribute;

    for(index = 0, indexAttribute = 0;
        indexAttribute < nb_attributes;
        ++indexAttribute, index += 5)
    {
        const char *localname = ((const char * *)attributes)[index];

        if (strcmp(localname, attr_name) == 0) {
            const char *valueBegin = ((const char * *)attributes)[index+3];
            const char *valueEnd = ((const char * *)attributes)[index+4];
            size_t len = valueEnd - valueBegin;

            if ((len + 1) > capacity) len = capacity - 1;

            memcpy(buff, valueBegin, len);
            buff[len] = 0;
            return buff;
        }
    }

    CPE_ERROR(em, "read attr %s not exist!", attr_name);
    return NULL;
}
