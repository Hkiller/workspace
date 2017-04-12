#ifndef PLUGIN_LAYOUT_ANIMATION_CARET_I_H
#define PLUGIN_LAYOUT_ANIMATION_CARET_I_H
#include "render/utils/ui_percent_decorator.h"
#include "plugin/layout/plugin_layout_animation_caret.h"
#include "plugin_layout_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_layout_animation_caret {
    int m_pos;
    ui_color m_color;
    uint8_t m_is_visiable;
	uint8_t	m_is_show;
	float m_update_time;
};

int plugin_layout_animation_caret_regist(plugin_layout_module_t module);
void plugin_layout_animation_caret_unregist(plugin_layout_module_t module);

#ifdef __cplusplus
}
#endif

#endif
