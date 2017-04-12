#ifndef CPE_POM_GRP_INTERNAL_TYPES_H
#define CPE_POM_GRP_INTERNAL_TYPES_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/bitarry.h"
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_types.h"
#include "cpe/pom_grp/pom_grp_types.h"
#include "cpe/utils/range.h"

#define POM_GRP_OBJ_CONTROL_MAGIC (38438u)

struct pom_grp_obj_mgr {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    pom_mgr_t m_omm;
    pom_grp_meta_t m_meta;
    LPDRMETALIB m_metalib;
    char * m_full_base;
    uint32_t m_full_capacity;
    int m_auto_validate;
};

struct pom_grp_meta {
    mem_allocrator_t m_alloc;
    const char * m_name;

    uint32_t m_omm_page_size;

    pom_class_id_t m_control_class_id;
    uint16_t m_control_obj_size;

    uint16_t m_page_count;
    uint16_t m_size_buf_start;
    uint16_t m_size_buf_count;

    uint16_t m_entry_count;
    uint16_t m_entry_capacity;
    struct pom_grp_entry_meta * * m_entry_buf;

    pom_grp_entry_meta_t m_main_entry;
    struct cpe_hash_table m_entry_ht;
};

struct pom_grp_entry_data_normal {
    LPDRMETA m_data_meta;
};

struct pom_grp_entry_data_list {
    LPDRMETA m_data_meta;
    uint32_t m_capacity;
    uint16_t m_size_idx;
    uint16_t m_standalone;
};

struct pom_grp_entry_data_ba {
    uint32_t m_bit_capacity;
};

struct pom_grp_entry_data_binary {
    uint32_t m_capacity;
};

union pom_grp_entry_data {
    struct pom_grp_entry_data_normal m_normal;
    struct pom_grp_entry_data_list m_list;
    struct pom_grp_entry_data_ba m_ba;
    struct pom_grp_entry_data_binary m_binary;
};

struct pom_grp_entry_meta {
    pom_grp_meta_t m_meta;
    uint16_t m_index;
    const char * m_name;
    pom_grp_entry_type_t m_type;
    union pom_grp_entry_data m_data;

    uint16_t m_page_begin;
    uint16_t m_page_count;
    pom_class_id_t m_class_id;
    uint16_t m_page_size;
    uint16_t m_obj_align;

    struct cpe_hash_entry m_hh;
};

extern cpe_hash_string_t pom_grp_control_class_name;

struct pom_grp_store {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    pom_grp_meta_t m_meta;
    pom_grp_store_table_t m_main_table;

    struct cpe_hash_table m_tables;
    struct cpe_hash_table m_entries;
};

TAILQ_HEAD(pom_grp_store_entry_list, pom_grp_store_entry);

struct pom_grp_store_table {
    pom_grp_store_t m_store;
    const char * m_name;
    LPDRMETA m_meta;

    struct pom_grp_store_entry_list m_entries;

    struct cpe_hash_entry m_hh;
};

struct pom_grp_store_entry {
    const char * m_name;
    pom_grp_store_table_t m_table;
    pom_grp_entry_meta_t m_entry_meta;

    struct cpe_hash_entry m_hh;
    TAILQ_ENTRY(pom_grp_store_entry) m_next;
};

#endif
