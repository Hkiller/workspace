#ifndef PLUGIN_BASICANIM_COLOR_I_H
#define PLUGIN_BASICANIM_COLOR_I_H
#include "render/utils/ui_color.h"
#include "render/utils/ui_rect.h"
#include "plugin/basicanim/plugin_basicanim_color.h"
#include "plugin_basicanim_module_i.h"

struct plugin_basicanim_color {
    uint8_t m_have_rect;
    struct ui_rect m_rect;
    uint8_t m_have_color;
    struct ui_color m_color;
};

int plugin_basicanim_color_register(plugin_basicanim_module_t module);
void plugin_basicanim_color_unregister(plugin_basicanim_module_t module);

#endif
