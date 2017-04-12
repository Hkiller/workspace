#include "cpe/utils/string_utils.h"
#include "render/utils/ui_rect.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin_tiledmap_layer_i.h"
#include "plugin_tiledmap_tile_i.h"

plugin_tiledmap_layer_t
plugin_tiledmap_layer_create(plugin_tiledmap_env_t env, const char * name) {
    plugin_tiledmap_module_t module = env->m_module;
    plugin_tiledmap_layer_t layer;

    layer = mem_alloc(module->m_alloc, sizeof(struct plugin_tiledmap_layer));
    if (layer == NULL) {
        CPE_ERROR(module->m_em, "plugin_tiledmap_layer_create: alloc fail!");
        return NULL;
    }

    layer->m_env = env;
    cpe_str_dup(layer->m_name, sizeof(layer->m_name), name);
    TAILQ_INIT(&layer->m_tiles);
    layer->m_trans = UI_TRANSFORM_IDENTITY;
    layer->m_alpha = 1.0f;
    
    cpe_hash_entry_init(&layer->m_hh_for_env);
    if (cpe_hash_table_insert_unique(&env->m_layers, layer) != 0) {
        CPE_ERROR(module->m_em, "plugin_tiledmap_layer_create: insert layer %s fail!", name);
        mem_free(module->m_alloc, layer);
        return NULL;
    }

    return layer;
}

void plugin_tiledmap_layer_free(plugin_tiledmap_layer_t layer) {
    plugin_tiledmap_env_t env = layer->m_env;

    while(!TAILQ_EMPTY(&layer->m_tiles)) {
        plugin_tiledmap_tile_free(TAILQ_FIRST(&layer->m_tiles));
    }

    cpe_hash_table_remove_by_ins(&env->m_layers, layer);

    mem_free(env->m_module->m_alloc, layer);
}

void plugin_tiledmap_layer_free_all(plugin_tiledmap_env_t env) {
    struct cpe_hash_it layer_it;
    plugin_tiledmap_layer_t layer;

    cpe_hash_it_init(&layer_it, &env->m_layers);

    layer = cpe_hash_it_next(&layer_it);
    while (layer) {
        plugin_tiledmap_layer_t next = cpe_hash_it_next(&layer_it);
        plugin_tiledmap_layer_free(layer);
        layer = next;
    }
}

const char * plugin_tiledmap_layer_name(plugin_tiledmap_layer_t layer) {
    return layer->m_name;
}

plugin_tiledmap_layer_t plugin_tiledmap_layer_find(plugin_tiledmap_env_t env, const char * name) {
    struct plugin_tiledmap_layer key;
    cpe_str_dup(key.m_name, sizeof(key.m_name), name);
    return cpe_hash_table_find(&env->m_layers, &key);
}

static int plugin_tiledmap_layer_load_one(
    plugin_tiledmap_layer_t layer, ui_rect_t to_rect,
    plugin_tiledmap_data_layer_t layer_data, ui_rect_t from_rect,
    plugin_tiledmap_fill_base_policy_t base_policy)
{
    plugin_tiledmap_module_t module = layer->m_env->m_module;
    /* float from_width = from_rect->rb.x - from_rect->lt.x; */
    /* float from_height = from_rect->rb.y - from_rect->lt.y; */
    /* float to_width = to_rect->rb.x - to_rect->lt.x; */
    /* float to_height = to_rect->rb.y - to_rect->rb.y; */
    float pos_adj_x;
    float pos_adj_y;
    struct plugin_tiledmap_data_tile_it tile_it;
    plugin_tiledmap_data_tile_t data_tile;
    ui_data_src_t src_cache = NULL;

    switch(base_policy) {
    case plugin_tiledmap_base_policy_left_top:
        pos_adj_x = to_rect->lt.x - from_rect->lt.x;
        pos_adj_y = to_rect->lt.y - from_rect->lt.y;
        break;
    case plugin_tiledmap_base_policy_left_bottom:
        pos_adj_x = to_rect->lt.x - from_rect->lt.x;
        pos_adj_y = to_rect->rb.y - from_rect->rb.y;
        break;
    case plugin_tiledmap_base_policy_right_top:
        pos_adj_x = to_rect->rb.x - from_rect->rb.x;
        pos_adj_y = to_rect->rb.y - from_rect->rb.y;
        break;
    case plugin_tiledmap_base_policy_right_bottom:
        pos_adj_x = to_rect->rb.x - from_rect->rb.x;
        pos_adj_y = to_rect->rb.y - from_rect->rb.y;
        break;
    default:
        CPE_ERROR(module->m_em, "plugin_tiledmap_layer_load: unknown base policy %d!", base_policy);
        return -1;
    };

    plugin_tiledmap_data_layer_tiles(&tile_it, layer_data);
    while((data_tile = plugin_tiledmap_data_layer_it_next(&tile_it))) {
        TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(data_tile);
        ui_rect tile_rect;
        plugin_tiledmap_tile_t tile;
        ui_vector_2 pos;
        uint8_t ignore = 0;
        
        if (plugin_tiledmap_data_tile_rect(data_tile, &tile_rect, &src_cache) != 0) {
            CPE_ERROR(module->m_em, "plugin_tiledmap_layer_load: calc tile rect faild!");
            return -1;
        }

        if (tile_rect.lt.x > from_rect->rb.x || tile_rect.lt.y > from_rect->rb.y
            || tile_rect.rb.x < from_rect->lt.x || tile_rect.rb.y < from_rect->lt.y)
        {
            if (module->m_debug) {
                CPE_INFO(
                    module->m_em, "plugin_tiledmap_layer_load: layer %s: tile=(%f,%f)-(%f,%f) ignore for from-rect=(%f,%f)-(%f,%f), pos-adj=(%f,%f)\n",
                    layer->m_name,
                    tile_rect.lt.x, tile_rect.lt.y, tile_rect.rb.x, tile_rect.rb.y,
                    from_rect->lt.x, from_rect->lt.y, from_rect->rb.x, from_rect->rb.y,
                    pos_adj_x, pos_adj_y);
            }

            continue;
        }

        tile_rect.lt.x += pos_adj_x;
        tile_rect.lt.y += pos_adj_y;
        tile_rect.rb.x += pos_adj_x;
        tile_rect.rb.y += pos_adj_y;

        if (tile_rect.lt.x > to_rect->rb.x || tile_rect.lt.y > to_rect->rb.y
            || tile_rect.rb.x < to_rect->lt.x || tile_rect.rb.y < to_rect->lt.y)
        {
            if (module->m_debug) {
                CPE_INFO(
                    module->m_em, "plugin_tiledmap_layer_load: layer %s: tile=(%f,%f)-(%f,%f) ignore for to-rect=(%f,%f)-(%f,%f), pos-adj=(%f,%f)\n",
                   layer->m_name,
                   tile_rect.lt.x, tile_rect.lt.y, tile_rect.rb.x, tile_rect.rb.y,
                   to_rect->lt.x, to_rect->lt.y, to_rect->rb.x, to_rect->rb.y,
                   pos_adj_x, pos_adj_y);
            }
            continue;
        }

        pos.x = tile_data->pos.x + pos_adj_x;
        pos.y = tile_data->pos.y + pos_adj_y;

        if (layer->m_env->m_extern_obj_create_fun) {
            if (layer->m_env->m_extern_obj_create_fun(layer->m_env->m_extern_obj_create_ctx, layer, &ignore, &pos, data_tile) != 0) {
                CPE_ERROR(module->m_em, "plugin_tiledmap_layer_load: create extern obj faild!");
                return -1;
            }
        }
        if (ignore) continue;
        
        tile = plugin_tiledmap_tile_create(layer, data_tile, &pos, src_cache);
        if (tile == NULL) {
            CPE_ERROR(module->m_em, "plugin_tiledmap_layer_load: create tile faild!");
            return -1;
        }
    }

    return 0;
}

int plugin_tiledmap_layer_load(
    plugin_tiledmap_layer_t layer, ui_rect_t input_to_rect,
    plugin_tiledmap_data_layer_t layer_data, ui_rect_t from_rect,
    plugin_tiledmap_fill_base_policy_t base_policy,
    plugin_tiledmap_fill_repeat_policy_t repeat_policy)
{
    plugin_tiledmap_module_t module = layer->m_env->m_module;
    ui_rect local_from_rect;
    float from_width;
    float from_height;

    if (from_rect == NULL) {
        TILEDMAP_LAYER const * layer_data_data = plugin_tiledmap_data_layer_data(layer_data);
        local_from_rect.lt.x = layer_data_data->cell_col_begin * layer_data_data->cell_w;
        local_from_rect.lt.y = layer_data_data->cell_row_begin * layer_data_data->cell_h;
        local_from_rect.rb.x = layer_data_data->cell_col_end * layer_data_data->cell_w;
        local_from_rect.rb.y = layer_data_data->cell_row_end * layer_data_data->cell_h;
        from_rect = &local_from_rect;
    }

    from_width = from_rect->rb.x - from_rect->lt.x;
    from_height = from_rect->rb.y - from_rect->rb.y;

    switch(repeat_policy) {
    case plugin_tiledmap_repeat_policy_none:
        return plugin_tiledmap_layer_load_one(layer, input_to_rect, layer_data, from_rect, base_policy);
    case plugin_tiledmap_repeat_policy_x: {
        ui_rect to_rect = *input_to_rect;
        do {
            int r = plugin_tiledmap_layer_load_one(layer, &to_rect, layer_data, from_rect, base_policy);
            if (r != 0) return r;

            if ((to_rect.rb.x - to_rect.lt.x) <= from_width) break;
        
            if (base_policy == plugin_tiledmap_base_policy_left_top || base_policy == plugin_tiledmap_base_policy_left_bottom) {
                to_rect.lt.x += from_width;
            }
            else {
                to_rect.rb.x -= from_width;
            }
        } while(0);
        break;
    }
    case plugin_tiledmap_repeat_policy_y: {
        ui_rect to_rect = *input_to_rect;
        do {
            int r = plugin_tiledmap_layer_load_one(layer, &to_rect, layer_data, from_rect, base_policy);
            if (r != 0) return r;

            if ((to_rect.rb.y - to_rect.lt.y) <= from_height) break;
        
            if (base_policy == plugin_tiledmap_base_policy_left_top || base_policy == plugin_tiledmap_base_policy_right_top) {
                to_rect.lt.y += from_height;
            }
            else {
                to_rect.rb.y -= from_height;
            }
        } while(0);
        break;
    }
    case plugin_tiledmap_repeat_policy_xy: {
        float to_width = input_to_rect->rb.x - input_to_rect->lt.x;
        float to_height = input_to_rect->rb.y - input_to_rect->lt.y;
        uint32_t x_count = (uint32_t)(to_width / from_width);
        uint32_t y_count = (uint32_t)(to_height / from_height);
        float x, y;

        if (plugin_tiledmap_layer_load_one(layer, input_to_rect, layer_data, from_rect, base_policy) != 0) return -1;

        for(x = 0; x < x_count; ++x) {
            for(y = 0; y < y_count; ++y) {
                ui_rect to_rect = *input_to_rect;
                
                if (x == 0 && y == 0) continue;

                if (base_policy == plugin_tiledmap_base_policy_left_top || base_policy == plugin_tiledmap_base_policy_right_top) {
                    to_rect.lt.y += y * from_height;
                }
                else {
                    to_rect.rb.y -= y * from_height;
                }

                if (base_policy == plugin_tiledmap_base_policy_left_top || base_policy == plugin_tiledmap_base_policy_left_bottom) {
                    to_rect.lt.x += x * from_width;
                }
                else {
                    to_rect.rb.x -= x * from_width;
                }

                if (plugin_tiledmap_layer_load_one(layer, &to_rect, layer_data, from_rect, base_policy) != 0) return -1;
            }
        }
        
        break;
    }
    default:
        CPE_ERROR(module->m_em, "plugin_tiledmap_layer_load: unknown repeat policy %d!", repeat_policy);
        return -1;
    }

    return 0;
}

static plugin_tiledmap_tile_t plugin_tiledmap_layer_tile_next(struct plugin_tiledmap_tile_it * it) {
    plugin_tiledmap_tile_t * data = (plugin_tiledmap_tile_t *)(it->m_data);
    plugin_tiledmap_tile_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

ui_transform_t plugin_tiledmap_layer_trans(plugin_tiledmap_layer_t layer) {
    return &layer->m_trans;
}

void plugin_tiledmap_layer_set_trans(plugin_tiledmap_layer_t layer, ui_transform_t trans) {
    layer->m_trans = *trans;
}

float plugin_tiledmap_layer_alpha(plugin_tiledmap_layer_t layer) {
    return layer->m_alpha;
}
    
void plugin_tiledmap_layer_set_alpha(plugin_tiledmap_layer_t layer, float alpha) {
    layer->m_alpha = alpha;
}

void plugin_tiledmap_layer_tiles(plugin_tiledmap_tile_it_t tile_it, plugin_tiledmap_layer_t layer) {
    *(plugin_tiledmap_tile_t *)(tile_it->m_data) = TAILQ_FIRST(&layer->m_tiles);
    tile_it->next = plugin_tiledmap_layer_tile_next;
}

int plugin_tiledmap_layer_rect(plugin_tiledmap_layer_t layer, ui_rect_t rect) {
    uint8_t have_data = 0;
    plugin_tiledmap_tile_t tile;
    ui_data_src_t src_cache = NULL;

    TAILQ_FOREACH(tile, &layer->m_tiles, m_next) {
        ui_rect tile_rect;
        if (plugin_tiledmap_data_tile_rect(tile->m_data_tile, &tile_rect, &src_cache) != 0) return -1;

        if (have_data) {
            if (tile_rect.lt.x < rect->lt.x) rect->lt.x = tile_rect.lt.x;
            if (tile_rect.lt.y < rect->lt.y) rect->lt.y = tile_rect.lt.y;
            if (tile_rect.rb.x > rect->rb.x) rect->rb.x = tile_rect.rb.x;
            if (tile_rect.rb.y > rect->rb.y) rect->rb.y = tile_rect.rb.y;
        }
        else {
            have_data = 1;
            *rect = tile_rect;
        }
    }

    if (!have_data) {
        rect->lt.x = 0.0f;
        rect->lt.y = 0.0f;
        rect->rb.x = 0.0f;
        rect->rb.y = 0.0f;
    }

    return 0;
}

uint32_t plugin_tiledmap_hash(plugin_tiledmap_layer_t layer) {
    return cpe_hash_str(layer->m_name, strlen(layer->m_name));
}

uint32_t plugin_tiledmap_eq(plugin_tiledmap_layer_t l, plugin_tiledmap_layer_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}
