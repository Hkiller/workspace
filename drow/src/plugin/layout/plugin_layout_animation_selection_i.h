#ifndef PLUGIN_LAYOUT_ANIMATION_SELECTION_I_H
#define PLUGIN_LAYOUT_ANIMATION_SELECTION_I_H
#include "render/utils/ui_vector_2.h"
#include "plugin/layout/plugin_layout_animation_selection.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_animation_selection {
    plugin_layout_animation_selection_type_t m_type;
    ui_color m_color;
    ui_vector_2 m_begin_pt;
    ui_vector_2 m_end_pt;
    int m_begin_pos;
    int m_end_pos;
};

int plugin_layout_animation_selection_regist(plugin_layout_module_t module);
void plugin_layout_animation_selection_unregist(plugin_layout_module_t module);

#ifdef __cplusplus
}
#endif

#endif
