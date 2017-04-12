#ifndef PLUGIN_BASICANIM_FRAME_H
#define PLUGIN_BASICANIM_FRAME_H
#include "render/model/ui_object_ref.h"
#include "plugin/basicanim/plugin_basicanim_frame.h"
#include "plugin_basicanim_module_i.h"

struct plugin_basicanim_frame {
    ui_data_frame_t m_frame;
};

int plugin_basicanim_frame_register(plugin_basicanim_module_t module);
void plugin_basicanim_frame_unregister(plugin_basicanim_module_t module);

#endif
