#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/buffer.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/particle/plugin_particle_obj_plugin_data.h"
#include "plugin/particle/plugin_particle_obj_plugin.h"
#include "plugin/particle/plugin_particle_obj_particle.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_layer.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_particle_gen_entity_slot_i.h"
#include "ui_sprite_particle_gen_entity_i.h"
#include "ui_sprite_particle_controled_obj_i.h"

int ui_sprite_particle_gen_entity_slot_init(void * ctx, plugin_particle_obj_plugin_data_t data) {
    ui_sprite_particle_gen_entity_t gen_entity = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(gen_entity));
    ui_sprite_particle_gen_entity_slot_t slot = (ui_sprite_particle_gen_entity_slot_t)plugin_particle_obj_plugin_data_data(data);
    plugin_particle_obj_emitter_t emitter = plugin_particle_obj_plugin_emitter(plugin_particle_obj_plugin_data_plugin(data));
    const char * emitter_name = plugin_particle_obj_emitter_name(emitter);
    const char * proto_name;
    ui_sprite_entity_t generated;
    ui_sprite_render_sch_t generated_render_sch;
    ui_sprite_particle_controled_obj_t obj;
    const char * args_begin;
    const char * args_end;
    char name_buf[64];

    if (gen_entity->m_cfg_prefix) {
        emitter_name += strlen(gen_entity->m_cfg_prefix);
    }

    if ((args_begin = strchr(emitter_name, '['))) {
        args_end = strchr(args_begin + 1, ']');
        if (args_end == NULL) {
            CPE_ERROR(
                gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: emitter name %s format error!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), emitter_name);
            return -1;
        }

        proto_name = cpe_str_dup_range(name_buf, sizeof(name_buf), emitter_name, args_begin);
        args_begin += 1;
    }
    else {
        proto_name = emitter_name;
    }
    
    slot->m_gen_entity = gen_entity;
    slot->m_controled_obj = NULL;

    /*开始创建对象 */
    generated = ui_sprite_entity_create(ui_sprite_entity_world(entity), "", proto_name);
    if (generated == NULL) {
        CPE_ERROR(
            gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: create from proto %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), proto_name);
        return -1;
    }

    obj = ui_sprite_particle_controled_obj_find(generated);
    if (obj == NULL) {
        obj = ui_sprite_particle_controled_obj_create(generated);
        if (obj == NULL) {
            CPE_ERROR(
                gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: create controled obj fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_entity_free(generated);
            return -1;
        }
    }

    generated_render_sch = ui_sprite_render_sch_find(generated);
    if (generated_render_sch) {
        char layer[32];

        if (args_begin && cpe_str_read_arg_range(layer, sizeof(layer), args_begin, args_end, "layer", ',', '=') == 0) {
            if (ui_sprite_render_sch_set_default_layer_by_name(generated_render_sch, layer) != 0) {
                CPE_ERROR(
                    gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: enter generated %d(%s) set default layer %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    ui_sprite_entity_id(generated), ui_sprite_entity_name(generated), layer);
                ui_sprite_entity_free(generated);
                return -1;
            }
        }
        else {
            /*根据当前的等级设置 */
            ui_sprite_render_sch_t self_render_sch;

            self_render_sch = ui_sprite_render_sch_find(entity);
            if (self_render_sch) {
                ui_sprite_render_anim_t self_anim;
                const char * layer;
                
                self_anim = ui_sprite_render_anim_find_by_render_obj(
                    self_render_sch,
                    ui_runtime_render_obj_from_data(plugin_particle_obj_emitter_obj(emitter)));
                if (self_anim == NULL) {
                    CPE_ERROR(
                        gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: enter generated %d(%s) fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                        ui_sprite_entity_id(generated), ui_sprite_entity_name(generated));
                    ui_sprite_entity_free(generated);
                    return -1;
                }

                layer = ui_sprite_render_layer_name(ui_sprite_render_anim_layer(self_anim));
                if (ui_sprite_render_sch_set_default_layer_by_name(generated_render_sch, layer) != 0) {
                    CPE_ERROR(
                        gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: enter generated %d(%s) set default layer %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                        ui_sprite_entity_id(generated), ui_sprite_entity_name(generated), layer);
                    ui_sprite_entity_free(generated);
                    return -1;
                }
            }
        }
    }
    
    if (ui_sprite_entity_enter(generated) != 0) {
        CPE_ERROR(
            gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: enter generated %d(%s) fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(generated), ui_sprite_entity_name(generated));
        ui_sprite_entity_free(generated);
        return -1;
    }

    if (generated_render_sch) {
        if (args_begin) {
            char * buf = cpe_str_mem_dup_range(gen_entity->m_module->m_alloc, args_begin, args_end);
            if (buf == NULL) {
                CPE_ERROR(
                    gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: alloc setup buf fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            }
            else {
                ui_sprite_render_sch_setup(generated_render_sch, buf);
                mem_free(gen_entity->m_module->m_alloc, buf);
            }
        }
    }
    
    ui_sprite_particle_controled_obj_set_slot(obj, slot);
    assert(slot->m_controled_obj == obj);
    assert(obj->m_slot == slot);
    
    return 0;
}

void ui_sprite_particle_gen_entity_slot_fini(void * ctx, plugin_particle_obj_plugin_data_t data) {
    ui_sprite_particle_gen_entity_slot_t slot = (ui_sprite_particle_gen_entity_slot_t)plugin_particle_obj_plugin_data_data(data);

    if (slot->m_controled_obj) {
        assert(slot->m_controled_obj->m_slot == slot);
        
        slot->m_controled_obj->m_remove_particle = 0;
        ui_sprite_entity_set_destory(ui_sprite_component_entity(ui_sprite_component_from_data(slot->m_controled_obj)));
        slot->m_controled_obj->m_slot = NULL;
        slot->m_controled_obj = NULL;
    }
}

void ui_sprite_particle_gen_entity_slot_update(void * ctx, plugin_particle_obj_plugin_data_t data) {
    ui_sprite_particle_gen_entity_t gen_entity = ctx;
    ui_sprite_particle_gen_entity_slot_t slot = plugin_particle_obj_plugin_data_data(data);
    plugin_particle_obj_particle_t particle;
    ui_sprite_entity_t controled;
    struct ui_transform particle_transform;
    ui_sprite_2d_transform_t p2d_trans;

    if (slot->m_controled_obj == NULL || !slot->m_controled_obj->m_is_binding) return;

    controled = ui_sprite_component_entity(ui_sprite_component_from_data(slot->m_controled_obj));
    p2d_trans = ui_sprite_2d_transform_find(controled);
    if (p2d_trans == NULL) {
        ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(gen_entity));
        CPE_ERROR(
            gen_entity->m_module->m_em, "entity %d(%s): particle gen entity: controled %d(%s) no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_entity_id(controled), ui_sprite_entity_name(controled));
        return;
    }
    
    particle = plugin_particle_obj_plugin_data_particle(data);
    plugin_particle_obj_particle_calc_transform(particle, &particle_transform);

    if (!slot->m_controled_obj->m_accept_scale) {
        ui_transform_set_scale(&particle_transform, &UI_VECTOR_3_IDENTITY);
    }

    if (!slot->m_controled_obj->m_accept_angle) {
        ui_transform_set_quation(&particle_transform, &UI_QUATERNION_IDENTITY);
    }
    
    ui_sprite_2d_transform_set_trans(p2d_trans, &particle_transform);
}

void ui_sprite_particle_gen_entity_slot_free(ui_sprite_particle_gen_entity_slot_t slot) {
    plugin_particle_obj_plugin_data_t plugin_data = plugin_particle_obj_plugin_data_from_data((void*)slot);
    plugin_particle_obj_particle_t particle = plugin_particle_obj_plugin_data_particle(plugin_data);

    plugin_particle_obj_particle_free(particle);
}
