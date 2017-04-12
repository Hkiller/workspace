#ifndef PLUGIN_BASICANIM_ACTOR_FRAME_H
#define PLUGIN_BASICANIM_ACTOR_FRAME_H
#include "render/utils/ui_transform.h"
#include "plugin_basicanim_actor_i.h"

struct plugin_basicanim_actor_layer {
    TAILQ_ENTRY(plugin_basicanim_actor_layer) m_next;
    ui_data_actor_layer_t m_data_layer;
    uint32_t m_next_index;
    UI_ACTOR_FRAME const * m_cur_frame;
    UI_ACTOR_FRAME const * m_next_frame;
};

plugin_basicanim_actor_layer_t
plugin_basicanim_actor_layer_create(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj,
    ui_data_actor_layer_t data_layer);

void plugin_basicanim_actor_layer_free(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj,
    plugin_basicanim_actor_layer_t actor_layer);

void plugin_basicanim_actor_layer_reset(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj,
    plugin_basicanim_actor_layer_t obj_layer);

void plugin_basicanim_actor_layer_update(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj,
    plugin_basicanim_actor_layer_t obj_layer, uint16_t frame);

#endif

