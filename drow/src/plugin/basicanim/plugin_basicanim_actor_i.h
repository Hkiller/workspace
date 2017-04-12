#ifndef PLUGIN_BASICANIM_ACTOR_H
#define PLUGIN_BASICANIM_ACTOR_H
#include "render/model/ui_object_ref.h"
#include "plugin/basicanim/plugin_basicanim_actor.h"
#include "plugin_basicanim_module_i.h"

struct plugin_basicanim_actor {
    ui_data_actor_t m_actor;
    float m_frame_time;
    uint16_t m_total_frame;
    uint16_t m_loop_start_frame;
    uint16_t m_loop_count;
    float m_runing_time;
    uint16_t m_cur_frame;
    uint8_t m_is_runing;
    plugin_basicanim_actor_layer_list_t m_layers;
};

int plugin_basicanim_actor_register(plugin_basicanim_module_t module);
void plugin_basicanim_actor_unregister(plugin_basicanim_module_t module);

#endif
