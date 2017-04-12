#ifndef RENDER_UTILS_STRING_TABLE_I_H
#define RENDER_UTILS_STRING_TABLE_I_H
#include "cpe/utils/buffer.h"
#include "render/utils/ui_string_table.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)
struct ui_env_strings_item {
    uint32_t m_msg_id;
    uint32_t m_msg_pos;
};

struct ui_env_strings_head {
    char m_magic[4];
    uint32_t m_record_count;
};
#pragma pack()

struct ui_string_table {
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    void * m_data; 
    uint32_t m_data_size;
    uint32_t m_record_count;
    struct ui_env_strings_item * m_items; 
    const char * m_strings;
    struct mem_buffer m_format_buf;
};

int ui_string_table_item_cmp(void const * l, void const * r);
int ui_string_table_load_update_data(ui_string_table_t string_table, const char * path);

#ifdef __cplusplus
}
#endif

#endif
