#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stackbuf.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/dr/dr_bson.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_error.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "../dr_internal_types.h"
#include "../dr_ctype_ops.h"
#include "dr_bson_internal_ops.h"

struct dr_bson_write_stack {
    LPDRMETA m_meta;
    LPDRMETAENTRY m_parent_entry;
    LPDRMETAENTRY m_entry;
    int m_entry_pos;
    int m_entry_count;
    int m_array_pos;
    unsigned char * m_output_data;
    uint32_t m_output_size;
    uint32_t m_output_capacity;

    uint32_t m_is_open;

    uint32_t m_array_begin_pos;

    char const * m_input_data;
    size_t m_input_data_capacity;
};

#define dr_bson_write_check_capacity(__capacity)                        \
    if (curStack->m_output_capacity - curStack->m_output_size < (__capacity)) { \
        CPE_ERROR(em, "dr_bson_write: not enouth buf!");                \
        return -1;                                                      \
    }

#define dr_bson_write_char(v) do {                                      \
        dr_bson_write_check_capacity(1);                                \
        (*((char *)(curStack->m_output_data + curStack->m_output_size))) = v; \
        curStack->m_output_size += 1;                                   \
    } while(0)

#define dr_bson_write_int32(v) do {                                     \
        uint32_t __v = (uint32_t)v;                                     \
        dr_bson_write_check_capacity(4);                                \
        BSON_COPY_HTON32(curStack->m_output_data + curStack->m_output_size, (&__v)); \
        curStack->m_output_size += 4;                                   \
    } while(0)

#define dr_bson_write_int64(v) do {                                     \
        uint64_t __v = (uint64_t)v;                                     \
        dr_bson_write_check_capacity(8);                                \
        BSON_COPY_HTON64(curStack->m_output_data + curStack->m_output_size, (&__v)); \
        curStack->m_output_size += 8;                                   \
    } while(0)

#define dr_bson_write_cstring(v) do {                                    \
        const char * __v = v;                                           \
        uint32_t len = strlen(__v) + 1;                                 \
        dr_bson_write_check_capacity(len);                              \
        memcpy(curStack->m_output_data + curStack->m_output_size, __v, len); \
        curStack->m_output_size += len;                             \
    } while(0)

#define dr_bson_write_string(v) do {                                    \
        const char * __v = v;                                           \
        uint32_t len = strlen(__v) + 1;                                 \
        dr_bson_write_check_capacity(len + 4);                          \
        BSON_COPY_HTON32(curStack->m_output_data + curStack->m_output_size, (&len)); \
        memcpy(curStack->m_output_data + curStack->m_output_size + 4, __v, len); \
        curStack->m_output_size += len + 4;                             \
    } while(0)

#define dr_bson_write_printf(__fmt, __v) do {                           \
        int32_t s;                                                      \
        s = snprintf(                                                   \
            (char *)curStack->m_output_data + curStack->m_output_size,  \
            curStack->m_output_capacity - curStack->m_output_size,      \
            __fmt,                                                      \
            __v);                                                       \
        if (s < 0) {                                                    \
            CPE_ERROR(em, "dr_bson_write: not enouth buf!");            \
            return -1;                                                  \
        }                                                               \
        curStack->m_output_size += s + 1;                               \
    } while(0)

static char dr_bson_calc_bson_type(LPDRMETAENTRY entry) {
    switch(entry->m_type) {
    case CPE_DR_TYPE_INT8:
    case CPE_DR_TYPE_INT16:
    case CPE_DR_TYPE_INT32:
        return dr_bson_type_int32;
    case CPE_DR_TYPE_UINT8:
    case CPE_DR_TYPE_UINT16:
    case CPE_DR_TYPE_UINT32:
        return dr_bson_type_int32;
    case CPE_DR_TYPE_INT64:
        return dr_bson_type_int64;
    case CPE_DR_TYPE_UINT64:
        return dr_bson_type_int64;
    case CPE_DR_TYPE_FLOAT:
    case CPE_DR_TYPE_DOUBLE:
        return dr_bson_type_double;
    case CPE_DR_TYPE_CHAR:
    case CPE_DR_TYPE_UCHAR:
    case CPE_DR_TYPE_STRING:
        return dr_bson_type_string;
    case CPE_DR_TYPE_UNION:
    case CPE_DR_TYPE_STRUCT:
        return dr_bson_type_embeded;
    case CPE_DR_TYPE_DATE:
    case CPE_DR_TYPE_TIME:
    case CPE_DR_TYPE_DATETIME:
    case CPE_DR_TYPE_MONEY:
    case CPE_DR_TYPE_IP:
    case CPE_DR_TYPE_VOID:
    default:
        return 0;
    }
}

int dr_bson_write(
    void * output, size_t output_capacity, uint8_t is_open,
    const void * input, size_t input_capacity, LPDRMETA meta,
    error_monitor_t em)
{
    struct dr_bson_write_stack processStack[CPE_DR_MAX_LEVEL];
    int stackPos;

    assert(output);
    assert(input);
    assert(meta);

    processStack[0].m_meta = meta;
    processStack[0].m_parent_entry = NULL;
    processStack[0].m_entry = dr_meta_entry_at(meta, 0);
    processStack[0].m_entry_pos = 0;
    processStack[0].m_entry_count = meta->m_entry_count;
    processStack[0].m_array_pos = 0;
    processStack[0].m_output_data = (unsigned char *)output;
    processStack[0].m_output_size = 0;
    processStack[0].m_output_capacity = output_capacity;
    processStack[0].m_array_begin_pos = 0;
    processStack[0].m_input_data = (char const *)input;
    processStack[0].m_input_data_capacity = input_capacity;
    processStack[0].m_is_open = is_open;

    for(stackPos = 0; stackPos >= 0;) {
        struct dr_bson_write_stack * curStack;

        assert(stackPos < CPE_DR_MAX_LEVEL);

        curStack = &processStack[stackPos];
        if (curStack->m_meta == NULL) {
            --stackPos;
            continue;
        }

        for(; curStack->m_entry_pos < curStack->m_entry_count
                && curStack->m_entry;
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

            refer = NULL;
            if (curStack->m_entry->m_array_count != 1) {
                refer = dr_entry_array_refer_entry(curStack->m_entry);
            }

            array_count = curStack->m_entry->m_array_count;
            if (refer) {
                dr_entry_try_read_int32(
                    &array_count,
                    curStack->m_input_data + curStack->m_entry->m_array_refer_data_start_pos,
                    refer,
                    em);
            }

            if (!curStack->m_is_open) { /*reserve for write size*/
                dr_bson_write_check_capacity(4);
                curStack->m_output_size += 4;
                curStack->m_is_open = 1;
            }

            if (curStack->m_array_pos == 0) {
                /*write type*/
                if (curStack->m_entry->m_array_count != 1) {
                    dr_bson_write_char(dr_bson_type_array);
                }
                else {
                    dr_bson_write_char(dr_bson_calc_bson_type(curStack->m_entry));
                }

                dr_bson_write_cstring(dr_entry_name(curStack->m_entry));
            }

            if (curStack->m_entry->m_array_count != 1 && curStack->m_array_pos == 0) {
                dr_bson_write_check_capacity(4);
                curStack->m_array_begin_pos = curStack->m_output_size;
                curStack->m_output_size += 4;
            }

            for(; curStack->m_array_pos < array_count; ++curStack->m_array_pos) {
                const char * entryData;

                entryData = curStack->m_input_data + dr_entry_data_start_pos(curStack->m_entry, curStack->m_array_pos);

                switch(curStack->m_entry->m_type) {
                case CPE_DR_TYPE_UNION:
                case CPE_DR_TYPE_STRUCT: {
                    if (curStack->m_entry->m_array_count != 1) {
                        dr_bson_write_char(dr_bson_type_embeded);
                        dr_bson_write_printf("%d", curStack->m_array_pos);
                    }

                    if (stackPos + 1 < CPE_DR_MAX_LEVEL) {
                        struct dr_bson_write_stack * nextStack;
                        nextStack = &processStack[stackPos + 1];
                        nextStack->m_meta = dr_entry_ref_meta(curStack->m_entry);
                        if (nextStack->m_meta == 0) break;
                        nextStack->m_parent_entry = curStack->m_entry;

                        nextStack->m_input_data = entryData;
                        nextStack->m_input_data_capacity = elementSize;

                        nextStack->m_output_data = curStack->m_output_data + curStack->m_output_size;
                        nextStack->m_output_size = 0;
                        nextStack->m_output_capacity = curStack->m_output_capacity - curStack->m_output_size;
                        nextStack->m_array_begin_pos = 0;

                        nextStack->m_entry_pos = 0;
                        nextStack->m_entry_count = nextStack->m_meta->m_entry_count;

                        nextStack->m_is_open = 0;

                        if (curStack->m_entry->m_type == CPE_DR_TYPE_UNION) {
                            LPDRMETAENTRY select_entry;
                            select_entry = dr_entry_select_entry(curStack->m_entry);
                            if (select_entry) {
                                int32_t union_entry_id;
                                dr_entry_try_read_int32(
                                    &union_entry_id,
                                    curStack->m_input_data + curStack->m_entry->m_select_data_start_pos,
                                    select_entry,
                                    em);
                                
                                nextStack->m_entry_pos =
                                    dr_meta_find_entry_idx_by_id(nextStack->m_meta, union_entry_id);
                                if (nextStack->m_entry_pos < 0) {
                                    dr_bson_write_int32(5);
                                    dr_bson_write_char(0);
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
                    break;
                }
                case CPE_DR_TYPE_CHAR:
                case CPE_DR_TYPE_INT8:
                case CPE_DR_TYPE_INT16:
                case CPE_DR_TYPE_INT32: {
                    int32_t value;
                    dr_entry_try_read_int32(&value, entryData, curStack->m_entry, NULL);

                    if (curStack->m_entry->m_array_count != 1) {
                        dr_bson_write_char(dr_bson_type_int32);
                        dr_bson_write_printf("%d", curStack->m_array_pos);
                    }

                    dr_bson_write_int32(value);
                    break;
                }
                case CPE_DR_TYPE_INT64: {
                    int64_t value;
                    dr_entry_try_read_int64(&value, entryData, curStack->m_entry, NULL);

                    if (curStack->m_entry->m_array_count != 1) {
                        dr_bson_write_char(dr_bson_type_int64);
                        dr_bson_write_printf("%d", curStack->m_array_pos);
                    }

                    dr_bson_write_int64(value);
                    break;
                }
                case CPE_DR_TYPE_UCHAR:
                case CPE_DR_TYPE_UINT8:
                case CPE_DR_TYPE_UINT16:
                case CPE_DR_TYPE_UINT32: {
                    uint32_t value;
                    dr_entry_try_read_uint32(&value, entryData, curStack->m_entry, NULL);

                    if (curStack->m_entry->m_array_count != 1) {
                        dr_bson_write_char(dr_bson_type_int32);
                        dr_bson_write_printf("%d", curStack->m_array_pos);
                    }

                    dr_bson_write_int32(value);
                    break;
                }
                case CPE_DR_TYPE_UINT64: {
                    uint64_t value;
                    dr_entry_try_read_uint64(&value, entryData, curStack->m_entry, NULL);

                    if (curStack->m_entry->m_array_count != 1) {
                        dr_bson_write_char(dr_bson_type_int64);
                        dr_bson_write_printf("%d", curStack->m_array_pos);
                    }

                    dr_bson_write_int64(value);
                    break;
                }
                case CPE_DR_TYPE_FLOAT:
                case CPE_DR_TYPE_DOUBLE: {
                    union {
                        double d;
                        int64_t i;
                    } value;
                    dr_entry_try_read_double(&value.d, entryData, curStack->m_entry, NULL);

                    if (curStack->m_entry->m_array_count != 1) {
                        dr_bson_write_char(dr_bson_type_double);
                        dr_bson_write_printf("%d", curStack->m_array_pos);
                    }

                    dr_bson_write_int64(value.i);
                    break;
                }
                case CPE_DR_TYPE_STRING: {
                    if (curStack->m_entry->m_array_count != 1) {
                        dr_bson_write_char(dr_bson_type_string);
                        dr_bson_write_printf("%d", curStack->m_array_pos);
                    }

                    dr_bson_write_string(entryData);
                    break;
                }
                case CPE_DR_TYPE_DATE:
                case CPE_DR_TYPE_TIME:
                case CPE_DR_TYPE_DATETIME:
                case CPE_DR_TYPE_MONEY:
                case CPE_DR_TYPE_IP:
                case CPE_DR_TYPE_VOID:
                default:
                    break;
                }
            }

            /*ENDENTRY:*/
            if (curStack->m_entry->m_array_count != 1 && curStack->m_array_pos >= array_count) {
                uint32_t array_size;

                dr_bson_write_char(0);

                array_size = curStack->m_output_size - curStack->m_array_begin_pos;
                BSON_COPY_HTON32(curStack->m_output_data + curStack->m_array_begin_pos, &array_size);
            }
        }

        if (stackPos == 0 && is_open) {
        }
        else {
            dr_bson_write_char(0);
            BSON_COPY_HTON32(curStack->m_output_data, &curStack->m_output_size);
        }

        if (--stackPos >= 0) {
            struct dr_bson_write_stack * preStack;
            preStack = &processStack[stackPos];
            preStack->m_output_size += curStack->m_output_size; 
        }
    }

    return (int)processStack[0].m_output_size;
}
