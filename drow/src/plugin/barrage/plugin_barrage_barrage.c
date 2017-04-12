#include "cpe/utils/math_ex.h"
#include "render/utils/ui_transform.h"
#include "plugin_barrage_barrage_i.h"
#include "plugin_barrage_emitter_i.h"
#include "plugin_barrage_bullet_i.h"
#include "plugin_barrage_data_barrage_i.h"

plugin_barrage_barrage_t
plugin_barrage_barrage_create(plugin_barrage_group_t group) {
    plugin_barrage_env_t env = group->m_env;
    plugin_barrage_module_t module = env->m_module;
    plugin_barrage_barrage_t barrage;

    barrage = TAILQ_FIRST(&env->m_free_barrages);
    if (barrage) {
        TAILQ_REMOVE(&env->m_free_barrages, barrage, m_next_for_group);
    }
    else {
        barrage = (plugin_barrage_barrage_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_barrage));
        if (barrage == NULL) {
            CPE_ERROR(module->m_em, "create emitter alloc fail!");
            return NULL;
        }
    }

    barrage->m_group = group;
    TAILQ_INIT(&barrage->m_emitters);

    barrage->m_collision_category = 0;
    barrage->m_collision_mask = 0;
    barrage->m_show_dead_anim_mask = 0;
    barrage->m_collision_group = 0;
    barrage->m_carray_data = NULL;
    barrage->m_speed_adj = 1.0f;
    barrage->m_emitter_adj = 1.0f;
    barrage->m_target_fun = NULL;
    barrage->m_target_fun_ctx = NULL;
    barrage->m_frame_loop = 0;
    barrage->m_is_enable = 0;
    barrage->m_loop_count = 0;
    barrage->m_pos = UI_VECTOR_2_ZERO;
    barrage->m_angle = 0;
    barrage->m_cache_target_valid = 0;
    barrage->m_cache_target = UI_VECTOR_2_ZERO;
    
    env->m_barrage_count++;
    TAILQ_INSERT_TAIL(&env->m_idle_barrages, barrage, m_next_for_env);
    TAILQ_INSERT_TAIL(&group->m_barrages, barrage, m_next_for_group);
    
    return barrage;
}

void plugin_barrage_barrage_free(plugin_barrage_barrage_t barrage) {
    plugin_barrage_env_t env = barrage->m_group->m_env;

    if (barrage->m_is_enable) {
        plugin_barrage_barrage_disable(barrage, 0);
        assert(!barrage->m_is_enable);
    }
    
    while(!TAILQ_EMPTY(&barrage->m_emitters)) {
        plugin_barrage_emitter_free(TAILQ_FIRST(&barrage->m_emitters));
    }

    if (barrage->m_carray_data) {
        plugin_barrage_data_free(env, barrage->m_carray_data);
        barrage->m_carray_data = NULL;
    }

    assert(env->m_barrage_count > 0);
    env->m_barrage_count--;
    TAILQ_REMOVE(&env->m_idle_barrages, barrage, m_next_for_env);
    TAILQ_REMOVE(&barrage->m_group->m_barrages, barrage, m_next_for_group);
    
    barrage->m_group = (plugin_barrage_group_t)env;
    TAILQ_INSERT_TAIL(&env->m_free_barrages, barrage, m_next_for_group);
}

void plugin_barrage_barrage_real_free(plugin_barrage_barrage_t barrage) {
    plugin_barrage_env_t env = (plugin_barrage_env_t)barrage->m_group;
    
    TAILQ_REMOVE(&env->m_free_barrages, barrage, m_next_for_group);
    mem_free(env->m_module->m_alloc, barrage);
}

dr_data_t plugin_barrage_barrage_carray_data(plugin_barrage_barrage_t barrage) {
    return barrage->m_carray_data ? &barrage->m_carray_data->m_data : NULL;
}
    
int plugin_barrage_barrage_set_carray_data(plugin_barrage_barrage_t barrage, dr_data_t data) {
    if (barrage->m_carray_data) {
        plugin_barrage_data_free(barrage->m_group->m_env, barrage->m_carray_data);
    }

    barrage->m_carray_data = plugin_barrage_data_create(barrage->m_group->m_env, data);
    if (barrage->m_carray_data == NULL) return -1;

    return 0;
}

void plugin_barrage_emitter_pos(plugin_barrage_emitter_t emitter, ui_vector_2_t result) {
    result->x = emitter->m_barrage->m_pos.x + emitter->m_data.world_pos.x;
    result->y = emitter->m_barrage->m_pos.y + emitter->m_data.world_pos.y;
}

float plugin_barrage_barrage_speed_adj(plugin_barrage_barrage_t barrage) {
    return barrage->m_speed_adj;
}

void plugin_barrage_barrage_set_speed_adj(plugin_barrage_barrage_t barrage, float speed_adj) {
    barrage->m_speed_adj = speed_adj;

    /* if (barrage->m_is_enable) { */
        
    /* } */
}

float plugin_barrage_barrage_emitter_adj(plugin_barrage_barrage_t barrage) {
    return barrage->m_emitter_adj;
}

void plugin_barrage_barrage_set_emitter_adj(plugin_barrage_barrage_t barrage, float emitter_adj) {
    barrage->m_emitter_adj = emitter_adj;
}

void plugin_barrage_barrage_set_target_fun(plugin_barrage_barrage_t barrage, void * ctx, plugin_barrage_target_fun_t fun) {
    plugin_barrage_emitter_t emitter;
    
    barrage->m_target_fun = fun;
    barrage->m_target_fun_ctx = ctx;

    TAILQ_FOREACH(emitter, &barrage->m_emitters, m_next_for_barrage) {
        plugin_barrage_bullet_t bullet;
    
        TAILQ_FOREACH(bullet, &emitter->m_bullets, m_next_for_emitter) {
            bullet->m_target_fun = fun;
            bullet->m_target_fun_ctx = ctx;
        }
    }
}
    
void plugin_barrage_barrage_set_collision_info(
    plugin_barrage_barrage_t barrage, 
    uint32_t collision_category, uint32_t collision_mask, uint32_t collision_group)
{
    barrage->m_collision_category = collision_category;
    barrage->m_collision_mask = collision_mask;    
    barrage->m_collision_group = collision_group;
}

void plugin_barrage_barrage_set_show_dead_anim_mask(plugin_barrage_barrage_t barrage, uint32_t mask) {
    barrage->m_show_dead_anim_mask = mask;    
}

uint8_t plugin_barrage_barrage_is_enable(plugin_barrage_barrage_t barrage) {
    return barrage->m_is_enable;
}

uint32_t plugin_barrage_barrage_loop_count(plugin_barrage_barrage_t barrage) {
    return barrage->m_loop_count;
}

void plugin_barrage_barrage_set_transform(plugin_barrage_barrage_t barrage, ui_transform_t transform) {
    ui_transform_get_pos_2(transform, &barrage->m_pos);
    barrage->m_angle = cpe_math_radians_to_angle(ui_transform_calc_angle_z_rad(transform));
}

void plugin_barrage_barrage_reset(plugin_barrage_barrage_t barrage) {
    barrage->m_data.frame = 0;
}

int plugin_barrage_barrage_enable(plugin_barrage_barrage_t barrage, uint32_t loop_count) {
    plugin_barrage_env_t env = barrage->m_group->m_env;
    plugin_barrage_module_t module = env->m_module;
    plugin_barrage_emitter_t emitter;
    
    if (barrage->m_frame_loop == 0) {
        CPE_ERROR(module->m_em, "%s: plugin_barrage_barrage_enable: no frame loop", plugin_barrage_module_name(module));
        return -1;
    }
    
    if (barrage->m_is_enable) plugin_barrage_barrage_disable(barrage, 0);

    barrage->m_is_enable = 1;
    barrage->m_loop_count = loop_count;

    plugin_barrage_barrage_reset(barrage);

    TAILQ_FOREACH(emitter, &barrage->m_emitters, m_next_for_barrage) {
        assert(!emitter->m_is_working);
        plugin_barrage_emitter_start(emitter);
    }

    TAILQ_REMOVE(&env->m_idle_barrages, barrage, m_next_for_env);
    TAILQ_INSERT_TAIL(&env->m_active_barrages, barrage, m_next_for_env);
    
    return 0;
}

void plugin_barrage_barrage_disable(plugin_barrage_barrage_t barrage, uint8_t clear_bullets) {
    plugin_barrage_env_t env = barrage->m_group->m_env;
    plugin_barrage_emitter_t emitter;

    if (!barrage->m_is_enable) return;
    
    TAILQ_FOREACH(emitter, &barrage->m_emitters, m_next_for_barrage) {
        if (emitter->m_is_working) plugin_barrage_emitter_stop(emitter);
        if (clear_bullets) plugin_barrage_emitter_clear_bullets(emitter);
    }

    barrage->m_cache_target_valid = 0;
    barrage->m_is_enable = 0;
    TAILQ_REMOVE(&env->m_active_barrages, barrage, m_next_for_env);
    TAILQ_INSERT_TAIL(&env->m_idle_barrages, barrage, m_next_for_env);
}

void plugin_barrage_barrage_clear_bullets(plugin_barrage_barrage_t barrage) {
    plugin_barrage_emitter_t emitter;

    TAILQ_FOREACH(emitter, &barrage->m_emitters, m_next_for_barrage) {
        plugin_barrage_emitter_clear_bullets(emitter);
    }
}
