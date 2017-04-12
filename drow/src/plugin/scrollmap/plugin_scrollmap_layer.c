#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "plugin_scrollmap_layer_i.h"
#include "plugin_scrollmap_range_i.h"
#include "plugin_scrollmap_obj_i.h"

plugin_scrollmap_layer_t
plugin_scrollmap_layer_create(plugin_scrollmap_env_t env, const char * name) {
    plugin_scrollmap_layer_t layer;

    assert(plugin_scrollmap_layer_find(env, name) == NULL);

    layer = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_scrollmap_layer));
    if (layer == NULL) return NULL;

    layer->m_env = env;
    cpe_str_dup(layer->m_name, sizeof(layer->m_name), name);
    layer->m_speed_adj = 1.0f;
    layer->m_curent_pos = 0.0f;

    TAILQ_INIT(&layer->m_idle_ranges);
    TAILQ_INIT(&layer->m_active_ranges);
    TAILQ_INIT(&layer->m_canceling_ranges);
    TAILQ_INIT(&layer->m_done_ranges);
    TAILQ_INIT(&layer->m_blocks);
    TAILQ_INIT(&layer->m_scripts);
    TAILQ_INIT(&layer->m_land_objs);

    TAILQ_INSERT_TAIL(&env->m_layers, layer, m_next_for_env);

    return layer;
}

plugin_scrollmap_layer_t
plugin_scrollmap_layer_find(plugin_scrollmap_env_t env, const char * name) {
    plugin_scrollmap_layer_t layer;

    TAILQ_FOREACH(layer, &env->m_layers, m_next_for_env) {
        if (strcmp(layer->m_name, name) == 0) return layer;
    }

    return NULL;
}

void plugin_scrollmap_layer_free(plugin_scrollmap_layer_t layer) {
    plugin_scrollmap_env_t env = layer->m_env;
    
    TAILQ_REMOVE(&env->m_layers, layer, m_next_for_env);

    while(!TAILQ_EMPTY(&layer->m_blocks)) {
        plugin_scrollmap_block_free(TAILQ_FIRST(&layer->m_blocks));
    }

    while(!TAILQ_EMPTY(&layer->m_scripts)) {
        plugin_scrollmap_script_free(TAILQ_FIRST(&layer->m_scripts));
    }

    while(!TAILQ_EMPTY(&layer->m_land_objs)) {
        plugin_scrollmap_obj_free(TAILQ_FIRST(&layer->m_land_objs));
    }

    while(!TAILQ_EMPTY(&layer->m_idle_ranges)) {
        plugin_scrollmap_range_free(TAILQ_FIRST(&layer->m_idle_ranges));
    }

    while(!TAILQ_EMPTY(&layer->m_active_ranges)) {
        plugin_scrollmap_range_free(TAILQ_FIRST(&layer->m_active_ranges));
    }

    while(!TAILQ_EMPTY(&layer->m_canceling_ranges)) {
        plugin_scrollmap_range_free(TAILQ_FIRST(&layer->m_canceling_ranges));
    }

    while(!TAILQ_EMPTY(&layer->m_done_ranges)) {
        plugin_scrollmap_range_free(TAILQ_FIRST(&layer->m_done_ranges));
    }

    mem_free(env->m_module->m_alloc, layer);
}

plugin_scrollmap_env_t plugin_scrollmap_layer_env(plugin_scrollmap_layer_t layer) {
    return layer->m_env;
}

const char * plugin_scrollmap_layer_name(plugin_scrollmap_layer_t layer) {
    return layer->m_name;
}

static plugin_scrollmap_block_t plugin_scrollmap_layer_block_next(plugin_scrollmap_block_it_t it) {
    plugin_scrollmap_block_t * data = (plugin_scrollmap_block_t *)it->m_data;
    plugin_scrollmap_block_t r;

    if (*data == NULL) return NULL;

    r = *data;
    *data = TAILQ_NEXT(r, m_next_for_layer);

    return r;
}

void plugin_scrollmap_layer_blocks(plugin_scrollmap_block_it_t it, plugin_scrollmap_layer_t layer) {
    plugin_scrollmap_block_t * data = (plugin_scrollmap_block_t *)it->m_data;

    *data = TAILQ_FIRST(&layer->m_blocks);
    it->m_next = plugin_scrollmap_layer_block_next;
}

void plugin_scrollmap_layer_cancel_loop(plugin_scrollmap_layer_t layer) {
    plugin_scrollmap_range_t range;

    TAILQ_FOREACH(range, &layer->m_active_ranges, m_next_for_layer) {
        plugin_scrollmap_range_cancel_loop(range);
    }
}

static plugin_scrollmap_layer_t plugin_scrollmap_layer_child_next(struct plugin_scrollmap_layer_it * it) {
    plugin_scrollmap_layer_t * data = (plugin_scrollmap_layer_t *)(it->m_data);
    plugin_scrollmap_layer_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_env);

    return r;
}

void plugin_scrollmap_env_layers(plugin_scrollmap_layer_it_t it, plugin_scrollmap_env_t env) {
    *(plugin_scrollmap_layer_t *)(it->m_data) = TAILQ_FIRST(&env->m_layers);
    it->next = plugin_scrollmap_layer_child_next;
}
