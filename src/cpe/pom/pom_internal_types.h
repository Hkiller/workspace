#ifndef CPE_POM_INTERNAL_TYPES_H
#define CPE_POM_INTERNAL_TYPES_H
#include "cpe/utils/hash.h"
#include "cpe/utils/error.h"
#include "cpe/pom/pom_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pom_class_mgr;

struct pom_buffer_mgr {
    pom_backend_t m_backend;
    void * m_backend_ctx;
    size_t m_page_size;
    size_t m_buf_size;
    struct cpe_urange_mgr m_free_pages;
    struct cpe_urange_mgr m_buffers;
    struct cpe_urange_mgr m_buffer_ids;
};

#define POM_CLASS_BUF_LEN (POM_MAX_TYPE_COUNT + 1)

struct pom_page;

struct pom_class {
    pom_class_id_t m_id;
    char m_name_buf[cpe_hs_len_to_binary_len(POM_MAX_TYPENAME_LEN)];
    cpe_hash_string_t m_name;
    struct cpe_hash_entry m_hh;
    struct cpe_urange_mgr m_urange_alloc;
    mem_allocrator_t m_alloc;
    size_t m_object_size;

    size_t m_page_size;
    size_t m_object_per_page;
    size_t m_alloc_buf_capacity;
    size_t m_object_buf_begin_in_page;

    size_t m_page_array_capacity;
    size_t m_page_array_size;
    void * * m_page_array;
};

struct pom_class_mgr {
    struct pom_class m_classes[POM_CLASS_BUF_LEN];
    struct cpe_hash_table m_classNameIdx;
};

struct pom_alloc_info {
    pom_oid_t m_oid;
    int m_stack_size;
    int m_free;
    struct cpe_hash_entry m_hh;
};

struct pom_debuger {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint32_t m_stack_size;
    struct cpe_hash_table m_alloc_infos;
};

struct pom_mgr {
    mem_allocrator_t m_alloc;
    struct pom_class_mgr m_classMgr;
    struct pom_buffer_mgr m_bufMgr;
    struct pom_debuger * m_debuger;
    int m_auto_validate;
};

#ifdef __cplusplus
}
#endif

#endif
