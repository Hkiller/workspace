#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "render/utils/ui_rect.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "plugin_tiledmap_data_tile_i.h"
#include "plugin_tiledmap_data_layer_i.h"

plugin_tiledmap_data_tile_t
plugin_tiledmap_data_tile_create(plugin_tiledmap_data_layer_t data_layer) {
    plugin_tiledmap_module_t module = data_layer->m_scene->m_module;
    plugin_tiledmap_data_tile_t data_tile;

    data_tile = (plugin_tiledmap_data_tile_t)mem_calloc(module->m_alloc, sizeof(struct plugin_tiledmap_data_tile));
    if(data_tile == NULL) {
        CPE_ERROR(module->m_em, "create tile proto: alloc fail!");
        return NULL;
    }

    data_tile->m_layer = data_layer;
    data_tile->m_data.scale.x = 1.0f;
    data_tile->m_data.scale.y = 1.0f;

    data_layer->m_tile_count++;
    TAILQ_INSERT_TAIL(&data_layer->m_tile_list, data_tile, m_next_for_layer);

    return data_tile;
}

void plugin_tiledmap_data_tile_free(plugin_tiledmap_data_tile_t tile) {
    plugin_tiledmap_data_layer_t data_layer = tile->m_layer;
    plugin_tiledmap_module_t module = data_layer->m_scene->m_module;

    data_layer->m_tile_count--;
    TAILQ_REMOVE(&data_layer->m_tile_list, tile, m_next_for_layer);

    mem_free(module->m_alloc, tile);
}

plugin_tiledmap_data_tile_t
plugin_tiledmap_data_tile_find_by_name(plugin_tiledmap_data_layer_t layer, const char * name) {
    plugin_tiledmap_data_tile_t tile;

    TAILQ_FOREACH(tile, &layer->m_tile_list, m_next_for_layer) {
        if (strcmp(tile->m_data.name, name) == 0) return tile;
    }

    return NULL;
}
    
TILEDMAP_TILE * plugin_tiledmap_data_tile_data(plugin_tiledmap_data_tile_t data_tile) {
    return &data_tile->m_data;
}

int plugin_tiledmap_data_tile_rect(
    plugin_tiledmap_data_tile_t tile, ui_rect_t rect, ui_data_src_t * src_cache)
{
    ui_data_src_t src_local_cache = NULL;
    uint8_t flip_type = tile->m_data.flip_type;
    uint8_t angle_type = tile->m_data.angle_type;

    if (src_cache == NULL) src_cache = &src_local_cache;
    
    if (angle_type == tiledmap_tile_angle_type_180) {
        flip_type ^= tiledmap_tile_flip_type_xy;
        angle_type = tiledmap_tile_angle_type_none;
    }
    else if (angle_type == tiledmap_tile_angle_type_270) {
        flip_type ^= tiledmap_tile_flip_type_xy;
        angle_type = tiledmap_tile_angle_type_90;
    }

    if (tile->m_data.ref_type == tiledmap_tile_ref_type_tag) {
        rect->lt.x = tile->m_data.pos.x + tile->m_data.ref_data.tag.rect.lt.x;
        rect->lt.y = tile->m_data.pos.y + tile->m_data.ref_data.tag.rect.lt.y;
        rect->rb.x = tile->m_data.pos.x + tile->m_data.ref_data.tag.rect.rb.x;
        rect->rb.y = tile->m_data.pos.y + tile->m_data.ref_data.tag.rect.rb.y;
    }
    else if(tile->m_data.ref_type == tiledmap_tile_ref_type_img) {
        float w, h;
        ui_data_module_t module = NULL;        
        ui_data_img_block_t img_block;
        UI_IMG_BLOCK const * img_block_data;

        if (*src_cache == NULL || ui_data_src_id(*src_cache) != tile->m_data.ref_data.img.module_id) {
            *src_cache = ui_data_src_find_by_id(tile->m_layer->m_scene->m_module->m_data_mgr, tile->m_data.ref_data.img.module_id);
            if (*src_cache == NULL) {
                CPE_ERROR(
                    tile->m_layer->m_scene->m_module->m_em,
                    "plugin_tiledmap_data_tile_rect: module "FMT_UINT32_T" not exist!",
                    tile->m_data.ref_data.img.module_id);
                return -1;
            }
        }

        module = ui_data_src_product(*src_cache);
        if (module == NULL) {
            CPE_ERROR(
                tile->m_layer->m_scene->m_module->m_em,
                "plugin_tiledmap_data_tile_rect: module "FMT_UINT32_T" not load!",
                tile->m_data.ref_data.img.module_id);
            return -1;
        }
        
        img_block = ui_data_img_block_find_by_id(module, tile->m_data.ref_data.img.img_block_id);
        if (img_block == NULL) {
            CPE_ERROR(
                tile->m_layer->m_scene->m_module->m_em,
                "plugin_tiledmap_data_tile_rect: module "FMT_UINT32_T" img_block "FMT_UINT32_T" not exist!",
                tile->m_data.ref_data.img.module_id, tile->m_data.ref_data.img.img_block_id);
            return -1;
        }

        img_block_data = ui_data_img_block_data(img_block);

        rect->lt.x = tile->m_data.pos.x;
        rect->lt.y = tile->m_data.pos.y;

        if (angle_type == tiledmap_tile_angle_type_90) {
            h = img_block_data->src_w * tile->m_data.scale.x;
            w = img_block_data->src_h * tile->m_data.scale.y;
        }
        else {
            w = img_block_data->src_w * tile->m_data.scale.x;
            h = img_block_data->src_h * tile->m_data.scale.y;
        }

        switch(flip_type) {
        case tiledmap_tile_flip_type_none:
            if (angle_type == tiledmap_tile_angle_type_90) {
                rect->lt.x -= w;
            }
            break;
        case tiledmap_tile_flip_type_x:
            if (angle_type == tiledmap_tile_angle_type_90) {
                rect->lt.x -= w;
                rect->lt.y -= h;
            }
            else {
                rect->lt.x -= w;
            }
            break;
        case tiledmap_tile_flip_type_y:
            if (angle_type == tiledmap_tile_angle_type_90) {
            }
            else {
                rect->lt.y -= h;
            }
            break;
        case tiledmap_tile_flip_type_xy:
            if (angle_type == tiledmap_tile_angle_type_90) {
                rect->lt.y -= h;
            }
            else {
                rect->lt.x -= w;
                rect->lt.y -= h;
            }
            break;
        default:
            break;
        }

        rect->rb.x = rect->lt.x + w;
        rect->rb.y = rect->lt.y + h;
    }
    else if (tile->m_data.ref_type == tiledmap_tile_ref_type_frame) {
        ui_data_sprite_t sprite = NULL;        
        ui_data_frame_t frame;
        ui_rect frame_rect;

        if (*src_cache == NULL || ui_data_src_id(*src_cache) != tile->m_data.ref_data.frame.sprite_id) {
            *src_cache = ui_data_src_find_by_id(tile->m_layer->m_scene->m_module->m_data_mgr, tile->m_data.ref_data.frame.sprite_id);
            if (*src_cache == NULL) {
                CPE_ERROR(
                    tile->m_layer->m_scene->m_module->m_em,
                    "plugin_tiledmap_data_tile_rect: sprite "FMT_UINT32_T" not exist!",
                    tile->m_data.ref_data.frame.sprite_id);
                return -1;
            }
        }

        sprite = ui_data_src_product(*src_cache);
        if (sprite == NULL) {
            CPE_ERROR(
                tile->m_layer->m_scene->m_module->m_em,
                "plugin_tiledmap_data_tile_rect: sprite "FMT_UINT32_T" not load!",
                tile->m_data.ref_data.frame.sprite_id);
            return -1;
        }

        frame = ui_data_frame_find_by_id(sprite, tile->m_data.ref_data.frame.frame_id);
        if (frame == NULL) {
            CPE_ERROR(
                tile->m_layer->m_scene->m_module->m_em,
                "plugin_tiledmap_data_tile_rect: sprite "FMT_UINT32_T" frame "FMT_UINT32_T" not exist!",
                tile->m_data.ref_data.frame.sprite_id, tile->m_data.ref_data.frame.frame_id);
            return -1;
        }

        if (ui_data_frame_bounding_rect(frame, &frame_rect) != 0) {
            CPE_ERROR(
                tile->m_layer->m_scene->m_module->m_em,
                "plugin_tiledmap_data_tile_rect: sprite " FMT_UINT32_T " frame " FMT_UINT32_T " calc frame fail!",
                tile->m_data.ref_data.frame.sprite_id, tile->m_data.ref_data.frame.frame_id);
            return -1;
        }

        if (angle_type == tiledmap_tile_angle_type_90) {
			float lt_tp_dis   = cpe_math_distance(0.0f, 0.0f, frame_rect.lt.x, frame_rect.lt.y);
			float  lt_tp_radian  = cpe_math_radians(0.0f, 0.0f, frame_rect.lt.x, frame_rect.lt.y);
			int32_t rect_w = ui_rect_width(&frame_rect);
			int32_t rect_h = ui_rect_height(&frame_rect);

			frame_rect.rb.x = lt_tp_dis * cpe_cos_radians(lt_tp_radian + M_PI_2);
			frame_rect.lt.y = lt_tp_dis * cpe_sin_radians(lt_tp_radian + M_PI_2);
			frame_rect.lt.x = frame_rect.rb.x - rect_w;
			frame_rect.rb.y = frame_rect.lt.y + rect_h;
        }

        rect->lt.x = tile->m_data.pos.x + frame_rect.lt.x * tile->m_data.scale.x;
        rect->lt.y = tile->m_data.pos.y + frame_rect.lt.y * tile->m_data.scale.y;
        rect->rb.x = tile->m_data.pos.x + frame_rect.rb.x * tile->m_data.scale.x;
        rect->rb.y = tile->m_data.pos.y + frame_rect.rb.y * tile->m_data.scale.y;
    }
    else {
        return -1;
    }

    return 0;
}
    
uint32_t plugin_tiledmap_data_tile_src_id(plugin_tiledmap_data_tile_t tile) {
    return tile->m_data.ref_type == tiledmap_tile_ref_type_img
        ? tile->m_data.ref_data.img.module_id
        : (tile->m_data.ref_type == tiledmap_tile_ref_type_frame
           ? tile->m_data.ref_data.frame.sprite_id
           : 0);
}

ui_data_src_type_t plugin_tiledmap_data_tile_src_type(plugin_tiledmap_data_tile_t tile) {
    return tile->m_data.ref_type == tiledmap_tile_ref_type_img
        ? ui_data_src_type_module
        : (tile->m_data.ref_type == tiledmap_tile_ref_type_frame
           ? ui_data_src_type_sprite
           : ui_data_src_type_all);
}
