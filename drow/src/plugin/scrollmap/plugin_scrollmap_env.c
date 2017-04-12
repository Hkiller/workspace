#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "plugin/moving/plugin_moving_env.h"
#include "plugin_scrollmap_module_i.h"
#include "plugin_scrollmap_data_layer_i.h"
#include "plugin_scrollmap_env_i.h"
#include "plugin_scrollmap_source_i.h"
#include "plugin_scrollmap_layer_i.h"
#include "plugin_scrollmap_block_i.h"
#include "plugin_scrollmap_tile_i.h"
#include "plugin_scrollmap_script_i.h"
#include "plugin_scrollmap_script_executor_i.h"
#include "plugin_scrollmap_obj_i.h"
#include "plugin_scrollmap_team_i.h"

plugin_scrollmap_env_t
plugin_scrollmap_env_create(plugin_scrollmap_module_t module, plugin_scrollmap_moving_way_t moving_way, ui_vector_2_t base_size) {
    plugin_scrollmap_env_t env;

    env = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_env));
    if (env == NULL) {
        CPE_ERROR(module->m_em, "create barrage env: creat res fail!");
        return NULL;
    }

    bzero(env, sizeof(*env));

    env->m_module = module;
    env->m_moving_way = moving_way;
    env->m_base_size = *base_size;
    env->m_runing_size = *base_size;
    env->m_logic_size_adj.x = 1.0f;
    env->m_logic_size_adj.y = 1.0f;
    env->m_resize_policy_x = plugin_scrollmap_resize_policy_percent;
    env->m_resize_policy_y = plugin_scrollmap_resize_policy_percent;
    env->m_move_speed = 0.0f;
    env->m_debug = 0;
    env->m_is_suspend = 0;
    env->m_obj_count = 0;
    env->m_team_count = 0;
    env->m_team_max_id = 0;
    env->m_obj_factory_ctx = NULL;
    env->m_obj_capacity = 0;
    env->m_obj_on_init = NULL;
    env->m_obj_on_update = NULL;
    env->m_obj_on_event = NULL;
    env->m_obj_on_destory = NULL;
    env->m_script_check_ctx = NULL;
    env->m_script_check_fun = NULL;

    env->m_moving_env = plugin_moving_env_create(module->m_moving_module);
    if (env->m_moving_env == NULL) {
        CPE_ERROR(module->m_em, "create barrage env: create moving env fail!");
        mem_free(module->m_alloc, env);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &env->m_tiles,
            module->m_alloc,
            (cpe_hash_fun_t) plugin_scrollmap_tile_hash,
            (cpe_hash_eq_t) plugin_scrollmap_tile_eq,
            CPE_HASH_OBJ2ENTRY(plugin_scrollmap_tile, m_hh),
            -1) != 0)
    {
        CPE_ERROR(module->m_em, "create barrage env: init tiles hash table fail!");
        plugin_moving_env_free(env->m_moving_env);
        mem_free(module->m_alloc, env);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &env->m_script_executors,
            module->m_alloc,
            (cpe_hash_fun_t)plugin_scrollmap_script_executor_hash,
            (cpe_hash_eq_t)plugin_scrollmap_script_executor_eq,
            CPE_HASH_OBJ2ENTRY(plugin_scrollmap_script_executor, m_hh),
            -1) != 0)
    {
        CPE_ERROR(module->m_em, "create barrage env: init script_executors hash table fail!");
        cpe_hash_table_fini(&env->m_tiles);
        plugin_moving_env_free(env->m_moving_env);
        mem_free(module->m_alloc, env);
        return NULL;
    }
    
    TAILQ_INIT(&env->m_sources);
    TAILQ_INIT(&env->m_layers);
    TAILQ_INIT(&env->m_free_blocks);
    TAILQ_INIT(&env->m_free_tiles);
    TAILQ_INIT(&env->m_free_scripts);

    TAILQ_INIT(&env->m_objs);
    TAILQ_INIT(&env->m_free_objs);

    TAILQ_INIT(&env->m_teams);
    TAILQ_INIT(&env->m_free_teams);
    
    mem_buffer_init(&env->m_dump_buffer, module->m_alloc);
    
    return env;
}

void plugin_scrollmap_env_free(plugin_scrollmap_env_t env) {
    while(!TAILQ_EMPTY(&env->m_layers)) {
        plugin_scrollmap_layer_free(TAILQ_FIRST(&env->m_layers));
    }

    while(!TAILQ_EMPTY(&env->m_sources)) {
        plugin_scrollmap_source_free(TAILQ_FIRST(&env->m_sources));
    }

    while(!TAILQ_EMPTY(&env->m_objs)) {
        plugin_scrollmap_obj_free(TAILQ_FIRST(&env->m_objs));
    }
    assert(env->m_obj_count == 0);
    
    while(!TAILQ_EMPTY(&env->m_teams)) {
        plugin_scrollmap_team_free(TAILQ_FIRST(&env->m_teams));
    }
    assert(env->m_team_count == 0);

    plugin_scrollmap_tile_free_all(env);
    assert(cpe_hash_table_count(&env->m_tiles) == 0);
    cpe_hash_table_fini(&env->m_tiles);
    
    plugin_scrollmap_script_executor_free_all(env);
    assert(cpe_hash_table_count(&env->m_script_executors) == 0);
    cpe_hash_table_fini(&env->m_script_executors);
    
    /*free list*/
    while(!TAILQ_EMPTY(&env->m_free_tiles)) {
        plugin_scrollmap_tile_real_free(TAILQ_FIRST(&env->m_free_tiles));
    }

    while(!TAILQ_EMPTY(&env->m_free_blocks)) {
        plugin_scrollmap_block_real_free(TAILQ_FIRST(&env->m_free_blocks));
    }

    while(!TAILQ_EMPTY(&env->m_free_scripts)) {
        plugin_scrollmap_script_real_free(TAILQ_FIRST(&env->m_free_scripts));
    }

    while(!TAILQ_EMPTY(&env->m_free_objs)) {
        plugin_scrollmap_obj_real_free(TAILQ_FIRST(&env->m_free_objs));
    }

    while(!TAILQ_EMPTY(&env->m_free_teams)) {
        plugin_scrollmap_team_real_free(TAILQ_FIRST(&env->m_free_teams));
    }

    mem_buffer_clear(&env->m_dump_buffer);

    plugin_moving_env_free(env->m_moving_env);
    env->m_moving_env = NULL;

    //TODO: Loki mem
    //mem_free(env->m_module->m_alloc, env);
}

float plugin_scrollmap_env_move_speed(plugin_scrollmap_env_t env) {
    return env->m_move_speed;
}

void plugin_scrollmap_env_set_move_speed(plugin_scrollmap_env_t env, float speed) {
    env->m_move_speed = speed;
}

void plugin_scrollmap_env_set_suspend(plugin_scrollmap_env_t env, uint8_t is_suspend) {
    env->m_is_suspend = is_suspend;
}

uint8_t plugin_scrollmap_env_is_suspend(plugin_scrollmap_env_t env) {
    return env->m_is_suspend;
}

ui_rect_t plugin_scrollmap_env_view_pos(plugin_scrollmap_env_t env) {
    return &env->m_view_pos;
}

plugin_scrollmap_moving_way_t plugin_scrollmap_env_moving_way(plugin_scrollmap_env_t env) {
    return env->m_moving_way;
}

ui_vector_2_t plugin_scrollmap_env_base_size(plugin_scrollmap_env_t env) {
    return &env->m_base_size;
}

ui_vector_2_t plugin_scrollmap_env_runing_size(plugin_scrollmap_env_t env) {
    return &env->m_runing_size;
}

void plugin_scrollmap_env_set_runing_size(plugin_scrollmap_env_t env, ui_vector_2_t runing_size) {
    env->m_runing_size = *runing_size;
    env->m_logic_size_adj.x = env->m_runing_size.x / env->m_base_size.x;
    env->m_logic_size_adj.y = env->m_runing_size.y / env->m_base_size.y;
}

ui_vector_2_t plugin_scrollmap_env_logic_size_adj(plugin_scrollmap_env_t env) {
    return &env->m_logic_size_adj;
}

plugin_scrollmap_resize_policy_t plugin_scrollmap_env_resize_policy_x(plugin_scrollmap_env_t env) {
    return env->m_resize_policy_x;
}

void plugin_scrollmap_env_set_resize_policy_x(plugin_scrollmap_env_t env, plugin_scrollmap_resize_policy_t policy) {
    env->m_resize_policy_x = policy;
}

plugin_scrollmap_resize_policy_t plugin_scrollmap_env_resize_policy_y(plugin_scrollmap_env_t env) {
    return env->m_resize_policy_y;
}

void plugin_scrollmap_env_set_resize_policy_y(plugin_scrollmap_env_t env, plugin_scrollmap_resize_policy_t policy) {
    env->m_resize_policy_y = policy;
}

void plugin_scrollmap_env_update(plugin_scrollmap_env_t env, float delta_s) {
    float delta_len;
    plugin_scrollmap_layer_t layer;
    plugin_scrollmap_layer_t layer_next;
    plugin_scrollmap_team_t team;
    plugin_scrollmap_team_t team_next;
    float tmp;
    
    if (env->m_is_suspend) return;

    delta_len = env->m_move_speed * delta_s;

    /*推进当前的逻辑位置 */
    switch(env->m_moving_way) {
    case plugin_scrollmap_moving_left:
        env->m_view_pos.lt.x += delta_len;
        env->m_view_pos.rb.x = env->m_view_pos.lt.x + env->m_runing_size.x;
        assert(0);
        break;
    case plugin_scrollmap_moving_right:
        env->m_view_pos.rb.x -= delta_len;
        env->m_view_pos.lt.x = env->m_view_pos.rb.x - env->m_runing_size.x;
        assert(0);
        break;
    case plugin_scrollmap_moving_up:
        env->m_view_pos.lt.y -= delta_len;
        env->m_view_pos.rb.y = env->m_view_pos.lt.y - env->m_runing_size.y;
        assert(0);
        break;
    case plugin_scrollmap_moving_down:
        tmp = env->m_view_pos.lt.y;
        env->m_view_pos.rb.y += delta_len;
        env->m_view_pos.lt.y = env->m_view_pos.rb.y + env->m_runing_size.y;
        delta_len = env->m_view_pos.lt.y - tmp;
        break;
    default:
        break;
    }

    
    /*更新所有的层 */
    for(layer = TAILQ_FIRST(&env->m_layers); layer; layer = layer_next) {
        layer_next = TAILQ_NEXT(layer, m_next_for_env);
        plugin_scrollmap_layer_update(env, layer, delta_len, delta_s);
    }

    /*更新所有的team */
    for(team = TAILQ_FIRST(&env->m_teams); team; team = team_next) {
        team_next = TAILQ_NEXT(team, m_next_for_env);
        plugin_scrollmap_team_update(env, team, delta_s);
    }
}

int plugin_scrollmap_env_load_data(plugin_scrollmap_env_t env, plugin_scrollmap_source_t source, float start_pos) {
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_data_layer_t data_layer;

    TAILQ_FOREACH(data_layer, &source->m_data->m_layers, m_next) {
        plugin_scrollmap_layer_t layer;
        plugin_scrollmap_range_t range;

        layer = plugin_scrollmap_layer_find(env, data_layer->m_data.name);
        if (layer == NULL) {
            layer = plugin_scrollmap_layer_create(env, data_layer->m_data.name);
            if (layer == NULL) {
                CPE_ERROR(module->m_em, "load data: create layer %s fail!", data_layer->m_data.name);
                return -1;
            }
        }

        range = plugin_scrollmap_range_create(layer, source, start_pos);
        if (range == NULL) {
            CPE_ERROR(
                module->m_em, "load data: create data range %s-%s fail!",
                data_layer->m_data.name, ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), source->m_data->m_src));
            return -1;
        }
    }

    return 0;
}

uint8_t plugin_scrollmap_env_debug(plugin_scrollmap_env_t env) {
    return env->m_debug;
}

void plugin_scrollmap_env_set_debug(plugin_scrollmap_env_t env, uint8_t is_debug) {
    env->m_debug = is_debug;
}

int plugin_scrollmap_env_set_obj_factory(
    plugin_scrollmap_env_t env,
    void * ctx, uint32_t obj_capacity,
    plugin_scrollmap_obj_name_fun_t name,
    plugin_scrollmap_obj_on_init_fun_t on_init,
    plugin_scrollmap_obj_on_update_fun_t on_update,
    plugin_scrollmap_obj_on_event_fun_t on_event,
    plugin_scrollmap_obj_on_destory_fun_t on_destory)
{
    assert(ctx);

    if (env->m_obj_factory_ctx) {
        CPE_ERROR(env->m_module->m_em, "plugin_scrollmap_env_set_obj_factory: already have obj factory!");
        return -1;
    }

    if (!TAILQ_EMPTY(&env->m_objs)) {
        CPE_ERROR(env->m_module->m_em, "plugin_scrollmap_env_set_obj_factory: already have obj!");
        return -1;
    }

    while(!TAILQ_EMPTY(&env->m_free_objs)) {
        plugin_scrollmap_obj_real_free(TAILQ_FIRST(&env->m_free_objs));
    }

    env->m_obj_factory_ctx = ctx;
    env->m_obj_capacity = obj_capacity;
    env->m_obj_name = name;
    env->m_obj_on_init = on_init;
    env->m_obj_on_update = on_update;
    env->m_obj_on_event = on_event;
    env->m_obj_on_destory = on_destory;
    
    return 0;
}

void plugin_scrollmap_env_clear_obj_factory(plugin_scrollmap_env_t env, void * ctx) {
    if (env->m_obj_factory_ctx != ctx) return;
    
    if (!TAILQ_EMPTY(&env->m_objs)) {
        plugin_scrollmap_obj_free(TAILQ_FIRST(&env->m_objs));
    }

    env->m_obj_factory_ctx = NULL;
    env->m_obj_capacity = 0;
    env->m_obj_on_init = NULL;
    env->m_obj_on_update = NULL;
    env->m_obj_on_event = NULL;
    env->m_obj_on_destory = NULL;
}

void plugin_scrollmap_env_set_script_filter(
    plugin_scrollmap_env_t env, void * ctx, plugin_scrollmap_script_check_fun_t check_fun)
{
    env->m_script_check_ctx = ctx;
    env->m_script_check_fun = check_fun;
}
