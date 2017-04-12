#include <assert.h>
#include "yajl/yajl_gen.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stackbuf.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_json.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"

struct DrJsonPrintProcessStack {
    LPDRMETA m_meta;
    LPDRMETAENTRY m_entry;
    int m_entry_pos;
    int m_entry_count;
    int m_array_pos;
    const char * m_src_data;
    size_t m_src_capacity;
};

static const char * yajl_errno_to_string(yajl_gen_status s) {
    static const char * s_errorMsgs[] = {
        "keys_must_be_strings"
        , "max_depth_exceeded"
        , "in_error_state"
        , "generation_complete"
        , "invalid_number"
        , "no_buf"
        , "invalid_string" };

    if (s < 1/*yajl_gen_keys_must_be_strings*/ || s > yajl_gen_invalid_string) {
        return "invalid yajl error";
    }
    else {
        return s_errorMsgs[s - 1];
    }
}

#define JSON_PRINT_CHECK_GEN_RESULT(s)                          \
    { yajl_gen_status __s = (s);                                \
    if (__s != yajl_gen_status_ok) {                            \
    CPE_ERROR(em, "yajl error: %s", yajl_errno_to_string(__s)); \
    return;                                                     \
    }                                                           \
    }

#define JSON_PRINT_GEN_STRING(g, str)               \
    do {                                            \
        const char * __p = (str);                   \
        JSON_PRINT_CHECK_GEN_RESULT(                \
            yajl_gen_string(g, (const unsigned char *)__p, strlen(__p))); \
    } while(0)

static void dr_print_print_numeric(yajl_gen g, int typeId, const void * data, error_monitor_t em) {
    struct write_stream_mem bufS;
    char buf[64 + 1];
    int len;
    
    write_stream_mem_init(&bufS, buf, sizeof(buf) - 1);
    len = dr_ctype_print_to_stream((write_stream_t)&bufS, data, typeId, em);
    if (len > 0) {
        assert(len < CPE_ARRAY_SIZE(buf));
        buf[len] = 0;
        yajl_gen_number(g, buf, len);
    }
    else {
        yajl_gen_null(g);
    }
}

static void dr_print_print_string(yajl_gen g, int typeId, size_t bufLen, const void * data, error_monitor_t em) {
    if (typeId == CPE_DR_TYPE_STRING || typeId == CPE_DR_TYPE_STRING + 1) {
        yajl_gen_string(g, data, strlen(data));
    }
    else {
        char buf[CPE_STACK_BUF_LEN(bufLen) + 1];
        struct write_stream_mem bufS = CPE_WRITE_STREAM_MEM_INITIALIZER(buf, CPE_STACK_BUF_LEN(bufLen) + 1);
        int len = dr_ctype_print_to_stream((write_stream_t)&bufS, data, typeId, em);

        if (len >= 0) {
            if (len >= (int)sizeof(buf)) len = (int)sizeof(buf) - 1;

            buf[len] = 0;
            JSON_PRINT_CHECK_GEN_RESULT(yajl_gen_string(g, (const unsigned char *)buf, len));
        }
        else {
            yajl_gen_null(g);
        }
    }
}

static void dr_print_print_basic_data(yajl_gen g, LPDRMETAENTRY entry, const void * data, error_monitor_t em) {
    switch(entry->m_type) {
    case CPE_DR_TYPE_INT8:
    case CPE_DR_TYPE_UINT8:
    case CPE_DR_TYPE_INT16:
    case CPE_DR_TYPE_UINT16:
    case CPE_DR_TYPE_INT32:
    case CPE_DR_TYPE_UINT32:
    case CPE_DR_TYPE_INT64:
    case CPE_DR_TYPE_UINT64:
    case CPE_DR_TYPE_FLOAT:
    case CPE_DR_TYPE_DOUBLE:
        dr_print_print_numeric(g, entry->m_type, data, em);
        break;
    case CPE_DR_TYPE_CHAR:
    case CPE_DR_TYPE_UCHAR:
        dr_print_print_numeric(g, entry->m_type, data, em);
        break;
    case CPE_DR_TYPE_STRING:
    case (CPE_DR_TYPE_STRING + 1):
        dr_print_print_string(g, entry->m_type, entry->m_unitsize, data, em);
        break;
    default:
        CPE_ERROR_EX(
            em, CPE_DR_ERROR_UNSUPPORTED_TYPE,
            "print basic data not supported type "FMT_INT32_T"!", entry->m_type);
        yajl_gen_null(g);
        break;
    }
}

void dr_json_print_i(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    yajl_gen g,
    error_monitor_t em)
{
    struct DrJsonPrintProcessStack processStack[CPE_DR_MAX_LEVEL];
    int stackPos;

    processStack[0].m_meta = meta;
    processStack[0].m_entry = dr_meta_entry_at(meta, 0);
    processStack[0].m_entry_pos = 0;
    processStack[0].m_entry_count = meta->m_entry_count;
    processStack[0].m_array_pos = 0;
    processStack[0].m_src_data = (const char *)input;
    processStack[0].m_src_capacity = capacity;

    yajl_gen_map_open(g);

    for(stackPos = 0; stackPos >= 0;) {
        struct DrJsonPrintProcessStack * curStack;

        assert(stackPos < CPE_DR_MAX_LEVEL);

        curStack = &processStack[stackPos];
        if (curStack->m_meta == NULL) {
            --stackPos;
            continue;
        }

        for(; curStack->m_entry_pos < curStack->m_entry_count
                && curStack->m_entry
                ;
            ++curStack->m_entry_pos
                , curStack->m_array_pos = 0
                , curStack->m_entry = dr_meta_entry_at(curStack->m_meta, curStack->m_entry_pos)
            )
        {
            size_t elementSize;
            int32_t array_count;
            LPDRMETAENTRY refer;

        LOOPENTRY:

            elementSize = dr_entry_element_size(curStack->m_entry);
            if (elementSize == 0) continue;

            if (curStack->m_array_pos == 0) {
                JSON_PRINT_GEN_STRING(g, dr_entry_name(curStack->m_entry));
            }

            refer = NULL;
            if (curStack->m_entry->m_array_count != 1) {
                if (curStack->m_array_pos == 0) {
                    yajl_gen_array_open(g);
                }

                refer = dr_entry_array_refer_entry(curStack->m_entry);
            }

            array_count = curStack->m_entry->m_array_count;
            if (refer) {
                dr_entry_try_read_int32(
                    &array_count,
                    curStack->m_src_data + curStack->m_entry->m_array_refer_data_start_pos,
                    refer,
                    em);
            }

            for(; curStack->m_array_pos < array_count; ++curStack->m_array_pos) {
                const char * entryData = curStack->m_src_data + dr_entry_data_start_pos(curStack->m_entry, curStack->m_array_pos);
                if ((size_t)(entryData - curStack->m_src_data) > curStack->m_src_capacity) {
                    CPE_ERROR(
                        em, "%s.%s[%d]: read size overflow, capacity=%d",
                        dr_meta_name(dr_entry_self_meta(curStack->m_entry)), dr_entry_name(curStack->m_entry),
                        curStack->m_array_pos, (int)curStack->m_src_capacity);
                    break;
                }

                if (curStack->m_entry->m_type <= CPE_DR_TYPE_COMPOSITE) {
                    if (stackPos + 1 < CPE_DR_MAX_LEVEL) {
                        struct DrJsonPrintProcessStack * nextStack;
                        nextStack = &processStack[stackPos + 1];

                        yajl_gen_map_open(g);

                        nextStack->m_meta = dr_entry_ref_meta(curStack->m_entry);
                        if (nextStack->m_meta == 0) {
                            yajl_gen_map_close(g);
                            break;
                        }

                        nextStack->m_src_data = entryData;
                        if (curStack->m_entry_pos + 1 == curStack->m_entry_count && curStack->m_array_pos + 1 == array_count) {
                            nextStack->m_src_capacity = curStack->m_src_capacity - (entryData - curStack->m_src_data);
                        }
                        else {
                            nextStack->m_src_capacity = curStack->m_src_capacity - (entryData - curStack->m_src_data);
                            if (nextStack->m_src_capacity > elementSize) nextStack->m_src_capacity = elementSize;
                        }

                        nextStack->m_entry_pos = 0;
                        nextStack->m_entry_count = nextStack->m_meta->m_entry_count;

                        if (curStack->m_entry->m_type == CPE_DR_TYPE_UNION) {
                            LPDRMETAENTRY select_entry;
                            select_entry = dr_entry_select_entry(curStack->m_entry);
                            if (select_entry) {
                                int32_t union_entry_id;
                                dr_entry_try_read_int32(
                                    &union_entry_id,
                                    curStack->m_src_data + curStack->m_entry->m_select_data_start_pos,
                                    select_entry,
                                    em);
                                nextStack->m_entry_pos =
                                    dr_meta_find_entry_idx_by_id(nextStack->m_meta, union_entry_id);
                                if (nextStack->m_entry_pos < 0) {
                                    yajl_gen_map_close(g);
                                    continue;
                                }

                                nextStack->m_entry_count = nextStack->m_entry_pos + 1;
                            }
                        }

                        nextStack->m_entry = dr_meta_entry_at(nextStack->m_meta, nextStack->m_entry_pos);

                        nextStack->m_array_pos = 0;

                        ++curStack->m_array_pos;
                        ++stackPos;
                        curStack = nextStack;
                        goto LOOPENTRY;
                    }
                }
                else {
                    if ((size_t)(entryData + elementSize - curStack->m_src_data) > curStack->m_src_capacity) {
                        CPE_ERROR(
                            em, "%s.%s[%d]: read size overflow, element-size=%d, capacity=%d",
                            dr_meta_name(dr_entry_self_meta(curStack->m_entry)), dr_entry_name(curStack->m_entry),
                            curStack->m_array_pos, (int)elementSize, (int)curStack->m_src_capacity);
                        break;
                    }

                    dr_print_print_basic_data(
                        g,
                        curStack->m_entry,
                        curStack->m_src_data + dr_entry_data_start_pos(curStack->m_entry, curStack->m_array_pos),
                        em);
                }
            }

            if (curStack->m_entry->m_array_count != 1) {
                yajl_gen_array_close(g);
            }
        }

        yajl_gen_map_close(g);
        --stackPos;
    }

    //yajl_gen_map_close(g);
}

struct dr_json_print_ctx {
    write_stream_t m_output;
    int m_total_size;
};

static int dr_json_do_print(struct dr_json_print_ctx * ctx, const void * buf, size_t size) {
    int r = stream_write(ctx->m_output, buf, size);
    if (r > 0) {
        ctx->m_total_size += r;
    }
    return r;
}

int dr_json_print(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    int flag,
    error_monitor_t em)
{
    int ret = 0;
    struct dr_json_print_ctx print_ctx = { output, 0 };
    yajl_gen g;

    if (output == NULL || input == NULL || meta == NULL) {
        CPE_ERROR(em, "dr_json_print: bad para!");
        return -1;
    }

    g = yajl_gen_alloc(NULL);
    if (g == NULL) {
        CPE_ERROR_EX(em, CPE_DR_ERROR_NO_MEMORY, "alloc yajl_gen fail!");
        return -1;
    }

    yajl_gen_config(g, yajl_gen_beautify, flag & DR_JSON_PRINT_MINIMIZE ? 0 : 1);
    //yajl_gen_config(g, yajl_gen_validate_utf8, flag & DR_JSON_PRINT_VALIDATE_UTF8 ? 1 : 0);
    yajl_gen_config(g, yajl_gen_print_callback, dr_json_do_print, &print_ctx);


    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        dr_json_print_i(output, input, capacity, meta, g, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        dr_json_print_i(output, input, capacity, meta, g, &logError);
    }

    yajl_gen_free(g);    

    return ret == 0 ? print_ctx.m_total_size : ret;
}

void dr_json_print_array_i(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    yajl_gen g,
    error_monitor_t em)
{
    size_t i;
    char * buf = (char *)input;
    size_t element_capacity = dr_meta_size(meta);
    size_t count = capacity / element_capacity;

    yajl_gen_array_open(g);

    for(i = 0; i < count; ++i, buf += element_capacity) {
        dr_json_print_i(output, buf, element_capacity, meta, g, em);
    }

    yajl_gen_array_close(g);
}

int dr_json_print_array(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    int flag,
    error_monitor_t em)
{
    int ret = 0;
    struct dr_json_print_ctx print_ctx = { output, 0 };
    yajl_gen g;

    if (output == NULL || input == NULL || meta == NULL) {
        CPE_ERROR(em, "dr_json_print_array: bad para!");
        return -1;
    }

    g = yajl_gen_alloc(NULL);
    if (g == NULL) {
        CPE_ERROR_EX(em, CPE_DR_ERROR_NO_MEMORY, "alloc yajl_gen fail!");
        return -1;
    }

    yajl_gen_config(g, yajl_gen_beautify, flag & DR_JSON_PRINT_MINIMIZE ? 0 : 1);
    //yajl_gen_config(g, yajl_gen_validate_utf8, flag & DR_JSON_PRINT_VALIDATE_UTF8 ? 1 : 0);
    yajl_gen_config(g, yajl_gen_print_callback, dr_json_do_print, &print_ctx);


    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        dr_json_print_array_i(output, input, capacity, meta, g, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        dr_json_print_array_i(output, input, capacity, meta, g, &logError);
    }

    yajl_gen_free(g);    

    return ret == 0 ? print_ctx.m_total_size : ret;
}

const char * dr_json_dump(mem_buffer_t buffer, const void * input, size_t capacity, LPDRMETA meta) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    dr_json_print((write_stream_t)&stream, input, capacity, meta, DR_JSON_PRINT_BEAUTIFY, NULL);

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

const char * dr_json_dump_inline(mem_buffer_t buffer, const void * input, size_t capacity, LPDRMETA meta) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    dr_json_print((write_stream_t)&stream, input, capacity, meta, DR_JSON_PRINT_MINIMIZE, NULL);

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

