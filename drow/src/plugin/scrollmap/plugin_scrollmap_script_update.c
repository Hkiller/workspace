#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/moving/plugin_moving_plan.h"
#include "plugin/moving/plugin_moving_control.h"
#include "plugin_scrollmap_script_i.h"
#include "plugin_scrollmap_team_i.h"
#include "plugin_scrollmap_obj_i.h"
#include "plugin_scrollmap_range_i.h"
#include "plugin_scrollmap_script_executor_i.h"
#include "plugin_scrollmap_data_layer_i.h"
#include "plugin_scrollmap_data_tile_i.h"
#include "plugin_scrollmap_data_block_i.h"
#include "plugin_scrollmap_data_script_i.h"

static int plugin_scrollmap_script_execute(plugin_scrollmap_env_t env, plugin_scrollmap_script_t script) {
    plugin_scrollmap_layer_t layer = script->m_layer;
    plugin_scrollmap_module_t module = env->m_module;
    SCROLLMAP_SCRIPT const * script_data = &script->m_script;

    switch(script_data->script_type) {
    case SCROLLMAP_SCRIPT_TYPE_SCENE_ADJ_SPEED:
        env->m_move_speed = script_data->script_data.scene_adj_speed.speed;
        if (env->m_debug) {
            CPE_INFO(
                module->m_em, "layer %s: env adj speed to %f",
                layer->m_name, env->m_move_speed);
        }
        break;
    case SCROLLMAP_SCRIPT_TYPE_LAYER_LOAD: {
        ui_data_src_t scene_src;
        plugin_scrollmap_source_t source;

        scene_src = ui_data_src_find_by_path(module->m_data_mgr, script_data->script_data.layer_load.res, ui_data_src_type_scrollmap_scene);
        if (scene_src == NULL) {
            CPE_ERROR(
                module->m_em, "layer %s: load %s src not exist",
                layer->m_name, script_data->script_data.layer_load.res);
            return -1;
        }
        
        source = plugin_scrollmap_source_find(env, scene_src);
        if (source == NULL) {
            source = plugin_scrollmap_source_create(env, scene_src);
            if (source == NULL) {
                CPE_ERROR(
                    module->m_em, "layer %s: load %s data fail",
                    layer->m_name, script_data->script_data.layer_load.res);
                return -1;
            }
        }
        
        if (plugin_scrollmap_env_load_data(env, source, env->m_view_pos.lt.y) != 0) {
            CPE_ERROR(
                module->m_em, "layer %s: install %s fail",
                layer->m_name, script_data->script_data.layer_load.res);
            return -1;
        }

        if (env->m_debug) {
            CPE_INFO(
                module->m_em, "layer %s: load %s",
                layer->m_name, script_data->script_data.layer_load.res);
        }
        break;
    }
    case SCROLLMAP_SCRIPT_TYPE_LAYER_LOOP: {
        float loop_begin;
        float loop_end;
        plugin_scrollmap_range_t range;
        
        if (script->m_range == NULL) {
            CPE_ERROR(module->m_em, "layer %s: script no data range, can`t loop", layer->m_name);
            return -1;
        }

        range = script->m_range;

        loop_begin = script_data->trigger_screen_pos_y + script_data->logic_pos.y;
        loop_end = loop_begin + script_data->script_data.layer_loop.loop_distance;

        range->m_loop_blocks_begin = range->m_next_block;
        
        if (range->m_loop_blocks_begin != TAILQ_END(&range->m_layer_data->m_blocks)
            && range->m_next_block && range->m_next_block->m_data.pos.y <= loop_end) {
            plugin_scrollmap_data_block_t last;
            plugin_scrollmap_data_tile_t tile;

            range->m_loop_begin = range->m_loop_blocks_begin->m_data.pos.y;
            
            for(last = range->m_loop_blocks_begin;
                TAILQ_NEXT(last, m_next) != TAILQ_END(&range->m_layer_data->m_blocks) && TAILQ_NEXT(last, m_next)->m_data.pos.y <= loop_end;
                last = TAILQ_NEXT(last, m_next));

            range->m_loop_end = last->m_data.pos.y;
            tile = plugin_scrollmap_data_tile_find(range->m_source->m_data, last->m_data.tile_id);
            if (tile) range->m_loop_end += tile->m_data.res_h;
            /* printf("        layer %s loop begin(with blocks), range=(%f,%f)\n", layer->m_name, range->m_loop_begin, range->m_loop_end); */
        }
        else {
            range->m_loop_begin = loop_begin;
            range->m_loop_end = loop_end;
            /* printf("        layer %s loop begin(no blocks), range=(%f,%f)\n", layer->m_name, range->m_loop_begin, range->m_loop_end); */
        }
        
        range->m_loop_scripts_begin = range->m_next_script;
        while(range->m_loop_scripts_begin != TAILQ_END(&range->m_layer_data->m_scripts)
              && range->m_loop_scripts_begin->m_data.logic_pos.y <= range->m_loop_begin)
        {
            range->m_loop_scripts_begin = TAILQ_NEXT(range->m_loop_scripts_begin, m_next);
        }

        /* printf("        layer %s loop begin, range=(%f,%f), next_script=%f\n", */
        /*        layer->m_name, */
        /*        range->m_loop_begin, range->m_loop_end, */
        /*        range->m_loop_scripts_begin->logic_pos.y); */
        
        range->m_is_first_loop = 1;
        range->m_loop_count = script_data->script_data.layer_loop.loop_count;
            
        if (env->m_debug) {
            CPE_INFO(
                module->m_em, "layer %s: data %s: loop [%f ~ %f] %d times",
                layer->m_name, ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), range->m_source->m_data->m_src),
                range->m_loop_begin, range->m_loop_end, range->m_loop_count);
        }

        break;
    }
    case SCROLLMAP_SCRIPT_TYPE_LAYER_CANCEL_LOOP:
        if (script->m_range == NULL) {
            CPE_ERROR(module->m_em, "layer %s: script no data range, can`t loop", layer->m_name);
            return -1;
        }

        plugin_scrollmap_range_cancel_loop(script->m_range);
        
        if (env->m_debug) {
            CPE_INFO(module->m_em, "layer %s: cancel loop blocks", layer->m_name)
        }

        break;
    case SCROLLMAP_SCRIPT_TYPE_LAYER_ADJ_SPEED:
        layer->m_speed_adj = script_data->script_data.layer_adj_speed.adj_speed;
        if (env->m_debug) {
            CPE_INFO(
                module->m_em, "layer %s: layer adj speed to %f",
                layer->m_name, layer->m_speed_adj);
        }
        break;
    case SCROLLMAP_SCRIPT_TYPE_GEN_LAND_OBJ: {
        plugin_scrollmap_obj_t obj;
        ui_transform transform = UI_TRANSFORM_IDENTITY;
        ui_vector_2 pos = UI_VECTOR_2_INITLIZER(
			script->m_pos.x + script_data->origin_pos.x * env->m_logic_size_adj.x,
			script->m_pos.y + script_data->origin_pos.y * env->m_logic_size_adj.y);

        ui_transform_set_pos_2(&transform, &pos);
        obj = plugin_scrollmap_obj_create(env, layer, &transform);
        if (obj == NULL
            || plugin_scrollmap_obj_do_create(obj, script_data->script_data.gen_free_obj.obj_type, NULL) != 0)
        {
            CPE_ERROR(
                module->m_em, "layer %s: gen land obj %s at (%f,%f) fail!",
                layer->m_name,
                script_data->script_data.gen_land_obj.obj_type,
                script->m_pos.x, script->m_pos.y);
            if (obj) plugin_scrollmap_obj_free(obj);
            return -1;
        }
        plugin_scrollmap_obj_set_move_by_layer(obj);
        
        if (env->m_debug) {
            CPE_INFO(
                module->m_em, "layer %s: gen land obj %s at (%f,%f)",
                layer->m_name,
                script_data->script_data.gen_land_obj.obj_type,
                script->m_pos.x, script->m_pos.y);
        }
        break;
    }
    case SCROLLMAP_SCRIPT_TYPE_GEN_FREE_OBJ: {
        plugin_scrollmap_obj_t obj;
        ui_transform transform = UI_TRANSFORM_IDENTITY;
        ui_vector_2 pos = UI_VECTOR_2_INITLIZER(
			script->m_pos.x + script_data->origin_pos.x * env->m_logic_size_adj.x,
			script->m_pos.y + script_data->origin_pos.y * env->m_logic_size_adj.y);

        ui_transform_set_pos_2(&transform, &pos);
        obj = plugin_scrollmap_obj_create(env, layer, &transform);
        if (obj == NULL
            || plugin_scrollmap_obj_do_create(obj, script_data->script_data.gen_free_obj.obj_type, NULL) != 0)
        {
            CPE_ERROR(
                module->m_em, "layer %s: gen free obj %s at (%f,%f) fail!",
                layer->m_name,
                script_data->script_data.gen_free_obj.obj_type,
                script->m_pos.x, script->m_pos.y);
            if (obj) plugin_scrollmap_obj_free(obj);
            return -1;
        }
        //plugin_scrollmap_obj_set_transform(obj, &pos);

        if (env->m_debug) {
            CPE_INFO(
                module->m_em, "layer %s: gen free obj %s at (%f,%f)",
                layer->m_name,
                script_data->script_data.gen_free_obj.obj_type,
                script->m_pos.x, script->m_pos.y);
        }
        break;
    }
    case SCROLLMAP_SCRIPT_TYPE_GEN_TEAM: {
        plugin_scrollmap_team_t team;
        
        team = plugin_scrollmap_team_create_from_res(env, layer, script_data->script_data.gen_team.res);
        if (team == NULL) {
            CPE_ERROR(
                module->m_em, "layer %s: gen team %s at (%f,%f) fail!",
                layer->m_name,
                script_data->script_data.gen_team.res,
                script->m_pos.x, script->m_pos.y);
            return -1;
        }

        if (env->m_debug) {
            CPE_INFO(
                module->m_em, "layer %s: gen team %s at (%f,%f)",
                layer->m_name,
                script_data->script_data.gen_team.res,
                script->m_pos.x, script->m_pos.y);
        }

        break;
    }
    case SCROLLMAP_SCRIPT_TYPE_CUSTOM: {
        plugin_scrollmap_script_executor_t executor =
            plugin_scrollmap_script_executor_find(env, script_data->script_data.custom.script);
        if (executor == NULL) {
            CPE_ERROR(
                module->m_em, "layer %s: execute custom script %s at (%f,%f): unknown script!",
                layer->m_name,
                script_data->script_data.custom.script,
                script->m_pos.x, script->m_pos.y);
            return -1;
        }

        executor->m_exec_fun(executor, script);

        if (env->m_debug) {
            CPE_INFO(
                module->m_em, "layer %s: execute custom script %s[%s] at (%f,%f)",
                layer->m_name,
                script_data->script_data.custom.script,
                script_data->script_data.custom.args,
                script->m_pos.x, script->m_pos.y);
        }

        break;
    }
    default:
        CPE_ERROR(
            module->m_em, "layer %s: unknown script complete condition %d",
            layer->m_name, script->m_script.complete_condition.type);
        return -1;
    }
    
    return 0;
}

static int plugin_scrollmap_script_complete(plugin_scrollmap_env_t env, plugin_scrollmap_script_t script) {
    plugin_scrollmap_layer_t layer = script->m_layer;
    plugin_scrollmap_module_t module = env->m_module;
    SCROLLMAP_SCRIPT const * script_data = &script->m_script;

    switch(script_data->script_type) {
    case SCROLLMAP_SCRIPT_TYPE_LAYER_LOOP: {
        plugin_scrollmap_range_t range;
        
        if (script->m_range == NULL) {
            CPE_ERROR(module->m_em, "layer %s: script no data range, can`t loop", layer->m_name);
            return -1;
        }

        range = script->m_range;
        range->m_loop_begin = range->m_loop_end = 0.0f;
        range->m_loop_blocks_begin = NULL;
        range->m_loop_scripts_begin = NULL;
        range->m_loop_count = 0;
        range->m_is_first_loop = 0;

        if (env->m_debug) {
            CPE_INFO(module->m_em, "layer %s: cancel loop blocks", layer->m_name)
        }

        break;
    }
    }
    
    return 0;
}

static uint8_t plugin_scrollmap_script_check_trigger(plugin_scrollmap_env_t env, SCROLLMAP_SCRIPT_TRIGGER const * trigger) {
    switch(trigger->type) {
    case SCROLLMAP_SCRIPT_TRIGGER_TYPE_NO_TEAM:
        if (trigger->data.no_team.name_prefix[0] == 0) {
            return TAILQ_EMPTY(&env->m_teams) ? 1 : 0;
        }
        else {
            plugin_scrollmap_team_t team;

            TAILQ_FOREACH(team, &env->m_teams, m_next_for_env) {
                ui_data_src_t plan_src =  plugin_moving_plan_src(plugin_moving_control_plan(team->m_moving.m_control));
                if (cpe_str_start_with(ui_data_src_data(plan_src), trigger->data.no_team.name_prefix)) return 0;
            }
            
            return 1;
        }
    case SCROLLMAP_SCRIPT_TRIGGER_TYPE_HAVE_TEAM:
        if (trigger->data.no_team.name_prefix[0] == 0) {
            return TAILQ_EMPTY(&env->m_teams) ? 0 : 1;
        }
        else {
            plugin_scrollmap_team_t team;

            TAILQ_FOREACH(team, &env->m_teams, m_next_for_env) {
                ui_data_src_t plan_src =  plugin_moving_plan_src(plugin_moving_control_plan(team->m_moving.m_control));
                if (cpe_str_start_with(ui_data_src_data(plan_src), trigger->data.no_team.name_prefix)) return 1;
            }
            
            return 0;
        }
    case SCROLLMAP_SCRIPT_TRIGGER_TYPE_NO_OBJ:
        if (trigger->data.no_obj.name_prefix[0] == 0) {
            return TAILQ_EMPTY(&env->m_objs) ? 1 : 0;
        }
        else {
            plugin_scrollmap_obj_t obj;

            TAILQ_FOREACH(obj, &env->m_objs, m_next_for_env) {
                const char * name;
                
                if (!obj->m_is_created) continue;

                name = env->m_obj_name(env->m_obj_factory_ctx, obj);
                if (cpe_str_start_with(name, trigger->data.no_obj.name_prefix)) return 0;
            }
            
            return 1;
        }
    case SCROLLMAP_SCRIPT_TRIGGER_TYPE_NONE:
    default:
        return 1;
    }
}

void plugin_scrollmap_script_update(plugin_scrollmap_env_t env, plugin_scrollmap_script_t script, float delta_len, float delta_s) {
    if(script->m_state == plugin_scrollmap_script_state_wait) {
        if (!plugin_scrollmap_script_check_trigger(env, &script->m_script.trigger)) return;

        if (script->m_script.trigger_delay > 0.0f) {
            script->m_state = plugin_scrollmap_script_state_delay;
            return;
        }
        else {
            if (plugin_scrollmap_script_execute(env, script) != 0) {
                plugin_scrollmap_script_free(script);
                return;
            }
            else {
                script->m_state = plugin_scrollmap_script_state_runing;
            }
        }
    }

    if (script->m_state == plugin_scrollmap_script_state_delay) {
        if (delta_s < script->m_script.trigger_delay) {
            script->m_script.trigger_delay -= delta_s;
            return;
        }
        
        delta_s -= script->m_script.trigger_delay;
        script->m_script.trigger_delay = 0.0f;
        
        if (plugin_scrollmap_script_execute(env, script) != 0) {
            plugin_scrollmap_script_free(script);
            return;
        }
        else {
            script->m_state = plugin_scrollmap_script_state_runing;
        }
    }

    assert(script->m_state == plugin_scrollmap_script_state_runing);

    switch(script->m_script.complete_condition.type) {
    case SCROLLMAP_SCRIPT_COMPLETE_TYPE_NONE:
        plugin_scrollmap_script_free(script);
        return;
    case SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_DURATION:
        if (delta_s < script->m_script.complete_condition.data.by_duration.duration) {
            script->m_script.complete_condition.data.by_duration.duration -= delta_s;
            break;
        }
        else {
            plugin_scrollmap_script_complete(env, script);
            plugin_scrollmap_script_free(script);
            return;
        }
    case SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_DISTANCE:
        if (delta_len < script->m_script.complete_condition.data.by_distance.distance) {
            script->m_script.complete_condition.data.by_distance.distance -= delta_len;
            break;
        }
        else {
            plugin_scrollmap_script_complete(env, script);
            plugin_scrollmap_script_free(script);
            return;
        }
        break;
    case SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_TRIGGER:
        if (plugin_scrollmap_script_check_trigger(env, &script->m_script.complete_condition.data.by_trigger)) {
            plugin_scrollmap_script_complete(env, script);
            plugin_scrollmap_script_free(script);
            return;
        }
        break;
    default:
        CPE_ERROR(
            env->m_module->m_em, "layer %s: unknown script complete condition %d",
            script->m_layer->m_name, script->m_script.complete_condition.type);
        plugin_scrollmap_script_free(script);
        break;
    }
}
