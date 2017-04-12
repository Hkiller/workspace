#ifndef PLUGIN_MASK_DATA_BLOCK_I_H
#define PLUGIN_MASK_DATA_BLOCK_I_H
#include "plugin_mask_data_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_mask_data_block {
    plugin_mask_data_t m_data;
    TAILQ_ENTRY(plugin_mask_data_block) m_next;
    uint32_t m_name;
    int32_t m_x;
    int32_t m_y;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_buf_x;
    uint32_t m_buf_y;
    uint32_t m_buf_width;
    uint32_t m_buf_height;
    void * m_buf;
    size_t m_buf_size;
};

void plugin_mask_data_block_real_free(plugin_mask_data_block_t block);
    
#ifdef __cplusplus
}
#endif

#endif
