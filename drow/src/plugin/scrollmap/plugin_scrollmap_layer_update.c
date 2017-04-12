#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "plugin_scrollmap_layer_i.h"
#include "plugin_scrollmap_range_i.h"
#include "plugin_scrollmap_obj_i.h"
#include "plugin_scrollmap_data_layer_i.h"
#include "plugin_scrollmap_data_block_i.h"
#include "plugin_scrollmap_data_script_i.h"
#include "plugin_scrollmap_data_tile_i.h"

static void
plugin_scrollmap_layer_gen_block(
    plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, plugin_scrollmap_range_t range,
    SCROLLMAP_BLOCK const * block_data, float left_distance);

static void
plugin_scrollmap_layer_gen_script(
    plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, plugin_scrollmap_range_t range,
    SCROLLMAP_SCRIPT const * script_data);

static void plugin_scrollmap_layer_update_land_obj(
    plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, float layer_delta_len, plugin_scrollmap_obj_t land_obj);

static void plugin_scrollmap_layer_range_update_next_pos(plugin_scrollmap_range_t range, float * check);

void plugin_scrollmap_layer_update(plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, float delta_len, float delta_s) {
    plugin_scrollmap_block_t block;
    plugin_scrollmap_block_t next_block;
    plugin_scrollmap_script_t script;
    plugin_scrollmap_script_t next_script;
    plugin_scrollmap_range_t range;
    plugin_scrollmap_range_t next_range;
    plugin_scrollmap_obj_t land_obj;
    plugin_scrollmap_obj_t next_land_obj;
    float total_delta_len = delta_len * layer->m_speed_adj;

    /* printf( */
    /*     "\n\nxxxx: [%f~%f]: layer %s: update %f + %f(%f) begin!\n", */
    /*     env->m_view_pos.rb.y, env->m_view_pos.lt.y, */
    /*     layer->m_name, layer->m_curent_pos, total_delta_len, delta_len); */
    
    while(total_delta_len > 0.0f) {
        float layer_delta_len = total_delta_len;

        /*搜索下一个处理点 */
        TAILQ_FOREACH(range, &layer->m_active_ranges, m_next_for_layer) {
            plugin_scrollmap_layer_range_update_next_pos(range, &layer_delta_len);
        }

        TAILQ_FOREACH(range, &layer->m_idle_ranges, m_next_for_layer) {
            float delta_len = range->m_start_pos - layer->m_curent_pos;
            assert(delta_len >= 0.0f);
            if (delta_len < layer_delta_len) layer_delta_len = delta_len;
        }

        /* printf("    ** %f + %f\n", layer->m_curent_pos, layer_delta_len); */
        
        /*推进坐标 */
        layer->m_curent_pos += layer_delta_len;
        TAILQ_FOREACH(range, &layer->m_active_ranges, m_next_for_layer) {
            range->m_logic_pos += layer_delta_len;
        }
        total_delta_len -= layer_delta_len;
        
        /*移动已经生成的地面对象 */
        for(land_obj = TAILQ_FIRST(&layer->m_land_objs); land_obj; land_obj = next_land_obj) {
            next_land_obj = TAILQ_NEXT(land_obj, m_move_by_layer.m_next);
            plugin_scrollmap_layer_update_land_obj(env, layer, layer_delta_len, land_obj);
        }

        /*处理各种已经生成的脚本 */
        for(script = TAILQ_FIRST(&layer->m_scripts); script; script = next_script) {
            next_script = TAILQ_NEXT(script, m_next_for_layer);
            plugin_scrollmap_script_update(env, script, layer_delta_len, delta_s);
        }

        /*移动已经生成的块 */
        for(block = TAILQ_FIRST(&layer->m_blocks); block; block = next_block) {
            next_block = TAILQ_NEXT(block, m_next_for_layer);

            block->m_pos.y += layer_delta_len;

            /*超出屏幕释放 */
            if (block->m_pos.y - block->m_tile->m_data->res_h > env->m_runing_size.y) {
                /* printf( */
                /*     "rect %f-%f: block (%f-%f) h=%d: free %p\n", */
                /*     screen_rect->lt.y, screen_rect->rb.y, */
                /*     block->m_pos.y, block->m_pos.y - block->m_tile->m_data.res_h, block->m_tile->m_data.res_h, block); */
                plugin_scrollmap_block_free(block);
                continue;
            }
        }

        /*检查需要新处理的range */
        for(range = TAILQ_FIRST(&layer->m_idle_ranges); range; range = next_range) {
            next_range = TAILQ_NEXT(range, m_next_for_layer);

            if (layer->m_curent_pos >= range->m_start_pos) {
                range->m_next_block = TAILQ_FIRST(&range->m_layer_data->m_blocks);
                range->m_next_script = TAILQ_FIRST(&range->m_layer_data->m_scripts);
                assert(range->m_logic_pos == 0.0f);
                
                assert(range->m_state == plugin_scrollmap_range_state_idle);
                plugin_scrollmap_range_set_state(range, plugin_scrollmap_range_state_active);
            }
        }
        
        /*处理所有range中的block和块 */
        for(range = TAILQ_FIRST(&layer->m_active_ranges); range; range = next_range) {
            next_range = TAILQ_NEXT(range, m_next_for_layer);

            /*生成新的block */
            while(range->m_next_block != TAILQ_END(&range->m_layer_data->m_blocks)
                && range->m_next_block->m_data.pos.y <= range->m_logic_pos)
            {
                plugin_scrollmap_layer_gen_block(env, layer, range, &range->m_next_block->m_data, total_delta_len);
                range->m_next_block = TAILQ_NEXT(range->m_next_block, m_next);
            }
        
            /*生成新的script */
            while(range->m_next_script != TAILQ_END(&range->m_layer_data->m_scripts)
                && range->m_next_script->m_data.logic_pos.y <= range->m_logic_pos )
            {
                plugin_scrollmap_layer_gen_script(env, layer, range, &range->m_next_script->m_data);
                range->m_next_script = TAILQ_NEXT(range->m_next_script, m_next);
            }

            /*更新daa_range的状态 */
            if (range->m_next_block == TAILQ_END(&range->m_layer_data->m_blocks) &&
                range->m_next_script == TAILQ_END(&range->m_layer_data->m_scripts))
            {
                assert(range->m_state == plugin_scrollmap_range_state_active);
                if (TAILQ_EMPTY(&range->m_blocks) && TAILQ_EMPTY(&range->m_scripts)) {
                    plugin_scrollmap_range_set_state(range, plugin_scrollmap_range_state_canceling);
                }
                else {
                    plugin_scrollmap_range_set_state(range, plugin_scrollmap_range_state_done);
                }
            }
        }

        /*处理range的循环 */
        TAILQ_FOREACH(range, &layer->m_active_ranges, m_next_for_layer) {
            if (range->m_loop_begin < range->m_loop_end &&
                range->m_logic_pos == range->m_loop_end)
            {
                if (range->m_loop_count > 0) {
                    range->m_loop_count--;

                    /*根据循环次数停止循环 */
                    if (range->m_loop_count == 0) {
                        plugin_scrollmap_range_cancel_loop(range);
                        continue;
                    }
                }

                /*进入到下一个循环 */
                range->m_logic_pos = range->m_loop_begin;
                range->m_next_block = range->m_loop_blocks_begin;
                range->m_next_script = range->m_loop_scripts_begin;
                range->m_is_first_loop = 0;
            }
        }

        /*检查需要canceling的source是否都ok了 */
        for(range = TAILQ_FIRST(&layer->m_canceling_ranges); range; range = next_range) {
            next_range = TAILQ_NEXT(range, m_next_for_layer);

            if (TAILQ_EMPTY(&range->m_blocks)) {
                plugin_scrollmap_range_set_state(range, plugin_scrollmap_range_state_done);
            }
        }

        /*Done了的source清理，是否需要延时清理，支持场景回退或者scale？ */
        for(range = TAILQ_FIRST(&layer->m_canceling_ranges); range; range = next_range) {
            next_range = TAILQ_NEXT(range, m_next_for_layer);

            plugin_scrollmap_range_free(range);
        }
    }

    /* printf("xxxx: layer %s: update end!\n\n", layer->m_name); */
}

static void plugin_scrollmap_layer_gen_script(
    plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, plugin_scrollmap_range_t range,
    SCROLLMAP_SCRIPT const * script_data)
{
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_script_t script;
    ui_vector_2 screen_pos;
    float trigger_screen_pos_y = script_data->trigger_screen_pos_y;

    if (env->m_resize_policy_y == plugin_scrollmap_resize_policy_percent) trigger_screen_pos_y *= env->m_logic_size_adj.y;

    screen_pos.x = script_data->logic_pos.x;
    screen_pos.y = trigger_screen_pos_y;

    /* printf("       script: trigger_screen_pos_y=%f, logic_pos=%f, world-y=%f, screen_pos=%f\n", */
    /*        trigger_screen_pos_y, script_data->logic_pos.y, layer->m_curent_pos, screen_pos.y); */

    if (env->m_script_check_fun == NULL
        || env->m_script_check_fun(env->m_script_check_ctx, layer, script_data))
    {
        script =  plugin_scrollmap_script_create(layer, script_data, &screen_pos);
        if (script == NULL) {
            CPE_ERROR(
                module->m_em, "layer %s: data %s: create script fail!",
                layer->m_name,
                ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), range->m_source->m_data->m_src));
            return;
        }
        plugin_scrollmap_script_set_range(script, range);
        
        plugin_scrollmap_script_update(env, script, 0.0f, 0.0f);
    }
    
    return;
}

static void plugin_scrollmap_layer_gen_block(
    plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, plugin_scrollmap_range_t range,
    SCROLLMAP_BLOCK const * block_data, float left_len)
{
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_block_t block;
    SCROLLMAP_PAIR block_pos;
    plugin_scrollmap_data_tile_t tile;

    block_pos.x = block_data->pos.x;
    block_pos.y = 0.0f;

    /* printf("      block at (%f,%f)\n", block_pos.x, block_pos.y); */
    
    /*活的当前block对应的tile */
    tile = plugin_scrollmap_data_tile_find(range->m_source->m_data, block_data->tile_id);
    if (tile == NULL) {
        CPE_ERROR(
            module->m_em, "layer %s: data %s: tile %d not exist!",
            layer->m_name,
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), range->m_source->m_data->m_src),
            block_data->tile_id);
        return;
    }

    /*已经直接通过了 */
    if (block_pos.x + tile->m_data.res_w < 0.0f || block_pos.x > env->m_runing_size.x) return;
    if (block_pos.y - tile->m_data.res_h > env->m_runing_size.y) return;

    /*创建block */
    block = plugin_scrollmap_block_create(layer, range, &tile->m_data, &block_pos);
    if (block == NULL) {
        CPE_ERROR(
            module->m_em, "layer %s: data %s: create block fail!",
            layer->m_name,
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), range->m_source->m_data->m_src));
        return;
    }

    /* printf( */
    /*     "rect %f-%f: block (%f-%f) h=%d: create %p\n", */
    /*     ctx->m_screen_rect->lt.y, ctx->m_screen_rect->rb.y, */
    /*     block->m_pos.y, block->m_pos.y - block->m_tile->m_data.res_h, block->m_tile->m_data.res_h, block); */
}

static void plugin_scrollmap_layer_update_land_obj(
    plugin_scrollmap_env_t env, plugin_scrollmap_layer_t layer, float layer_delta_len, plugin_scrollmap_obj_t land_obj)
{
    ui_vector_2 pos;
    
    assert(land_obj->m_is_created);

    ui_transform_get_pos_2(&land_obj->m_transform, &pos);
    pos.y += layer_delta_len;
    ui_transform_set_pos_2(&land_obj->m_transform, &pos);
    env->m_obj_on_update(env->m_obj_factory_ctx, land_obj);

    if (pos.y - land_obj->m_range > env->m_runing_size.y) {
        if (env->m_debug) {
            CPE_INFO(
                env->m_module->m_em, "layer %s: update land obj: top y %f go throw, destory!",
                layer->m_name, pos.y);
        }
        plugin_scrollmap_obj_free(land_obj);
        return;
    }
}

static void plugin_scrollmap_layer_range_update_next_pos(plugin_scrollmap_range_t range, float * check) {
    float delta_len;

    if (range->m_loop_begin <  range->m_loop_end) {
        assert(range->m_loop_end >= range->m_logic_pos);
        delta_len = range->m_loop_end - range->m_logic_pos;
        assert(delta_len >= 0.0f);
        if (delta_len < *check) *check = delta_len;
    }

    while(range->m_next_block != TAILQ_END(&range->m_layer_data->m_blocks)) {
        if (range->m_next_block->m_data.pos.y < range->m_logic_pos) {
            range->m_next_block = TAILQ_NEXT(range->m_next_block, m_next);
            continue;
        }
        
        delta_len = range->m_next_block->m_data.pos.y - range->m_logic_pos;
        assert(delta_len >= 0.0f);
        if (delta_len < *check) *check = delta_len;
        break;
    }

    while(range->m_next_script != TAILQ_END(range->m_layer_data->m_scripts)) {
        if (range->m_next_script->m_data.logic_pos.y < range->m_logic_pos) {
            range->m_next_script = TAILQ_NEXT(range->m_next_script, m_next);
            continue;
        }
        
        delta_len = range->m_next_script->m_data.logic_pos.y - range->m_logic_pos;
        assert(delta_len >= 0.0f);
        if (delta_len < *check) *check = delta_len;
        break;
    }
}
