#ifndef CPE_DR_METAINOUT_BUILDER_TYPES_H
#define CPE_DR_METAINOUT_BUILDER_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/dr/dr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct dr_metalib_source_relation;
typedef TAILQ_HEAD(dr_metalib_source_list, dr_metalib_source) dr_metalib_source_list_t;
typedef TAILQ_HEAD(dr_metalib_source_relation_list, dr_metalib_source_relation) dr_metalib_source_relation_list_t;
typedef TAILQ_HEAD(dr_metalib_source_element_list, dr_metalib_source_element) dr_metalib_source_element_list_t;

struct dr_metalib_builder {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;

    struct DRInBuildMetaLib * m_inbuild_lib;
    struct dr_metalib_source_list m_sources_in_order;
    struct cpe_hash_table m_sources;
    struct cpe_hash_table m_elements;
};

struct dr_metalib_source {
    dr_metalib_builder_t m_builder;
    const char * m_name;
    dr_metalib_source_type_t m_type;
    dr_metalib_source_format_t m_format;
    dr_metalib_source_from_t m_from;
    dr_metalib_source_state_t m_state;
    size_t m_capacity;

    dr_metalib_source_relation_list_t m_includes;
    dr_metalib_source_relation_list_t m_include_by;
    dr_metalib_source_element_list_t m_elements;

    TAILQ_ENTRY(dr_metalib_source) m_next;

    struct cpe_hash_entry m_hh;
};

struct dr_metalib_source_relation {
    dr_metalib_source_t m_user;
    dr_metalib_source_t m_using;
    TAILQ_ENTRY(dr_metalib_source_relation) m_next_for_includes;
    TAILQ_ENTRY(dr_metalib_source_relation) m_next_for_include_by;
};

struct dr_metalib_source_element {
    dr_metalib_source_t m_source;
    dr_metalib_source_element_type_t m_type;
    TAILQ_ENTRY(dr_metalib_source_element) m_next;

    struct cpe_hash_entry m_hh;
};

#ifdef __cplusplus
}
#endif

#endif


