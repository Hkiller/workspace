#ifndef CPE_DR_XML_H
#define CPE_DR_XML_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/error.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DR_XML_PRINT_BEAUTIFY 0x01

int dr_xml_read(
    void * result,
    size_t capacity,
    const char * input,
    LPDRMETA meta,
    error_monitor_t em);

int dr_xml_read_to_buffer(
    struct mem_buffer * result, 
    const char * input,
    LPDRMETA meta,
    error_monitor_t em);

int dr_xml_print(
    write_stream_t output,
    const void * input,
    size_t capacity,
    LPDRMETA meta,
    int flag,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
