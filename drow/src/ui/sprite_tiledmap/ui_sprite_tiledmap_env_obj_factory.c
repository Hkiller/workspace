#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/tiledmap/plugin_tiledmap_env.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin/tiledmap/plugin_tiledmap_render_layer.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_def.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui/sprite_render/ui_sprite_render_layer.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui_sprite_tiledmap_env_i.h"

static ui_sprite_render_layer_t ui_sprite_tiledmap_env_find_render_layer(ui_sprite_render_env_t env, plugin_tiledmap_layer_t map_layer);
    
int ui_sprite_tiledmap_env_create_obj(
    void * ctx, plugin_tiledmap_layer_t layer,
    uint8_t * ignore,
    ui_vector_2_t pos, plugin_tiledmap_data_tile_t tile)
{
    ui_sprite_tiledmap_env_t env = ctx;
    ui_sprite_tiledmap_module_t module = env->m_module;
    ui_sprite_world_t world;
    TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(tile);
    char * sep;
    char * proto;
    char * name;
    char * args;
    ui_sprite_entity_t entity;
    ui_sprite_2d_transform_t transform;
    ui_sprite_render_sch_t sch;
    
    if (strchr(tile_data->name, '[') == NULL) {
        *ignore = 0;
        return 0;
    }

    mem_buffer_clear_data(&module->m_dump_buffer);
    name = mem_buffer_strdup(&module->m_dump_buffer, tile_data->name);
    if (name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tiledmap_env_create_obj: dup name format error");
        return -1;
    }

    sep = strchr(name, '[');
    assert(sep);
    *sep = 0;
    args = sep + 1;

    sep = strchr(args, ']');
    if (sep == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tiledmap_env_create_obj: tile-name %s format error", tile_data->name);
        return -1;
    }
    *sep = 0;
    
    proto = cpe_str_read_and_remove_arg(args, "proto", ',', '=');
    if (proto == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tiledmap_env_create_obj: tile-name %s format error, no proto", tile_data->name);
        return -1;
    }
    
    world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(env));
    entity = ui_sprite_entity_create(world, name, proto);
    if (entity == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_tiledmap_env_create_obj: tile-name=%s, entity %s proto=%s create fail!", tile_data->name, name, proto);
        return -1;
    }

    if ((transform = ui_sprite_2d_transform_find(entity))) {
        ui_vector_2 init_pos;
        ui_rect entity_rect;
        
        init_pos = ui_sprite_2d_transform_origin_pos(transform);
        if (init_pos.x == 0.0f && init_pos.y == 0.0f) {
            ui_sprite_2d_transform_set_origin_pos(transform, *pos);
        }

        if (plugin_tiledmap_data_tile_rect(tile, &entity_rect, NULL) != 0) {
            CPE_ERROR(module->m_em, "ui_sprite_tiledmap_env_create_obj: tile-name=%s, get rect fail!", tile_data->name);
            return -1;
        }

        entity_rect.lt.x -= pos->x;
        entity_rect.lt.y -= pos->y;
        entity_rect.rb.x -= pos->x;
        entity_rect.rb.y -= pos->y;
        
        ui_sprite_2d_transform_merge_rect(transform, &entity_rect);
    }

    if ((sch = ui_sprite_render_sch_find(entity))) {
        ui_sprite_render_layer_t render_layer;

        if ((render_layer = ui_sprite_tiledmap_env_find_render_layer(ui_sprite_render_sch_env(sch), layer))) {
            ui_sprite_render_sch_set_default_layer(sch, render_layer);
        }

        if ((sep = cpe_str_read_and_remove_arg(args, "anim-set-to", ',', '='))) {
            char res[64];

            switch(tile_data->ref_type) {
            case tiledmap_tile_ref_type_img:
                snprintf(res, sizeof(res), "img-block:%u#%u", tile_data->ref_data.img.module_id, tile_data->ref_data.img.img_block_id);
                break;
            case tiledmap_tile_ref_type_frame:
                snprintf(res, sizeof(res), "frame:%u#%u", tile_data->ref_data.frame.sprite_id, tile_data->ref_data.frame.frame_id);
                break;
            default:
                CPE_ERROR(module->m_em, "ui_sprite_tiledmap_env_create_obj: tile-name=%s, unknown ref type %d!", tile_data->name, tile_data->ref_type);
                ui_sprite_entity_free(entity);
                return -1;
            }

            if (ui_sprite_render_def_create(sch, sep, res, 0) == NULL) {
                CPE_ERROR(module->m_em, "ui_sprite_tiledmap_env_create_obj: tile-name=%s, create anim %s ==> %s fail!", tile_data->name, sep, res);
                ui_sprite_entity_free(entity);
                return -1;
            }
        }
    }
    
    if (ui_sprite_entity_enter(entity) != 0) {
        CPE_ERROR(module->m_em, "ui_sprite_tiledmap_env_create_obj: tile-name=%s, entity enter fail!", tile_data->name);
        return -1;
    }

    *ignore = 1;
    
    return 0;
}

static ui_sprite_render_layer_t
ui_sprite_tiledmap_env_find_render_layer(ui_sprite_render_env_t env, plugin_tiledmap_layer_t map_layer) {
    struct ui_sprite_render_layer_it layer_it;
    ui_sprite_render_layer_t layer;
    
    ui_sprite_render_env_layers(env, &layer_it);
    while((layer = ui_sprite_render_layer_it_next(&layer_it))) {
        struct ui_sprite_render_anim_it anim_it;
        ui_sprite_render_anim_t anim;
        
        ui_sprite_render_layer_anims(layer, &anim_it);
        while((anim = ui_sprite_render_anim_it_next(&anim_it))) {
            ui_runtime_render_obj_ref_t render_obj_ref = ui_sprite_render_anim_obj(anim);
            ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);

            if (strcmp(ui_runtime_render_obj_type_name(render_obj), "tiledmap-layer") != 0) continue;

            if (plugin_tiledmap_render_layer_layer(ui_runtime_render_obj_data(render_obj)) == map_layer) return layer;
        }
    }
        
    return NULL;
}
