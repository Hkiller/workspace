#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_texture.h"
#include "plugin_scrollmap_tile_i.h"

plugin_scrollmap_tile_t
plugin_scrollmap_tile_create(plugin_scrollmap_env_t env, SCROLLMAP_TILE const * tile_data) {
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_tile_t tile;

    tile = TAILQ_FIRST(&env->m_free_tiles);
    if (tile) {
        TAILQ_REMOVE(&env->m_free_tiles, tile, m_next);
    }
    else {
        tile = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_scrollmap_tile));
        if (tile == NULL) {
            CPE_ERROR(module->m_em, "scrollmap_tile_create: alloc fail!");
            return NULL;
        }
    }

    tile->m_env = env;
    tile->m_data = tile_data;
    tile->m_transform = UI_TRANSFORM_IDENTITY;

    cpe_hash_entry_init(&tile->m_hh);
    if (cpe_hash_table_insert(&env->m_tiles, tile) != 0) {
        CPE_ERROR(module->m_em, "scrollmap_tile_create: tile data duplicate!");
        TAILQ_INSERT_TAIL(&env->m_free_tiles, tile, m_next);
        return NULL;
    }

    if (tile_data->res_type == scrollmap_tile_res_type_module) {
        ui_data_mgr_t data_mgr = ui_runtime_module_data_mgr(env->m_module->m_runtime);
        ui_data_src_t src;
        ui_data_module_t data_module;
        ui_data_img_block_t img_block;
        UI_IMG_BLOCK const * img_block_data;
		uint8_t flip_way = tile_data->flip_way;
		uint8_t angle_way = tile_data->angle_way;
        ui_quaternion o_r = UI_QUATERNION_IDENTITY;
        ui_vector_3 o_s;
        ui_vector_3 o_t;
        float invw;
        float invh;
        ui_cache_res_t texture;
        
        src =  ui_data_src_find_by_id(data_mgr, tile_data->src_id);
        if (src == NULL) {
            CPE_ERROR(module->m_em, "scrollmap_tile_create: src %d not exist", tile_data->src_id);
            goto LOAD_RES_FAIL;
        }

		if (angle_way == scrollmap_tile_angle_type_180) {
			flip_way ^= scrollmap_tile_flip_type_xy;
			angle_way = scrollmap_tile_angle_type_none;
		}
		else if (angle_way == scrollmap_tile_angle_type_270) {
			flip_way ^= scrollmap_tile_flip_type_xy;
			angle_way = scrollmap_tile_angle_type_90;
		}

		if(angle_way == scrollmap_tile_angle_type_90) {
            ui_quaternion_set_z_radians(&o_r, (float)(M_PI / 2.0f));
		}

        o_s.x = (flip_way & 0x01) ? -1.0f : 1.0f;
        o_s.y = (flip_way & 0x02) ? -1.0f : 1.0f;
        o_s.z = 1.0f;

        o_t.x = tile_data->res_w / 2.0f;
        o_t.y = - tile_data->res_h / 2.0f;
        o_t.z = 0.0f;

        ui_transform_set_pos_3(&tile->m_transform, &o_t);
        ui_transform_set_quation_scale(&tile->m_transform, &o_r, &o_s);

        src =  ui_data_src_find_by_id(data_mgr, tile_data->src_id);
        if (src == NULL) {
            CPE_ERROR(module->m_em, "scrollmap_tile_create: src %d not exist", tile_data->src_id);
            goto LOAD_RES_FAIL;
        }

        if (ui_data_src_product(src) == NULL) {
            CPE_ERROR(module->m_em, "scrollmap_tile_create: src %d not loaded", tile_data->src_id);
            goto LOAD_RES_FAIL;
        }

        if (ui_data_src_type(src) != ui_data_src_type_module) {
            CPE_ERROR(module->m_em, "scrollmap_tile_create: src %d is not module", tile_data->src_id);
            goto LOAD_RES_FAIL;
        }
            
        data_module = ui_data_src_product(src);
        
        img_block = ui_data_img_block_find_by_id(data_module, tile_data->res_id);            
        if (img_block == NULL) {
            CPE_ERROR(module->m_em, "scrollmap_tile_create: src %d no img block %d", tile_data->src_id, tile_data->res_id);
            goto LOAD_RES_FAIL;
        }

        img_block_data = ui_data_img_block_data(img_block);
        texture = ui_data_img_block_using_texture(img_block);
        if (texture == NULL) {
            CPE_ERROR(
                module->m_em, "scrollmap_tile_create: use texture %s not exsit",
                ui_data_img_block_using_texture_path(img_block));
            goto LOAD_RES_FAIL;
        }

        invw = 1.0f / ui_cache_texture_width(texture);
        invh = 1.0f / ui_cache_texture_height(texture);
        
        tile->m_module.m_uv.lt.x = (img_block_data->src_x + 0.375f) * invw;
        tile->m_module.m_uv.lt.y = (img_block_data->src_y + 0.375f) * invh;
        tile->m_module.m_uv.rb.x = (img_block_data->src_x + img_block_data->src_w - 0.375f) * invw;
        tile->m_module.m_uv.rb.y = (img_block_data->src_y + img_block_data->src_h - 0.375f) * invh;

		/* uv1.x = (x  +0.375f)		*invw; */
        /* uv1.y = (y  +0.375f)			*invh; */
		/* uv2.x = (x+w-0.375f)		*invw; */
        /* uv2.y = (y+h-0.375f)			*invh; */
		/* uv3.x = (x  +0.375f)		*invw; */
        /* uv3.y = (y+h-0.375f)			*invh; */
		/* uv4.x = (x+w-0.375f)		*invw; */
        /* uv4.y = (y  +0.375f)			*invh; */
        
    }
    else if (tile_data->res_type == scrollmap_tile_res_type_tag) {
		uint8_t flip_way = tile_data->flip_way;
		uint8_t angle_way = tile_data->angle_way;
        ui_quaternion o_r = UI_QUATERNION_IDENTITY;
        ui_vector_3 o_s;
        ui_vector_3 o_t;

		if (angle_way == scrollmap_tile_angle_type_180) {
			flip_way ^= scrollmap_tile_flip_type_xy;
			angle_way = scrollmap_tile_angle_type_none;
		}
		else if (angle_way == scrollmap_tile_angle_type_270) {
			flip_way ^= scrollmap_tile_flip_type_xy;
			angle_way = scrollmap_tile_angle_type_90;
		}

		if(angle_way == scrollmap_tile_angle_type_90) {
            ui_quaternion_set_z_radians(&o_r, (float)(M_PI / 2.0f));
		}

        o_s.x = (flip_way & 0x01) ? -1.0f : 1.0f;
        o_s.y = (flip_way & 0x02) ? -1.0f : 1.0f;
        o_s.z = 1.0f;

        o_t.x = tile_data->res_w / 2.0f;
        o_t.y = - tile_data->res_h / 2.0f;
        o_t.z = 0.0f;

        ui_transform_set_pos_3(&tile->m_transform, &o_t);
        ui_transform_set_quation_scale(&tile->m_transform, &o_r, &o_s);
    }
    else {
        CPE_ERROR(module->m_em, "scrollmap_tile_create: not support type = %d", tile_data->res_type);
        goto LOAD_RES_FAIL;
    }
    
    return tile;

LOAD_RES_FAIL:
    cpe_hash_table_remove_by_ins(&env->m_tiles, tile);
    TAILQ_INSERT_TAIL(&env->m_free_tiles, tile, m_next);
    return NULL; 
}

void plugin_scrollmap_tile_free(plugin_scrollmap_tile_t tile) {
    plugin_scrollmap_env_t env = tile->m_env;

    cpe_hash_table_remove_by_ins(&env->m_tiles, tile);
    TAILQ_INSERT_TAIL(&env->m_free_tiles, tile, m_next);
}

void plugin_scrollmap_tile_real_free(plugin_scrollmap_tile_t tile) {
    plugin_scrollmap_env_t env = tile->m_env;
    
    TAILQ_REMOVE(&env->m_free_tiles, tile, m_next);
    mem_free(env->m_module->m_alloc, tile);
}

plugin_scrollmap_tile_t plugin_scrollmap_tile_find(plugin_scrollmap_env_t env, SCROLLMAP_TILE const * tile_data) {
    struct plugin_scrollmap_tile key;
    key.m_data = tile_data;
    return cpe_hash_table_find(&env->m_tiles, &key);
}

void plugin_scrollmap_tile_free_all(plugin_scrollmap_env_t env) {
    struct cpe_hash_it tile_it;
    plugin_scrollmap_tile_t tile;

    cpe_hash_it_init(&tile_it, &env->m_tiles);

    tile = (plugin_scrollmap_tile_t)cpe_hash_it_next(&tile_it);
    while (tile) {
        plugin_scrollmap_tile_t next = (plugin_scrollmap_tile_t)cpe_hash_it_next(&tile_it);
        plugin_scrollmap_tile_free(tile);
        tile = next;
    }
}

uint32_t plugin_scrollmap_tile_hash(const plugin_scrollmap_tile_t tile) {
    return (uint32_t)((ptr_int_t)tile->m_data);
}

int plugin_scrollmap_tile_eq(const plugin_scrollmap_tile_t l, const plugin_scrollmap_tile_t r) {
    return l->m_data == r->m_data;
}
