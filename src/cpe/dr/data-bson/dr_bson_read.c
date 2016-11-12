#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_bson.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"
#include "dr_bson_internal_ops.h"

struct dr_bson_read_stack {
    LPDRMETA m_meta;
    LPDRMETAENTRY m_from_entry;
    size_t m_output_start_pos;
    const char * m_input_data;
    int32_t m_input_pos;
    int32_t m_input_capacity;
    int32_t m_array_count;
    int32_t m_for_array;
};

struct dr_bson_read_ctx {
    char * m_output_buf;
    size_t m_output_capacity;
    struct mem_buffer * m_output_alloc;
    size_t m_used_size;
    error_monitor_t m_em;
};

static int dr_bson_read_stack_init(
    struct dr_bson_read_stack * process_stack, struct dr_bson_read_ctx * ctx,
    LPDRMETA meta, LPDRMETAENTRY from_entry, size_t output_start_pos, int for_array,
    const char * input, int32_t input_capacity)
{
    int32_t doc_size;

    if (input_capacity < 5) {
        CPE_ERROR(ctx->m_em, "init stack for meta %s: input capacity %d too small!", dr_meta_name(meta), input_capacity);
        return -1;
    }

    BSON_COPY_HTON32(&doc_size, input);
    if (doc_size > input_capacity) {
        CPE_ERROR(
            ctx->m_em, "init stack for meta %s: doc size %d overflow, input capacity is %d!",
            dr_meta_name(meta), doc_size, input_capacity);
        return -1;
    }

    process_stack->m_meta = meta;
    process_stack->m_from_entry = from_entry;
    process_stack->m_output_start_pos = output_start_pos;
    process_stack->m_input_data = input + 4;
    process_stack->m_input_pos = 0;
    process_stack->m_input_capacity = doc_size - 5;
    process_stack->m_array_count = 0;
    process_stack->m_for_array = for_array;
    return 0;
}

#define dr_bson_read_check_capacity(__capacity)                         \
    if (curStack->m_input_capacity - curStack->m_input_pos < (__capacity)) { \
        CPE_ERROR(em, "dr_bson_read: %s.%s: not enouth buf, capacity=%d, require=%d!" \
                  , dr_meta_name(curStack->m_meta), dr_entry_name(entry) \
                  , (int)(curStack->m_input_capacity - curStack->m_input_pos) \
                  , (int)(__capacity));                                 \
        return -1;                                                      \
    }

#define dr_bson_read_check_min_size(__len, __min_size)                  \
    if ((__len) < (__min_size)) {                                       \
        CPE_ERROR(em, "dr_bson_read: %s.%s: size %d to small, at least %d!" \
                  , dr_meta_name(curStack->m_meta), dr_entry_name(entry) \
                  , (int)(__len)                                       \
                  , (int)(__min_size));                                 \
        goto DR_BSON_READ_IGNORE;                                       \
    }

#define dr_bson_read_check_end_by_zero(__len)                           \
    if (curStack->m_input_data[curStack->m_input_pos + (__len) - 1] != 0) { \
        CPE_ERROR(em, "dr_bson_read: %s.%s: not end by zero!"           \
                  , dr_meta_name(curStack->m_meta), dr_entry_name(entry)); \
        goto DR_BSON_READ_IGNORE;                                       \
    }

#define dr_bson_read_type_error()                                       \
    CPE_ERROR(                                                          \
        em, "dr_bson_read: %s.%s: not support read type %s from bson type %d!", \
        dr_meta_name(curStack->m_meta), dr_entry_name(entry), dr_type_name(entry->m_type), e_type); \
    goto DR_BSON_READ_IGNORE

#define dr_bson_read_start_pos()                                \
    dr_entry_data_start_pos(entry, array_pos)

#define dr_bson_read_by_int32() do {                                    \
        char * write_buf;                                               \
        int32_t v;                                                      \
        dr_bson_read_check_capacity(4);                                 \
        BSON_COPY_HTON32(&v, curStack->m_input_data + curStack->m_input_pos); \
        if ((write_buf = dr_bson_read_get_write_pos(&ctx, curStack, dr_bson_read_start_pos(), element_size))) { \
            dr_entry_set_from_int32(write_buf, v, entry, em);           \
        }                                                               \
        curStack->m_input_pos += 4;                                     \
    } while(0)

#define dr_bson_read_by_uint32() do {                                    \
        char * write_buf;                                               \
        uint32_t v;                                                      \
        dr_bson_read_check_capacity(4);                                 \
        BSON_COPY_HTON32(&v, curStack->m_input_data + curStack->m_input_pos); \
        if ((write_buf = dr_bson_read_get_write_pos(&ctx, curStack, dr_bson_read_start_pos(), element_size))) {  \
            dr_entry_set_from_uint32(write_buf, v, entry, em);          \
        }                                                               \
        curStack->m_input_pos += 4;                                     \
    } while(0)

#define dr_bson_read_by_int64() do {                                    \
        char * write_buf;                                               \
        int64_t v;                                                      \
        dr_bson_read_check_capacity(8);                                 \
        BSON_COPY_HTON64(&v, curStack->m_input_data + curStack->m_input_pos); \
        if ((write_buf = dr_bson_read_get_write_pos(&ctx, curStack, dr_bson_read_start_pos(), element_size))) { \
            dr_entry_set_from_int64(write_buf, v, entry, em);           \
        }                                                               \
        curStack->m_input_pos += 8;                                     \
    } while(0)

#define dr_bson_read_by_uint64() do {                                    \
        char * write_buf;                                               \
        uint64_t v;                                                      \
        dr_bson_read_check_capacity(8);                                 \
        BSON_COPY_HTON64(&v, curStack->m_input_data + curStack->m_input_pos); \
        if ((write_buf = dr_bson_read_get_write_pos(&ctx, curStack, dr_bson_read_start_pos(), element_size))) { \
            dr_entry_set_from_uint64(write_buf, v, entry, em);          \
        }                                                               \
        curStack->m_input_pos += 8;                                     \
    } while(0)

#define dr_bson_read_by_double() do {                                    \
        char * write_buf;                                               \
        double v;                                                       \
        dr_bson_read_check_capacity(8);                                 \
        BSON_COPY_HTON64(&v, curStack->m_input_data + curStack->m_input_pos); \
        if ((write_buf = dr_bson_read_get_write_pos(&ctx, curStack, dr_bson_read_start_pos(), element_size))) { \
            dr_entry_set_from_double(write_buf, v, entry, em);          \
        }                                                               \
        curStack->m_input_pos += 8;                                     \
    } while(0)

#define dr_bson_read_by_string() do {                                   \
        char * write_buf;                                               \
        int32_t len;                                                    \
        BSON_COPY_HTON32(&len, curStack->m_input_data + curStack->m_input_pos); \
        dr_bson_read_check_capacity(len + 4);                           \
        dr_bson_read_check_min_size(len, 1);                            \
        dr_bson_read_check_end_by_zero(len + 4);                        \
        if ((write_buf = dr_bson_read_get_write_pos(&ctx, curStack, dr_bson_read_start_pos(), element_size))) { \
            dr_entry_set_from_string(write_buf, curStack->m_input_data + curStack->m_input_pos + 4, entry, em); \
        }                                                               \
        curStack->m_input_pos += len + 4;                               \
    } while(0)

static char * dr_bson_read_get_write_pos(
    struct dr_bson_read_ctx * ctx,
    struct dr_bson_read_stack * stackInfo,
    int start_pos,
    size_t capacity)
{
    size_t start;
    size_t total_size;
    if (start_pos < 0) return NULL;

    assert(ctx);
    assert(stackInfo);

    start = stackInfo->m_output_start_pos + start_pos;
    total_size = start + capacity;

    if (ctx->m_output_buf) {
        if (total_size > ctx->m_output_capacity) return NULL;

        if (total_size > ctx->m_used_size) ctx->m_used_size = total_size;

        return ctx->m_output_buf + start;
    }
    else {
        assert(ctx->m_output_alloc);

        if (mem_buffer_size(ctx->m_output_alloc) < total_size) {
            if (mem_buffer_set_size(ctx->m_output_alloc, total_size) != 0) return NULL;
        }

        assert(total_size <= mem_buffer_size(ctx->m_output_alloc));

        if (total_size > ctx->m_used_size) ctx->m_used_size = total_size;

        return ((char*)mem_buffer_make_continuous(ctx->m_output_alloc, 0)) + start;
    }
}
    
static int dr_bson_read_i(
    void * output,
    size_t output_capacity,
    struct mem_buffer * result_buf, 
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em)
{
    struct dr_bson_read_ctx ctx;
    struct dr_bson_read_stack processStack[CPE_DR_MAX_LEVEL];
    int stackPos;

    assert(input);
    assert(meta);

    ctx.m_output_buf = output;
    ctx.m_output_capacity = output_capacity;
    ctx.m_output_alloc = result_buf;
    ctx.m_em = em;
    ctx.m_used_size = dr_meta_size(meta);

    if (dr_bson_read_stack_init(&processStack[0], &ctx, meta, NULL, 0, 0, input, input_capacity) != 0) return -1;

    for(stackPos = 0; stackPos >= 0;) {
        struct dr_bson_read_stack * curStack;

        assert(stackPos < (sizeof(processStack) / sizeof(processStack[0])));

    DR_BSON_READ_STACK:
        curStack = &processStack[stackPos];
        if (curStack->m_meta == NULL) {
            --stackPos;
            continue;
        }

        while(curStack->m_input_pos < curStack->m_input_capacity) {
            uint8_t e_type;
            const char * e_name;
            LPDRMETAENTRY entry;
            size_t element_size;
            int32_t array_pos;

            e_type = (uint8_t)curStack->m_input_data[curStack->m_input_pos];
            curStack->m_input_pos++;

            e_name = curStack->m_input_data + curStack->m_input_pos;
            curStack->m_input_pos += strlen(e_name) + 1;

            if (curStack->m_for_array) {
                char * end;

                assert(curStack->m_from_entry && curStack->m_from_entry->m_array_count != 1);

                array_pos = (int)strtol(e_name, &end, 10);
                if (*end != 0) {
                    CPE_ERROR(em, "dr_bson_read: %s.???: read array pos from %s fail!", dr_meta_name(curStack->m_meta), e_name);
                    goto DR_BSON_READ_IGNORE;
                }

                entry = curStack->m_from_entry;

                element_size = dr_entry_element_size(entry);

                if (array_pos + 1 > curStack->m_array_count) curStack->m_array_count = array_pos + 1;
            }
            else {
                array_pos = 0;
                entry = dr_meta_find_entry_by_name(curStack->m_meta, e_name);
                if (entry == NULL) goto DR_BSON_READ_IGNORE;
            
                element_size = dr_entry_element_size(entry);

                if (entry->m_array_count != 1) {
                    int32_t len;
                    BSON_COPY_HTON32(&len, curStack->m_input_data + curStack->m_input_pos);

                    dr_bson_read_check_capacity(len);
                    dr_bson_read_check_min_size(len, 5);
                    dr_bson_read_check_end_by_zero(len);

                    curStack->m_input_pos += len;

                    if (stackPos + 1 < (sizeof(processStack) / sizeof(processStack[0]))) {
                        struct dr_bson_read_stack * nextStack;
                        size_t total_size;
                        nextStack = &processStack[stackPos + 1];

                        dr_bson_read_stack_init(
                            nextStack, &ctx, curStack->m_meta, entry, curStack->m_output_start_pos, 1,
                            curStack->m_input_data + curStack->m_input_pos - len, len);

                        total_size = curStack->m_output_start_pos + dr_bson_read_start_pos() + element_size;

                        ++stackPos;

                        if (total_size > ctx.m_output_capacity) {
                            CPE_ERROR(
                                em, "dr_bson_read: %s.%s: array not enouth buf, capacity=%d, require=%d!"
                                , dr_meta_name(curStack->m_meta), dr_entry_name(entry), (int)ctx.m_output_capacity , (int)total_size);
                            return -1;
                        }

                        if (total_size > ctx.m_used_size) ctx.m_used_size = total_size;

                        goto DR_BSON_READ_STACK;
                    }
                    else {
                        goto DR_BSON_READ_IGNORE;
                    }
                }

                if (curStack->m_from_entry && curStack->m_from_entry->m_type == CPE_DR_TYPE_UNION) {
                    struct dr_bson_read_stack * preStack;
                    LPDRMETAENTRY selectEntry;
                
                    selectEntry = dr_entry_select_entry(curStack->m_from_entry);
                    if (selectEntry) {
                        char * writeBuf;
                        size_t selectElementSize;

                        assert(stackPos > 0);
                        preStack = &processStack[stackPos - 1];
                        selectElementSize = dr_entry_element_size(selectEntry);

                        writeBuf = dr_bson_read_get_write_pos(
                            &ctx, preStack, selectEntry->m_data_start_pos, selectElementSize);
                        dr_entry_set_from_uint32(writeBuf, entry->m_id, selectEntry, em);
                    }
                }
            }

            switch(entry->m_type) {
            case CPE_DR_TYPE_UNION:
            case CPE_DR_TYPE_STRUCT: {
                switch(e_type) {
                case dr_bson_type_embeded: {
                    int32_t len;
                    BSON_COPY_HTON32(&len, curStack->m_input_data + curStack->m_input_pos);

                    dr_bson_read_check_capacity(len);
                    dr_bson_read_check_min_size(len, 5);
                    dr_bson_read_check_end_by_zero(len);

                    curStack->m_input_pos += len;

                    if (stackPos + 1 < (sizeof(processStack) / sizeof(processStack[0]))) {
                        struct dr_bson_read_stack * nextStack;
                        size_t total_size;
                        nextStack = &processStack[stackPos + 1];

                        dr_bson_read_stack_init(
                            nextStack, &ctx, dr_entry_ref_meta(entry), entry, curStack->m_output_start_pos + dr_bson_read_start_pos(), 0,
                            curStack->m_input_data + curStack->m_input_pos - len, len);

                        total_size = curStack->m_output_start_pos + dr_bson_read_start_pos() + element_size;

                        ++stackPos;

                        if (total_size > ctx.m_output_capacity) {
                            CPE_ERROR(
                                em, "dr_bson_read: %s.%s: embered not enouth buf, capacity=%d, require=%d!"
                                , dr_meta_name(curStack->m_meta), dr_entry_name(entry), (int)ctx.m_output_capacity , (int)total_size);
                            return -1;
                        }

                        if (total_size > ctx.m_used_size) ctx.m_used_size = total_size;

                        goto DR_BSON_READ_STACK;
                    }
                    else {
                        goto DR_BSON_READ_IGNORE;
                    }
                    break;
                }
                case dr_bson_type_null:
                    break;
                default:
                    dr_bson_read_type_error();
                }

                break;
            }
            case CPE_DR_TYPE_CHAR:
            case CPE_DR_TYPE_INT8:
            case CPE_DR_TYPE_INT16:
            case CPE_DR_TYPE_INT32:
            case CPE_DR_TYPE_INT64:
                switch(e_type) {
                case dr_bson_type_int32:
                    dr_bson_read_by_int32();
                    break;
                case dr_bson_type_timestamp:
                case dr_bson_type_int64:
                    dr_bson_read_by_int64();
                    break;
                case dr_bson_type_string:
                    dr_bson_read_by_string();
                    break;
                case dr_bson_type_double:
                    dr_bson_read_by_double();
                    break;
                case dr_bson_type_null:
                    break;
                default:
                    dr_bson_read_type_error();
                }
                break;
            case CPE_DR_TYPE_UCHAR:
            case CPE_DR_TYPE_UINT8:
            case CPE_DR_TYPE_UINT16:
            case CPE_DR_TYPE_UINT32:
            case CPE_DR_TYPE_UINT64:
                switch(e_type) {
                case dr_bson_type_int32:
                    dr_bson_read_by_uint32();
                    break;
                case dr_bson_type_timestamp:
                case dr_bson_type_int64:
                    dr_bson_read_by_uint64();
                    break;
                case dr_bson_type_string:
                    dr_bson_read_by_string();
                    break;
                case dr_bson_type_null:
                    break;
                case dr_bson_type_double:
                    dr_bson_read_by_double();
                    break;
                default:
                    dr_bson_read_type_error();
                }
                break;
            case CPE_DR_TYPE_FLOAT:
            case CPE_DR_TYPE_DOUBLE:
                switch(e_type) {
                case dr_bson_type_double:
                    dr_bson_read_by_double();
                    break;
                case dr_bson_type_string:
                    dr_bson_read_by_string();
                    break;
                case dr_bson_type_null:
                    break;
                default:
                    dr_bson_read_type_error();
                }
                break;
            case CPE_DR_TYPE_STRING:
                switch(e_type) {
                case dr_bson_type_string:
                    dr_bson_read_by_string();
                    break;
                case dr_bson_type_null:
                    break;
                case dr_bson_type_int32:
                    dr_bson_read_by_uint32();
                    break;
                case dr_bson_type_int64:
                    dr_bson_read_by_uint64();
                    break;
                case dr_bson_type_double:
                    dr_bson_read_by_double();
                    break;
                default:
                    dr_bson_read_type_error();
                }
                break;
            case CPE_DR_TYPE_DATE:
            case CPE_DR_TYPE_TIME:
            case CPE_DR_TYPE_DATETIME:
            case CPE_DR_TYPE_MONEY:
            case CPE_DR_TYPE_IP:
            case CPE_DR_TYPE_VOID:
            default:
                goto DR_BSON_READ_IGNORE;
            }

            continue;
        DR_BSON_READ_IGNORE:

            switch(e_type) {
            case dr_bson_type_double:
                curStack->m_input_pos += 8;
                break;
            case dr_bson_type_string: {
                int32_t len;
                BSON_COPY_HTON32(&len, curStack->m_input_data + curStack->m_input_pos);
                curStack->m_input_pos += 4 + len;
                break;
            }
            case dr_bson_type_binary: {
                int32_t len;
                BSON_COPY_HTON32(&len, curStack->m_input_data + curStack->m_input_pos);
                curStack->m_input_pos += 4 + 1 + len;
                break;
            }
            case dr_bson_type_array:
            case dr_bson_type_js:
            case dr_bson_type_symbol:
            case dr_bson_type_js_w_s:
            case dr_bson_type_embeded: {
                int32_t len;
                BSON_COPY_HTON32(&len, curStack->m_input_data + curStack->m_input_pos);
                curStack->m_input_pos += len;
                break;
            }
            case dr_bson_type_undefined:
                break;
            case dr_bson_type_oid:
                curStack->m_input_pos += 12;
                break;
            case dr_bson_type_boolean:
                curStack->m_input_pos += 1;
                break;
            case dr_bson_type_datetime:
                curStack->m_input_pos += 8;
                break;
            case dr_bson_type_null:
                break;
            case dr_bson_type_reg: {
                int32_t len;
                len = strlen(curStack->m_input_data + curStack->m_input_pos) + 1;
                curStack->m_input_pos += len;
                len = strlen(curStack->m_input_data + curStack->m_input_pos) + 1;
                curStack->m_input_pos += len;
                break;
            }
            case dr_bson_type_dbp:{
                int32_t len;
                BSON_COPY_HTON32(&len, curStack->m_input_data + curStack->m_input_pos);
                curStack->m_input_pos += len + 12;
                break;
            }
            case dr_bson_type_int32:
                curStack->m_input_pos += 4;
                break;
            case dr_bson_type_timestamp:
            case dr_bson_type_int64:
                curStack->m_input_pos += 8;
                break;
            case dr_bson_type_min:
            case dr_bson_type_max:
                break;
            default:
                CPE_ERROR(em, "unknown bson type %d", e_type);
                return -1;
            }
        }

        if (curStack->m_for_array) {
            LPDRMETAENTRY refer;

            assert(curStack->m_from_entry && curStack->m_from_entry->m_array_count != 1);

            refer = dr_entry_array_refer_entry(curStack->m_from_entry);
            if (refer) {
                struct dr_bson_read_stack * preStack;
                char * writeBuf;

                assert(stackPos > 0);
                preStack = &processStack[stackPos - 1];

                writeBuf = dr_bson_read_get_write_pos(&ctx, preStack, refer->m_data_start_pos, dr_entry_element_size(refer));
                if (writeBuf) {
                    dr_entry_set_from_uint32(writeBuf, curStack->m_array_count, refer, em);
                }
            }
        }

        --stackPos;
    }

    return (int)ctx.m_used_size;
}

int dr_bson_read(
    void * output,
    size_t output_capacity,
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em)
{
    int ret = 0;
    int size = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        size = dr_bson_read_i(output, output_capacity, NULL, input, input_capacity, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        size = dr_bson_read_i(output, output_capacity, NULL, input, input_capacity, meta, &logError);
    }

    return ret == 0 ? size : ret;
}

int dr_bson_read_to_buffer(
    struct mem_buffer * result, 
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em)
{
    int ret = 0;
    int size = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        size = dr_bson_read_i(NULL, 0, result, input, input_capacity, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        size = dr_bson_read_i(NULL, 0, result, input, input_capacity, meta, &logError);
    }

    return ret == 0 ? size : ret;
}
