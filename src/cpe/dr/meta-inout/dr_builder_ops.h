#ifndef CPE_DR_METAINOUT_BUILDER_OPS_H
#define CPE_DR_METAINOUT_BUILDER_OPS_H
#include "dr_builder_types.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t dr_metalib_source_hash(dr_metalib_source_t source);
int dr_metalib_source_cmp(dr_metalib_source_t l, dr_metalib_source_t r);

void dr_metalib_source_analize_xml(
    dr_metalib_source_t source,
    struct DRInBuildMetaLib * inbuild_lib,
    const void * buf,
    int bufSize,
    error_monitor_t em);

uint32_t dr_metalib_source_element_hash(dr_metalib_source_element_t element);
int dr_metalib_source_element_cmp(dr_metalib_source_element_t l, dr_metalib_source_element_t r);

dr_metalib_source_element_t
dr_metalib_source_element_create(
    dr_metalib_source_t source,
    dr_metalib_source_element_type_t type,
    const char * name);

void dr_metalib_source_element_free(
    dr_metalib_source_element_t element);

#ifdef __cplusplus
}
#endif

#endif
