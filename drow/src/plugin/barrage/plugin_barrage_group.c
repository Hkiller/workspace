#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin_barrage_group_i.h"
#include "plugin_barrage_barrage_i.h"
#include "plugin_barrage_bullet_i.h"
#include "plugin_barrage_bullet_proto_i.h"

plugin_barrage_group_t plugin_barrage_group_create(plugin_barrage_env_t env, const char * name) {
    plugin_barrage_module_t module = env->m_module;
    plugin_barrage_group_t group;
    ui_runtime_render_obj_t render_obj;

    if (plugin_barrage_group_find(env, name) != NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_group_create: group %s already exist!", name);
        return NULL;
    }

    render_obj = ui_runtime_render_obj_create_by_type(module->m_runtime, NULL, "particle");
    if (render_obj == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_env_create: create particle render obj fail!");
        return NULL;
    }
    
    group = (plugin_barrage_group_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_group));
    if (group == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_group_create: alloc fail!");
        ui_runtime_render_obj_free(render_obj);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &group->m_bullet_protos,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_barrage_bullet_proto_hash,
            (cpe_hash_eq_t) plugin_barrage_bullet_proto_eq,
            CPE_HASH_OBJ2ENTRY(plugin_barrage_bullet_proto, m_hh),
            -1) != 0)
    {
        CPE_ERROR(module->m_em, "plugin_barrage_group_create: init proto hashtable fail!");
        ui_runtime_render_obj_free(render_obj);
        mem_free(module->m_alloc, group);
        return NULL;
    }

    group->m_env = env;
    cpe_str_dup(group->m_name, sizeof(group->m_name), name);
    group->m_particle_obj = (plugin_particle_obj_t)ui_runtime_render_obj_data(render_obj);
    
    TAILQ_INIT(&group->m_barrages);
    TAILQ_INIT(&group->m_bullets);

    TAILQ_INSERT_TAIL(&env->m_groups, group, m_next_for_env);

    return group;
}

void plugin_barrage_group_free(plugin_barrage_group_t group) {
    plugin_barrage_module_t module = group->m_env->m_module;

    while(!TAILQ_EMPTY(&group->m_barrages)) {
        plugin_barrage_barrage_free(TAILQ_FIRST(&group->m_barrages));
    }

    while(!TAILQ_EMPTY(&group->m_bullets)) {
        plugin_barrage_bullet_free(TAILQ_FIRST(&group->m_bullets));
    }

    plugin_barrage_bullet_proto_free_all(group);
    cpe_hash_table_fini(&group->m_bullet_protos);

    ui_runtime_render_obj_free(ui_runtime_render_obj_from_data(group->m_particle_obj));
    
    TAILQ_REMOVE(&group->m_env->m_groups, group, m_next_for_env);

    mem_free(module->m_alloc, group);
}

plugin_barrage_group_t
plugin_barrage_group_find(plugin_barrage_env_t env, const char * name) {
    plugin_barrage_group_t group;

    TAILQ_FOREACH(group, &env->m_groups, m_next_for_env) {
        if (strcmp(group->m_name, name) == 0) return group;
    }

    return NULL;
}

static plugin_barrage_bullet_t plugin_barrage_group_bullets_next(struct plugin_barrage_bullet_it * it) {
    plugin_barrage_bullet_t * data = (plugin_barrage_bullet_t *)(it->m_data);
    plugin_barrage_bullet_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_group);

    return r;
}

void plugin_barrage_group_bullets(plugin_barrage_bullet_it_t it, plugin_barrage_group_t group) {
    *(plugin_barrage_bullet_t *)(it->m_data) = TAILQ_FIRST(&group->m_bullets);
    it->next = plugin_barrage_group_bullets_next;
}

static int plugin_barrage_calc_value_angle(float * result, ui_vector_2_t self_pos, plugin_barrage_target_fun_t fun, void * fun_ctx, error_monitor_t em) {
    ui_vector_2 target_pos;
        
    if (fun == NULL) {
        CPE_ERROR(em, "plugin_barrage_calc_value: no target select fun!");
        return -1;
    }

    if (fun(fun_ctx, &target_pos) != 0) {
        CPE_ERROR(em, "plugin_barrage_calc_value: bullet calc target pos fail!");
        return -1;
    }

    *result = cpe_math_angle(self_pos->x, self_pos->y, target_pos.x, target_pos.y);
    return 0;
}
                                    
int plugin_barrage_calc_value(
    float * result, uint8_t calc_type,
    plugin_barrage_emitter_t emitter, plugin_barrage_bullet_t bullet, error_monitor_t em)
{
    switch(calc_type) {
    case barrage_value_calc_target_angle: {
        if (bullet) {
            return plugin_barrage_calc_value_angle(result, plugin_barrage_bullet_pos(bullet), bullet->m_target_fun, bullet->m_target_fun_ctx, em);
        }
        else {
            ui_vector_2 emitter_pos;
        
            assert(emitter);
            plugin_barrage_emitter_pos(emitter, &emitter_pos);
            
            return plugin_barrage_calc_value_angle(result, &emitter_pos, emitter->m_barrage->m_target_fun, emitter->m_barrage->m_target_fun_ctx, em);
        }
    }
    case barrage_value_calc_target_angle_locked: {
        ui_vector_2 emitter_pos;
        
        if (emitter == NULL) return -1;

        if (!emitter->m_barrage->m_cache_target_valid) {
            if (emitter->m_barrage->m_target_fun == NULL) {
                CPE_ERROR(em, "plugin_barrage_calc_value: no target select fun!");
                return -1;
            }

            if (emitter->m_barrage->m_target_fun(emitter->m_barrage->m_target_fun_ctx, &emitter->m_barrage->m_cache_target) != 0) {
                CPE_ERROR(em, "plugin_barrage_calc_value: emitter calc target pos fail!");
                return -1;
            }
            
            emitter->m_barrage->m_cache_target_valid = 1;
        }

        assert(emitter->m_barrage->m_cache_target_valid);

        plugin_barrage_emitter_pos(emitter, &emitter_pos);

        *result = cpe_math_angle(emitter_pos.x, emitter_pos.y, emitter->m_barrage->m_cache_target.x, emitter->m_barrage->m_cache_target.y);
        return 0;
        
    }
    default:
        return -1;
    }
    return 0;
}
