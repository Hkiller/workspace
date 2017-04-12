#include "render/utils/ui_rect.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_action.h"
#include "render/cache/ui_cache_res.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin_basicanim_actor_layer_i.h"

plugin_basicanim_actor_layer_t
plugin_basicanim_actor_layer_create(plugin_basicanim_module_t module, plugin_basicanim_actor_t obj, ui_data_actor_layer_t data_layer) {
    plugin_basicanim_actor_layer_t obj_layer;

    obj_layer = mem_alloc(module->m_alloc, sizeof(struct plugin_basicanim_actor_layer));
    if (obj_layer == NULL) {
        CPE_ERROR(module->m_em, "plugin_basicanim_actor_layer_create: alloc fail!");
        return NULL;
    }

    obj_layer->m_data_layer = data_layer;
    TAILQ_INSERT_TAIL(&obj->m_layers, obj_layer, m_next);

    plugin_basicanim_actor_layer_reset(module, obj, obj_layer);
    plugin_basicanim_actor_layer_update(module, obj, obj_layer, 0);

    return obj_layer;
}

void plugin_basicanim_actor_layer_free(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj, plugin_basicanim_actor_layer_t obj_layer)
{
    TAILQ_REMOVE(&obj->m_layers, obj_layer, m_next);
    mem_free(module->m_alloc, obj_layer);
}

void plugin_basicanim_actor_layer_reset(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj, plugin_basicanim_actor_layer_t obj_layer)
{
    ui_data_actor_frame_t data_frame;
    
    obj_layer->m_next_index = 0;
    obj_layer->m_cur_frame = NULL;

    data_frame = 
        obj_layer->m_next_index < ui_data_actor_layer_frame_count(obj_layer->m_data_layer)
        ? ui_data_actor_frame_get_at(obj_layer->m_data_layer, obj_layer->m_next_index)
        : NULL;
    obj_layer->m_next_frame = data_frame ? ui_data_actor_frame_data(data_frame) : NULL;
}

void plugin_basicanim_actor_layer_update(
    plugin_basicanim_module_t module, plugin_basicanim_actor_t obj,
    plugin_basicanim_actor_layer_t obj_layer, uint16_t frame)
{
    ui_data_actor_frame_t data_frame;

    while(obj_layer->m_next_frame && obj_layer->m_next_frame->start_frame <= frame) {
        obj_layer->m_cur_frame = obj_layer->m_next_frame;
        obj_layer->m_next_index++;

        data_frame = 
            obj_layer->m_next_index < ui_data_actor_layer_frame_count(obj_layer->m_data_layer)
            ? ui_data_actor_frame_get_at(obj_layer->m_data_layer, obj_layer->m_next_index)
            : NULL;
        
        obj_layer->m_next_frame = data_frame ? ui_data_actor_frame_data(data_frame) : NULL;

        if (obj_layer->m_cur_frame->particle[0]) {
            ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_from_data(obj);
            
            ui_runtime_render_obj_ref_t child_obj_ref =
                ui_runtime_render_obj_ref_create_by_res(ui_runtime_render_obj_module(render_obj), obj_layer->m_cur_frame->particle, NULL);
            if (child_obj_ref) ui_runtime_render_obj_add_child(render_obj, child_obj_ref, NULL, 1);
        }

        if (obj_layer->m_cur_frame->event[0]) {
            ui_runtime_render_obj_send_event(ui_runtime_render_obj_from_data(obj), obj_layer->m_cur_frame->event);
        }
        
        if (obj_layer->m_cur_frame->sound[0] && module->m_runtime) {
            ui_cache_res_t sound_res = ui_cache_res_find_by_path(ui_runtime_module_cache_mgr(module->m_runtime), obj_layer->m_cur_frame->sound);
            if (sound_res == NULL) {
                CPE_ERROR(module->m_em, "plugin_basicanim_actor: sound %s not exist!", obj_layer->m_cur_frame->sound);
            }
            else {
                ui_runtime_module_sound_play_by_res(module->m_runtime, "sfx", sound_res, 0);
            }
        }
    }
}
