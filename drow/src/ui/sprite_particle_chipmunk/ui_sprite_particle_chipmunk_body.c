#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/particle/plugin_particle_obj_plugin_data.h"
#include "plugin/particle/plugin_particle_obj_particle.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_particle_chipmunk_body_i.h"
#include "ui_sprite_particle_chipmunk_with_collision_i.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_sprite_particle_chipmunk_body_init(void * ctx, plugin_particle_obj_plugin_data_t data) {
    ui_sprite_particle_chipmunk_with_collision_t with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ctx;
    ui_sprite_particle_chipmunk_body_t body = (ui_sprite_particle_chipmunk_body_t)plugin_particle_obj_plugin_data_data(data);

    body->m_with_collision = with_collision;
    body->m_state = ui_sprite_particle_chipmunk_body_state_active;
    body->m_scale = UI_VECTOR_2_ZERO;
    
    bzero(&body->m_body, sizeof(body->m_body));
    cpBodyInit(&body->m_body, 0.0f, 0.0f);
    cpBodySetType(&body->m_body, CP_BODY_TYPE_KINEMATIC);
    bzero(&body->m_shape, sizeof(body->m_shape));
    
    return 0;
}

void ui_sprite_particle_chipmunk_body_fini(void * ctx, plugin_particle_obj_plugin_data_t data) {
    ui_sprite_particle_chipmunk_with_collision_t with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ctx;
    ui_sprite_particle_chipmunk_body_t body = (ui_sprite_particle_chipmunk_body_t)plugin_particle_obj_plugin_data_data(data);
    cpSpace * space = (cpSpace *)plugin_chipmunk_env_space(with_collision->m_chipmunk_env->m_env);

    switch(body->m_state) {
    case ui_sprite_particle_chipmunk_body_state_active: {
        if (body->m_shape.shape.space) {
            cpSpaceRemoveShape(space, (cpShape*)&body->m_shape);
            cpShapeDestroy(&body->m_shape.shape);
            bzero(&body->m_shape, sizeof(body->m_shape));
        }
    
        cpBodyDestroy(&body->m_body);
        break;
    }
    case ui_sprite_particle_chipmunk_body_state_colliede:
        TAILQ_REMOVE(&with_collision->m_chipmunk_env->m_collided_bodys, body, m_next_for_env);
        break;
    default:
        assert(0);
    }
}

static void ui_sprite_particle_chipmunk_body_calc_collision(cpVect verts[4], plugin_particle_obj_emitter_t emitter);    
void ui_sprite_particle_chipmunk_body_update(void * ctx, plugin_particle_obj_plugin_data_t data) {
    ui_sprite_particle_chipmunk_with_collision_t with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ctx;
    ui_sprite_particle_chipmunk_body_t body = (ui_sprite_particle_chipmunk_body_t)plugin_particle_obj_plugin_data_data(data);
    plugin_particle_obj_t particle_obj = plugin_particle_obj_plugin_data_obj(data);
    plugin_particle_obj_particle_t particle = plugin_particle_obj_plugin_data_particle(data);
    plugin_particle_obj_emitter_t emitter = plugin_particle_obj_particle_emitter(particle);
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);
    ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_from_data(particle_obj);
    ui_transform particle_transform;
    ui_vector_2 particle_pos;
    ui_vector_2 scale = plugin_particle_obj_particle_base_scale(particle);
    float angle_rad;

    cpe_assert_float_sane(scale.x);
    cpe_assert_float_sane(scale.y);
    
    plugin_particle_obj_particle_calc_transform(particle, &particle_transform);

    if (emitter_data->xform_mod == UI_PARTICLE_XFORM_LOCAL) {
        ui_transform_t parent_transform = ui_runtime_render_obj_transform(render_obj);
        if (parent_transform) {
            ui_vector_2 r = UI_VECTOR_2_POSITIVE_UNIT_X;
            if (parent_transform->m_s.x < 0.0f) r.x *= -1.0;
            
            ui_transform_adj_by_parent(&particle_transform, parent_transform);

            ui_transform_inline_adj_vector_2_no_t(&particle_transform, &r);
            angle_rad = cpe_math_radians(0.0f, 0.0f, r.x, r.y);
        }
        else {
            /*计算对象旋转角度 */
            ui_vector_2 r;
            ui_transform_adj_vector_2_no_t(&particle_transform, &r, &UI_VECTOR_2_POSITIVE_UNIT_X);
            angle_rad = cpe_math_radians(0.0f, 0.0f, r.x, r.y);
        }
    }
    else {
        /*计算对象旋转角度 */
        ui_vector_2 r;
        ui_transform_adj_vector_2_no_t(&particle_transform, &r, &UI_VECTOR_2_POSITIVE_UNIT_X);
        angle_rad = cpe_math_radians(0.0f, 0.0f, r.x, r.y);
    }
    
    if (body->m_scale.x != scale.x || body->m_scale.y != scale.y) {
        cpSpace * space = (cpSpace *)plugin_chipmunk_env_space(with_collision->m_chipmunk_env->m_env);
        cpVect verts[4];
        uint8_t i;
        
        /*保存当前scale */
        body->m_scale = scale;

        ui_sprite_particle_chipmunk_body_calc_collision(verts, emitter);

        //printf("xxxxxxx: scale=(%f,%f)\n", particle_transform.m_s.x, particle_transform.m_s.y);
        for(i = 0; i < CPE_ARRAY_SIZE(verts); ++i) {
            verts[i].x *= scale.x;
            verts[i].y *= scale.y;
        }

        if (body->m_shape.shape.space) {
            cpSpaceRemoveShape(body->m_shape.shape.space, (cpShape*)&body->m_shape);
        }

        if (scale.x * scale.y < 0) {
            cpVect t;
            t = verts[0];
            verts[0] = verts[3];
            verts[3] = t;

            t = verts[1];
            verts[1] = verts[2];
            verts[2] = t;
        }
    
        cpPolyShapeInitRaw(
        	&body->m_shape, &body->m_body,
        	4, verts,
        	0.0f
        );

        cpShapeSetFilter(
            (cpShape*)&body->m_shape.shape,
            cpShapeFilterNew(with_collision->m_collision_group, with_collision->m_collision_category, with_collision->m_collision_mask));
        cpShapeSetCollisionType((cpShape*)&body->m_shape, body->m_with_collision->m_chipmunk_env->m_collision_type);
        cpShapeSetUserData((cpShape*)&body->m_shape, body);

        cpSpaceAddShape(space, (cpShape*)&body->m_shape);
    }

    ui_transform_get_pos_2(&particle_transform, &particle_pos);
    cpBodySetPosition(&body->m_body, cpv(particle_pos.x, particle_pos.y));
    cpBodySetAngle(&body->m_body, angle_rad);
}

void ui_sprite_particle_chipmunk_body_on_show_collision(
    ui_sprite_particle_chipmunk_body_t body, plugin_particle_obj_t particle_obj, const char * collision_emitter, ui_vector_2_t pos)
{
    plugin_particle_obj_emitter_t emitter;
    ui_transform trans = UI_TRANSFORM_IDENTITY;
    
    emitter = plugin_particle_obj_emitter_find(particle_obj, collision_emitter);
    if (emitter == NULL) {
        ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(body->m_with_collision));
        CPE_ERROR(
            body->m_with_collision->m_module->m_em,
            "entity %d(%s): show collision: collision emitter %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), collision_emitter);
        return;
    }

    ui_transform_set_pos_2(&trans, pos);
    if (plugin_particle_obj_emitter_spawn_at_world(emitter, &trans, 0) != 0) {
        ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(body->m_with_collision));
        CPE_ERROR(
            body->m_with_collision->m_module->m_em,
            "entity %d(%s): show collision: spawn collision emitter %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), collision_emitter);
        return;
    }
}

static void ui_sprite_particle_chipmunk_body_on_collided_process_follows(
    ui_sprite_particle_chipmunk_body_t body, plugin_particle_obj_particle_t particle)
{
    struct plugin_particle_obj_particle_it follow_it;
    plugin_particle_obj_particle_t follow_particle, next_follow_particle;
    
    plugin_particle_obj_particle_follow_particles(&follow_it, particle);
    for(follow_particle = plugin_particle_obj_particle_it_next(&follow_it); follow_particle; follow_particle = next_follow_particle) {
        plugin_particle_obj_emitter_t emitter;
        char arg_value[64];
        const char * user_text;
        
        next_follow_particle = plugin_particle_obj_particle_it_next(&follow_it);

        ui_sprite_particle_chipmunk_body_on_collided_process_follows(body, follow_particle);

        emitter =  plugin_particle_obj_particle_emitter(follow_particle);

        user_text = plugin_particle_obj_emitter_user_text(emitter);
        if (user_text[0]) {
            if (cpe_str_read_arg(arg_value, sizeof(arg_value), user_text, "clt-anim", ',', '=') == 0) {
                ui_vector_2 pos = plugin_particle_obj_particle_world_pos(follow_particle);
                ui_sprite_particle_chipmunk_body_on_show_collision(body, plugin_particle_obj_emitter_obj(emitter), arg_value, &pos);
            }

            if (cpe_str_read_arg(arg_value, sizeof(arg_value), user_text, "clt-rm", ',', '=') == 0) {
                if (atoi(arg_value)) {
                    plugin_particle_obj_particle_free(follow_particle);
                }
            }
        }
    }
}

uint8_t ui_sprite_particle_chipmunk_body_on_collided(ui_sprite_particle_chipmunk_body_t body, ui_vector_2_t pt) {
    plugin_particle_obj_plugin_data_t data;
    plugin_particle_obj_particle_t particle;
    plugin_particle_obj_emitter_t emitter;
    char arg_value[64];
    uint8_t particle_rm = 1;
    const char * user_text;
    
    data = plugin_particle_obj_plugin_data_from_data(body);
    particle = plugin_particle_obj_plugin_data_particle(data);
    emitter =  plugin_particle_obj_particle_emitter(particle);

    user_text = plugin_particle_obj_emitter_user_text(emitter);
    if (user_text[0]) {
        if (cpe_str_read_arg(arg_value, sizeof(arg_value), user_text, "clt-anim", ',', '=') == 0) {
            ui_vector_2 pos;

            if (pt) {
                pos = *pt;
            }
            else {
                pos = plugin_particle_obj_particle_world_pos(particle);
            }
        
            ui_sprite_particle_chipmunk_body_on_show_collision(body, plugin_particle_obj_emitter_obj(emitter), arg_value, &pos);
        }

        if (cpe_str_read_arg(arg_value, sizeof(arg_value), user_text, "clt-rm", ',', '=') == 0) {
            particle_rm = atoi(arg_value);
        }
    }
    
    ui_sprite_particle_chipmunk_body_on_collided_process_follows(body, particle);
    
    return particle_rm;
}

void ui_sprite_particle_chipmunk_body_free(ui_sprite_particle_chipmunk_body_t body) {
    plugin_particle_obj_plugin_data_t plugin_data = plugin_particle_obj_plugin_data_from_data((void*)body);
    plugin_particle_obj_particle_t particle = plugin_particle_obj_plugin_data_particle(plugin_data);

    plugin_particle_obj_particle_free(particle);
}

void ui_sprite_particle_chipmunk_body_calc_collision(cpVect verts[4], plugin_particle_obj_emitter_t emitter) {
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);
    ui_vector_2 texture_size = *plugin_particle_obj_emitter_tile_size(emitter);
    ui_vector_2_t normalized = plugin_particle_obj_emitter_normalized_vtx_4(emitter);
    ui_rect texture_rect;
    ui_rect collision_rect;
    ui_rect adj_rect;
    uint8_t i;
    
    texture_rect.lt.x = emitter_data->atlas_x;
    texture_rect.lt.y = emitter_data->atlas_y;
    texture_rect.rb.x = emitter_data->atlas_x + texture_size.x;
    texture_rect.rb.y = emitter_data->atlas_y + texture_size.y;

    collision_rect.lt.x = emitter_data->collision_atlas_x;
    collision_rect.lt.y = emitter_data->collision_atlas_y;
    collision_rect.rb.x = emitter_data->collision_atlas_x + emitter_data->collision_atlas_w;
    collision_rect.rb.y = emitter_data->collision_atlas_y + emitter_data->collision_atlas_h;

    adj_rect.lt.x = collision_rect.lt.x - texture_rect.lt.x;
    adj_rect.lt.y = collision_rect.lt.y - texture_rect.lt.y;
    adj_rect.rb.x = collision_rect.rb.x - texture_rect.rb.x;
    adj_rect.rb.y = collision_rect.rb.y - texture_rect.rb.y;

    /*lb, lt, rt, rb*/        
    verts[0] = cpv(adj_rect.lt.x, adj_rect.rb.y);
    verts[1] = cpv(adj_rect.lt.x, adj_rect.lt.y);
    verts[2] = cpv(adj_rect.rb.x, adj_rect.lt.y);
    verts[3] = cpv(adj_rect.rb.x, adj_rect.rb.y);

    for(i = 0; i < 4; ++i) {
        verts[i].x += normalized[i].x * texture_size.x;
        verts[i].y += normalized[i].y * texture_size.y;
    }
}
    
#ifdef __cplusplus
}
#endif

