#ifndef PLUGIN_LAYOUT_FONT_META_I_H
#define PLUGIN_LAYOUT_FONT_META_I_H
#include "plugin/layout/plugin_layout_font_meta.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_font_meta {
    plugin_layout_module_t m_module;
    TAILQ_ENTRY(plugin_layout_font_meta) m_next;
    plugin_layout_font_face_list_t m_faces;
    plugin_layout_font_category_t m_category;
    char m_name[32];
    void * m_ctx;
    uint32_t m_meta_capacity;
    plugin_layout_font_meta_init_fun_t m_init_meta;
    plugin_layout_font_meta_fini_fun_t m_fini_meta;
    plugin_layout_font_meta_on_cache_clear_fun_t m_on_cache_clear;
    uint32_t m_face_capacity;
    plugin_layout_font_face_init_fun_t m_init_face;
    plugin_layout_font_face_fini_fun_t m_fini_face;
    uint32_t m_element_capacity;
    plugin_layout_font_element_init_fun_t m_init_element;
    plugin_layout_font_element_fini_fun_t m_fini_element;
    plugin_layout_font_element_render_fun_t m_render_element;
    plugin_layout_font_meta_basic_layout_fun_t m_basic_layout;
};

#ifdef __cplusplus
}
#endif

#endif
