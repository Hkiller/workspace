#ifndef CPE_DR_METALIB_BUILDER_H
#define CPE_DR_METALIB_BUILDER_H
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*builder operations*/
dr_metalib_builder_t dr_metalib_builder_create(mem_allocrator_t alloc, error_monitor_t em);
void dr_metalib_builder_free(dr_metalib_builder_t builder);
void dr_metalib_builder_analize(dr_metalib_builder_t builder);
struct DRInBuildMetaLib * dr_metalib_bilder_lib(dr_metalib_builder_t builder);
void dr_metalib_builder_sources(struct dr_metalib_source_it * it, dr_metalib_builder_t builder);

dr_metalib_source_t dr_metalib_builder_add_file(dr_metalib_builder_t builder, const char * name, const char * file);
dr_metalib_source_t dr_metalib_builder_add_buf(dr_metalib_builder_t builder, const char * name, dr_metalib_source_format_t format, const char * buf);
void dr_metalib_source_free(dr_metalib_source_t source);

/*source operations*/
const char * dr_metalib_source_name(dr_metalib_source_t source);
const char * dr_metalib_source_file(dr_metalib_source_t source);
const void * dr_metalib_source_buf(dr_metalib_source_t source);
size_t dr_metalib_source_buf_capacity(dr_metalib_source_t source);

const char * dr_metalib_source_libname(dr_metalib_source_t source);

dr_metalib_source_type_t dr_metalib_source_type(dr_metalib_source_t source);
dr_metalib_source_format_t dr_metalib_source_format(dr_metalib_source_t source);
dr_metalib_source_from_t dr_metalib_source_from(dr_metalib_source_t source);
dr_metalib_source_state_t dr_metalib_source_state(dr_metalib_source_t source);
void dr_metalib_source_analize(dr_metalib_source_t source);
void dr_metalib_source_includes(struct dr_metalib_source_it * it, dr_metalib_source_t source);
void dr_metalib_source_include_by(struct dr_metalib_source_it * it, dr_metalib_source_t source);

dr_metalib_source_t dr_metalib_source_find(dr_metalib_builder_t builder, const char * name);

int dr_metalib_source_add_include(dr_metalib_source_t user_source, dr_metalib_source_t using_source);

dr_metalib_source_t
dr_metalib_source_add_include_file(
    dr_metalib_source_t user_source,
    const char * name, 
    const char * file,
    dr_metalib_source_from_t from);

#define dr_metalib_source_next(it) ((it)->next ? (it)->next(it) : NULL)

/*element operations*/
const char * dr_metalib_source_element_name(dr_metalib_source_element_t element);
dr_metalib_source_element_type_t
dr_metalib_source_element_type(dr_metalib_source_element_t element);
void dr_metalib_source_elements(dr_metalib_source_element_it_t it, dr_metalib_source_t using_source);

#define dr_metalib_source_element_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
