#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_types.h"
#include "cfg_internal_ops.h"

static cfg_t cfg_check_or_create_sub(cfg_t cfg, int type, const char * next_cfg_str) {
    if(cfg->m_type == CPE_CFG_TYPE_STRUCT) {
        if (type == CPE_CFG_TYPE_STRUCT) {
            return cfg_struct_add_struct(cfg, next_cfg_str, cfg_merge_use_exist);
        }
        else if (type == CPE_CFG_TYPE_SEQUENCE) {
            return cfg_struct_add_seq(cfg, next_cfg_str, cfg_merge_use_exist);
        }
        else {
            return NULL;
        }
    }
    else if (cfg->m_type == CPE_CFG_TYPE_SEQUENCE) {
        int pos;
        pos = atoi(next_cfg_str);

        if (next_cfg_str[0] == 0 || pos == cfg_seq_count(cfg)) {
            if (type == CPE_CFG_TYPE_STRUCT) {
                return cfg_seq_add_struct(cfg);
            }
            else if (type == CPE_CFG_TYPE_SEQUENCE) {
                return cfg_seq_add_seq(cfg);
            }
            else {
                return NULL;
            }
        }
        else if (pos < cfg_seq_count(cfg)) {
            return cfg_seq_at(cfg, pos);
        }
        else {
            return NULL;
        }
    }
    else {
        return NULL;
    }
}

cfg_t cfg_check_or_create(cfg_t cfg, const char * path, error_monitor_t em, char * buf, size_t buf_capacity) {
    const char * root = path;
    const char * end = path + strlen(path);
    const char * nextSeqTag = strchr(path, '[');
    const char * nextNameTag = strchr(path, '.');
    const char * next_cfg_str;

    if (nextSeqTag == NULL) nextSeqTag = end;
    if (nextNameTag == NULL) nextNameTag = end;

    next_cfg_str = NULL;

    while(cfg && path < end) {
        if (path[0] == '[') {
			const char * seqEndTag;
            seqEndTag = strchr(nextSeqTag, ']');
            if (seqEndTag == NULL) {
                CPE_ERROR(em, "cfg_check_or_create: path=%s, pos=%d: seq tag not closed", root, (int)(nextSeqTag - root));
                return NULL;
            }

            if (next_cfg_str) {
                cfg = cfg_check_or_create_sub(cfg, CPE_CFG_TYPE_SEQUENCE, next_cfg_str);
                if (cfg == NULL) break;
            }

            next_cfg_str = cpe_str_dup_range(buf, buf_capacity, path + 1, seqEndTag);
            if (next_cfg_str == NULL) {
                CPE_ERROR(em, "cfg_check_or_create: path=%s, pos=%d: name too long, capacity=%d", root, (int)(nextSeqTag - root), (int)buf_capacity);
                return NULL;
            }

            path = seqEndTag + 1;
            nextSeqTag = strchr(path, '[');
            if (nextSeqTag == NULL) nextSeqTag = end;

            if (*path == '.') {
                path += 1;
                nextNameTag = strchr(path, '.');
                if (nextNameTag == NULL) nextNameTag = end;
            }
        }
        else {
            const char * next_cfg_str_begin;

            if (next_cfg_str) {
                cfg = cfg_check_or_create_sub(cfg, CPE_CFG_TYPE_STRUCT, next_cfg_str);
                if (cfg == NULL) break;
            }

            next_cfg_str_begin = path;

            if (nextSeqTag < nextNameTag) {
                path = nextSeqTag;
                nextSeqTag = strchr(nextSeqTag, '[');
                if (nextSeqTag == NULL) nextSeqTag = end;
            }
            else if (nextNameTag < nextSeqTag) {
                path = nextNameTag + 1;
                nextNameTag = strchr(path, '.');
                if (nextNameTag == NULL) nextNameTag = end;
            }
            else {
                path = end;
            }

            if (*(path - 1) == '.') {
                next_cfg_str = cpe_str_dup_range(buf, buf_capacity, next_cfg_str_begin, path - 1);
            }
            else {
                next_cfg_str = cpe_str_dup_range(buf, buf_capacity, next_cfg_str_begin, path);
            }

            if (next_cfg_str == NULL) {
                CPE_ERROR(em, "cfg_check_or_create: path=%s, pos=%d: name too long, capacity=%d", root, (int)(path - root), (int)buf_capacity);
                return NULL;
            }
        }
    }

    if (next_cfg_str == NULL) {
        CPE_ERROR(em, "cfg_check_or_create: path=%s, pos=%d: no any path", root, (int)(path - root));
        return NULL;
    }
    else {
        return cfg;
    }
}

cfg_t cfg_add_struct(cfg_t c, const char * path, error_monitor_t em) {
    char buf[256];
    cfg_t last_cfg;

    last_cfg = cfg_check_or_create(c, path, em, buf, sizeof(buf));
    if (last_cfg == NULL) return NULL;

    return cfg_check_or_create_sub(last_cfg, CPE_CFG_TYPE_STRUCT, buf);
}

cfg_t cfg_add_seq(cfg_t c, const char * path, error_monitor_t em) {
    char buf[256];
    cfg_t last_cfg;

    last_cfg = cfg_check_or_create(c, path, em, buf, sizeof(buf));
    if (last_cfg == NULL) return NULL;

    return cfg_check_or_create_sub(last_cfg, CPE_CFG_TYPE_SEQUENCE, buf);
}

#define CFG_DEF_ADD_VALUE_FUN(__str_type, __type)                       \
    cfg_t cfg_add_ ## __str_type(cfg_t c, const char * path, __type value, error_monitor_t em) { \
        char buf[256];                                                  \
        cfg_t last_cfg;                                                 \
                                                                        \
        last_cfg = cfg_check_or_create(c, path, em, buf, sizeof(buf));  \
        if (last_cfg == NULL) return NULL;                              \
                                                                        \
        if (last_cfg->m_type == CPE_CFG_TYPE_STRUCT) {                  \
            return cfg_struct_add_ ## __str_type(last_cfg, buf, value, cfg_replace); \
        }                                                               \
        else {                                                          \
            int pos;                                                    \
            int seq_count;                                              \
                                                                        \
            pos = atoi(buf);                                            \
            seq_count = cfg_seq_count(last_cfg);                        \
                                                                        \
            if (buf[0] == 0 || pos == seq_count) {                      \
                return cfg_seq_add_ ## __str_type(last_cfg, value);     \
            }                                                           \
            else if (pos < seq_count) {                                 \
                return cfg_seq_add_ ## __str_type(last_cfg, value);     \
            }                                                           \
            else {                                                      \
                return NULL;                                            \
            }                                                           \
        }                                                               \
    }

CFG_DEF_ADD_VALUE_FUN(string, const char *)
CFG_DEF_ADD_VALUE_FUN(int8, int8_t)
CFG_DEF_ADD_VALUE_FUN(uint8, uint8_t)
CFG_DEF_ADD_VALUE_FUN(int16, int16_t)
CFG_DEF_ADD_VALUE_FUN(uint16, uint16_t)
CFG_DEF_ADD_VALUE_FUN(int32, int32_t)
CFG_DEF_ADD_VALUE_FUN(uint32, uint32_t)
CFG_DEF_ADD_VALUE_FUN(int64, int64_t)
CFG_DEF_ADD_VALUE_FUN(uint64, uint64_t)
CFG_DEF_ADD_VALUE_FUN(float, float)
CFG_DEF_ADD_VALUE_FUN(double, double)
CFG_DEF_ADD_VALUE_FUN(value_from_string_auto, const char *)

cfg_t cfg_add_value_from_string(cfg_t c, const char * path, int typeId, const char * value, error_monitor_t em) {
    char buf[256];
    cfg_t last_cfg;

    last_cfg = cfg_check_or_create(c, path, em, buf, sizeof(buf));
    if (last_cfg == NULL) return NULL;

    if (last_cfg->m_type == CPE_CFG_TYPE_STRUCT) {
        return cfg_struct_add_value_from_string(last_cfg, buf, typeId, value, cfg_replace);
    }
    else {
        int pos;
        int seq_count;

        pos = atoi(buf);
        seq_count = cfg_seq_count(last_cfg);

        if (buf[0] == 0 || pos == seq_count) {
            return cfg_seq_add_value_from_string(last_cfg, typeId, value);
        }
        else if (pos < seq_count) {
            return cfg_seq_add_value_from_string(last_cfg, typeId, value);
        }
        else {
            return NULL;
        }
    }
}

cfg_t cfg_add_value_from_binary(cfg_t c, const char * path, int typeId, const void * value, error_monitor_t em) {
    char buf[256];
    cfg_t last_cfg;

    last_cfg = cfg_check_or_create(c, path, em, buf, sizeof(buf));
    if (last_cfg == NULL) return NULL;

    if (last_cfg->m_type == CPE_CFG_TYPE_STRUCT) {
        return cfg_struct_add_value_from_binary(last_cfg, buf, typeId, value, cfg_replace);
    }
    else {
        int pos;
        int seq_count;

        pos = atoi(buf);
        seq_count = cfg_seq_count(last_cfg);

        if (buf[0] == 0 || pos == seq_count) {
            return cfg_seq_add_value_from_binary(last_cfg, typeId, value);
        }
        else if (pos < seq_count) {
            return cfg_seq_add_value_from_binary(last_cfg, typeId, value);
        }
        else {
            return NULL;
        }
    }
}
