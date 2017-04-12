#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_scrollmap_data_layer_i.h"

plugin_scrollmap_data_layer_t
plugin_scrollmap_data_layer_create(plugin_scrollmap_data_scene_t scene) {
    plugin_scrollmap_module_t module = scene->m_module;
    plugin_scrollmap_data_layer_t layer;

    layer = TAILQ_FIRST(&module->m_free_data_layers);
    if (layer) {
        TAILQ_REMOVE(&module->m_free_data_layers, layer, m_next);
    }
    else {
        layer = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_data_layer));
        if (layer == NULL) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_data_layer_create: alloc fail!");
            return NULL;
        }
    }

    bzero(&layer->m_data, sizeof(layer->m_data));
    layer->m_scene = scene;
    layer->m_script_count = 0;
    layer->m_block_count = 0;
    TAILQ_INIT(&layer->m_scripts);
    TAILQ_INIT(&layer->m_blocks);

    scene->m_layer_count++;
    TAILQ_INSERT_TAIL(&scene->m_layers, layer, m_next);

    return layer;
}

void plugin_scrollmap_data_layer_free(plugin_scrollmap_data_layer_t layer) {
    plugin_scrollmap_module_t module = layer->m_scene->m_module;

    while(!TAILQ_EMPTY(&layer->m_blocks)) {
        plugin_scrollmap_data_block_free(TAILQ_FIRST(&layer->m_blocks));
    }
    assert(layer->m_block_count == 0);

    while(!TAILQ_EMPTY(&layer->m_scripts)) {
        plugin_scrollmap_data_script_free(TAILQ_FIRST(&layer->m_scripts));
    }
    assert(layer->m_script_count == 0);

    assert(layer->m_scene->m_layer_count > 0);
    layer->m_scene->m_layer_count--;
    TAILQ_REMOVE(&layer->m_scene->m_layers, layer, m_next);

    layer->m_scene = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_data_layers, layer, m_next);
}

void plugin_scrollmap_data_layer_real_free(plugin_scrollmap_data_layer_t layer) {
    plugin_scrollmap_module_t module = (void*)layer->m_scene;
    TAILQ_REMOVE(&module->m_free_data_layers, layer, m_next);
    mem_free(module->m_alloc, layer);
}

plugin_scrollmap_data_layer_t
plugin_scrollmap_data_layer_find(plugin_scrollmap_data_scene_t scene, const char * name) {
    plugin_scrollmap_data_layer_t layer;

    TAILQ_FOREACH(layer, &scene->m_layers, m_next) {
        if (strcmp(layer->m_data.name, name) == 0) return layer;
    }

    return NULL;
}

SCROLLMAP_LAYER * plugin_scrollmap_data_layer_data(plugin_scrollmap_data_layer_t layer) {
    return &layer->m_data;
}

static plugin_scrollmap_data_layer_t plugin_scrollmap_data_layer_next(struct plugin_scrollmap_data_layer_it * it) {
    plugin_scrollmap_data_layer_t * data = (plugin_scrollmap_data_layer_t *)(it->m_data);
    plugin_scrollmap_data_layer_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_scrollmap_data_scene_layers(plugin_scrollmap_data_scene_t scene, plugin_scrollmap_data_layer_it_t it) {
    *(plugin_scrollmap_data_layer_t *)(it->m_data) = TAILQ_FIRST(&scene->m_layers);
    it->next = plugin_scrollmap_data_layer_next;
}
