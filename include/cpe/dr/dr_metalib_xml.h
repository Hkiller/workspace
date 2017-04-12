#ifndef CPE_DR_XML_METALIB_H
#define CPE_DR_XML_METALIB_H
#include <stdio.h>
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_define.h"
#include "cpe/dr/dr_external.h"
#include "cpe/dr/dr_metalib_build.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

CPE_DR_API int dr_create_lib_from_xml(
    mem_buffer_t buffer,
    const char* buf, int bufSize,
    uint8_t dft_align,
    FILE* errorFp);

CPE_DR_API int dr_create_lib_from_xml_ex(
    mem_buffer_t buffer,
    const char* buf, int bufSize,
    uint8_t dft_align,
    error_monitor_t er);

CPE_DR_API int dr_save_lib_to_xml_file(
    LPDRMETALIB metaLib,
    const char * fileName,
    error_monitor_t em);

CPE_DR_API char * dr_save_lib_to_xml_buf(
    mem_buffer_t buffer,
    LPDRMETALIB metaLib,
    error_monitor_t em);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
