#ifndef PLUGIN_LAYOUT_LAYOUT_RICH_BLOCK_I_H
#define PLUGIN_LAYOUT_LAYOUT_RICH_BLOCK_I_H
#include "plugin/layout/plugin_layout_layout_rich_block.h"
#include "plugin_layout_layout_rich_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_layout_rich_block {
    plugin_layout_layout_rich_t m_rich;
    TAILQ_ENTRY(plugin_layout_layout_rich_block) m_next;

    plugin_layout_font_id_t m_font_id;
    plugin_layout_font_draw_t m_font_draw;
    const char * m_text_begin;
    const char * m_text_end;
    uint8_t m_text_own;
    
    struct plugin_layout_font_id m_buf_font_id;
    struct plugin_layout_font_draw m_buf_font_draw;
};

void plugin_layout_layout_rich_block_real_free(plugin_layout_layout_rich_block_t block);
    
#ifdef __cplusplus
}
#endif

#endif
