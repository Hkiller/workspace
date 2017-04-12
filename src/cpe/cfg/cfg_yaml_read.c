#include <assert.h>
#include "yaml.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_ctypes_info.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "cfg_internal_ops.h"

enum cfg_yaml_parse_state {
    cfg_yaml_parse_in_map,
    cfg_yaml_parse_in_seq
};

struct cfg_yaml_read_ctx {
    yaml_parser_t m_parser;
    yaml_event_t m_input_event;

    cfg_t m_curent;
    enum cfg_yaml_parse_state m_state;

    struct {
        cfg_t m_node;
        enum cfg_yaml_parse_state m_state;
    } m_node_stack[CPE_CFG_MAX_LEVEL];
    int m_node_stack_pos;

    const char * m_name;
    struct mem_buffer m_name_buffer;

    cfg_policy_t m_policy;
    error_monitor_t m_em;
};

static int cfg_read_from_stream_read_handler(
    void *data,
    unsigned char *buffer,
    size_t size,
    size_t *size_read)
{
    int readSize;

    readSize = stream_read((read_stream_t)data, (char*)buffer, size);

    if (readSize >= 0) {
        *size_read = (size_t)readSize;
        return 1;
    }
    else {
        return 0;
    }
}

static int cfg_yaml_read_ctx_init(
    struct cfg_yaml_read_ctx * ctx,
    cfg_t root,
    const char * name,
    read_stream_t stream,
    cfg_policy_t policy,
    error_monitor_t em)
{
    bzero(ctx, sizeof(struct cfg_yaml_read_ctx));

    if (cfg_is_value(root)) {
        CPE_ERROR(em, "can`t read data into a data node!");
        return -1;
    }
        
    if (!yaml_parser_initialize(&ctx->m_parser)) {
        CPE_ERROR(em, "yaml parsser initialize fail!");
        return -1;
    }

    yaml_parser_set_input(&ctx->m_parser, cfg_read_from_stream_read_handler, stream);

    ctx->m_name = name;
    mem_buffer_init(&ctx->m_name_buffer, NULL);

    ctx->m_state =
        root->m_type == CPE_CFG_TYPE_STRUCT
        ? cfg_yaml_parse_in_map
        : cfg_yaml_parse_in_seq;

    ctx->m_curent = root;
    ctx->m_node_stack_pos = -1;
    ctx->m_policy = policy;
    ctx->m_em = em;

    return 0;
}

static void cfg_yaml_read_ctx_fini(struct cfg_yaml_read_ctx * ctx) {
    mem_buffer_clear(&ctx->m_name_buffer);
    yaml_parser_delete(&ctx->m_parser);
    yaml_event_delete(&ctx->m_input_event);
}

static void cfg_yaml_on_stream_begin(struct cfg_yaml_read_ctx * ctx) {
    if (ctx->m_curent == NULL) return;
}

static void cfg_yaml_on_stream_end(struct cfg_yaml_read_ctx * ctx) {
    if (ctx->m_curent == NULL) return;
}

static void cfg_yaml_on_document_begin(struct cfg_yaml_read_ctx * ctx) {
    if (ctx->m_curent == NULL) return;
}

static void cfg_yaml_on_document_end(struct cfg_yaml_read_ctx * ctx) {
    if (ctx->m_curent == NULL) return;
}

static void cfg_yaml_on_alias(struct cfg_yaml_read_ctx * ctx) {
    if (ctx->m_curent == NULL) return;
}

static int cfg_yaml_get_type_from_tag(struct cfg_yaml_read_ctx * ctx) {
    const char * tag = (const char *)ctx->m_input_event.data.scalar.tag;

    if (tag == NULL) return CPE_CFG_TYPE_STRING;

    if (tag[0] == '!') {
        int id = dr_type_id_from_name(tag + 1);
        return id == CPE_DR_TYPE_UNKOWN
            ? CPE_CFG_TYPE_STRING
            : id;
    }
    else {
        return CPE_CFG_TYPE_STRING;
    }
}

int32_t cfg_yaml_read_bool(const char * value) {
    if (strcmp(value, "true") == 0 || strcmp(value, "y") == 0) {
        return 1;
    }
    else if (strcmp(value, "false") == 0 || strcmp(value, "n") == 0) {
        return 0;
    }
    else {
        return -1;
    }
}

#ifdef _MSC_VER

#define cfg_yaml_do_add_value(__type, __ctx, ...)                   \
    if ((__ctx)->m_curent->m_type == CPE_CFG_TYPE_STRUCT) {                 \
    cfg_struct_add_ ## __type(                                          \
    (__ctx)->m_curent, (__ctx)->m_name, __VA_ARGS__, (__ctx)->m_policy); \
    }                                                                       \
else {                                                                  \
    cfg_seq_add_ ## __type((__ctx)->m_curent, __VA_ARGS__);                  \
}

#else

#define cfg_yaml_do_add_value(__type, __ctx, args...)                   \
if ((__ctx)->m_curent->m_type == CPE_CFG_TYPE_STRUCT) {                 \
    cfg_struct_add_ ## __type(                                          \
        (__ctx)->m_curent, (__ctx)->m_name, ##args, (__ctx)->m_policy); \
}                                                                       \
else {                                                                  \
    cfg_seq_add_ ## __type((__ctx)->m_curent, ##args);                  \
}

#endif

static void cfg_yaml_add_value(struct cfg_yaml_read_ctx * ctx, const char * value) {
	int typeId;
    const char * tag;
	
    tag = (const char *)ctx->m_input_event.data.scalar.tag;
    if (tag) {
        if (strcmp(tag, YAML_BOOL_TAG) == 0) {
            int32_t v = cfg_yaml_read_bool(value);
            if (v >= 0) {
                cfg_yaml_do_add_value(int32, ctx, v);
            }
            return;
        }
        if (strcmp(tag, YAML_INT_TAG) == 0) {
            cfg_yaml_do_add_value(value_from_string, ctx, CPE_CFG_TYPE_INT32, value);
            return;
        }
    }
    else {
        if (!ctx->m_input_event.data.scalar.quoted_implicit) {
            cfg_yaml_do_add_value(value_from_string_auto, ctx, value);
            return;
        }
    }

    typeId = cfg_yaml_get_type_from_tag(ctx) ;
    cfg_yaml_do_add_value(value_from_string, ctx, typeId, value);
}

static void cfg_yaml_on_scalar(struct cfg_yaml_read_ctx * ctx) {
    if (ctx->m_curent == NULL) return;

    if(ctx->m_state == cfg_yaml_parse_in_map) {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return;

        if (ctx->m_name == NULL) {
            mem_buffer_clear_data(&ctx->m_name_buffer);
            if (ctx->m_input_event.data.scalar.length > 0) {
                ctx->m_name = mem_buffer_strdup_len(
                    &ctx->m_name_buffer,
                    (const char *)ctx->m_input_event.data.scalar.value,
                    ctx->m_input_event.data.scalar.length);
                if (ctx->m_name == NULL) {
                    CPE_ERROR(ctx->m_em, "dump scalar as name, no memory!");
                    ctx->m_name = "";
                }
            }
            else {
                ctx->m_name = "";
            }
        }
        else {
            const char * tag = (const char *)ctx->m_input_event.data.scalar.tag;
            if (tag && strcmp(tag, YAML_NULL_TAG) == 0) {
                cfg_t oldValue = cfg_struct_find_cfg(ctx->m_curent, ctx->m_name);
                if (oldValue
                    && (ctx->m_policy == cfg_replace || ctx->m_policy == cfg_merge_use_new))
                {
                    cfg_struct_item_delete((struct cfg_struct *)ctx->m_curent, oldValue);
                }
            }
            else if (ctx->m_input_event.data.scalar.length > 0) {
                const char * value = 
                    mem_buffer_strdup_len(
                        &ctx->m_name_buffer,
                        (const char *)ctx->m_input_event.data.scalar.value,
                        ctx->m_input_event.data.scalar.length);
                if (value == NULL) {
                    CPE_ERROR(ctx->m_em, "dump scalar as map value, no memory!");
                }
                else {
                    cfg_yaml_add_value(ctx, value);
                }
            }
            else {
                cfg_struct_add_string(ctx->m_curent, ctx->m_name, "", ctx->m_policy);
            }
            ctx->m_name = NULL;
        }
    }
    else {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return;

        if (ctx->m_input_event.data.scalar.length > 0) {
            const char * value = 
                mem_buffer_strdup_len(
                    &ctx->m_name_buffer,
                    (const char *)ctx->m_input_event.data.scalar.value,
                    ctx->m_input_event.data.scalar.length);
            if (value == NULL) {
                CPE_ERROR(ctx->m_em, "dump scalar as seq value, no memory!");
            }
            else {
                cfg_yaml_add_value(ctx, value);
            }
        }
        else {
            //ignore empty in sequence
        }
    }
}

static void cfg_yaml_on_sequence_begin(struct cfg_yaml_read_ctx * ctx) {
    enum cfg_yaml_parse_state old_state = ctx->m_state;
    ctx->m_state = cfg_yaml_parse_in_seq;
    ++ctx->m_node_stack_pos;
    if (ctx->m_node_stack_pos >= CPE_CFG_MAX_LEVEL) {
        ctx->m_curent = NULL;
        return;
    }
    ctx->m_node_stack[ctx->m_node_stack_pos].m_node = ctx->m_curent;
    ctx->m_node_stack[ctx->m_node_stack_pos].m_state = old_state;

    if (ctx->m_curent == NULL) return;

    if (old_state == cfg_yaml_parse_in_map) {
        const char * name = NULL;

        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return;

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
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return;

        if (ctx->m_node_stack_pos == 0) {
            //DO NOTHING
        }
        else {
            ctx->m_curent = cfg_seq_add_seq(ctx->m_curent);
        }
    }
}

static void cfg_yaml_on_map_begin(struct cfg_yaml_read_ctx * ctx) {
    enum cfg_yaml_parse_state old_state = ctx->m_state;
    ctx->m_state = cfg_yaml_parse_in_map;
    ++ctx->m_node_stack_pos;
    if (ctx->m_node_stack_pos >= CPE_CFG_MAX_LEVEL) {
        ctx->m_curent = NULL;
        return;
    }
    ctx->m_node_stack[ctx->m_node_stack_pos].m_node = ctx->m_curent;
    ctx->m_node_stack[ctx->m_node_stack_pos].m_state = old_state;

    if (ctx->m_curent == NULL) return;

    if (old_state == cfg_yaml_parse_in_map) {
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_STRUCT) return;

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
        if (ctx->m_curent->m_type != CPE_CFG_TYPE_SEQUENCE) return;

        ctx->m_curent = cfg_seq_add_struct(ctx->m_curent);
    }
}

static void cfg_yaml_on_state_end(struct cfg_yaml_read_ctx * ctx) {
    if (ctx->m_node_stack_pos >= 0 && ctx->m_node_stack_pos < CPE_CFG_MAX_LEVEL) {
        ctx->m_curent = ctx->m_node_stack[ctx->m_node_stack_pos].m_node;
        ctx->m_state = ctx->m_node_stack[ctx->m_node_stack_pos].m_state;
    }
    else {
        ctx->m_curent = NULL;
    }
    --ctx->m_node_stack_pos;
}

static void cfg_yaml_notify_parse_error(struct cfg_yaml_read_ctx * ctx) {
    switch (ctx->m_parser.error) {
        case YAML_MEMORY_ERROR:
            CPE_ERROR(ctx->m_em, "Memory error: Not enough memory for parsing\n");
            break;
        case YAML_READER_ERROR:
            if (ctx->m_parser.problem_value != -1) {
                CPE_ERROR(
                    ctx->m_em, "Reader error: %s: #%X at %d\n",
                    ctx->m_parser.problem,
                    ctx->m_parser.problem_value,
                    (int)ctx->m_parser.problem_offset);
            }
            else {
                CPE_ERROR(
                    ctx->m_em,  "Reader error: %s at %d\n",
                    ctx->m_parser.problem,
                    (int)ctx->m_parser.problem_offset);
            }
            break;
        case YAML_SCANNER_ERROR:
            CPE_ERROR_SET_LINE(ctx->m_em, (int)(ctx->m_parser.problem_mark.line + 1));
            if (ctx->m_parser.context) {
                CPE_ERROR(
                    ctx->m_em, "Scanner error: %s at line %d, column %d\n"
                    "%s at line %d, column %d\n",
                    ctx->m_parser.context,
                    (int)ctx->m_parser.context_mark.line+1,
                    (int)ctx->m_parser.context_mark.column+1,
                    ctx->m_parser.problem,
                    (int)ctx->m_parser.problem_mark.line+1,
                    (int)ctx->m_parser.problem_mark.column+1);
            }
            else {
                CPE_ERROR(
                    ctx->m_em, "Scanner error: %s at line %d, column %d\n",
                    ctx->m_parser.problem,
                    (int)ctx->m_parser.problem_mark.line+1,
                    (int)ctx->m_parser.problem_mark.column+1);
            }
            break;
        case YAML_PARSER_ERROR:
            CPE_ERROR_SET_LINE(ctx->m_em, ctx->m_parser.problem_mark.line + 1);
            if (ctx->m_parser.context) {
                CPE_ERROR(
                    ctx->m_em,
                    "Parser error: %s at line %d, column %d\n"
                    "%s at line %d, column %d\n",
                    ctx->m_parser.context,
                    (int)ctx->m_parser.context_mark.line+1,
                    (int)ctx->m_parser.context_mark.column+1,
                    ctx->m_parser.problem,
                    (int)ctx->m_parser.problem_mark.line+1,
                    (int)ctx->m_parser.problem_mark.column+1);
            }
            else {
                CPE_ERROR(
                    ctx->m_em, "Parser error: %s at line %d, column %d\n",
                    ctx->m_parser.problem,
                    (int)ctx->m_parser.problem_mark.line+1,
                    (int)ctx->m_parser.problem_mark.column+1);
            }
            break;
        default:
            /* Couldn't happen. */
            CPE_ERROR(ctx->m_em, "Yaml Internal error\n");
            break;
    }
}

typedef void (*cfg_yaml_read_event_process_fun_t)(struct cfg_yaml_read_ctx * ctx);

static
cfg_yaml_read_event_process_fun_t
g_yaml_read_event_processors[YAML_MAPPING_END_EVENT + 1] = {
    /*YAML_NO_EVENT*/ 0,
    /*YAML_STREAM_START_EVENT*/ cfg_yaml_on_stream_begin,
    /*YAML_STREAM_END_EVENT*/ cfg_yaml_on_stream_end,
    /*YAML_DOCUMENT_START_EVENT*/ cfg_yaml_on_document_begin,
    /*YAML_DOCUMENT_END_EVENT*/ cfg_yaml_on_document_end,
    /*YAML_ALIAS_EVENT*/ cfg_yaml_on_alias,
    /*YAML_SCALAR_EVENT*/ cfg_yaml_on_scalar,
    /*YAML_SEQUENCE_START_EVENT*/ cfg_yaml_on_sequence_begin,
    /*YAML_SEQUENCE_END_EVENT*/ cfg_yaml_on_state_end,
    /*YAML_MAPPING_START_EVENT*/ cfg_yaml_on_map_begin,
    /*YAML_MAPPING_END_EVENT*/ cfg_yaml_on_state_end
};

static void cfg_yaml_read_i(cfg_t cfg, const char * name, read_stream_t stream, cfg_policy_t policy, error_monitor_t em) {
    struct cfg_yaml_read_ctx ctx;
    int done = 0;

    if (cfg_yaml_read_ctx_init(&ctx, cfg, name, stream, policy, em) != 0) return;

    while (!done) {
        if (!yaml_parser_parse(&ctx.m_parser, &ctx.m_input_event)) {
            cfg_yaml_notify_parse_error(&ctx);
            break;
        }

        if (ctx.m_input_event.type == YAML_STREAM_END_EVENT) done = 1;

        if (ctx.m_input_event.type >= sizeof(g_yaml_read_event_processors)/sizeof(cfg_yaml_read_event_process_fun_t))
        {
            CPE_ERROR(em, "unknown yaml event %d!", ctx.m_input_event.type);
            done = 1;
        }
        else {
            cfg_yaml_read_event_process_fun_t processor = g_yaml_read_event_processors[ctx.m_input_event.type];
            if (processor == NULL) {
                CPE_ERROR(em, "no processor for yaml event %d!", ctx.m_input_event.type);
                done = 1;
            }
            else {
                processor(&ctx);
            }
        }
        
        yaml_event_delete(&ctx.m_input_event);
    }

    cfg_yaml_read_ctx_fini(&ctx);
}

int cfg_yaml_read(cfg_t cfg, read_stream_t stream, cfg_policy_t policy, error_monitor_t em) {
    return cfg_yaml_read_with_name(cfg, NULL, stream, policy, em);
}

int cfg_yaml_read_with_name(cfg_t cfg, const char * name, read_stream_t stream, cfg_policy_t policy, error_monitor_t em) {
    int ret = 0;
    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        cfg_yaml_read_i(cfg, name, stream, policy, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        cfg_yaml_read_i(cfg, name, stream, policy, &logError);
    }

    return ret;
}

int cfg_yaml_read_file(cfg_t cfg, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em) {
    struct vfs_read_stream fstream;
    vfs_file_t fp;
    int rv;

    fp = vfs_file_open(vfs, path, "r");
    if (fp == NULL) return -1;

    CPE_ERROR_SET_FILE(em, path);

    vfs_read_stream_init(&fstream, fp);

    rv = cfg_yaml_read(cfg, (read_stream_t)&fstream, policy, em);
    
    CPE_ERROR_SET_FILE(em, NULL);

    vfs_file_close(fp);

    return rv;
}

int cfg_yaml_read_file_with_name(cfg_t cfg, const char * name, vfs_mgr_t vfs, const char * path, cfg_policy_t policy, error_monitor_t em) {
    struct vfs_read_stream fstream;
    vfs_file_t fp;
    int rv;

    fp = vfs_file_open(vfs, path, "r");
    if (fp == NULL) return -1;

    CPE_ERROR_SET_FILE(em, path);

    vfs_read_stream_init(&fstream, fp);

    rv = cfg_yaml_read_with_name(cfg, name, (read_stream_t)&fstream, policy, em);
    
    CPE_ERROR_SET_FILE(em, NULL);

    vfs_file_close(fp);

    return rv;
}

