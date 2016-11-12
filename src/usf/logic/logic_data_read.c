#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "usf/logic/logic_data.h"
#include "logic_internal_ops.h"

#define LOGIC_DATA_DEF_DATA_NO_TRY_FUN(__to, __to_type)                 \
    __to_type logic_context_read_ ## __to(                              \
        logic_context_t context,                                        \
        const char * path)                                              \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
                                                                        \
        if (path == NULL) return (__to_type)0;                          \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) return (__to_type)0;                           \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) return (__to_type)0;         \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_context_data_find(context, name);                  \
        if (data == NULL) return (__to_type)0;                          \
                                                                        \
        return dr_meta_read_ ## __to(logic_data_data(data), data->m_meta, sub_path); \
    }                                                                   \
    __to_type logic_context_read_with_dft_ ## __to(                     \
        logic_context_t context,                                        \
        const char * path,                                              \
        __to_type dft)                                                  \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
                                                                        \
        if (path == NULL) return dft;                                   \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) return dft;                                    \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) return dft;                  \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_context_data_find(context, name);                  \
        if (data == NULL) return dft;                                   \
                                                                        \
        return dr_meta_read_with_dft_ ## __to(logic_data_data(data), data->m_meta, sub_path, dft); \
    }\
    __to_type logic_require_read_ ## __to(                              \
        logic_require_t require,                                        \
        const char * path)                                              \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
                                                                        \
        if (path == NULL) return (__to_type)0;                          \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) return (__to_type)0;                           \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) return (__to_type)0;         \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_require_data_find(require, name);                  \
        if (data == NULL) return (__to_type)0;                          \
                                                                        \
        return dr_meta_read_ ## __to(logic_data_data(data), data->m_meta, sub_path); \
    }                                                                   \
    __to_type logic_require_read_with_dft_ ## __to(                     \
        logic_require_t require,                                        \
        const char * path,                                              \
        __to_type dft)                                                  \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
                                                                        \
        if (path == NULL) return dft;                                   \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) return dft;                                    \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) return dft;                  \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_require_data_find(require, name);                  \
        if (data == NULL) return dft;                                   \
                                                                        \
        return dr_meta_read_with_dft_ ## __to(logic_data_data(data), data->m_meta, sub_path, dft); \
    }                                                                   \
    __to_type logic_stack_read_ ## __to(                              \
        logic_stack_node_t stack,                                        \
        const char * path)                                              \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
                                                                        \
        if (path == NULL) return (__to_type)0;                          \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) return (__to_type)0;                           \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) return (__to_type)0;         \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_stack_data_find(stack, name);                  \
        if (data == NULL) return (__to_type)0;                          \
                                                                        \
        return dr_meta_read_ ## __to(logic_data_data(data), data->m_meta, sub_path); \
    }                                                                   \
    __to_type logic_stack_read_with_dft_ ## __to(                     \
        logic_stack_node_t stack,                                        \
        const char * path,                                              \
        __to_type dft)                                                  \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
                                                                        \
        if (path == NULL) return dft;                                   \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) return dft;                                    \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) return dft;                  \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_stack_data_find(stack, name);                      \
        if (data == NULL) return dft;                                   \
                                                                        \
        return dr_meta_read_with_dft_ ## __to(logic_data_data(data), data->m_meta, sub_path, dft); \
    }

#define LOGIC_DATA_DEF_DATA_FUN(__to, __to_type)                        \
    int logic_require_try_read_ ## __to(                                \
        __to_type * result,                                             \
        logic_require_t require,                                        \
        const char * path, error_monitor_t em)                          \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
        int r;                                                          \
                                                                        \
        if (path == NULL) {                                             \
            CPE_ERROR(em, "logic_data_read_data: path is null!");       \
            return -1;                                                  \
        }                                                               \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) {                                              \
            CPE_ERROR(em, "logic_data_read_data: can`t find first name from path(%s)!", path); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) {                            \
            CPE_ERROR(em, "logic_data_read_data: first name too long, path(%s)!", path); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_require_data_find(require, name);                  \
        if (data == NULL) {                                             \
            CPE_ERROR(em, "logic_data_read_data: data %s not exist!", name); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        r = dr_meta_try_read_ ## __to(result, logic_data_data(data), data->m_meta, sub_path, em); \
        if (r != 0) {                                                   \
            CPE_ERROR(em, "logic_data_read_data: read %s from %s fail!", sub_path, name); \
        }                                                               \
        return r;                                                       \
    }                                                                   \
    int logic_stack_try_read_ ## __to(                                \
        __to_type * result,                                             \
        logic_stack_node_t stack,                                        \
        const char * path, error_monitor_t em)                          \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
        int r;                                                          \
                                                                        \
        if (path == NULL) {                                             \
            CPE_ERROR(em, "logic_data_read_data: path is null!");       \
            return -1;                                                  \
        }                                                               \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) {                                              \
            CPE_ERROR(em, "logic_data_read_data: can`t find first name from path(%s)!", path); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) {                            \
            CPE_ERROR(em, "logic_data_read_data: first name too long, path(%s)!", path); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_stack_data_find(stack, name);                  \
        if (data == NULL) {                                             \
            CPE_ERROR(em, "logic_data_read_data: data %s not exist!", name); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        r = dr_meta_try_read_ ## __to(result, logic_data_data(data), data->m_meta, sub_path, em); \
        if (r != 0) {                                                   \
            CPE_ERROR(em, "logic_data_read_data: read %s from %s fail!", sub_path, name); \
        }                                                               \
        return r;                                                       \
    }                                                                   \
    int logic_context_try_read_ ## __to(                                \
        __to_type * result,                                             \
        logic_context_t context,                                        \
        const char * path, error_monitor_t em)                          \
    {                                                                   \
        char name[128];                                                 \
        const char * sep;                                               \
        const char * sub_path;                                          \
        size_t name_len;                                                \
        logic_data_t data;                                              \
        int r;                                                          \
                                                                        \
        if (path == NULL) {                                             \
            CPE_ERROR(em, "logic_data_read_data: path is null!");       \
            return -1;                                                  \
        }                                                               \
                                                                        \
        sep = strchr(path, '.');                                        \
        if (sep == NULL) {                                              \
            CPE_ERROR(em, "logic_data_read_data: can`t find first name from path(%s)!", path); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        name_len = sep - path;                                          \
        if ((name_len + 1) > sizeof(name)) {                            \
            CPE_ERROR(em, "logic_data_read_data: first name too long, path(%s)!", path); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        memcpy(name, path, name_len);                                   \
        name[name_len] = 0;                                             \
                                                                        \
        sub_path = sep + 1;                                             \
                                                                        \
        data = logic_context_data_find(context, name);                  \
        if (data == NULL) {                                             \
            CPE_ERROR(em, "logic_data_read_data: data %s not exist!", name); \
            return -1;                                                  \
        }                                                               \
                                                                        \
        r = dr_meta_try_read_ ## __to(result, logic_data_data(data), data->m_meta, sub_path, em); \
        if (r != 0) {                                                   \
            CPE_ERROR(em, "logic_data_read_data: read %s from %s fail!", sub_path, name); \
        }                                                               \
        return r;                                                       \
    }                                                                   \
    LOGIC_DATA_DEF_DATA_NO_TRY_FUN(__to, __to_type)                     \

LOGIC_DATA_DEF_DATA_FUN(int8, int8_t);
LOGIC_DATA_DEF_DATA_FUN(uint8, uint8_t);
LOGIC_DATA_DEF_DATA_FUN(int16, int16_t);
LOGIC_DATA_DEF_DATA_FUN(uint16, uint16_t);
LOGIC_DATA_DEF_DATA_FUN(int32, int32_t);
LOGIC_DATA_DEF_DATA_FUN(uint32, uint32_t);
LOGIC_DATA_DEF_DATA_FUN(int64, int64_t);
LOGIC_DATA_DEF_DATA_FUN(uint64, uint64_t);
LOGIC_DATA_DEF_DATA_FUN(float, float);
LOGIC_DATA_DEF_DATA_FUN(double, double);
LOGIC_DATA_DEF_DATA_NO_TRY_FUN(string, const char *);
