#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_object_ref.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/moving/plugin_moving_env.h"
#include "plugin/moving/plugin_moving_control.h"
#include "plugin_scrollmap_team_i.h"
#include "plugin_scrollmap_obj_i.h"
#include "plugin_scrollmap_env_i.h"
#include "plugin_scrollmap_obj_type_map_i.h"

plugin_scrollmap_team_t
plugin_scrollmap_team_create_from_src(plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, ui_data_src_t src) {
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_team_t team;
    void * product;

    product = ui_data_src_product(src);
    if (product == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_scrollmap_team_create_from_res: src %s not loaded!", ui_data_src_data(src));
        return NULL;
    }
    
    team = TAILQ_FIRST(&env->m_free_teams);
    if (team) {
        TAILQ_REMOVE(&env->m_free_teams, team, m_next_for_env);
    }
    else {
        team = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_team));
        if (team == NULL) return NULL;
    }

    team->m_env = env;
    team->m_layer = layer;
    team->m_team_id = env->m_team_max_id + 1;
    team->m_transform = UI_TRANSFORM_IDENTITY;
    TAILQ_INIT(&team->m_members);
    TAILQ_INIT(&team->m_obj_type_maps);

    switch(ui_data_src_type(src)) {
    case ui_data_src_type_moving_plan:
        if (plugin_scrollmap_team_init_from_moving(team, src, product) != 0) goto CREATE_FAIL;
        break;
    case ui_data_src_type_particle:
        if (plugin_scrollmap_team_init_from_particle(team, src, product) != 0) goto CREATE_FAIL;
        break;
    case ui_data_src_type_spine_skeleton:
        if (plugin_scrollmap_team_init_from_spine(team, src, product) != 0) goto CREATE_FAIL;
        break;
    default:
        CPE_ERROR(
            env->m_module->m_em, "plugin_scrollmap_team_create_from_res: src %s type %d not support!",
            ui_data_src_data(src), ui_data_src_type(src));
        goto CREATE_FAIL;
    }
    
    env->m_team_count++;
    TAILQ_INSERT_TAIL(&env->m_teams, team, m_next_for_env);

    env->m_team_max_id++;
    
    return team;

CREATE_FAIL:
    while(!TAILQ_EMPTY(&team->m_members)) {
        plugin_scrollmap_obj_free(TAILQ_FIRST(&team->m_members));
    }
    
    TAILQ_INSERT_TAIL(&env->m_free_teams, team, m_next_for_env);
    return NULL;
};

plugin_scrollmap_team_t
plugin_scrollmap_team_create_from_res(plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, const char * res) {
    ui_data_src_t team_src;
    ui_data_src_t root;
    
    root = ui_data_mgr_src_root(plugin_moving_env_data_mgr(env->m_moving_env));
    
    team_src = ui_data_src_child_find_by_path(root, res, ui_data_src_type_moving_plan);
    if (team_src == NULL) team_src = ui_data_src_child_find_by_path(root, res, ui_data_src_type_particle);
    if (team_src == NULL) team_src = ui_data_src_child_find_by_path(root, res, ui_data_src_type_spine_skeleton);

    if (team_src == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_scrollmap_team_create_from_res: res %s not exist!", res);
        return NULL;
    }

    return plugin_scrollmap_team_create_from_src(env, layer, team_src);
}

void plugin_scrollmap_team_free(plugin_scrollmap_team_t team) {
    plugin_scrollmap_env_t env = team->m_env;

    while(!TAILQ_EMPTY(&team->m_members)) {
        plugin_scrollmap_obj_free(TAILQ_FIRST(&team->m_members));
    }

    switch(team->m_type) {
    case plugin_scrollmap_team_type_moving:
        plugin_scrollmap_team_fini_moving(team);
        break;
    case plugin_scrollmap_team_type_particle:
        plugin_scrollmap_team_fini_particle(team);
        break;
    case plugin_scrollmap_team_type_spine:
        plugin_scrollmap_team_fini_spine(team);
        break;
    default:
        assert(0);
    }

    while(!TAILQ_EMPTY(&team->m_obj_type_maps)) {
        plugin_scrollmap_obj_type_map_free(TAILQ_FIRST(&team->m_obj_type_maps));
    }
    
    env->m_team_count--;
    TAILQ_REMOVE(&env->m_teams, team, m_next_for_env);

    TAILQ_INSERT_HEAD(&env->m_free_teams, team, m_next_for_env);
}

void plugin_scrollmap_team_real_free(plugin_scrollmap_team_t team) {
    plugin_scrollmap_env_t env = team->m_env;

    TAILQ_REMOVE(&env->m_free_teams, team, m_next_for_env);

    mem_free(env->m_module->m_alloc, team);
}

uint16_t plugin_scrollmap_team_id(plugin_scrollmap_team_t team) {
    return team->m_team_id;
}

plugin_scrollmap_team_t plugin_scrollmap_team_find_by_id(plugin_scrollmap_env_t env, uint16_t team_id) {
    plugin_scrollmap_team_t team;

    TAILQ_FOREACH(team, &env->m_teams, m_next_for_env) {
        if  (team->m_team_id == team_id) return team;
    }

    return NULL;
}

ui_transform_t plugin_scrollmap_team_transform(plugin_scrollmap_team_t team) {
    return &team->m_transform;
}

void plugin_scrollmap_team_set_transform(plugin_scrollmap_team_t team, ui_transform_t trans) {
    team->m_transform = *trans;
}

void plugin_scrollmap_team_update(plugin_scrollmap_env_t env, plugin_scrollmap_team_t team, float delta_s) {
    switch(team->m_type) {
    case plugin_scrollmap_team_type_moving:
        plugin_scrollmap_team_update_moving(team, delta_s);
        return;
    case plugin_scrollmap_team_type_particle:
        plugin_scrollmap_team_update_particle(team, delta_s);
        break;
    case plugin_scrollmap_team_type_spine:
        plugin_scrollmap_team_update_spine(team, delta_s);
        break;
    default:
        assert(0);
    }
}

uint16_t plugin_scrollmap_env_team_count(plugin_scrollmap_env_t env) {
    return env->m_team_count;
}

static plugin_scrollmap_team_t plugin_scrollmap_env_team_next(plugin_scrollmap_team_it_t it) {
    plugin_scrollmap_team_t * data = (plugin_scrollmap_team_t *)it->m_data;
    plugin_scrollmap_team_t r;

    if (*data == NULL) return NULL;

    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_env);

    return r;
}

void plugin_scrollmap_env_teams(plugin_scrollmap_team_it_t it, plugin_scrollmap_env_t env) {
    plugin_scrollmap_team_t * data = (plugin_scrollmap_team_t *)it->m_data;

    *data = TAILQ_FIRST(&env->m_teams);
    it->m_next = plugin_scrollmap_env_team_next;
}
