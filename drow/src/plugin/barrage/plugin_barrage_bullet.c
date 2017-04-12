#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_transform.h"
#include "plugin_barrage_bullet_i.h"
#include "plugin_barrage_bullet_proto_i.h"
#include "plugin_barrage_trigger_op_i.h"
#include "plugin_barrage_env_i.h"
#include "plugin_barrage_group_i.h"
#include "plugin_barrage_emitter_i.h"

plugin_barrage_bullet_t
plugin_barrage_bullet_create(
    plugin_barrage_emitter_t emitter,
    plugin_barrage_data_bullet_trigger_t frame_trigger,
    plugin_barrage_data_bullet_trigger_t check_triggers)
{
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;
    plugin_barrage_group_t group = emitter->m_barrage->m_group;
    plugin_barrage_module_t module = env->m_module;
    plugin_barrage_bullet_t bullet;
    plugin_particle_obj_particle_t particle;

    particle = plugin_particle_obj_particle_create(emitter->m_bullet_proto->m_emitter);
    if (particle == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_create: create particle fail!");
        return NULL;
    }
    
    if (!TAILQ_EMPTY(&env->m_free_bullets)) {
        bullet = TAILQ_FIRST(&env->m_free_bullets);
        TAILQ_REMOVE(&env->m_free_bullets, bullet, m_next_for_env);        
    }
    else {
        bullet = (plugin_barrage_bullet_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_bullet));
        if (bullet == NULL) {
            CPE_ERROR(module->m_em, "crate bullet: alloc fail!");
            plugin_particle_obj_particle_free(particle);
            return NULL;
        }
    }

    bullet->m_env = env;
    bullet->m_group = group;
    bullet->m_emitter = emitter;
    bullet->m_proto = emitter->m_bullet_proto;
    bullet->m_particle = particle;
    bullet->m_collision_group = emitter->m_barrage->m_collision_group;
    bullet->m_collision_category = emitter->m_barrage->m_collision_category;
    bullet->m_collision_mask = emitter->m_barrage->m_collision_mask;
    bullet->m_flip_type = emitter->m_flip_type;
    bullet->m_show_dead_anim_mask = emitter->m_barrage->m_show_dead_anim_mask;
    bullet->m_next_trigger = frame_trigger;
    bullet->m_check_triggers = check_triggers;
    bullet->m_state = plugin_barrage_bullet_state_active;
    bullet->m_carray_data = emitter->m_barrage->m_carray_data;
    bullet->m_speed_adj = emitter->m_barrage->m_speed_adj;
    bullet->m_target_fun = emitter->m_barrage->m_target_fun;
    bullet->m_target_fun_ctx = emitter->m_barrage->m_target_fun_ctx;

    bzero(&bullet->m_body, sizeof(bullet->m_body));
    cpBodyInit(&bullet->m_body, 0.0f, 0.0f);
    cpBodySetType(&bullet->m_body, CP_BODY_TYPE_KINEMATIC);
    bzero(&bullet->m_shape, sizeof(bullet->m_shape));

    TAILQ_INIT(&bullet->m_trigger_ops);

    if (bullet->m_carray_data) bullet->m_carray_data->m_ref_count++;

    env->m_bullet_count++;
    
    TAILQ_INSERT_TAIL(&emitter->m_bullets, bullet, m_next_for_emitter);
    TAILQ_INSERT_TAIL(&group->m_bullets, bullet, m_next_for_group);
    TAILQ_INSERT_TAIL(&bullet->m_env->m_active_bullets, bullet, m_next_for_env);

    return bullet;
}

void plugin_barrage_bullet_free(plugin_barrage_bullet_t bullet) {
    plugin_barrage_env_t env = bullet->m_env;
    cpSpace * space = (cpSpace *)plugin_chipmunk_env_space(env->m_chipmunk_env);

    plugin_barrage_trigger_op_free_all(env, &bullet->m_trigger_ops);

    env->m_bullet_count--;
    switch(bullet->m_state) {
    case plugin_barrage_bullet_state_active: {
        TAILQ_REMOVE(&env->m_active_bullets, bullet, m_next_for_env);
        plugin_barrage_bullet_remove_emitter(bullet);

        if (bullet->m_shape.shape.space) {
            cpSpaceRemoveShape(space, (cpShape*)&bullet->m_shape);
        }
    
        cpShapeDestroy(&bullet->m_shape.shape);
        cpBodyDestroy(&bullet->m_body);
        break;
    }
    case plugin_barrage_bullet_state_colliede:
        TAILQ_REMOVE(&env->m_collided_bullets, bullet, m_next_for_env);
        break;
    default:
        assert(0);
    }

    if (bullet->m_carray_data) {
        plugin_barrage_data_free(bullet->m_env, bullet->m_carray_data);
        bullet->m_carray_data = NULL;
    }

    plugin_particle_obj_particle_free(bullet->m_particle);
    
    TAILQ_REMOVE(&bullet->m_group->m_bullets, bullet, m_next_for_group);
    
    /*put into free list*/
    bzero(bullet, sizeof(*bullet));
    bullet->m_env = env;
    TAILQ_INSERT_HEAD(&env->m_free_bullets, bullet, m_next_for_env);
}

void plugin_barrage_bullet_real_free(plugin_barrage_bullet_t bullet) {
    plugin_barrage_module_t module = bullet->m_env->m_module;
    TAILQ_REMOVE(&bullet->m_env->m_free_bullets, bullet, m_next_for_env);        
    mem_free(module->m_alloc, bullet);
}

dr_data_t plugin_barrage_bullet_carray_data(plugin_barrage_bullet_t bullet) {
    return bullet->m_carray_data ? &bullet->m_carray_data->m_data : NULL;
}
    
void plugin_barrage_bullet_remove_emitter(plugin_barrage_bullet_t bullet) {
    if (bullet->m_emitter) {
        TAILQ_REMOVE(&bullet->m_emitter->m_bullets, bullet, m_next_for_emitter);
        bullet->m_emitter = NULL;
    }
}

plugin_barrage_bullet_state_t plugin_barrage_bullet_state(plugin_barrage_bullet_t bullet) {
    return bullet->m_state;
}

ui_vector_2_t plugin_barrage_bullet_pos(plugin_barrage_bullet_t bullet) {
    return plugin_particle_obj_particle_pos(bullet->m_particle);
}
    
BARRAGE_BULLET const * plugin_barrage_bullet_data(plugin_barrage_bullet_t bullet) {
    return &bullet->m_data;
}

void plugin_barrage_bullet_update_speed_angle(plugin_barrage_bullet_t bullet) {
	if (bullet->m_data.angle_to_speed) {
		plugin_particle_obj_particle_set_spin_init(
            bullet->m_particle, cpe_math_radians_add(bullet->m_data.speed_angle_rad, (float)(M_PI / 2.0f)));
	}

	bullet->m_data.speed_pair.x = bullet->m_data.speed * cpe_cos_radians(bullet->m_data.speed_angle_rad) * bullet->m_speed_adj;
	bullet->m_data.speed_pair.y = bullet->m_data.speed * cpe_sin_radians(bullet->m_data.speed_angle_rad) * bullet->m_speed_adj;
}

void plugin_barrage_bullet_update_shape(plugin_barrage_bullet_t bullet) {
	cpVect verts[4];
    cpSpace * space = (cpSpace *)plugin_chipmunk_env_space(bullet->m_env->m_chipmunk_env);
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(bullet->m_proto->m_emitter);
    ui_vector_2 texture_size = *plugin_particle_obj_emitter_tile_size(bullet->m_proto->m_emitter);
    ui_vector_2_t normalized = plugin_particle_obj_emitter_normalized_vtx_4(bullet->m_proto->m_emitter);
    ui_vector_2 scale = plugin_particle_obj_particle_base_scale(bullet->m_particle);
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
        verts[i].x *= scale.x;
        verts[i].y *= scale.y;
    }
    
    if (bullet->m_shape.shape.space) {
        cpSpaceRemoveShape(bullet->m_shape.shape.space, (cpShape*)&bullet->m_shape);
    }

    switch(bullet->m_flip_type) {
    case plugin_barrage_data_emitter_flip_type_x:
    case plugin_barrage_data_emitter_flip_type_y: {
        cpVect t;
        t = verts[0];
        verts[0] = verts[3];
        verts[3] = t;

        t = verts[1];
        verts[1] = verts[2];
        verts[2] = t;
        break;
    }
    default:
        break;
    }
    
    cpPolyShapeInitRaw(
        &bullet->m_shape, &bullet->m_body,
        4, verts,
        0.0f
        );
    
    cpShapeSetFilter(
        (cpShape*)&bullet->m_shape.shape,
        cpShapeFilterNew(bullet->m_collision_group, bullet->m_collision_category, bullet->m_collision_mask));
    cpShapeSetCollisionType((cpShape*)&bullet->m_shape, bullet->m_env->m_collision_type);
    cpShapeSetUserData((cpShape*)&bullet->m_shape, bullet);
    
    cpSpaceAddShape(space, (cpShape*)&bullet->m_shape);
}

void plugin_barrage_bullet_on_collided(plugin_barrage_bullet_t bullet) {
    plugin_barrage_env_t env = bullet->m_env;

    assert(bullet->m_state == plugin_barrage_bullet_state_active);

    /*进入消弹状态 */
    bullet->m_state = plugin_barrage_bullet_state_colliede;
    TAILQ_REMOVE(&env->m_active_bullets, bullet, m_next_for_env);
    TAILQ_INSERT_TAIL(&env->m_collided_bullets, bullet, m_next_for_env);
    
    plugin_barrage_bullet_remove_emitter(bullet);

    if (bullet->m_shape.shape.space) {
        cpSpaceRemoveShape(bullet->m_shape.shape.space, (cpShape*)&bullet->m_shape);
    }
    
    cpShapeDestroy(&bullet->m_shape.shape);
    cpBodyDestroy(&bullet->m_body);
}

int plugin_barrage_bullet_show_dead_anim(plugin_barrage_bullet_t bullet) {
    plugin_particle_obj_emitter_t dead_emitter;

    const char * dead_anim = plugin_particle_obj_emitter_dead_anim(bullet->m_proto->m_emitter);
    
    if (dead_anim[0]) {
        ui_transform trans;
        dead_emitter = plugin_particle_obj_emitter_find(
            plugin_particle_obj_emitter_obj(bullet->m_proto->m_emitter), dead_anim);
        if (dead_emitter == NULL) {
            CPE_ERROR(
                bullet->m_env->m_module->m_em,
                "plugin_barrage_bullet_show_dead_anim: dead emitter %s not exist!", dead_anim);
            return -1;
        }

        plugin_particle_obj_particle_calc_base_transform(bullet->m_particle, &trans);
        plugin_particle_obj_emitter_spawn_at_world(dead_emitter, &trans, 0);
    }

    return 0;
}
