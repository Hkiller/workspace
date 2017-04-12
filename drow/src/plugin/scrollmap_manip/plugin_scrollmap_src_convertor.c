#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/package_manip/plugin_package_manip_src_convertor.h"
#include "plugin/tiledmap/plugin_tiledmap_module.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin/scrollmap/plugin_scrollmap_data_scene.h"
#include "plugin_scrollmap_manip_i.h"

static plugin_scrollmap_data_tile_t
plugin_scrollmap_manip_tile_check_create(
    plugin_scrollmap_manip_t module, plugin_scrollmap_data_scene_t scene, plugin_tiledmap_data_tile_t tiledmap_tile);

static plugin_scrollmap_data_layer_t
plugin_scrollmap_manip_layer_check_create(plugin_scrollmap_manip_t module, plugin_scrollmap_data_scene_t scene, const char * layer_name);

static plugin_scrollmap_data_block_t
plugin_scrollmap_manip_block_create(
    plugin_scrollmap_manip_t module, plugin_scrollmap_data_layer_t layer,
    plugin_tiledmap_data_tile_t tiledmap_tile, plugin_scrollmap_data_tile_t tile);

static plugin_scrollmap_data_script_t
plugin_scrollmap_manip_script_create(plugin_scrollmap_manip_t module, plugin_scrollmap_data_layer_t layer);

static int plugin_scrollmap_manip_script_setup_for_script(
    plugin_scrollmap_manip_t module, plugin_scrollmap_data_script_t script, plugin_tiledmap_data_tile_t tiledmap_tile);

static int plugin_scrollmap_manip_src_convertor(void * ctx, ui_data_src_t tiledmap_scene_src, ui_data_src_t to_src, cfg_t args) {
    plugin_scrollmap_manip_t module = ctx;
    plugin_tiledmap_data_scene_t tiledmap_scene;
    struct plugin_tiledmap_data_layer_it tiledmap_layer_it;
    plugin_tiledmap_data_layer_t tiledmap_layer;
    plugin_scrollmap_data_scene_t scrollmap_scene;
    int rv = -1;

    if (ui_data_src_load_state(tiledmap_scene_src) != ui_data_src_state_loaded) {
        if (ui_data_src_check_load_with_usings(tiledmap_scene_src, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "plugin_scrollmap_manip_src_convertor: tiledmap %s load fail!",
                ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), tiledmap_scene_src));
            return -1;
        }
    }
    
    tiledmap_scene = ui_data_src_product(tiledmap_scene_src);
    assert(tiledmap_scene);

    scrollmap_scene = plugin_scrollmap_data_scene_create(module->m_scrollmap_module, to_src);
    if (scrollmap_scene == NULL) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_manip_src_convertor: create scrollmap scene fail!");
        return -1;
    }
    
    plugin_tiledmap_data_scene_layers(&tiledmap_layer_it, tiledmap_scene);
    while((tiledmap_layer = plugin_tiledmap_data_layer_it_next(&tiledmap_layer_it))) {
        plugin_scrollmap_data_layer_t scrollmap_layer = NULL;
        TILEDMAP_LAYER const * tiledmap_layer_data = plugin_tiledmap_data_layer_data(tiledmap_layer);
        struct plugin_tiledmap_data_tile_it tiledmap_tile_it;
        plugin_tiledmap_data_tile_t tiledmap_tile;

        plugin_tiledmap_data_layer_tiles(&tiledmap_tile_it, tiledmap_layer);
        
        if (strstr(tiledmap_layer_data->name, "script") == tiledmap_layer_data->name) {
            scrollmap_layer = plugin_scrollmap_manip_layer_check_create(
                module, scrollmap_scene, tiledmap_layer_data->name + strlen("script") + 1);
            if (scrollmap_layer == NULL) goto COMPLETE;
            
            while((tiledmap_tile = plugin_tiledmap_data_tile_it_next(&tiledmap_tile_it))) {
                plugin_scrollmap_data_script_t script = plugin_scrollmap_manip_script_create(module, scrollmap_layer);
                if (script == NULL) continue;

                if (plugin_scrollmap_manip_script_setup_for_script(module, script, tiledmap_tile) != 0) {
                    plugin_scrollmap_data_script_free(script);
                    goto COMPLETE;
                }
            }
        }
        else if (strstr(tiledmap_layer_data->name, "land-obj") == tiledmap_layer_data->name) {
            scrollmap_layer = plugin_scrollmap_manip_layer_check_create(
                module, scrollmap_scene, tiledmap_layer_data->name + strlen("land-obj") + 1);
            if (scrollmap_layer == NULL) goto COMPLETE;
            
            /* while((tiledmap_tile = plugin_tiledmap_data_tile_it_next(&tiledmap_tile_it))) { */
            /*     SCROLLMAP_SCRIPT * script = plugin_scrollmap_manip_script_create(&ctx, ctx.m_cur_layer); */
            /*     if (script == NULL) continue; */

            /*     if (plugin_scrollmap_manip_script_setup_for_land_obj(&ctx, script, tiledmap_tile) != 0) { */
            /*         plugin_scrollmap_manip_script_free_last(&ctx, ctx.m_cur_layer, script); */
            /*         goto COMPLETE; */
            /*     } */
            /* } */
        }
        else {
            scrollmap_layer = plugin_scrollmap_manip_layer_check_create(module, scrollmap_scene, tiledmap_layer_data->name);
            if (scrollmap_layer == NULL) goto COMPLETE;

            while((tiledmap_tile = plugin_tiledmap_data_tile_it_next(&tiledmap_tile_it))) {
                plugin_scrollmap_data_tile_t tile;
                plugin_scrollmap_data_block_t block;

                tile = plugin_scrollmap_manip_tile_check_create(module, scrollmap_scene, tiledmap_tile);
                if (tile == NULL) goto COMPLETE;

                block = plugin_scrollmap_manip_block_create(module, scrollmap_layer, tiledmap_tile, tile);
                if (block == NULL) goto COMPLETE;
            }
        }

        assert(scrollmap_layer);
        plugin_scrollmap_data_layer_sort_blocks(scrollmap_layer);
        plugin_scrollmap_data_layer_sort_scripts(scrollmap_layer);
    }

    rv = 0;
    
COMPLETE:
    if (rv != 0) {
        if (scrollmap_scene) {
            ui_data_src_unload(to_src);
        }
    }
    
    return rv;
}

static int plugin_scrollmap_manip_tile_cmp(SCROLLMAP_TILE const * l, SCROLLMAP_TILE const * r) {
    if (l->res_type != r->res_type) return (int)l->res_type - (int)r->res_type;
    if (l->src_id != r->src_id) return (int)l->src_id - (int)r->src_id;
    if (l->res_id != r->res_id) return (int)l->res_id - (int)r->res_id;
    if (l->angle_way != r->angle_way) return (int)l->angle_way - (int)r->angle_way;
    return (int)l->flip_way - (int)r->flip_way;
}

static plugin_scrollmap_data_tile_t
plugin_scrollmap_manip_tile_check_create(
    plugin_scrollmap_manip_t module, plugin_scrollmap_data_scene_t scene, plugin_tiledmap_data_tile_t tiledmap_tile)
{
    TILEDMAP_TILE const * tiledmap_tile_data = plugin_tiledmap_data_tile_data(tiledmap_tile);
    SCROLLMAP_TILE key;
    plugin_scrollmap_data_tile_t  r;
    SCROLLMAP_TILE * r_data;
    ui_rect bounding_rect;
    struct plugin_scrollmap_data_tile_it tile_it;
    plugin_scrollmap_data_tile_t check_tile;
    
    switch(tiledmap_tile_data->ref_type) {
    case tiledmap_tile_ref_type_img:
        key.res_type = scrollmap_tile_res_type_module;
        key.src_id = tiledmap_tile_data->ref_data.img.module_id;
        key.res_id = tiledmap_tile_data->ref_data.img.img_block_id;
        break;
    case tiledmap_tile_ref_type_frame:
        key.res_type = scrollmap_tile_res_type_sprite;
        key.src_id = tiledmap_tile_data->ref_data.frame.sprite_id;
        key.res_id = tiledmap_tile_data->ref_data.frame.frame_id;
        break;
    default:
        CPE_ERROR(module->m_em, "tiledmap_2_scrollmap: unknown tile map ref type %d", tiledmap_tile_data->ref_type);
        return NULL;
    }

    key.flip_way = tiledmap_tile_data->flip_type;
    key.angle_way = tiledmap_tile_data->angle_type;

    if (key.angle_way == scrollmap_tile_angle_type_180) {
        key.flip_way ^= scrollmap_tile_flip_type_xy;
        key.angle_way = scrollmap_tile_angle_type_none;
    }
    else if (key.angle_way == scrollmap_tile_angle_type_270) {
        key.flip_way ^= scrollmap_tile_flip_type_xy;
        key.angle_way = scrollmap_tile_angle_type_90;
    }

    plugin_scrollmap_data_scene_tiles(scene, &tile_it);
    while((check_tile = plugin_scrollmap_data_tile_it_next(&tile_it))) {
        if (plugin_scrollmap_manip_tile_cmp(plugin_scrollmap_data_tile_data(check_tile), &key) == 0) return check_tile;
    }

    r = plugin_scrollmap_data_tile_create(scene);
    if (r == NULL) return NULL;
    r_data = plugin_scrollmap_data_tile_data(r);
    
    r_data->id = plugin_scrollmap_data_scene_tile_count(scene);
    r_data->res_type = key.res_type;
    r_data->src_id = key.src_id;
    r_data->res_id = key.res_id;
    r_data->flip_way = key.flip_way;
    r_data->angle_way = key.angle_way;

    if (plugin_tiledmap_data_tile_rect(tiledmap_tile, &bounding_rect, NULL) != 0) {
        return NULL;
    }

    r_data->res_w = bounding_rect.rb.x - bounding_rect.lt.x;
    r_data->res_h = bounding_rect.rb.y - bounding_rect.lt.y;

    bounding_rect.lt.x -= tiledmap_tile_data->pos.x;
    bounding_rect.lt.y -= tiledmap_tile_data->pos.y;
    bounding_rect.rb.x -= tiledmap_tile_data->pos.x;
    bounding_rect.rb.y -= tiledmap_tile_data->pos.y;
    //printf("lt.x=%f,lt.y=%f,rb.x=%f,rb.y=%f\n",
     //   bounding_rect.lt.x,bounding_rect.lt.y,bounding_rect.rb.x,bounding_rect.rb.y);
    
    r_data->origin_pos.x = - bounding_rect.lt.x;
    r_data->origin_pos.y = - bounding_rect.rb.y;

    return r;
}

static plugin_scrollmap_data_layer_t
plugin_scrollmap_manip_layer_check_create(plugin_scrollmap_manip_t module, plugin_scrollmap_data_scene_t scene, const char * layer_name) {
    plugin_scrollmap_data_layer_t r;

    r = plugin_scrollmap_data_layer_find(scene, layer_name);
    if (r == NULL) {
        SCROLLMAP_LAYER * layer_data;
        
        r = plugin_scrollmap_data_layer_create(scene);
        if (r == NULL) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_manip_layer_check_create: create fail");
            return NULL;
        }

        layer_data = plugin_scrollmap_data_layer_data(r);
        cpe_str_dup(layer_data->name, sizeof(layer_data->name), layer_name);
    }

    return r;
}

static plugin_scrollmap_data_block_t
plugin_scrollmap_manip_block_create(
    plugin_scrollmap_manip_t module, plugin_scrollmap_data_layer_t layer,
    plugin_tiledmap_data_tile_t tiledmap_tile, plugin_scrollmap_data_tile_t tile)
{
    TILEDMAP_TILE const * tiledmap_tile_data = plugin_tiledmap_data_tile_data(tiledmap_tile);
    SCROLLMAP_LAYER * layer_data = plugin_scrollmap_data_layer_data(layer);
    SCROLLMAP_TILE const * tile_data = plugin_scrollmap_data_tile_data(tile);
    plugin_scrollmap_data_block_t r;
    SCROLLMAP_BLOCK * r_data;
    SCROLLMAP_RANGE range;

    r = plugin_scrollmap_data_block_create(layer);
    if (r == NULL) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_manip_block_create: create fail");
        return NULL;
    }
    r_data = plugin_scrollmap_data_block_data(r);
    
    r_data->layer = layer_data->id;
    r_data->tile_id = tile_data->id;

    r_data->pos.x = tiledmap_tile_data->pos.x - tile_data->origin_pos.x;
    r_data->pos.y = - (tiledmap_tile_data->pos.y - tile_data->origin_pos.y);

    /* printf( */
    /*     "xxxxxxx: pos=(%f,%f), origin-pos=(%f,%f), result=(%f,%f)\n", */
    /*     tiledmap_tile_data->pos.x, tiledmap_tile_data->pos.y, */
    /*     tile_data->origin_pos.x, tile_data->origin_pos.y, */
    /*     r->pos.x, r->pos.y); */

    range.begin = r_data->pos.y;
    range.end = range.begin + tile_data->res_h;

    if (layer_data->range.begin >= layer_data->range.end) {
        layer_data->range = range;
    }
    else {
        if (range.begin < layer_data->range.begin) layer_data->range.begin = range.begin;
        if (range.end > layer_data->range.end) layer_data->range.end = range.end;
    }

    return r;
}

static plugin_scrollmap_data_script_t
plugin_scrollmap_manip_script_create(plugin_scrollmap_manip_t module, plugin_scrollmap_data_layer_t layer) {
    plugin_scrollmap_data_script_t r;
    SCROLLMAP_SCRIPT * r_data;
    SCROLLMAP_LAYER const * layer_data;

    layer_data = plugin_scrollmap_data_layer_data(layer);
    
    r = plugin_scrollmap_data_script_create(layer);
    if (r == NULL) return NULL;
    
    r_data = plugin_scrollmap_data_script_data(r);
    
    r_data->layer = layer_data->id;

    return r;
}

static int plugin_scrollmap_manip_script_read_res_info(
    plugin_scrollmap_manip_t module, plugin_tiledmap_data_tile_t tiledmap_tile,
    SCROLLMAP_PAIR * res_size, SCROLLMAP_PAIR * origin_pos)
{
    TILEDMAP_TILE const * tiledmap_tile_data = plugin_tiledmap_data_tile_data(tiledmap_tile);
    ui_rect bounding_rect;
    
    if (plugin_tiledmap_data_tile_rect(tiledmap_tile, &bounding_rect, NULL) != 0) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_script_read_res_info: calc tile rect fail!");
        return -1;
    }

    bounding_rect.lt.x -= tiledmap_tile_data->pos.x;
    bounding_rect.lt.y -= tiledmap_tile_data->pos.y;
    bounding_rect.rb.x -= tiledmap_tile_data->pos.x;
    bounding_rect.rb.y -= tiledmap_tile_data->pos.y;
    
    res_size->x = bounding_rect.rb.x - bounding_rect.lt.x;
    res_size->y = bounding_rect.rb.y - bounding_rect.lt.y;
    origin_pos->x = - bounding_rect.lt.x;
    origin_pos->y = - bounding_rect.rb.y;

    return 0;
}


static int plugin_scrollmap_manip_script_setup_for_script(
    plugin_scrollmap_manip_t module, plugin_scrollmap_data_script_t script, plugin_tiledmap_data_tile_t tiledmap_tile)
{
    SCROLLMAP_SCRIPT * script_data = plugin_scrollmap_data_script_data(script);
    char block_name_buf[256];
    char * block_name;
    char * sep;
    SCROLLMAP_PAIR res_size;
	SCROLLMAP_PAIR origin_pos;
    TILEDMAP_TILE const * tiledmap_tile_data = plugin_tiledmap_data_tile_data(tiledmap_tile);

    block_name = cpe_str_dup(block_name_buf, sizeof(block_name_buf), tiledmap_tile_data->name);
    if (block_name[0] == '{') {
        const char * condition_begin = cpe_str_trim_head(block_name + 1);

        sep = strchr(block_name + 1, '}');
        if (sep == NULL) {
            CPE_ERROR(module->m_em, "script name %s format error, no condition sep!", block_name);
            return -1;
        }

        cpe_str_dup_range(script_data->condition, sizeof(script_data->condition), condition_begin, cpe_str_trim_tail(sep, condition_begin));

        block_name = cpe_str_trim_head(sep + 1);
    }
    else {
        script_data->condition[0] = 0;
    }
    
    /*读取脚本的附加参数 */
    sep = strrchr(block_name, '[');
    if (sep) {
        char * addition_args = sep + 1;

        if (addition_args[strlen(addition_args) - 1] != ']') {
            CPE_ERROR(module->m_em, "script name %s format error!", block_name);
            return -1;
        }

        *sep = 0;
        addition_args[strlen(addition_args) - 1] = 0;

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "trigger-delay", ',', '='))) {
            script_data->trigger_delay = atof(sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "complete-by-distance", ',', '='))) {
            script_data->complete_condition.type = SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_DISTANCE;
            script_data->complete_condition.data.by_distance.distance = atof(sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "complete-by-duration", ',', '='))) {
            script_data->complete_condition.type = SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_DURATION;
            script_data->complete_condition.data.by_duration.duration = atof(sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "complete-on-no-team", ',', '='))) {
            script_data->complete_condition.type = SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_TRIGGER;
            script_data->complete_condition.data.by_trigger.type = SCROLLMAP_SCRIPT_TRIGGER_TYPE_NO_TEAM;
            cpe_str_dup(
                script_data->complete_condition.data.by_trigger.data.no_team.name_prefix,
                sizeof(script_data->complete_condition.data.by_trigger.data.no_team.name_prefix), sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "complete-on-have-team", ',', '='))) {
            script_data->complete_condition.type = SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_TRIGGER;
            script_data->complete_condition.data.by_trigger.type = SCROLLMAP_SCRIPT_TRIGGER_TYPE_HAVE_TEAM;
            cpe_str_dup(
                script_data->complete_condition.data.by_trigger.data.have_team.name_prefix,
                sizeof(script_data->complete_condition.data.by_trigger.data.have_team.name_prefix), sep);
        }
        
        if ((sep = cpe_str_read_and_remove_arg(addition_args, "complete-on-no-obj", ',', '='))) {
            script_data->complete_condition.type = SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_TRIGGER;
            script_data->complete_condition.data.by_trigger.type = SCROLLMAP_SCRIPT_TRIGGER_TYPE_NO_OBJ;
            cpe_str_dup(
                script_data->complete_condition.data.by_trigger.data.no_obj.name_prefix,
                sizeof(script_data->complete_condition.data.by_trigger.data.no_obj.name_prefix), sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "complete-on-have-obj", ',', '='))) {
            script_data->complete_condition.type = SCROLLMAP_SCRIPT_COMPLETE_TYPE_BY_TRIGGER;
            script_data->complete_condition.data.by_trigger.type = SCROLLMAP_SCRIPT_TRIGGER_TYPE_HAVE_OBJ;
            cpe_str_dup(
                script_data->complete_condition.data.by_trigger.data.have_obj.name_prefix,
                sizeof(script_data->complete_condition.data.by_trigger.data.have_obj.name_prefix), sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "trigger-on-no-team", ',', '='))) {
            script_data->trigger.type = SCROLLMAP_SCRIPT_TRIGGER_TYPE_NO_TEAM;
            cpe_str_dup(script_data->trigger.data.no_team.name_prefix, sizeof(script_data->trigger.data.no_team.name_prefix), sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "trigger-on-have-team", ',', '='))) {
            script_data->trigger.type = SCROLLMAP_SCRIPT_TRIGGER_TYPE_HAVE_TEAM;
            cpe_str_dup(script_data->trigger.data.have_team.name_prefix, sizeof(script_data->trigger.data.have_team.name_prefix), sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "trigger-on-no-obj", ',', '='))) {
            script_data->trigger.type = SCROLLMAP_SCRIPT_TRIGGER_TYPE_NO_OBJ;
            cpe_str_dup(script_data->trigger.data.no_obj.name_prefix, sizeof(script_data->trigger.data.no_obj.name_prefix), sep);
        }

        if ((sep = cpe_str_read_and_remove_arg(addition_args, "trigger-on-have-obj", ',', '='))) {
            script_data->trigger.type = SCROLLMAP_SCRIPT_TRIGGER_TYPE_HAVE_OBJ;
            cpe_str_dup(script_data->trigger.data.have_obj.name_prefix, sizeof(script_data->trigger.data.have_obj.name_prefix), sep);
        }
    }

    /*读取脚本触发屏幕坐标 */
    sep = strrchr(block_name, '@');
    if (sep) {
        script_data->trigger_screen_pos_y = atoi(sep + 1);
        *sep = 0;
    }

    /*从引用资源读取脚本类型 */
    if (plugin_scrollmap_manip_script_read_res_info(module, tiledmap_tile, &res_size, &origin_pos) != 0) {
        return -1;
    }

    /*填写位置 */
    script_data->logic_pos.x = tiledmap_tile_data->pos.x - origin_pos.x;
    script_data->logic_pos.y = - tiledmap_tile_data->pos.y;
    script_data->origin_pos.x = origin_pos.x;
    script_data->origin_pos.y = origin_pos.y;

    /* printf("script %s: tile=(%f,%f), origin=(%f,%f), screen=%d, result=(%f,%f)\n", */
    /*        block_name, tiledmap_tile_data->pos.x, tiledmap_tile_data->pos.y, */
    /*        origin_pos.x, origin_pos.y, */
    /*        script_data->trigger_screen_pos_y, */
    /*        script_data->logic_pos.x, script_data->logic_pos.y); */
    
    if (cpe_str_start_with(block_name, "gen_land_obj")) {
        const char * script_args = block_name + strlen("gen_land_obj") + 1;

        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_GEN_LAND_OBJ;
		cpe_str_dup(script_data->script_data.gen_land_obj.obj_type, sizeof(script_data->script_data.gen_land_obj.obj_type), script_args);
	}
    else if (cpe_str_start_with(block_name, "gen_free_obj")) {
        const char * script_args = block_name + strlen("gen_free_obj") + 1;
        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_GEN_FREE_OBJ;
        cpe_str_dup(script_data->script_data.gen_free_obj.obj_type, sizeof(script_data->script_data.gen_free_obj.obj_type), script_args);
    }
    else if (cpe_str_start_with(block_name, "gen_team")) {
        const char * script_args = block_name + strlen("gen_team") + 1;

        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_GEN_TEAM;
        cpe_str_dup(script_data->script_data.gen_team.res, sizeof(script_data->script_data.gen_team.res), script_args);
    }
    else if (cpe_str_start_with(block_name, "layer_adj_speed")) {
        const char * script_args = block_name + strlen("layer_adj_speed") + 1;

        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_LAYER_ADJ_SPEED;
        script_data->script_data.layer_adj_speed.adj_speed = atof(script_args);
    }
    else if (cpe_str_start_with(block_name, "layer_load")) {
        const char * script_args = block_name + strlen("layer_load") + 1;

        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_LAYER_LOAD;
        cpe_str_dup(script_data->script_data.layer_load.res, sizeof(script_data->script_data.layer_load.res), script_args);

    }
    else if (cpe_str_start_with(block_name, "layer_loop")) {
        const char * script_args = block_name + strlen("layer_loop") + 1;

        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_LAYER_LOOP;
        script_data->script_data.layer_loop.loop_distance = res_size.y;
        script_data->script_data.layer_loop.loop_count = 0;

        if (script_args) {
            script_data->script_data.layer_loop.loop_count = atoi(script_args);
        }
    }
    else if (cpe_str_start_with(block_name, "layer_cancel_loop")) {
        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_LAYER_CANCEL_LOOP;
    }
    else if (cpe_str_start_with(block_name, "layer_group_loop")) {
        const char * script_args = block_name + strlen("layer_group_loop") + 1;
        char * sep;

        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_LAYER_GROUP_LOOP;
        script_data->script_data.layer_group_loop.loop_distance = res_size.y;
        script_data->script_data.layer_group_loop.loop_count = 0;

        if (script_args == NULL) {
            CPE_ERROR(module->m_em, "layer group loop no arguments");
            return -1;
        }

        sep = strchr(script_args, '_');
        if (sep) {
            *sep = 0;
            script_data->script_data.layer_loop.loop_count = atoi(sep + 1);
        }
            
        cpe_str_dup(script_data->script_data.layer_group_loop.layer_prefix,
                    sizeof(script_data->script_data.layer_group_loop.layer_prefix),
                    script_args);
    }
    else if (cpe_str_start_with(block_name, "scene_adj_speed")) {
        const char * script_args = block_name + strlen("scene_adj_speed") + 1;
        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_SCENE_ADJ_SPEED;
        script_data->script_data.scene_adj_speed.speed = atof(script_args);
    }
    else {
        const char * script_args = strchr(block_name, '|');

        script_data->script_type = SCROLLMAP_SCRIPT_TYPE_CUSTOM;
        if (script_args) {
            size_t len = script_args - block_name;
            if ((len + 1) > CPE_ARRAY_SIZE(script_data->script_data.custom.script)) {
                len = CPE_ARRAY_SIZE(script_data->script_data.custom.script) - 1;
            }
            memcpy(script_data->script_data.custom.script, block_name, len);
            script_data->script_data.custom.script[len] = 0;
            cpe_str_dup(script_data->script_data.custom.args, sizeof(script_data->script_data.custom.args), script_args + 1);
        }
        else {
            cpe_str_dup(script_data->script_data.custom.script, sizeof(script_data->script_data.custom.script), block_name);
            script_data->script_data.custom.args[0] = 0;
        }
    }

    return 0;
}

int plugin_scrollmap_manip_src_convertor_regist(plugin_scrollmap_manip_t module) {
    plugin_package_manip_src_convertor_t src_convertor;

    src_convertor = plugin_package_manip_src_convertor_create(
        module->m_package_manip, "tiledmap-to-scrollmap",
        ui_data_src_type_tiledmap_scene,
        ui_data_src_type_scrollmap_scene,
        plugin_scrollmap_manip_src_convertor, module);
    if (src_convertor == NULL) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_manip_src_convertor_regist: create convertor fail!");
        return -1;
    }
    
    return 0;
}

void plugin_scrollmap_manip_src_convertor_unregist(plugin_scrollmap_manip_t module) {
    plugin_package_manip_src_convertor_t src_convertor;

    src_convertor = plugin_package_manip_src_convertor_find(module->m_package_manip, "tiledmap-to-scrollmap");
    if (src_convertor) {
        plugin_package_manip_src_convertor_free(src_convertor);
    }
}
