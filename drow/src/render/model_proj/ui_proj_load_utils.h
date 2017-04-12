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

#define UI_R_XML_READ_ATTR_STRING_ID(__val, __attr_name)                \
    do {                                                                \
        char str_buf[128];                                              \
        UI_R_XML_READ_ATTR_STRING(str_buf, (__attr_name));              \
        (__val) = ui_string_table_builder_msg_alloc(ctx->m_string_table, str_buf); \
    } while(0)

#define UI_R_XML_READ_ATTR_TEMPLATE(__val, __type, __attr_name)     \
    do {                                                            \
        char template_name[64];                                     \
        UI_R_XML_READ_ATTR_STRING(template_name, __attr_name)       \
        if (template_name[0]) {                                     \
            mem_buffer_clear_data(gd_app_tmp_buffer(ctx->m_app));   \
            mem_buffer_printf(                                      \
                gd_app_tmp_buffer(ctx->m_app),                      \
                "Template/%s/%s",                                   \
                ui_data_proj_control_tag_name(__type),              \
                template_name);                                     \
            __val = ui_string_table_builder_msg_alloc(              \
                ctx->m_string_table,                                \
                mem_buffer_make_continuous(                         \
                    gd_app_tmp_buffer(ctx->m_app), 0));             \
        }                                                           \
    } while(0)

#define UI_R_XML_READ_ATTR_STRING_LEN(__val, __len, __attr_name)         \
    if (cpe_xml_find_attr_string(__val, __len, __attr_name, nb_attributes, attributes, ctx->m_em) == NULL) { \
        return;                                                         \
    }

#define UI_R_XML_READ_ATTR_VECTOR_2(__vec)                 \
    do {                                                    \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->value[0], "x"); \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->value[1], "y"); \
    } while(0)

#define UI_R_XML_READ_ATTR_UNIT_VECTOR_2(__vec)        \
    do {                                                \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->x.k, "xk");   \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->x.b, "xb");  \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->y.k, "yk");  \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->y.b, "yb");  \
    } while(0)

#define UI_R_XML_READ_ATTR_UNIT(__uint)        \
    do {                                                \
        UI_R_XML_READ_ATTR_FLOAT((__uint)->k, "k");   \
        UI_R_XML_READ_ATTR_FLOAT((__uint)->b, "b");  \
    } while(0)

#define UI_R_XML_READ_ATTR_UNIT_RECT(__rect)        \
    do {                                                \
        UI_R_XML_READ_ATTR_FLOAT((__rect)->lt.x.k, "ltk");   \
        UI_R_XML_READ_ATTR_FLOAT((__rect)->lt.x.b, "ltb");  \
        UI_R_XML_READ_ATTR_FLOAT((__rect)->lt.y.k, "tpk");  \
        UI_R_XML_READ_ATTR_FLOAT((__rect)->lt.y.b, "tpb");  \
        UI_R_XML_READ_ATTR_FLOAT((__rect)->rb.x.k, "rtk");   \
        UI_R_XML_READ_ATTR_FLOAT((__rect)->rb.x.b, "rtb");  \
        UI_R_XML_READ_ATTR_FLOAT((__rect)->rb.y.k, "bmk");  \
        UI_R_XML_READ_ATTR_FLOAT((__rect)->rb.y.b, "bmb");  \
    } while(0)

#define UI_R_XML_READ_ATTR_VECTOR_3(__vec)                 \
    do {                                                    \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->value[0], "x"); \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->value[1], "y"); \
        UI_R_XML_READ_ATTR_FLOAT((__vec)->value[2], "z"); \
    } while(0)

#define UI_R_XML_READ_ATTR_RECT(__rect)                        \
    do {                                                        \
        UI_R_XML_READ_ATTR_INT32((__rect)->lt, "LT");   \
        UI_R_XML_READ_ATTR_INT32((__rect)->tp, "TP");   \
        UI_R_XML_READ_ATTR_INT32((__rect)->rt, "RT");   \
        UI_R_XML_READ_ATTR_INT32((__rect)->bm, "BM");   \
    } while(0)

#define UI_R_XML_READ_ATTR_COLOR(__color)              \
    do {                                                \
        UI_R_XML_READ_ATTR_FLOAT((__color)->a, "A");   \
        UI_R_XML_READ_ATTR_FLOAT((__color)->r, "R");   \
        UI_R_XML_READ_ATTR_FLOAT((__color)->g, "G");   \
        UI_R_XML_READ_ATTR_FLOAT((__color)->b, "B");   \
    } while(0)

#define UI_R_XML_READ_ATTR_URL(__url)                                   \
    do {                                                                \
        uint8_t frame_type;                                             \
        UI_R_XML_READ_ATTR_UINT8(frame_type, "Restype");                \
        if (frame_type == 0) {                                          \
            (__url)->type = UI_OBJECT_TYPE_IMG_BLOCK;                   \
            UI_R_XML_READ_ATTR_UINT32((__url)->res_id, "Frameid");      \
            UI_R_XML_READ_ATTR_UINT32((__url)->src_id, "Resfile");      \
            if ((__url)->src_id == (uint32_t)-1) {                      \
                (__url)->type = UI_OBJECT_TYPE_NONE;                    \
            }                                                           \
        }                                                               \
        else {                                                          \
            (__url)->type = UI_OBJECT_TYPE_FRAME;                       \
            UI_R_XML_READ_ATTR_UINT32((__url)->res_id, "Frameid");      \
            UI_R_XML_READ_ATTR_UINT32((__url)->src_id, "Resfile");      \
            if ((__url)->src_id == (uint32_t)-1) {                      \
                (__url)->type = UI_OBJECT_TYPE_NONE;                    \
            }                                                           \
        }                                                               \
    } while(0)

#define UI_R_XML_READ_ATTR_URECT(__urect)              \
    do {                                                \
        UI_R_XML_READ_ATTR_FLOAT((__urect)->lt.x.k, "ltk");   \
        UI_R_XML_READ_ATTR_FLOAT((__urect)->lt.x.b, "ltb");   \
        UI_R_XML_READ_ATTR_FLOAT((__urect)->lt.y.k, "tpk");   \
        UI_R_XML_READ_ATTR_FLOAT((__urect)->lt.y.b, "tpb");   \
        UI_R_XML_READ_ATTR_FLOAT((__urect)->rb.x.k, "rtk");   \
        UI_R_XML_READ_ATTR_FLOAT((__urect)->rb.x.b, "rtb");   \
        UI_R_XML_READ_ATTR_FLOAT((__urect)->rb.y.k, "bmk");   \
        UI_R_XML_READ_ATTR_FLOAT((__urect)->rb.y.b, "bmb");   \
    } while(0)

#define UI_R_XML_READ_ATTR_FONTINFO(__fontinfo)              \
    do {                                                \
        UI_R_XML_READ_ATTR_UINT32((__fontinfo)->face, "face");   \
        UI_R_XML_READ_ATTR_INT32((__fontinfo)->size, "size");   \
        UI_R_XML_READ_ATTR_INT32((__fontinfo)->outline, "outline");   \
        UI_R_XML_READ_ATTR_UINT32((__fontinfo)->artfile, "artfile");   \
    } while(0)

#define UI_R_XML_READ_ATTR_FONTDROW(__fontdrow)              \
    do {                                                \
        UI_R_XML_READ_ATTR_UINT32((__fontdrow)->drow_flag, "DrawFlag");   \
        UI_R_XML_READ_ATTR_INT32((__fontdrow)->horz_grap, "HorzGrap");   \
        UI_R_XML_READ_ATTR_INT32((__fontdrow)->vert_grap, "VertGrap");   \
        UI_R_XML_READ_ATTR_UINT32((__fontdrow)->shadow_orien, "ShadowOrien");   \
        UI_R_XML_READ_ATTR_UINT32((__fontdrow)->shadow_width, "ShadowWidth");   \
        UI_R_XML_READ_ATTR_UINT32((__fontdrow)->stroke_width, "StrokeWidth");   \
    } while(0)

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

#define UI_R_XML_READ_VALUE_STRING_ID(__value)                          \
    do {                                                                \
        char str_buf[128];                                              \
        UI_R_XML_READ_VALUE_STRING(str_buf);                            \
        __value = ui_string_table_builder_msg_alloc(ctx->m_string_table, str_buf); \
    } while(0)

#define UI_R_XML_READ_VALUE_TEMPLATE(__type, __value)               \
    do {                                                            \
        char template_name[64];                                     \
        UI_R_XML_READ_VALUE_STRING(template_name);                  \
        if (template_name[0]) {                                     \
            mem_buffer_clear_data(gd_app_tmp_buffer(ctx->m_app));   \
            mem_buffer_printf(                                      \
                gd_app_tmp_buffer(ctx->m_app),                      \
                "Template/%s/%s",                                   \
                ui_data_proj_control_tag_name(__type),              \
                template_name);                                     \
            __value = ui_string_table_builder_msg_alloc(            \
                ctx->m_string_table,                                \
                mem_buffer_make_continuous(                         \
                    gd_app_tmp_buffer(ctx->m_app), 0));             \
        }                                                           \
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
