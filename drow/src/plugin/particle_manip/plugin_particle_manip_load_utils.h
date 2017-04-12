#ifndef UI_R_LOAD_UTILS_H
#define UI_R_LOAD_UTILS_H
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils_xml/xml_utils.h"
#include "render/model/ui_model_types.h"

#define UI_R_XML_READ_ATTR_UINT32(__val, __attr_name)                   \
    if (cpe_xml_find_attr_uint32(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_ATTR_INT32(__val, __attr_name)                   \
    if (cpe_xml_find_attr_int32(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_ATTR_UINT16(__val, __attr_name)                   \
    if (cpe_xml_find_attr_uint16(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_ATTR_INT16(__val, __attr_name)                   \
    if (cpe_xml_find_attr_int16(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_ATTR_UINT8(__val, __attr_name)                   \
    if (cpe_xml_find_attr_uint8(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_ATTR_INT8(__val, __attr_name)                   \
    if (cpe_xml_find_attr_int8(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_ATTR_FLOAT(__val, __attr_name)                   \
    if (cpe_xml_find_attr_float(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_ATTR_BOOL(__val, __attr_name)                   \
    if (cpe_xml_find_attr_bool(&(__val), __attr_name, nb_attributes, attributes, ctx->m_em) != 0) { \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_ATTR_STRING(__val, __attr_name)                  \
    if (cpe_xml_find_attr_string(__val, sizeof(__val), __attr_name, nb_attributes, attributes, ctx->m_em) == NULL) { \
        return;                                                         \
    }

#define UI_R_XML_TRY_READ_ATTR_STRING(__val, __attr_name)                  \
    if (cpe_xml_find_attr_string(__val, sizeof(__val), __attr_name, nb_attributes, attributes, NULL) == NULL) { \
        return;                                                         \
    }

#define UI_R_XML_READ_ATTR_STRING_ID(__val, __attr_name)                \
    do {                                                                \
        char str_buf[128];                                              \
        UI_R_XML_READ_ATTR_STRING(str_buf, (__attr_name));              \
        (__val) = ui_string_table_builder_msg_alloc(ctx->m_string_table, str_buf); \
    } while(0)

#define UI_R_XML_TRY_READ_ATTR_STRING_ID(__val, __attr_name)            \
    do {                                                                \
        char str_buf[128];                                              \
        UI_R_XML_TRY_READ_ATTR_STRING(str_buf, (__attr_name));          \
        (__val) = ui_string_table_builder_msg_alloc(ctx->m_string_table, str_buf); \
    } while(0)

#define UI_R_XML_READ_ATTR_STRING_LEN(__val, __len, __attr_name)         \
    if (cpe_xml_find_attr_string(__val, __len, __attr_name, nb_attributes, attributes, ctx->m_em) == NULL) { \
        return;                                                         \
    }

#define UI_R_XML_READ_VALUE_BOOL(__value)                              \
    if (cpe_xml_read_value_bool(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read bool from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }

#define UI_R_XML_READ_VALUE_FLOAT(__value)                              \
    if (cpe_xml_read_value_float(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read float from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }

#define UI_R_XML_READ_VALUE_STRING(__value)                            \
        if (len + 1 <= sizeof(__value)) {                               \
            memcpy(__value, ch, len);                                   \
            __value[len] = 0;                                           \
        }                                                               \
        else {                                                          \
            char buf[64];                                               \
            if (len >= sizeof(buf)) len = sizeof(buf) - 1;              \
            memcpy(buf, ch, len);                                       \
            buf[len] = 0;                                               \
            CPE_ERROR(ctx->m_em, "%s: read str from %s fail, overflow!",\
                      ctx->m_cur_tag_name, buf);                        \
            return;                                                     \
        }

#define UI_R_XML_READ_VALUE_STRING_ID(__val)                            \
    do {                                                                \
        char str_buf[128];                                              \
        UI_R_XML_READ_VALUE_STRING(str_buf);                            \
        (__val) = ui_string_table_builder_msg_alloc(ctx->m_string_table, str_buf); \
    } while(0)

#define UI_R_XML_READ_VALUE_INT32(__value)                      \
    if (cpe_xml_read_value_int32(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read long from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_VALUE_UINT32(__value)                      \
    if (cpe_xml_read_value_uint32(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read long from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_VALUE_INT16(__value)                      \
    if (cpe_xml_read_value_int16(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read long from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_VALUE_UINT16(__value)                      \
    if (cpe_xml_read_value_uint16(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read long from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_VALUE_INT8(__value)                      \
    if (cpe_xml_read_value_int8(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read long from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }                                                                   \

#define UI_R_XML_READ_VALUE_UINT8(__value)                      \
    if (cpe_xml_read_value_uint8(&__value, (const char *)ch, (size_t)len) != 0) { \
        char buf[64];                                                   \
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;                  \
        memcpy(buf, ch, len);                                           \
        buf[len] = 0;                                                   \
        CPE_ERROR(ctx->m_em, "%s: read long from %s fail!", ctx->m_cur_tag_name, buf); \
        return;                                                         \
    }                                                                   \

#endif
