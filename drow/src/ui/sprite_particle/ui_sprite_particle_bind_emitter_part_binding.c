#include "cpe/utils/math_ex.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui/sprite_2d/ui_sprite_2d_part_attr.h"
#include "ui/sprite_2d/ui_sprite_2d_part_binding.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "ui_sprite_particle_bind_emitter_part_binding_i.h"

static void ui_sprite_particle_bind_emitter_part_binding_sync(ui_sprite_particle_bind_emitter_part_binding_t binding, uint8_t force);
static void ui_sprite_particle_bind_emitter_part_binding_on_update(ui_sprite_2d_part_t part, void * ctx);

ui_sprite_particle_bind_emitter_part_binding_t
ui_sprite_particle_bind_emitter_part_binding_create(
    ui_sprite_particle_bind_emitter_part_t bind_emitter_part, ui_sprite_2d_part_t part, plugin_particle_obj_emitter_t emitter)
{
    ui_sprite_particle_module_t module = bind_emitter_part->m_module;
    ui_sprite_particle_bind_emitter_part_binding_t binding;

    binding = TAILQ_FIRST(&module->m_free_bind_emitter_part_binding);
    if (binding) {
        TAILQ_REMOVE(&module->m_free_bind_emitter_part_binding, binding, m_next);
    }
    else {
        binding = mem_alloc(module->m_alloc, sizeof(struct ui_sprite_particle_bind_emitter_part_binding));
        if (binding == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_particle_bind_emitter_part_binding_create: alloc fail!");
            return NULL;
        }
    }

    binding->m_owner = bind_emitter_part;
    binding->m_part = part;
    binding->m_emitter = emitter;
    binding->m_accept_flip = bind_emitter_part->m_cfg_accept_flip;
    binding->m_accept_scale = bind_emitter_part->m_cfg_accept_scale;
    binding->m_accept_angle = bind_emitter_part->m_cfg_accept_angle;
    binding->m_angle_adj_rad = bind_emitter_part->m_angle_adj_rad;

    if (ui_sprite_2d_part_binding_create(part, binding, ui_sprite_particle_bind_emitter_part_binding_on_update) == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_bind_emitter_part_binding_create: alloc fail!");
        return NULL;
    }
    ui_sprite_particle_bind_emitter_part_binding_sync(binding, 1);
    
    TAILQ_INSERT_TAIL(&bind_emitter_part->m_bindings, binding, m_next);

    return binding;
}

void ui_sprite_particle_bind_emitter_part_binding_free(ui_sprite_particle_bind_emitter_part_binding_t binding) {
    ui_sprite_particle_bind_emitter_part_t bind_emitter_part = binding->m_owner;
    ui_sprite_particle_module_t module = bind_emitter_part->m_module;

    ui_sprite_2d_part_remove_bindings(binding->m_part, binding);
    
    TAILQ_REMOVE(&bind_emitter_part->m_bindings, binding, m_next);

    binding->m_owner = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_bind_emitter_part_binding, binding, m_next);
}

void ui_sprite_particle_bind_emitter_part_binding_real_free(ui_sprite_particle_bind_emitter_part_binding_t binding) {
    ui_sprite_particle_module_t module = (void*)binding->m_owner;

    TAILQ_REMOVE(&module->m_free_bind_emitter_part_binding, binding, m_next);

    mem_free(module->m_alloc, binding);
}

void ui_sprite_paritcle_bind_emitter_part_binding_update(
    ui_sprite_particle_bind_emitter_part_binding_t binding, ui_transform_t anim_l_r)
{
    ui_transform part_trans = *ui_sprite_2d_part_trans(binding->m_part);
    //ui_transform emitter_trans;
    ui_vector_2 pos;
    ui_vector_3 s;

    assert(anim_l_r);
    
    ui_transform_assert_sane(&part_trans);

    /* struct mem_buffer buff; */
    /* mem_buffer_init(&buff, NULL); */
    /* printf( */
    /*     "    entity %d(%s): emitter binding update: part %s: accept_angle=%d, trans=%s\n", */
    /*     ui_sprite_entity_id(ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(binding->m_owner))), */
    /*     ui_sprite_entity_name(ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(binding->m_owner))), */
    /*     ui_sprite_2d_part_name(binding->m_part), */
    /*     binding->m_accept_angle, */
    /*     anim_l_r ? ui_transform_dump_2d(&buff, anim_l_r) : "null"); */
    /* mem_buffer_clear(&buff); */

    ui_transform_get_pos_2(&part_trans, &pos);
    ui_transform_inline_adj_vector_2_no_t(anim_l_r, &pos);
    ui_transform_set_pos_2(&part_trans, &pos);
    
    if (!binding->m_accept_angle) {
        if (binding->m_angle_adj_rad) {
            ui_quaternion q;
            ui_quaternion_set_z_radians(&q, binding->m_angle_adj_rad);
            ui_transform_set_quation(&part_trans, &q);
            ui_transform_assert_sane(&part_trans);
        }
        else {
            ui_transform_set_quation(&part_trans, &UI_QUATERNION_IDENTITY);
            ui_transform_assert_sane(&part_trans);
        }
    }
    else {
        if (binding->m_angle_adj_rad) {
            float angle_rad = ui_transform_calc_angle_z_rad(&part_trans);
            ui_quaternion q;
            uint8_t flip_count = 0;
            
            if (!binding->m_accept_flip) {
                if (part_trans.m_s.x < 0) flip_count++;
                if (part_trans.m_s.y < 0) flip_count++;
            }
            
            angle_rad +=
                (flip_count % 2)
                ? - binding->m_angle_adj_rad
                : + binding->m_angle_adj_rad;
            
            ui_quaternion_set_z_radians(&q, angle_rad);
            ui_transform_set_quation(&part_trans, &q);
            ui_transform_assert_sane(&part_trans);
        }
    }
    
    if (!binding->m_accept_scale) {
        if(!binding->m_accept_flip) {
            s = UI_VECTOR_3_IDENTITY;
        }
        else {
            s.x = part_trans.m_s.x < 0.0f ? -1.0f : 1.0f;
            s.y = part_trans.m_s.y < 0.0f ? -1.0f : 1.0f;
            s.z = part_trans.m_s.z < 0.0f ? -1.0f : 1.0f;
        }
    }
    else {
        s = part_trans.m_s;
    }
        
    if(!binding->m_accept_flip) {
        s.x = fabs(s.x);
        s.y = fabs(s.y);
        s.z = fabs(s.z);
    }

    ui_transform_set_scale(&part_trans, &s);
    ui_transform_assert_sane(&part_trans);

    plugin_particle_obj_emitter_set_transform(binding->m_emitter, &part_trans);
}

static void ui_sprite_particle_bind_emitter_part_binding_sync(ui_sprite_particle_bind_emitter_part_binding_t binding, uint8_t force) {
    ui_sprite_2d_part_attr_t attr;
    const char * str_value;

    if ((attr = ui_sprite_2d_part_attr_find(binding->m_part, "enable")) && (force || ui_sprite_2d_part_attr_is_value_changed(attr))) {
        str_value = ui_sprite_2d_part_attr_value(attr);
        
        if (atoi(str_value)) {
            switch(plugin_particle_obj_emitter_use_state(binding->m_emitter)) {
            case plugin_particle_obj_emitter_use_state_suspend:
                plugin_particle_obj_emitter_set_use_state(binding->m_emitter, plugin_particle_obj_emitter_use_state_active);
                break;
            case plugin_particle_obj_emitter_use_state_active:
                if (plugin_particle_obj_emitter_is_closing(binding->m_emitter)) {
                    plugin_particle_obj_emitter_set_close(binding->m_emitter, 0);
                }
                plugin_particle_obj_emitter_spawn(binding->m_emitter, 0);
                break;
            case plugin_particle_obj_emitter_use_state_passive:
                plugin_particle_obj_emitter_spawn(binding->m_emitter, 0);
                break;
            }
        }
        else {
            switch(plugin_particle_obj_emitter_use_state(binding->m_emitter)) {
            case plugin_particle_obj_emitter_use_state_active:
                plugin_particle_obj_emitter_set_close(binding->m_emitter, 1);
                break;
            case plugin_particle_obj_emitter_use_state_suspend:
            case plugin_particle_obj_emitter_use_state_passive:
            default:
                break;
            }
        }
    }

    if ((str_value = ui_sprite_2d_part_value(binding->m_part, "rotate", NULL))) {
        binding->m_accept_angle = atoi(str_value);
    }

    if ((str_value = ui_sprite_2d_part_value(binding->m_part, "flip", NULL))) {
        binding->m_accept_flip = atoi(str_value);
    }

    if ((str_value = ui_sprite_2d_part_value(binding->m_part, "scale", NULL))) {
        binding->m_accept_scale = atoi(str_value);
    }

    if ((attr = ui_sprite_2d_part_attr_find(binding->m_part, "rotate-adj")) && (force || ui_sprite_2d_part_attr_is_value_changed(attr))) {
        str_value = ui_sprite_2d_part_attr_value(attr);
        binding->m_angle_adj_rad = cpe_math_angle_to_radians(atof(str_value));
    }
}

static void ui_sprite_particle_bind_emitter_part_binding_on_update(ui_sprite_2d_part_t part, void * ctx) {
    ui_sprite_particle_bind_emitter_part_binding_sync(ctx, 0);
}
