#include <assert.h>
#include "yaml.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_yaml.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

#define YAML_PARSE_CTX_BUF_LEN CPE_DR_MACRO_LEN + 1

#define YAML_PARSE_CTX_COPY_STR_TMP(__ctx, str, len) do {   \
        size_t __len = len;                                 \
        if (__len >= YAML_PARSE_CTX_BUF_LEN) {              \
            __len = YAML_PARSE_CTX_BUF_LEN - 1;             \
        }                                                   \
        memcpy(__ctx->m_buf, str, __len);                   \
        __ctx->m_buf[__len] = 0;                            \
    } while(0)

enum dr_yaml_read_select_state {
    dr_yaml_read_select_not_use
    , dr_yaml_read_select_use
    , dr_yaml_read_select_error
};

struct dr_yaml_parse_stack_info {
    LPDRMETA m_meta;
    size_t m_start_pos;
    int m_capacity;
    LPDRMETAENTRY m_entry;
    int m_in_name;
    int m_in_array;
    int m_array_count;
    enum dr_yaml_read_select_state m_select_state;

    int32_t m_select_data;
};

struct dr_yaml_parse_ctx {
    yaml_parser_t m_parser;
    yaml_event_t m_input_event;
    
    char * m_output_buf;
    size_t m_output_capacity;
    struct mem_buffer * m_output_alloc;
    error_monitor_t m_em;

    int m_root_in_array;
    int m_root_array_count;
    LPDRMETA m_root_meta;

    struct dr_yaml_parse_stack_info m_typeStacks[CPE_DR_MAX_LEVEL];
    int m_stackPos;

    int m_size;

    char m_buf[YAML_PARSE_CTX_BUF_LEN]; //used to store tmp data
};

static char * dr_yaml_parse_get_write_pos(
    struct dr_yaml_parse_ctx * ctx,
    struct dr_yaml_parse_stack_info * stackInfo,
    const char * element_name,
    int start_pos,
    size_t capacity)
{
    size_t start;
    size_t total_size;

    assert(start_pos >= 0);

    assert(ctx);
    assert(stackInfo);

    start = stackInfo->m_start_pos + start_pos;
    total_size = start + capacity;

    if (ctx->m_output_buf) {
        if (total_size > ctx->m_output_capacity) {
            CPE_ERROR_EX(
                ctx->m_em, dr_code_error_not_enough_output,
                "process %s.%s, not enough output buf, require %d + %d, but only %d!",
                dr_meta_name(stackInfo->m_meta), element_name, (int)start, (int)capacity, (int)ctx->m_output_capacity);
            return NULL;
        }

        if (total_size > (size_t)ctx->m_size) ctx->m_size = (int)total_size;

        return ctx->m_output_buf + start;
    }
    else {
        assert(ctx->m_output_alloc);

        if (mem_buffer_size(ctx->m_output_alloc) < total_size) {
            if (mem_buffer_set_size(ctx->m_output_alloc, total_size) != 0) return NULL;
        }

        assert(total_size <= mem_buffer_size(ctx->m_output_alloc));

        if (total_size > ctx->m_size) ctx->m_size = (int)total_size;

        return ((char*)mem_buffer_make_continuous(ctx->m_output_alloc, 0)) + start;
    }
}

static char * dr_yaml_parse_get_read_pos(
    struct dr_yaml_parse_ctx * ctx,
    struct dr_yaml_parse_stack_info * stackInfo,
    int start_pos,
    size_t capacity)
{
    size_t start;
    size_t total_size;

    if (start_pos < 0) return NULL;

    assert(ctx);
    assert(stackInfo);

    start = stackInfo->m_start_pos + start_pos;
    total_size = start + capacity;

    if (total_size > ctx->m_size) return NULL;

    if (ctx->m_output_buf) {
        return ctx->m_output_buf + start;
    }
    else {
        return ((char*)mem_buffer_make_continuous(ctx->m_output_alloc, 0)) + start;
    }
}

static void dr_yaml_parse_stack_init(
    struct dr_yaml_parse_stack_info * stackInfo, LPDRMETA meta, size_t start_pos, int capacity)
{
    stackInfo->m_meta = meta;
    stackInfo->m_start_pos = start_pos;
    stackInfo->m_capacity = capacity;
    stackInfo->m_entry = NULL;
    stackInfo->m_in_name = 0;
    stackInfo->m_in_array = 0;
    stackInfo->m_array_count = 0;
    stackInfo->m_select_state = dr_yaml_read_select_not_use;
    stackInfo->m_select_data = 0;
}

static int dr_yaml_do_parse_calc_start_pos(
    struct dr_yaml_parse_ctx * c, 
    struct dr_yaml_parse_stack_info * parseType)
{
    if (parseType->m_entry->m_array_count == 1) {
        if (parseType->m_in_array) {
            CPE_ERROR_EX(
                c->m_em, dr_code_error_format_error,
                "process %s.%s, expect not in array!",
                dr_meta_name(parseType->m_meta), c->m_buf);
            return -1;
        }

        return (int)dr_entry_data_start_pos(parseType->m_entry, 0);
    }
    else {
        if (parseType->m_entry->m_array_count > 1
            && parseType->m_array_count >= parseType->m_entry->m_array_count)
        {
            CPE_ERROR_EX(
                c->m_em, dr_code_error_format_error,
                "process %s.%s, array count overflow!",
                dr_meta_name(parseType->m_meta), c->m_buf);
            return -1;
        }

        return (int)dr_entry_data_start_pos(parseType->m_entry, parseType->m_array_count++);
    }
}

static void dr_yaml_do_parse_from_string(
    struct dr_yaml_parse_ctx * c, 
    struct dr_yaml_parse_stack_info * parseType,
    const char * s, size_t l)
{
    size_t elementSize;
    char * writePos;
    int startPos;

    if (parseType->m_entry == NULL) return;

    elementSize = dr_entry_element_size(parseType->m_entry);

    startPos = dr_yaml_do_parse_calc_start_pos(c, parseType);
    if (startPos < 0) return;

    writePos = dr_yaml_parse_get_write_pos(c, parseType, dr_entry_name(parseType->m_entry), startPos, elementSize);
    if (writePos == NULL) return;

    YAML_PARSE_CTX_COPY_STR_TMP(c, s, l);
    dr_entry_set_from_string(writePos, c->m_buf, parseType->m_entry, c->m_em);
}

static void dr_yaml_on_scalar(struct dr_yaml_parse_ctx * ctx) {
    struct dr_yaml_parse_ctx * c = (struct dr_yaml_parse_ctx *) ctx;
    struct dr_yaml_parse_stack_info * parseType;

    if (c->m_stackPos < 0 || c->m_stackPos >= CPE_DR_MAX_LEVEL) {
        return;
    }

    parseType = &c->m_typeStacks[c->m_stackPos];
    
    if(!parseType->m_in_array) {
        if (parseType->m_meta == NULL) return;
        if (parseType->m_meta->m_type >  CPE_DR_TYPE_COMPOSITE) return;

        if (!parseType->m_in_name) {
            LPDRMETAENTRY entry;
            
            parseType->m_in_name = 1;
            if (parseType->m_meta == NULL) {
                return;
            }

            parseType->m_entry = NULL;
            
            YAML_PARSE_CTX_COPY_STR_TMP(
                c,
                (const char *)ctx->m_input_event.data.scalar.value,
                ctx->m_input_event.data.scalar.length);

            entry = dr_meta_find_entry_by_name(parseType->m_meta, c->m_buf);
            if (entry == NULL) {
                return;
            }

            switch(parseType->m_select_state) {
            case dr_yaml_read_select_error:
                return;
            case dr_yaml_read_select_not_use:
                break;
            case dr_yaml_read_select_use:
                if (parseType->m_select_data < entry->m_select_range_min
                    || parseType->m_select_data > entry->m_select_range_max)
                {
                    return;
                }
                break;
            default:
                CPE_ERROR_EX(
                    c->m_em, dr_code_error_internal, "process %s.%s, unknown select state %d",
                    dr_meta_name(parseType->m_meta), c->m_buf, parseType->m_select_state);
                return;
            }

            parseType->m_entry = entry;
        }
        else {
            const char * tag;
            parseType->m_in_name = 0;

            if (parseType->m_entry == NULL) return;
            
            tag = (const char *)ctx->m_input_event.data.scalar.tag;
            if (tag && strcmp(tag, YAML_NULL_TAG) == 0) {
            }
            else if (ctx->m_input_event.data.scalar.length > 0) {
                if (parseType->m_entry->m_type <= CPE_DR_TYPE_COMPOSITE) return;
                dr_yaml_do_parse_from_string(
                    c, parseType, (const char *)ctx->m_input_event.data.scalar.value, ctx->m_input_event.data.scalar.length);
            }
            else {
            }
        }
    }
    else {
        if (parseType->m_entry == NULL) return;
        if (parseType->m_entry->m_type <= CPE_DR_TYPE_COMPOSITE) return;
        dr_yaml_do_parse_from_string(
            c, parseType, (const char *)ctx->m_input_event.data.scalar.value, ctx->m_input_event.data.scalar.length);
    }
}

static void dr_yaml_start_map(struct dr_yaml_parse_ctx * c) {
    LPDRMETA refType = NULL;
    int nextStackPos = c->m_stackPos + 1;
    struct dr_yaml_parse_stack_info * curStack;
    struct dr_yaml_parse_stack_info * nestStackNode;

    if (nextStackPos < 0 || nextStackPos >= CPE_DR_MAX_LEVEL) {
        if (c->m_stackPos < CPE_DR_MAX_LEVEL) {
            CPE_ERROR_EX(c->m_em, dr_code_error_internal, "too complex, max nest level %d reached", CPE_DR_MAX_LEVEL);
        }

        ++c->m_stackPos;
        return;
    }

    nestStackNode = &c->m_typeStacks[nextStackPos];

    if (c->m_stackPos < 0) {
        if (c->m_root_in_array) {
            dr_yaml_parse_stack_init(
                nestStackNode,
                c->m_root_meta,
                c->m_root_array_count * dr_meta_size(c->m_root_meta),
                (int)dr_meta_size(c->m_root_meta));
        }
        else {
            dr_yaml_parse_stack_init(nestStackNode, c->m_root_meta, 0, -1);
        }

        ++c->m_stackPos;
        return;
    }

    curStack = NULL;
    if (c->m_stackPos >= 0 && c->m_stackPos < CPE_DR_MAX_LEVEL) {
        curStack = &c->m_typeStacks[c->m_stackPos];
    }

    ++c->m_stackPos;
    if (curStack == NULL  || curStack->m_entry == NULL) {
        return;
    }

    refType = dr_entry_ref_meta(curStack->m_entry);
    if (refType) { /*composite*/
        LPDRMETAENTRY selectEntry;
        int startPos;

        startPos = dr_yaml_do_parse_calc_start_pos(c, curStack);
        if (startPos < 0) return;

        dr_yaml_parse_stack_init(
            nestStackNode,
            refType,
            curStack->m_start_pos + startPos,
            curStack->m_entry->m_unitsize);

        selectEntry = dr_entry_select_entry(curStack->m_entry);
        if (selectEntry) {
            size_t select_entry_capaity = dr_entry_element_size(selectEntry);
            const char * read_pos = dr_yaml_parse_get_read_pos(c, curStack, curStack->m_entry->m_select_data_start_pos, select_entry_capaity);
            if (read_pos && dr_entry_try_read_int32(&nestStackNode->m_select_data, read_pos, selectEntry, c->m_em) == 0) {
                nestStackNode->m_select_state = dr_yaml_read_select_use;
            }
            else {
                nestStackNode->m_select_state = dr_yaml_read_select_error;
            }
        }
    }
}

static void dr_yaml_end_map(struct dr_yaml_parse_ctx * c) {
    if (c->m_stackPos >= 0 && c->m_stackPos < CPE_DR_MAX_LEVEL) {
        /*clear current stack*/
        dr_yaml_parse_stack_init(&c->m_typeStacks[c->m_stackPos], NULL, 0, 0);
    }
    --c->m_stackPos;

    if (c->m_stackPos == -1) {
        if (c->m_root_in_array) {
            ++c->m_root_array_count;
        }
    }
    else {
        struct dr_yaml_parse_stack_info * curStack = &c->m_typeStacks[c->m_stackPos];
        curStack->m_in_name = 0;
    }
}

static void dr_yaml_start_array(struct dr_yaml_parse_ctx * c) {
    if (c->m_stackPos < 0) {
        c->m_root_in_array = 1;
        return;
    }
    else if (c->m_stackPos >= CPE_DR_MAX_LEVEL) {
        return;
    }
    else {
        c->m_typeStacks[c->m_stackPos].m_in_array = 1;
        return;
    }
}

static void dr_yaml_end_array(struct dr_yaml_parse_ctx * c) {
    struct dr_yaml_parse_stack_info * curStack;
    LPDRMETAENTRY refer;

    if (c->m_stackPos < 0 || c->m_stackPos >= CPE_DR_MAX_LEVEL) {
        return;
    }

    curStack = &c->m_typeStacks[c->m_stackPos];

    curStack->m_in_array = 0;

    refer = curStack->m_entry ? dr_entry_array_refer_entry(curStack->m_entry) : NULL;
    if (refer) {
        char * ref_write_pos;
        ref_write_pos = dr_yaml_parse_get_write_pos(
            c, curStack, dr_entry_name(refer), curStack->m_entry->m_array_refer_data_start_pos, dr_entry_element_size(refer));
        if (ref_write_pos) {
            dr_entry_set_from_int32(ref_write_pos, curStack->m_array_count, refer, c->m_em);
        }
    }

    curStack->m_array_count = 0;
    curStack->m_in_name = 0;
}

static void dr_yaml_notify_parse_error(struct dr_yaml_parse_ctx * ctx) {
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

typedef void (*dr_yaml_read_event_process_fun_t)(struct dr_yaml_parse_ctx * ctx);

static
dr_yaml_read_event_process_fun_t
g_yaml_read_event_processors[YAML_MAPPING_END_EVENT + 1] = {
    /*YAML_NO_EVENT*/ 0,
    /*YAML_STREAM_START_EVENT*/ NULL,
    /*YAML_STREAM_END_EVENT*/ NULL,
    /*YAML_DOCUMENT_START_EVENT*/ NULL,
    /*YAML_DOCUMENT_END_EVENT*/ NULL,
    /*YAML_ALIAS_EVENT*/ NULL,
    /*YAML_SCALAR_EVENT*/ dr_yaml_on_scalar,
    /*YAML_SEQUENCE_START_EVENT*/ dr_yaml_start_array,
    /*YAML_SEQUENCE_END_EVENT*/ dr_yaml_end_array,
    /*YAML_MAPPING_START_EVENT*/ dr_yaml_start_map,
    /*YAML_MAPPING_END_EVENT*/ dr_yaml_end_map
};

static int dr_read_from_stream_read_handler(
    void *data, unsigned char *buffer, size_t size, size_t *size_read)
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

static void dr_yaml_parse_ctx_init(
    struct dr_yaml_parse_ctx * ctx,
    void * result,
    size_t capacity,
    struct mem_buffer * result_buffer, 
    LPDRMETA meta,
    read_stream_t stream,
    error_monitor_t em)
{
    bzero(ctx, sizeof(struct dr_yaml_parse_ctx));

    if (!yaml_parser_initialize(&ctx->m_parser)) {
        CPE_ERROR(em, "yaml parsser initialize fail!");
        return;
    }

    yaml_parser_set_input(&ctx->m_parser, dr_read_from_stream_read_handler, stream);
    
    ctx->m_output_buf = result;
    ctx->m_output_capacity = capacity;
    ctx->m_output_alloc = result_buffer;

    ctx->m_root_meta = meta;
    ctx->m_root_in_array = 0;
    ctx->m_root_array_count = 0;

    ctx->m_stackPos = -1;
    ctx->m_em = em;
    ctx->m_size = (int)dr_meta_size(meta);
}

static int dr_yaml_read_i(
    void * result,
    size_t capacity,
    struct mem_buffer * result_buf, 
    const char * input,
    LPDRMETA meta,
    error_monitor_t em)
{
    struct read_stream_mem rs;
    struct dr_yaml_parse_ctx ctx;
    int done = 0;

    read_stream_mem_init(&rs, input, strlen(input));
    dr_yaml_parse_ctx_init(&ctx, result, capacity, result_buf, meta, (read_stream_t)&rs, em);

    while (!done) {
        if (!yaml_parser_parse(&ctx.m_parser, &ctx.m_input_event)) {
            dr_yaml_notify_parse_error(&ctx);
            break;
        }

        if (ctx.m_input_event.type == YAML_STREAM_END_EVENT) done = 1;

        if (ctx.m_input_event.type >= sizeof(g_yaml_read_event_processors)/sizeof(dr_yaml_read_event_process_fun_t))
        {
            CPE_ERROR(em, "unknown yaml event %d!", ctx.m_input_event.type);
            done = 1;
        }
        else {
            dr_yaml_read_event_process_fun_t processor = g_yaml_read_event_processors[ctx.m_input_event.type];
            if (processor) {
                processor(&ctx);
            }
        }
        
        yaml_event_delete(&ctx.m_input_event);
    }

    yaml_parser_delete(&ctx.m_parser);
    yaml_event_delete(&ctx.m_input_event);

    return ctx.m_size;
}

int dr_yaml_read(
    void * result,
    size_t capacity,
    const char * input,
    LPDRMETA meta,
    error_monitor_t em)
{
    int ret = 0;
    int size = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        size = dr_yaml_read_i(result, capacity, NULL, input, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        size = dr_yaml_read_i(result, capacity, NULL, input, meta, &logError);
    }

    return ret == 0 ? size : ret;
}

int dr_yaml_read_to_buffer(
    struct mem_buffer * result, 
    const char * input,
    LPDRMETA meta,
    error_monitor_t em)
{
    int ret = 0;
    int size = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        size = dr_yaml_read_i(NULL, 0, result, input, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        size = dr_yaml_read_i(NULL, 0, result, input, meta, &logError);
    }

    return ret == 0 ? size : ret;
}
