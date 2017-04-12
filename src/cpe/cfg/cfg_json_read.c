#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "yajl/yajl_parse.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cfg_internal_ops.h"

enum cfg_json_parse_state {
    cfg_json_parse_in_map,
    cfg_json_parse_in_seq
};

struct cfg_json_read_ctx {
    cfg_t m_curent;
    enum cfg_json_parse_state m_state;

    struct {
        cfg_t m_node;
        enum cfg_json_parse_state m_state;
    } m_node_stack[CPE_CFG_MAX_LEVEL];
    int m_node_stack_pos;

    const char * m_name;
    struct mem_buffer m_name_buffer;

    cfg_policy_t m_policy;
    error_monitor_t m_em;
};

static int cfg_json_read_ctx_init(struct cfg_json_read_ctx * ctx, cfg_t root, const char * name, cfg_policy_t policy, error_monitor_t em);
static void cfg_json_read_ctx_fini(struct cfg_json_read_ctx * ctx);

static int cfg_json_on_null(void * i_ctx) {
    struct cfg_json_read_ctx * ctx = i_ctx;
    
    if (ctx->m_curent == NULL) return 1;

    if(ctx->m_state == cfg_json_parse_in_map) {
        cfg_t oldValue = cfg_struct_find_cfg(ctx->m_curent, ctx->m_name);
        if (oldValue
            && (ctx->m_policy == cfg_replace || ctx->m_policy == cfg_merge_use_new))
        {
            cfg_struct_item_delete((struct cfg_struct *)ctx->m_curent, oldValue);
        }
    }
    
    return 1;
}

static int cfg_json_on_boolean(void * i_ctx, int boolean) {
    struct cfg_json_read_ctx * ctx = i_ctx;

    if (ctx->m_curent == NULL) return 1;

    if(ctx->m_state == cfg_json_parse_in_map) {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return 1;
        if (ctx->m_name == NULL) return 1;

        cfg_struct_add_uint8(ctx->m_curent, ctx->m_name, boolean ? 1 : 0, ctx->m_policy);
        ctx->m_name = NULL;
    }
    else {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return 1;
        cfg_seq_add_uint8(ctx->m_curent, boolean ? 1 : 0);
    }

    return 1;
}

static int cfg_json_on_integer(void * i_ctx, long long integerVal) {
    struct cfg_json_read_ctx * ctx = i_ctx;

    if (ctx->m_curent == NULL) return 1;

    if(ctx->m_state == cfg_json_parse_in_map) {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return 1;
        if (ctx->m_name == NULL) return 1;

        cfg_struct_add_int64(ctx->m_curent, ctx->m_name, (int64_t)integerVal, ctx->m_policy);
        ctx->m_name = NULL;
    }
    else {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return 1;
        cfg_seq_add_int64(ctx->m_curent, (int64_t)integerVal);
    }

    return 1;
}

static int cfg_json_on_double(void * i_ctx, double doubleVal) {
    struct cfg_json_read_ctx * ctx = i_ctx;

    if (ctx->m_curent == NULL) return 1;

    if(ctx->m_state == cfg_json_parse_in_map) {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return 1;
        if (ctx->m_name == NULL) return 1;

        cfg_struct_add_double(ctx->m_curent, ctx->m_name, doubleVal, ctx->m_policy);
        ctx->m_name = NULL;
    }
    else {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return 1;
        cfg_seq_add_double(ctx->m_curent, doubleVal);
    }

    return 1;
}

static int cfg_json_on_string(void * i_ctx, const unsigned char * stringVal, size_t stringLen) {
    struct cfg_json_read_ctx * ctx = i_ctx;
    const char * s;
    
    if (ctx->m_curent == NULL) return 1;

    s = mem_buffer_strdup_len(&ctx->m_name_buffer, (const char *)stringVal, stringLen);
    
    if(ctx->m_state == cfg_json_parse_in_map) {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return 1;
        if (ctx->m_name == NULL) return 1;

        cfg_struct_add_string(ctx->m_curent, ctx->m_name, s, ctx->m_policy);
        ctx->m_name = NULL;
    }
    else {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return 1;
        cfg_seq_add_string(ctx->m_curent, s);
    }

    return 1;
}

static int cfg_json_on_sequence_begin(void * i_ctx) {
    struct cfg_json_read_ctx * ctx = i_ctx;
    enum cfg_json_parse_state old_state = ctx->m_state;
    ctx->m_state = cfg_json_parse_in_seq;
    ++ctx->m_node_stack_pos;
    if (ctx->m_node_stack_pos >= CPE_CFG_MAX_LEVEL) {
        ctx->m_curent = NULL;
        return 1;
    }
    ctx->m_node_stack[ctx->m_node_stack_pos].m_node = ctx->m_curent;
    ctx->m_node_stack[ctx->m_node_stack_pos].m_state = old_state;

    if (ctx->m_curent == NULL) return 1;

    if (old_state == cfg_json_parse_in_map) {
        const char * name = NULL;

        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return 1;

        if (ctx->m_name != NULL) {
            name = ctx->m_name;
            ctx->m_name = NULL;
        }
        else if (ctx->m_node_stack_pos == 0) {
            name = "";
        }

        if (name) {
            ctx->m_curent = cfg_struct_add_seq(ctx->m_curent, name, ctx->m_policy);
        }
        else {
            CPE_ERROR(ctx->m_em, "no name for new seq!");
            ctx->m_curent = NULL;
        }
    }
    else {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return 1;

        if (ctx->m_node_stack_pos == 0) {
            //DO NOTHING
        }
        else {
            ctx->m_curent = cfg_seq_add_seq(ctx->m_curent);
        }
    }

    return 1;
}

static int cfg_json_on_map_begin(void * i_ctx) {
    struct cfg_json_read_ctx * ctx = i_ctx;
    enum cfg_json_parse_state old_state = ctx->m_state;
    ctx->m_state = cfg_json_parse_in_map;
    ++ctx->m_node_stack_pos;
    if (ctx->m_node_stack_pos >= CPE_CFG_MAX_LEVEL) {
        ctx->m_curent = NULL;
        return 1;
    }
    ctx->m_node_stack[ctx->m_node_stack_pos].m_node = ctx->m_curent;
    ctx->m_node_stack[ctx->m_node_stack_pos].m_state = old_state;

    if (ctx->m_curent == NULL) return 1;

    if (old_state == cfg_json_parse_in_map) {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return 1;

        if (ctx->m_name != NULL) {
            cfg_t next = cfg_struct_add_struct(ctx->m_curent, ctx->m_name, ctx->m_policy);
            if (next == NULL) {
                ctx->m_curent = cfg_struct_find_cfg(ctx->m_curent, ctx->m_name);
            }
            else {
                ctx->m_curent = next;
            }

            ctx->m_name = NULL;
        }
        else if (ctx->m_node_stack_pos == 0) {
            //DO NOTHING
        }
        else {
            CPE_ERROR(ctx->m_em, "no name for new map!");
            ctx->m_curent = NULL;
        }
    }
    else {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return 1;

        ctx->m_curent = cfg_seq_add_struct(ctx->m_curent);
    }

    return 1;
}

static int cfg_json_on_map_key(void * i_ctx, const unsigned char * stringVal, size_t stringLen) {
    struct cfg_json_read_ctx * ctx = i_ctx;

    mem_buffer_clear_data(&ctx->m_name_buffer);
    ctx->m_name = mem_buffer_strdup_len(&ctx->m_name_buffer, (const char *)stringVal, stringLen);
    if (ctx->m_name == NULL) {
        CPE_ERROR(ctx->m_em, "dump scalar as name, no memory!");
        ctx->m_name = "";
    }

    return 1;
}

static int cfg_json_on_state_end(void * i_ctx) {
    struct cfg_json_read_ctx * ctx = i_ctx;

    if (ctx->m_node_stack_pos >= 0 && ctx->m_node_stack_pos < CPE_CFG_MAX_LEVEL) {
        ctx->m_curent = ctx->m_node_stack[ctx->m_node_stack_pos].m_node;
        ctx->m_state = ctx->m_node_stack[ctx->m_node_stack_pos].m_state;
    }
    else {
        ctx->m_curent = NULL;
    }
    --ctx->m_node_stack_pos;

    return 1;
}

static yajl_callbacks g_cfg_json_callbacks = {
    cfg_json_on_null,
    cfg_json_on_boolean,
    cfg_json_on_integer,
    cfg_json_on_double,
    NULL,
    cfg_json_on_string,
    cfg_json_on_map_begin,
    cfg_json_on_map_key,
    cfg_json_on_state_end,
    cfg_json_on_sequence_begin,
    cfg_json_on_state_end
};

static void cfg_json_read_i(cfg_t cfg, const char * name, read_stream_t stream, cfg_policy_t policy, error_monitor_t em) {
    struct cfg_json_read_ctx ctx;
    yajl_handle handler;
    yajl_status stat;
    struct mem_buffer data_buffer;

    if (cfg_json_read_ctx_init(&ctx, cfg, name, policy, em) != 0) return;

    handler = yajl_alloc(&g_cfg_json_callbacks, NULL, (void*)&ctx);
    if (handler == NULL) {
        CPE_ERROR(em, "json parsser initialize fail!");
        cfg_json_read_ctx_fini(&ctx);
        return;
    }

    mem_buffer_init(&data_buffer, NULL);

    do {
        char read_buf[128];
        ssize_t sz = stream_read(stream, read_buf, CPE_ARRAY_SIZE(read_buf));
        if (sz < 0) {
            CPE_ERROR(em, "cfg json load read data fail!");
            yajl_free(handler);
            cfg_json_read_ctx_fini(&ctx);
            mem_buffer_clear(&data_buffer);
            return;
        }
        
        mem_buffer_append(&data_buffer, read_buf, sz);
        if (sz < CPE_ARRAY_SIZE(read_buf)) break;
    } while(1);
    
    stat = yajl_parse(handler, (const unsigned char *)mem_buffer_make_continuous(&data_buffer, 0), mem_buffer_size(&data_buffer));
    if (stat != yajl_status_ok) {
        unsigned char * err_str = yajl_get_error(
            handler, 1,
            (const unsigned char *)mem_buffer_make_continuous(&data_buffer, 0),
            mem_buffer_size(&data_buffer));
        CPE_ERROR(em, "parse json error, stat=%s\n%s", (const char *)yajl_status_to_string(stat), err_str);
        
        free(err_str);
        yajl_free(handler);
        cfg_json_read_ctx_fini(&ctx);
        mem_buffer_clear(&data_buffer);
        return;
    }

    yajl_free(handler);
    mem_buffer_clear(&data_buffer);
    cfg_json_read_ctx_fini(&ctx);
}

int cfg_json_read(cfg_t cfg, read_stream_t stream, cfg_policy_t policy, error_monitor_t em) {
    return cfg_json_read_with_name(cfg, NULL, stream, policy, em);
}

int cfg_json_read_with_name(cfg_t cfg, const char * name, read_stream_t stream, cfg_policy_t policy, error_monitor_t em) {
    int ret = 0;
    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        cfg_json_read_i(cfg, name, stream, policy, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        cfg_json_read_i(cfg, name, stream, policy, &logError);
    }

    return ret;
}

static int cfg_json_read_ctx_init(struct cfg_json_read_ctx * ctx, cfg_t root, const char * name, cfg_policy_t policy, error_monitor_t em) {
    bzero(ctx, sizeof(struct cfg_json_read_ctx));

    if (cfg_is_value(root)) {
        CPE_ERROR(em, "can`t read data into a data node!");
        return -1;
    }

    ctx->m_name = name;
    mem_buffer_init(&ctx->m_name_buffer, NULL);

    ctx->m_state =
        root->m_type == CPE_CFG_TYPE_STRUCT
        ? cfg_json_parse_in_map
        : cfg_json_parse_in_seq;

    ctx->m_curent = root;
    ctx->m_node_stack_pos = -1;
    ctx->m_policy = policy;
    ctx->m_em = em;

    return 0;
}

static void cfg_json_read_ctx_fini(struct cfg_json_read_ctx * ctx) {
    mem_buffer_clear(&ctx->m_name_buffer);
}

int cfg_json_read_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em) {
    struct vfs_read_stream fstream;
    vfs_file_t fp;
    int rv;

    fp = vfs_file_open(vfs, path, "r");
    if (fp == NULL) return -1;

    CPE_ERROR_SET_FILE(em, path);

    vfs_read_stream_init(&fstream, fp);

    rv = cfg_json_read(cfg, (read_stream_t)&fstream, policy, em);
    
    CPE_ERROR_SET_FILE(em, NULL);

    vfs_file_close(fp);

    return rv;
}
