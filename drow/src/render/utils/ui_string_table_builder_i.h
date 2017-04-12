#ifndef RENDER_UTILS_STRING_TABLE_BUILDER_I_H
#define RENDER_UTILS_STRING_TABLE_BUILDER_I_H
#include "cpe/utils/hash.h"
#include "render/utils/ui_string_table_builder.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_string_table_builder_msg * ui_string_table_builder_msg_t;

struct ui_string_table_builder_msg {
    uint32_t m_id;
    struct cpe_hash_entry m_hh_for_id;
    struct cpe_hash_entry m_hh_for_str;
    const char * m_str;
    uint32_t m_ref_count;
};
    
struct ui_string_table_builder {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint32_t m_last_id;
    uint32_t m_record_count;
    struct cpe_hash_table m_msgs_by_id;
    struct cpe_hash_table m_msgs_by_str;
};

uint32_t ui_string_table_builder_str_hash(ui_string_table_builder_msg_t msg);
int ui_string_table_builder_str_eq(ui_string_table_builder_msg_t l, ui_string_table_builder_msg_t r);

uint32_t ui_string_table_builder_id_hash(ui_string_table_builder_msg_t msg);
int ui_string_table_builder_id_eq(ui_string_table_builder_msg_t l, ui_string_table_builder_msg_t r);
    
#ifdef __cplusplus
}
#endif

#endif
