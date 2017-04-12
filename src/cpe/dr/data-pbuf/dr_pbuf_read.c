#include <assert.h>
#include <limits.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_pbuf.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"
#include "dr_pbuf_internal_ops.h"

struct dr_pbuf_read_array_info {
    LPDRMETAENTRY m_entry;
    uint32_t m_count;
};

struct dr_pbuf_read_stack {
    LPDRMETA m_meta;
    LPDRMETAENTRY m_from_entry;
    size_t m_output_start_pos;
    uint8_t const * m_input_data;
    size_t m_input_size;
    size_t m_input_capacity;

    struct dr_pbuf_read_array_info *m_array_info_begin;
    struct dr_pbuf_read_array_info *m_array_info_end;
};

struct dr_pbuf_read_ctx {
    char * m_output_buf;
    size_t m_output_capacity;
    struct mem_buffer * m_output_alloc;
    size_t m_used_size;
    error_monitor_t m_em;
};

static
struct dr_pbuf_read_array_info *
dr_pbuf_read_get_array_info(error_monitor_t em,
    struct dr_pbuf_read_array_info * arrayInfo_begin,
    struct dr_pbuf_read_array_info ** arrayInfo_end,
    struct dr_pbuf_read_array_info * arrayInfo_capacity,
    LPDRMETA meta,
    LPDRMETAENTRY entry)
{
    struct dr_pbuf_read_array_info * info;
    for(info = arrayInfo_begin;
        info < *arrayInfo_end;
        ++info)
    {
        if (info->m_entry == entry) {
            return info;
        }
    }
    if (*arrayInfo_end < arrayInfo_capacity){
        info = *arrayInfo_end;
        *arrayInfo_end = info + 1;
        info->m_entry = entry;
        info->m_count = 0;
        return info;
    }

    CPE_ERROR(
        em, "dr_pbuf_read_get_array_info: arrayInfo not enough! seriously, do you need so many array? %s.%s",
        dr_meta_name(meta), dr_entry_name(entry));
    
    assert(0 && "force crash to check this situation: capacity of arrayInfo not enough");

    return NULL;
}

static char * dr_pbuf_read_get_write_pos(
    struct dr_pbuf_read_ctx * ctx,
    struct dr_pbuf_read_stack * stackInfo,
    LPDRMETAENTRY entry,
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
        if (total_size > ctx->m_output_capacity) {
            CPE_ERROR_EX(
                ctx->m_em, dr_code_error_not_enough_output,
                "dr_pbuf_read: %s.%s: not enouth output, total_size=%d, output-capacity=%d!",
                dr_meta_name(stackInfo->m_meta), dr_entry_name(entry), (int)total_size, (int)ctx->m_output_capacity);
            return NULL;
        }

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

#define dr_pbuf_read_stack_init(                            \
    __stackInfo, __from_entry, __start_pos,                  \
    __meta, __input, __input_capacity)                      \
    do {                                                    \
        bzero(__stackInfo, sizeof(*__stackInfo));           \
        (__stackInfo)->m_meta = __meta;                     \
        (__stackInfo)->m_from_entry = __from_entry;         \
        (__stackInfo)->m_output_start_pos = __start_pos;    \
        (__stackInfo)->m_input_data = __input;              \
        (__stackInfo)->m_input_size = 0;                    \
        (__stackInfo)->m_input_capacity = __input_capacity; \
    } while(0)

#define dr_pbuf_read_check_capacity(__capacity)                         \
    if (curStack->m_input_capacity - curStack->m_input_size             \
        < (__capacity))                                                 \
    {                                                                   \
        CPE_ERROR_EX(                                                   \
            em, dr_code_error_not_enough_input                          \
            , "dr_pbuf_read: %s: not enouth input, require=%d, only=%d!" \
            , entry ? dr_entry_name(entry) : "??"                       \
            , (int)(__capacity)                                         \
            , (int)(curStack->m_input_capacity - curStack->m_input_size)); \
        return -1;                                                      \
    }

#define dr_pbuf_read_decode_varint(v) do {                         \
        int r = dr_pbuf_decode(                                 \
            curStack->m_input_data + curStack->m_input_size, &v);   \
        dr_pbuf_read_check_capacity(r);                             \
        curStack->m_input_size += r;                                \
    } while(0)

#define dr_pbuf_read_ignore_value(t)            \
    switch(t) {                                 \
    case CPE_PBUF_TYPE_VARINT: {                \
        struct dr_pbuf_longlong rb;         \
        dr_pbuf_read_decode_varint(rb);         \
        break;                                  \
    }                                           \
    case CPE_PBUF_TYPE_64BIT:                   \
        dr_pbuf_read_check_capacity(8);         \
        curStack->m_input_size += 8;            \
        break;                                  \
    case CPE_PBUF_TYPE_LENGTH: {                \
        struct dr_pbuf_longlong rb;         \
        dr_pbuf_read_decode_varint(rb);         \
        dr_pbuf_read_check_capacity(rb.low);    \
        curStack->m_input_size += rb.low;       \
        break;                                  \
    }                                           \
    case CPE_PBUF_TYPE_32BIT:                   \
        dr_pbuf_read_check_capacity(4);         \
        curStack->m_input_size += 4;            \
        break;                                  \
    }                                           \

#define dr_pbuf_read_start_pos()                                \
    dr_entry_data_start_pos(entry, (array_info ? array_info->m_count : 0))

#define dr_pbuf_read_type_error()                                       \
    CPE_ERROR(                                                          \
        em, "dr_pbuf_read: %s.%s: not support read type %d from pbuf type %d!", \
        dr_meta_name(curStack->m_meta), dr_entry_name(entry), (int)entry->m_type, value_type); \
    goto DR_PBUF_READ_IGNORE                                            \

#define dr_pbuf_read_by_float() do {                                    \
        char * writeBuf;                                                \
        union {                                                         \
            uint8_t b[4];                                               \
            float f;                                                    \
        } u;                                                            \
        uint8_t const * i = curStack->m_input_data + curStack->m_input_size; \
        writeBuf = dr_pbuf_read_get_write_pos(                          \
            &ctx, curStack, entry, (int)dr_pbuf_read_start_pos(), elementSize); \
        if (writeBuf == NULL) return -1;                 \
                                                                        \
        dr_pbuf_read_check_capacity(4);                                 \
                                                                        \
        u.b[0] = i[0];                                                  \
        u.b[1] = i[1];                                                  \
        u.b[2] = i[2];                                                  \
        u.b[3] = i[3];                                                  \
                                                                        \
        curStack->m_input_size += 4;                                    \
                                                                        \
        dr_entry_set_from_float(writeBuf, u.f, entry, em);              \
    } while(0)

#define dr_pbuf_read_by_double() do {                                   \
        char * writeBuf;                                                \
        union {                                                         \
            uint8_t b[8];                                               \
            double d;                                                    \
        } u;                                                            \
        uint8_t const * i = curStack->m_input_data + curStack->m_input_size; \
        writeBuf = dr_pbuf_read_get_write_pos(                          \
            &ctx, curStack, entry, (int)dr_pbuf_read_start_pos(), elementSize); \
        if (writeBuf == NULL) return -1;                 \
                                                                        \
        dr_pbuf_read_check_capacity(8);                                 \
                                                                        \
        u.b[0] = i[0];                                                  \
        u.b[1] = i[1];                                                  \
        u.b[2] = i[2];                                                  \
        u.b[3] = i[3];                                                  \
        u.b[4] = i[4];                                                  \
        u.b[5] = i[5];                                                  \
        u.b[6] = i[6];                                                  \
        u.b[7] = i[7];                                                  \
                                                                        \
        curStack->m_input_size += 8;                                    \
                                                                        \
        dr_entry_set_from_float(writeBuf, u.d, entry, em);              \
    } while(0)

#define dr_pbuf_read_by_int64() do {                                    \
        char * writeBuf;                                                \
        union {                                                         \
            struct dr_pbuf_longlong sep;                            \
            int64_t i64;                                                \
        } u;                                                            \
        writeBuf = dr_pbuf_read_get_write_pos(                          \
            &ctx, curStack, entry, (int)dr_pbuf_read_start_pos(), elementSize); \
        if (writeBuf == NULL) return -1;                 \
        dr_pbuf_read_decode_varint(u.sep);                              \
        dr_pbuf_dezigzag64(&u.sep);                                 \
        dr_entry_set_from_int64(writeBuf, u.i64, entry, em);            \
    } while(0)

#define dr_pbuf_read_by_uint64() do {                                   \
        char * writeBuf;                                                \
        union {                                                         \
            struct dr_pbuf_longlong sep;                            \
            uint64_t u64;                                               \
        } u;                                                            \
        writeBuf = dr_pbuf_read_get_write_pos(                          \
            &ctx, curStack, entry, (int)dr_pbuf_read_start_pos(), elementSize); \
        if (writeBuf == NULL) return -1;                 \
        dr_pbuf_read_decode_varint(u.sep);                              \
        dr_entry_set_from_uint64(writeBuf, u.u64, entry, em);           \
    } while(0)

#define dr_pbuf_read_packed_array(__by_op)          \
    do {                                            \
        struct dr_pbuf_longlong len_buf;        \
        size_t end;                                 \
                                                    \
        dr_pbuf_read_decode_varint(len_buf);        \
                                                    \
        end = curStack->m_input_size + len_buf.low; \
                                                    \
        while(curStack->m_input_size < end) {       \
            dr_pbuf_read_ ## __by_op();               \
            ++array_info->m_count;                  \
        }                                           \
    } while(0)

static int dr_pbuf_read_i(
    void * output,
    size_t output_capacity,
    struct mem_buffer * result_buf, 
    const void * input,
    size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em)
{
    struct dr_pbuf_read_ctx ctx;
    struct dr_pbuf_read_stack processStack[CPE_DR_MAX_LEVEL];
    struct dr_pbuf_read_array_info processArray[CPE_DR_MAX_LEVEL * 20];
    union {
        struct dr_pbuf_longlong sep;
        uint64_t u64;
        int64_t i64;
    } buf;

    int stackPos;

    assert(input);
    assert(meta);

    ctx.m_output_buf = output;
    ctx.m_output_capacity = output_capacity;
    ctx.m_output_alloc = result_buf;
    ctx.m_em = em;
    ctx.m_used_size = 0;

    dr_pbuf_read_stack_init(
        &processStack[0], NULL, 0, meta, input, input_capacity);

    processStack[0].m_array_info_begin = processStack[0].m_array_info_end = processArray;

    for(stackPos = 0; stackPos >= 0;) {
        struct dr_pbuf_read_stack * curStack;
        struct dr_pbuf_read_array_info * arrayInfo;

        assert(stackPos < CPE_DR_MAX_LEVEL);

    DR_PBUF_READ_STACK:
        curStack = &processStack[stackPos];
        if (curStack->m_meta == NULL) {
            --stackPos;
            continue;
        }

        while(curStack->m_input_size < curStack->m_input_capacity) {
            uint32_t entry_id;
            uint32_t value_type;
            int entry_pos;
            LPDRMETAENTRY entry = NULL;
            struct dr_pbuf_read_array_info * array_info;
            size_t elementSize;

            dr_pbuf_read_decode_varint(buf.sep);
            entry_id = buf.sep.low >> 3;
            value_type = buf.sep.low & 0x7;

            /*find entry*/
            entry_pos = dr_meta_find_entry_idx_by_id(curStack->m_meta, (int)entry_id);
            if (entry_pos < 0) goto DR_PBUF_READ_IGNORE;

            entry = dr_meta_entry_at(curStack->m_meta, entry_pos);
            if (entry == NULL) goto DR_PBUF_READ_IGNORE;

            if (curStack->m_from_entry && curStack->m_from_entry->m_type == CPE_DR_TYPE_UNION) {
                struct dr_pbuf_read_stack * preStack;
                LPDRMETAENTRY selectEntry;
                
                selectEntry = dr_entry_select_entry(curStack->m_from_entry);
                if (selectEntry) {
                    char * writeBuf;
                    size_t selectElementSize;

                    assert(stackPos > 0);
                    preStack = &processStack[stackPos - 1];
                    selectElementSize = dr_entry_element_size(selectEntry);

                    writeBuf = dr_pbuf_read_get_write_pos(
                        &ctx, preStack, selectEntry, selectEntry->m_data_start_pos, selectElementSize);
                    if (writeBuf == NULL) return -1;

                    dr_entry_set_from_uint32(writeBuf, entry->m_id, selectEntry, em);
                }
            }

            /*get write pos*/
            elementSize = dr_entry_element_size(entry);


            array_info = NULL;
            if (entry->m_array_count != 1) {
                array_info = dr_pbuf_read_get_array_info(em, curStack->m_array_info_begin, &curStack->m_array_info_end, processArray + CPE_ARRAY_SIZE(processArray), curStack->m_meta, entry);
                if (array_info == NULL) goto DR_PBUF_READ_IGNORE;
            }

            switch(entry->m_type) {
            case CPE_DR_TYPE_UNION:
            case CPE_DR_TYPE_STRUCT: {
                switch(value_type) {
                case CPE_PBUF_TYPE_LENGTH: {
                    struct dr_pbuf_longlong len_buf;
                    size_t len;

                    dr_pbuf_read_decode_varint(len_buf);

                    len = len_buf.low;

                    dr_pbuf_read_check_capacity(len);
                    curStack->m_input_size += len;

                    if (len > 0 && stackPos + 1 < CPE_DR_MAX_LEVEL) {
                        struct dr_pbuf_read_stack * nextStack;

                        nextStack = &processStack[stackPos + 1];

                        dr_pbuf_read_stack_init(
                            nextStack, entry, curStack->m_output_start_pos + dr_pbuf_read_start_pos(),
                            dr_entry_ref_meta(entry),
                            curStack->m_input_data + curStack->m_input_size - len, len);
                        
                        nextStack->m_array_info_begin = nextStack->m_array_info_end = curStack->m_array_info_end;

                        ++stackPos;
                        if (array_info) ++array_info->m_count;

                        goto DR_PBUF_READ_STACK;
                    }

                    break;
                }
                default: 
                    dr_pbuf_read_type_error();
                }
                break;
            }
            case CPE_DR_TYPE_CHAR:
            case CPE_DR_TYPE_INT8:
            case CPE_DR_TYPE_INT16:
            case CPE_DR_TYPE_INT32:
            case CPE_DR_TYPE_INT64: {
                switch(value_type) {
                case CPE_PBUF_TYPE_VARINT: {
                    dr_pbuf_read_by_int64();
                    if (array_info) ++array_info->m_count;
                    break;
                }
                case CPE_PBUF_TYPE_LENGTH: {
                    if (array_info) {
                        dr_pbuf_read_packed_array(by_int64);
                        break;
                    }
                }
                default: 
                    dr_pbuf_read_type_error();
                }
                break;
            }
            case CPE_DR_TYPE_UCHAR:
            case CPE_DR_TYPE_UINT8:
            case CPE_DR_TYPE_UINT16:
            case CPE_DR_TYPE_UINT32:
            case CPE_DR_TYPE_UINT64: {
                switch(value_type) {
                case CPE_PBUF_TYPE_VARINT: {
                    dr_pbuf_read_by_uint64();
                    if (array_info) ++array_info->m_count;
                    break;
                }
                case CPE_PBUF_TYPE_LENGTH: {
                    if (array_info) {
                        dr_pbuf_read_packed_array(by_uint64);
                        break;
                    }
                }
                default: 
                    dr_pbuf_read_type_error();
                }
                break;
            }
            case CPE_DR_TYPE_FLOAT: {
                switch(value_type) {
                case CPE_PBUF_TYPE_32BIT: {
                    dr_pbuf_read_by_float();
                    if (array_info) ++array_info->m_count;
                    break;
                }
                case CPE_PBUF_TYPE_LENGTH: {
                    if (array_info) {
                        dr_pbuf_read_packed_array(by_float);
                        break;
                    }
                }
                default: 
                    dr_pbuf_read_type_error();
                }
                break;
            }
            case CPE_DR_TYPE_DOUBLE: {
                switch(value_type) {
                case CPE_PBUF_TYPE_64BIT: {
                    dr_pbuf_read_by_double();
                    if (array_info) ++array_info->m_count;
                    break;
                }
                case CPE_PBUF_TYPE_LENGTH: {
                    if (array_info) {
                        dr_pbuf_read_packed_array(by_double);
                        break;
                    }
                }
                default: 
                    dr_pbuf_read_type_error();
                }
                break;
            }
            case CPE_DR_TYPE_STRING: {
                switch(value_type) {
                case CPE_PBUF_TYPE_LENGTH: {
                    char * writeBuf;
                    struct dr_pbuf_longlong len_buf;
                    size_t len;

                    writeBuf = dr_pbuf_read_get_write_pos(
                        &ctx, curStack, entry, (int)dr_pbuf_read_start_pos(), elementSize);
                    if (writeBuf == NULL) return -1;

                    dr_pbuf_read_decode_varint(len_buf);
                    dr_pbuf_read_check_capacity(len_buf.low);

                    len = len_buf.low;
                    if ((len + 1) >= entry->m_size) len = entry->m_size - 1;

                    memcpy(writeBuf, curStack->m_input_data + curStack->m_input_size, len);
                    writeBuf[len] = 0;

                    curStack->m_input_size += len_buf.low;
                    if (array_info) ++array_info->m_count;
                    break;
                }
                default: 
                    dr_pbuf_read_type_error();
                }
                break;
            }
            case CPE_DR_TYPE_DATE:
            case CPE_DR_TYPE_TIME:
            case CPE_DR_TYPE_DATETIME:
            case CPE_DR_TYPE_MONEY:
            case CPE_DR_TYPE_IP:
            case CPE_DR_TYPE_VOID:
            default:
                goto DR_PBUF_READ_IGNORE;
            }

            continue;
        DR_PBUF_READ_IGNORE:
            dr_pbuf_read_ignore_value(value_type);
        }

        for(arrayInfo = curStack->m_array_info_begin; arrayInfo < curStack->m_array_info_end; arrayInfo++) {
            LPDRMETAENTRY refer;
            struct dr_pbuf_read_array_info * info = arrayInfo;
            if (info->m_entry == NULL) break;

            refer = dr_entry_array_refer_entry(info->m_entry);
            if (refer) {
                char * writeBuf =
                    dr_pbuf_read_get_write_pos(&ctx, curStack, refer, refer->m_data_start_pos, dr_entry_element_size(refer));
                if (writeBuf == NULL) return -1;

                dr_entry_set_from_uint32(writeBuf, info->m_count, refer, em);
            }
        }

        --stackPos;
    }

    return (int)ctx.m_used_size;
}

int dr_pbuf_read(
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
        size = dr_pbuf_read_i(output, output_capacity, NULL, input, input_capacity, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        size = dr_pbuf_read_i(output, output_capacity, NULL, input, input_capacity, meta, &logError);
    }

    return ret == 0 ? size : ret;
}

int dr_pbuf_read_to_buffer(
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
        size = dr_pbuf_read_i(NULL, 0, result, input, input_capacity, meta, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        size = dr_pbuf_read_i(NULL, 0, result, input, input_capacity, meta, &logError);
    }

    return ret == 0 ? size : ret;
}

int dr_pbuf_read_with_size(
    void * result, size_t capacity,
    const void * input, size_t input_capacity, size_t * input_used,
    LPDRMETA meta,
    error_monitor_t em)
{
    int rv;
    uint16_t data_input_size;

    if (input_capacity < sizeof(uint16_t)) {
        CPE_ERROR(em, "dr_pbuf_read_with_size: not enouth input to contain size, input-capacity=%d!", (int)input_capacity);
        return dr_code_error_not_enough_input;
    }

    CPE_COPY_NTOH16(&data_input_size, input);
    if (data_input_size > input_capacity) {
        CPE_ERROR(
            em, "dr_pbuf_read_with_size: not enouth input to contain data, input-capacity=%d, data-size=%d!",
            (int)input_capacity, data_input_size);
        return dr_code_error_not_enough_input;
    }

    rv = dr_pbuf_read(result, capacity, ((char*)input) + sizeof(uint16_t), data_input_size - sizeof(uint16_t), meta, em);
    if (rv < 0) {
        return rv;
    }

    if (input_used) *input_used = data_input_size;

    return rv;
}

int dr_pbuf_array_read(
    void * output, size_t output_capacity,
    const void * input, size_t input_capacity,
    LPDRMETA meta,
    error_monitor_t em)
{
    size_t element_size = dr_meta_size(meta);
    size_t output_used = 0;
    size_t input_used = 0;

    while(input_used < input_capacity) {
        size_t element_input_used = 0;

        int r = dr_pbuf_read_with_size(
            ((char *)output) + output_used, element_size,
            ((const char *)input) + input_used, input_capacity - input_used, &element_input_used,
            meta, em);
        if (r < 0) return r;

        assert(r <= element_size);

        input_used += element_input_used;
        output_used += element_size;
    }

    return (int)output_used;
}

