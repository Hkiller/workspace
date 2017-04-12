#include <assert.h>
#include <stdio.h>
#include "spine/BoundingBoxAttachment.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_body.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj_shape.h"
#include "ui_sprite_spine_chipmunk_body_i.h"
#include "ui_sprite_spine_chipmunk_with_collision_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_spine_chipmunk_body_t
ui_sprite_spine_chipmunk_body_create(
    ui_sprite_spine_chipmunk_with_collision_t with_collision, const char * name, struct spSlot * slot, struct spAttachment * attachment)
{
    ui_sprite_spine_chipmunk_module_t module = with_collision->m_module;
    ui_sprite_spine_chipmunk_body_t body;

    body = (ui_sprite_spine_chipmunk_body_t)mem_alloc(with_collision->m_module->m_alloc, sizeof(struct ui_sprite_spine_chipmunk_body));
    if (body == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_body_create: alloc fail!");
        return NULL;
    }

    body->m_with_collision = with_collision;
    cpe_str_dup(body->m_name, sizeof(body->m_name), name);
    body->m_state = ui_sprite_spine_chipmunk_body_state_active;
    body->m_slot = slot;
    body->m_attachment = attachment;
    body->m_scale = UI_VECTOR_2_ZERO;
    body->m_chipmunk_body = NULL;

    TAILQ_INSERT_TAIL(&with_collision->m_bodies, body, m_next);
    
    return body;
}

static ui_sprite_chipmunk_obj_body_t
ui_sprite_spine_chipmunk_body_create_body(
    ui_sprite_spine_chipmunk_body_t body, ui_sprite_chipmunk_obj_t chipmunk_obj, spBoundingBoxAttachment * binding_box)
{
    ui_sprite_spine_chipmunk_with_collision_t with_collision = body->m_with_collision;
    ui_sprite_spine_chipmunk_module_t module = body->m_with_collision->m_module;
    ui_sprite_chipmunk_obj_body_t chipmunk_body;
    ui_sprite_chipmunk_obj_shape_t chipmunk_shape;
    ui_sprite_chipmunk_obj_shape_node_buf_t node_buf;
    CHIPMUNK_FIXTURE * fixture;
    CHIPMUNK_PAIR * verts;
    int i, j;

    chipmunk_body = ui_sprite_chipmunk_obj_body_create(chipmunk_obj, 0, body->m_name, 1);
    if (chipmunk_body == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_body_create_body: create chipmunk body fail!");
        return NULL;
    }

    chipmunk_shape = ui_sprite_chipmunk_obj_shape_create_mamaged_from_body(chipmunk_body);
    if (chipmunk_shape == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_body_create_body: create chipmunk shape fail!");
        ui_sprite_chipmunk_obj_body_free(chipmunk_body);
        return NULL;
    }

    node_buf = ui_sprite_chipmunk_obj_shape_alloc_node_buf(chipmunk_shape, (uint32_t)(binding_box->super. verticesCount / 2));
    if (node_buf == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_body_create_body: alloc node buf fail, capacity = %d!", binding_box->super.verticesCount / 2);
        ui_sprite_chipmunk_obj_body_free(chipmunk_body);
        return NULL;
    }

    fixture = ui_sprite_chipmunk_obj_shape_fixture_data(chipmunk_shape);
    fixture->mass = 0.0f;
    fixture->density = 0.0f;
    fixture->elasticity = 0.0f;
    fixture->friction = 0.0f;
    fixture->surface_velocity.x = 0.0f;
    fixture->surface_velocity.y = 0.0f;
    fixture->collision_mask = with_collision->m_collision_mask;
    fixture->collision_group = with_collision->m_collision_group;
    fixture->collision_category = with_collision->m_collision_category;
    fixture->is_sensor = 1;
    fixture->fixture_type = chipmunk_fixture_type_polygon;
    
    verts = ui_sprite_chipmunk_obj_shape_node_buf_data(node_buf);
    if (body->m_scale.x * body->m_scale.y < 0) {
        for(i = 0, j = 0; j + 1 < binding_box->super.verticesCount; i++, j+=2) {
            verts[i].x = binding_box->super.vertices[j] * body->m_scale.x;
            verts[i].y = - binding_box->super.vertices[j + 1] * body->m_scale.y;
        }
    }
    else {
        for(i = 0, j = binding_box->super.verticesCount; j >= 2; i++, j-=2) {
            verts[i].x = binding_box->super.vertices[j - 2] * body->m_scale.x;
            verts[i].y = - binding_box->super.vertices[j - 1] * body->m_scale.y;
        }
    }
    ui_sprite_chipmunk_obj_shape_node_buf_set_count(node_buf, i);
    
    if (ui_sprite_chipmunk_obj_body_set_type(chipmunk_body, chipmunk_obj_type_kinematic) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_body_create_body: set type fail!");
        ui_sprite_chipmunk_obj_body_free(chipmunk_body);
        return NULL;
    }
    
    if (ui_sprite_chipmunk_obj_body_set_runing_mode(chipmunk_body, ui_sprite_chipmunk_runing_mode_passive) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_spine_chipmunk_body_create_body: set runing mode fail!");
        ui_sprite_chipmunk_obj_body_free(chipmunk_body);
        return NULL;
    }
    
    return chipmunk_body;
}

void ui_sprite_spine_chipmunk_body_update(ui_sprite_chipmunk_obj_t chipmunk_obj, ui_sprite_spine_chipmunk_body_t body, ui_transform_t local_trans) {
    if(body->m_slot->attachment != body->m_attachment) {
        if (body->m_chipmunk_body) {
            if (ui_sprite_chipmunk_obj_body_is_in_space(body->m_chipmunk_body)) {
                ui_sprite_chipmunk_obj_body_remove_from_space(body->m_chipmunk_body);
            }
        }
    }
    else {
        spBoundingBoxAttachment * binding_box = (spBoundingBoxAttachment *)body->m_attachment;
        ui_transform bone_transform;
        
        if (plugin_spine_bone_calc_transform(body->m_slot->bone, &bone_transform) != 0) return;

        ui_transform_adj_by_parent(&bone_transform, local_trans);
        
        if (body->m_chipmunk_body == NULL
            || cpe_float_cmp(body->m_scale.x, bone_transform.m_s.x, 0.01) != 0
            || cpe_float_cmp(body->m_scale.y, bone_transform.m_s.y, 0.01) != 0)
        {
            body->m_scale.x = bone_transform.m_s.x;
            body->m_scale.y = bone_transform.m_s.y;

            if (body->m_chipmunk_body) {
                if (ui_sprite_chipmunk_obj_body_is_in_space(body->m_chipmunk_body)) {
                    ui_sprite_chipmunk_obj_body_remove_from_space(body->m_chipmunk_body);
                }
                ui_sprite_chipmunk_obj_body_free(body->m_chipmunk_body);
            }

            body->m_chipmunk_body = ui_sprite_spine_chipmunk_body_create_body(body, chipmunk_obj, binding_box);
            if (body->m_chipmunk_body == NULL) return;
        }

        if (!ui_sprite_chipmunk_obj_body_is_in_space(body->m_chipmunk_body)) {
            if (ui_sprite_chipmunk_obj_body_add_to_space(body->m_chipmunk_body) != 0) {
                CPE_ERROR(body->m_with_collision->m_module->m_em, "ui_sprite_spine_chipmunk_body_create_body: add to space fail!");
                return;
            }
        }
        
        /*计算对象位置 */
        do {
            ui_vector_2 pos;
            ui_transform_get_pos_2(&bone_transform, &pos);
            ui_sprite_chipmunk_obj_body_set_local_pos(body->m_chipmunk_body, &pos);
        } while(0);
            
        /*计算对象旋转角度 */
        do {
            ui_vector_2 r;
            float angle;
            ui_transform_adj_vector_2_no_t(&bone_transform, &r, &UI_VECTOR_2_POSITIVE_UNIT_X);
            angle = cpe_math_angle(0.0f, 0.0f, r.x, r.y);
            ui_sprite_chipmunk_obj_body_set_local_angle(body->m_chipmunk_body, angle);
        } while(0);
    }
}

void ui_sprite_spine_chipmunk_body_free(ui_sprite_spine_chipmunk_body_t body) {
    ui_sprite_spine_chipmunk_with_collision_t with_collision = body->m_with_collision;

    if (body->m_chipmunk_body) {
        if (ui_sprite_chipmunk_obj_body_is_in_space(body->m_chipmunk_body)) {
            ui_sprite_chipmunk_obj_body_remove_from_space(body->m_chipmunk_body);
        }
        
        ui_sprite_chipmunk_obj_body_free(body->m_chipmunk_body);
        body->m_chipmunk_body = NULL;
    }
    
    switch(body->m_state) {
    case ui_sprite_spine_chipmunk_body_state_active: {
        TAILQ_REMOVE(&with_collision->m_bodies, body, m_next);
        break;
    }
    case ui_sprite_spine_chipmunk_body_state_colliede:
        TAILQ_REMOVE(&with_collision->m_collided_bodies, body, m_next);
        break;
    default:
        assert(0);
    }

    mem_free(with_collision->m_module->m_alloc, body);
}
    
#ifdef __cplusplus
}
#endif

