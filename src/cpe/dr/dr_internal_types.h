#ifndef CPE_DR_INTERNALS_TYPES_H
#define CPE_DR_INTERNALS_TYPES_H
#include "cpe/pal/pal_stdio.h"
#include "cpe/dr/dr_types.h"
#include "cpe/dr/dr_ctypes_info.h"

#pragma pack(push,1)

struct tagDRMetaLib {
    int16_t m_magic;
    int16_t m_build_version;
    int32_t m_size;
    int32_t m_id;
    int32_t m_tag_set_version;
    int32_t m_meta_max_count;
    int32_t m_meta_count;
    int32_t m_macro_max_count;
    int32_t m_macro_count;
    int32_t m_macrogroup_max_count;
    int32_t m_macrogroup_count;
    int32_t m_version;
    int32_t m_startpos_macro;
    int32_t m_startpos_meta_by_id;
    int32_t m_startpos_meta_by_name;
    int32_t m_startpos_meta_by_orig; /*what is this index?*/
    int32_t m_startpos_meta;
    int32_t m_startpos_str;
    int32_t m_buf_size_str;
    int32_t m_buf_size_meta;
    int32_t m_buf_size_macros;
    char m_name[CPE_DR_NAME_LEN];
};

struct tagDRMeta {
    int32_t m_id;
    int32_t m_based_version;
    int32_t m_current_version;
    int32_t m_type;
    int32_t m_meta_size;
    int32_t m_real_data_size;
    uint32_t m_entry_count;
    uint32_t m_index_count;
    int32_t m_index_pos_from_meta;
    int32_t m_index_entry_pos_from_meta;
    int32_t m_self_pos;
    int32_t m_align;
    int32_t m_require_align;
    int32_t m_name_pos;
    int32_t m_desc_pos;
    uint16_t m_key_num;
    int32_t m_key_start_from_meta;
};

struct tagDRMetaEntry {
    int32_t m_id;
    int32_t m_version;
    int32_t m_type;
    int32_t m_name_pos;
    int32_t m_unitsize;
    int32_t m_size;
    int32_t m_array_count;
    int32_t m_data_start_pos_3; /*same as start pos?*/
    int32_t m_data_start_pos;
    int32_t m_array_refer_data_start_pos;
    int32_t m_array_refer_entry_pos;
    int32_t m_select_data_start_pos;
    int32_t m_select_entry_pos;
    int32_t m_ref_type_pos;
    int32_t m_select_range_max;
    int32_t m_select_range_min;
    int32_t m_desc_pos;
    int32_t m_cname_pos;
    int32_t m_dft_value_pos;
    int32_t m_self_to_meta_pos;
};

struct tagDRMacro {
    int32_t m_name_pos;
    int32_t m_value;
    int32_t m_desc_pos;
};

struct tagDRMacrosGroup {
    int8_t a;
};

struct dr_index_info {
    int32_t m_name_pos;
    int32_t m_diff_to_meta;
    int32_t m_entry_num;
    int32_t m_entry_start_pos_to_meta;
};

struct dr_index_entry_info {
    int32_t m_data_start_pos;
    int32_t m_entry_diff_to_base;
};

struct idx_meta_by_id {
    int32_t m_id;
    int32_t m_diff_to_base;
};

struct idx_meta_by_name {
    int32_t m_name_pos;
    int32_t m_diff_to_base;
};

struct idx_meta_by_orig {
    int32_t m_diff_to_base;
};

struct dr_idx_entry_info {
    int32_t m_data_start_pos;
    int32_t m_entry_diff_to_base;
};

#pragma pack(pop)

#endif
