#ifndef CPE_DR_BUILD_INBUILD_TYPES_H
#define CPE_DR_BUILD_INBUILD_TYPES_H
#include "cpe/utils/buffer.h"
#include "cpe/utils/hash.h"
#include "../dr_internal_types.h"
#include "cpe/pal/pal_queue.h"

struct DRInBuildMacro;
struct DRInBuildMeta;
struct dr_inbuild_key_entry;
struct dr_inbuild_index;
struct dr_inbuild_index_entry;

struct DRInBuildMetaLib {
    struct tagDRLibParam m_data;
    struct mem_buffer m_tmp_buf;
    uint8_t m_dft_align;

    TAILQ_HEAD(DRInBuildMacroList, DRInBuildMacro) m_macros;
    struct cpe_hash_table m_index_macros;

    TAILQ_HEAD(DRInBuildMetaList, DRInBuildMeta) m_metas;
    struct cpe_hash_table m_index_metas;
};

struct DRInBuildMeta {
    struct DRInBuildMetaLib * m_lib;
    TAILQ_ENTRY(DRInBuildMeta) m_next;
    TAILQ_HEAD(DRInBuildMetaEntryList, DRInBuildMetaEntry) m_entries;

    int m_key_entrie_count;
    TAILQ_HEAD(dr_inbuild_key_entry_list, dr_inbuild_key_entry) m_key_entries;

    int m_index_count;
    TAILQ_HEAD(dr_inbuild_index_list, dr_inbuild_index) m_indexes;
    
    struct cpe_hash_entry m_hh;
    uint8_t m_processed;
    int m_entries_count;

    struct tagDRMeta m_data;
    char const * m_desc;
    char const * m_name;
};

struct dr_inbuild_key_entry {
    TAILQ_ENTRY(dr_inbuild_key_entry) m_next;
    const char * m_entry_name;
};

struct dr_inbuild_index {
    TAILQ_ENTRY(dr_inbuild_index) m_next;
    struct DRInBuildMeta * m_meta;
    const char * m_index_name;
    int m_entry_count;
    TAILQ_HEAD(dr_inbuild_index_entry_list, dr_inbuild_index_entry) m_entries;
};

struct dr_inbuild_index_entry {
    TAILQ_ENTRY(dr_inbuild_index_entry) m_next;
    const char * m_entry_name;
};

struct DRInBuildMetaEntry {
    TAILQ_ENTRY(DRInBuildMetaEntry) m_next;

    struct DRInBuildMeta * m_meta;

    struct tagDRMetaEntry m_data;
    int m_ignore;
    char const * m_name;
    char const * m_desc;
    char const * m_cname;
    char const * m_ref_type_name;
    char const * m_selector_path;
    char const * m_refer_path;
    char const * m_dft_value;
};

struct DRInBuildMacro {
    TAILQ_ENTRY(DRInBuildMacro) m_next;
    struct cpe_hash_entry m_hh;

    struct tagDRMacro m_data;
    char * m_name;
    char * m_desc;
};

#endif


