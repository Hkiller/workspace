#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "../dr_internal_types.h"

struct DRCtypeTypePrintOps {
    int (* printf_to_stream)(write_stream_t output, const void * input, LPDRMETAENTRY entry, error_monitor_t em);
};

int dr_printf_int8_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%d", *((int8_t*)data));
}
int dr_printf_int16_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%d", *((int16_t*)data));
}
int dr_printf_int32_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%d", *((int32_t*)data));
}
int dr_printf_int64_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%I64d", *((int64_t*)data));
}
int dr_printf_uint8_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%u", *((uint8_t*)data));
}
int dr_printf_uint16_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%u", *((uint16_t*)data));
}
int dr_printf_uint32_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%u", *((uint32_t*)data));
}
int dr_printf_uint64_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%I64u", *((uint64_t*)data));
}
int dr_printf_float_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%f", *((float*)data));
}
int dr_printf_double_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%f", *((double*)data));
}

int dr_printf_char_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    char tmp = *((char*)data);
    return stream_printf(stream, "%d", tmp);
}

int dr_printf_uchar_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    unsigned char tmp = *((unsigned char*)data);
    return stream_printf(stream, "%d", tmp);
}

int dr_printf_string_to_stream(write_stream_t stream, const void * data, LPDRMETAENTRY entry, error_monitor_t em) {
    return stream_printf(stream, "%s", (char*)data);
}

struct DRCtypeTypePrintOps g_dr_print_ops[] = {
    {/*CPE_DR_TYPE_UNION*/ NULL}
    , {/*CPE_DR_TYPE_STRUCT*/ NULL}
    , {/*CPE_DR_TYPE_CHAR*/ dr_printf_char_to_stream}
    , {/*CPE_DR_TYPE_UCHAR*/ dr_printf_uchar_to_stream}
    , {/*CPE_DR_TYPE_INT8*/ dr_printf_int8_to_stream}
    , {/*CPE_DR_TYPE_INT16*/ dr_printf_int16_to_stream}
    , {/*CPE_DR_TYPE_UINT16*/ dr_printf_uint16_to_stream}
    , {/*CPE_DR_TYPE_INT32*/ dr_printf_int32_to_stream}
    , {/*CPE_DR_TYPE_UINT32*/ dr_printf_uint32_to_stream}
    , {/*CPE_DR_TYPE_INT32*/ dr_printf_int32_to_stream}
    , {/*CPE_DR_TYPE_UINT32*/ dr_printf_uint32_to_stream}
    , {/*CPE_DR_TYPE_INT64*/ dr_printf_int64_to_stream}
    , {/*CPE_DR_TYPE_UINT64*/ dr_printf_uint64_to_stream}
    , {/*CPE_DR_TYPE_DATE*/ NULL}
    , {/*CPE_DR_TYPE_TIME*/ NULL}
    , {/*CPE_DR_TYPE_DATETIME*/ NULL}
    , {/*CPE_DR_TYPE_MONEY*/ NULL}
    , {/*CPE_DR_TYPE_FLOAT*/ dr_printf_float_to_stream}
    , {/*CPE_DR_TYPE_DOUBLE*/ dr_printf_double_to_stream}
    , {/*CPE_DR_TYPE_IP*/ NULL}
    , {/*CPE_DR_TYPE_WCHAR*/ dr_printf_char_to_stream}
    , {/*CPE_DR_TYPE_STRING*/ dr_printf_string_to_stream}
    , {/*CPE_DR_TYPE_STRING*/ dr_printf_string_to_stream}
    , {/*CPE_DR_TYPE_VOID*/ NULL}
    , {/*CPE_DR_TYPE_UINT8*/ dr_printf_uint8_to_stream}
};

int dr_entry_print_to_stream(write_stream_t output, const void * input, LPDRMETAENTRY entry, error_monitor_t em) {
    if (entry == NULL) {
        return -1;
    }

    if (entry->m_type < 0 || entry->m_type > sizeof(g_dr_print_ops) / sizeof(struct DRCtypeTypePrintOps) ) {
        CPE_ERROR(em, "print %d, type is unknown", entry->m_type);
        return -1;
    }

    if (g_dr_print_ops[entry->m_type].printf_to_stream) {
        return g_dr_print_ops[entry->m_type].printf_to_stream(output, input, entry, em);
    }
    else {
        CPE_ERROR(em, "print %d, type not support", entry->m_type);
        return -1;
    }
}

int dr_ctype_print_to_stream(write_stream_t output, const void * input, int type, error_monitor_t em) {
    if (type < 0 || type > sizeof(g_dr_print_ops) / sizeof(struct DRCtypeTypePrintOps) ) {
        return -1;
    }

    if (g_dr_print_ops[type].printf_to_stream) {
        return g_dr_print_ops[type].printf_to_stream(output, input, NULL, em);
    }
    else {
        return -1;
    }
}

const char * dr_ctype_to_string(mem_buffer_t buffer, const void * input, int type) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    dr_ctype_print_to_stream((write_stream_t)&stream, input, type, NULL);

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}

const char * dr_entry_to_string(mem_buffer_t buffer, const void * input, LPDRMETAENTRY entry) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);

    mem_buffer_clear_data(buffer);

    dr_entry_print_to_stream((write_stream_t)&stream, input, entry, NULL);

    stream_putc((write_stream_t)&stream, 0);

    return mem_buffer_make_continuous(buffer, 0);
}
