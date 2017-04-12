#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "plugin_scrollmap_range_i.h"
#include "plugin_scrollmap_block_i.h"

static void plugin_scrollmap_range_dequeue(plugin_scrollmap_range_t range);
static void plugin_scrollmap_range_enqueue(plugin_scrollmap_range_t range);

plugin_scrollmap_range_t
plugin_scrollmap_range_create(
    plugin_scrollmap_layer_t layer, plugin_scrollmap_source_t source,
    float start_pos)
{
    plugin_scrollmap_env_t env = layer->m_env;
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_data_layer_t layer_data;
    plugin_scrollmap_range_t range;

    layer_data = plugin_scrollmap_data_layer_find(source->m_data, layer->m_name);
    if (layer_data == NULL) {
        CPE_ERROR(
            module->m_em, "layer %s not exist in data source %s",
            layer->m_name,
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), source->m_data->m_src));
        return NULL;
    }
    
    range = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_range));
    if (range == NULL) {
        CPE_ERROR(module->m_em, "create env_range fail");
        return NULL;
    }

    range->m_layer = layer;
    range->m_source = source;
    range->m_layer_data = layer_data;
    range->m_loop_begin = 0.0f;
    range->m_loop_end = 0.0f;
    range->m_loop_blocks_begin = NULL;
    range->m_loop_scripts_begin = NULL;
    range->m_next_block = NULL;
    range->m_next_script = NULL;
    range->m_start_pos = start_pos;
    range->m_logic_pos = 0.0f;
    range->m_state = plugin_scrollmap_range_state_idle;

    TAILQ_INIT(&range->m_blocks);
    TAILQ_INIT(&range->m_scripts);

    plugin_scrollmap_range_enqueue(range);

    TAILQ_INSERT_TAIL(&source->m_ranges, range, m_next_for_source);

    return range;
}

void plugin_scrollmap_range_free(plugin_scrollmap_range_t range) {
    plugin_scrollmap_env_t env = range->m_layer->m_env;

    while(!TAILQ_EMPTY(&range->m_blocks)) {
        plugin_scrollmap_block_free(TAILQ_FIRST(&range->m_blocks));
    }

    while(!TAILQ_EMPTY(&range->m_scripts)) {
        plugin_scrollmap_script_set_range(TAILQ_FIRST(&range->m_scripts), NULL);
    }

    plugin_scrollmap_range_dequeue(range);

    TAILQ_REMOVE(&range->m_source->m_ranges, range, m_next_for_source);

    mem_free(env->m_module->m_alloc, range);
}

static void plugin_scrollmap_range_dequeue(plugin_scrollmap_range_t range) {
    plugin_scrollmap_layer_t layer = range->m_layer;

    switch(range->m_state) {
    case plugin_scrollmap_range_state_idle:
        TAILQ_REMOVE(&layer->m_idle_ranges, range, m_next_for_layer);
        break;
    case plugin_scrollmap_range_state_active:
        TAILQ_REMOVE(&layer->m_active_ranges, range, m_next_for_layer);
        break;
    case plugin_scrollmap_range_state_canceling:
        TAILQ_REMOVE(&layer->m_canceling_ranges, range, m_next_for_layer);
        break;
    case plugin_scrollmap_range_state_done:
        TAILQ_REMOVE(&layer->m_done_ranges, range, m_next_for_layer);
        break;
    }
}

static void plugin_scrollmap_range_enqueue(plugin_scrollmap_range_t range) {
    plugin_scrollmap_layer_t layer = range->m_layer;

    switch(range->m_state) {
    case plugin_scrollmap_range_state_idle:
        TAILQ_INSERT_TAIL(&layer->m_idle_ranges, range, m_next_for_layer);
        break;
    case plugin_scrollmap_range_state_active:
        TAILQ_INSERT_TAIL(&layer->m_active_ranges, range, m_next_for_layer);
        break;
    case plugin_scrollmap_range_state_canceling:
        TAILQ_INSERT_TAIL(&layer->m_canceling_ranges, range, m_next_for_layer);
        break;
    case plugin_scrollmap_range_state_done:
        TAILQ_INSERT_TAIL(&layer->m_done_ranges, range, m_next_for_layer);
        break;
    default:
        assert(0);
    }
}

void plugin_scrollmap_range_set_state(plugin_scrollmap_range_t range, plugin_scrollmap_range_state_t state) {
    plugin_scrollmap_range_dequeue(range);
    range->m_state = state;
    plugin_scrollmap_range_enqueue(range);
}

plugin_scrollmap_range_state_t plugin_scrollmap_range_state(plugin_scrollmap_range_t range) {
    return range->m_state;
}

int plugin_scrollmap_range_cancel(plugin_scrollmap_range_t range) {
    switch(range->m_state) {
    case plugin_scrollmap_range_state_idle:
        plugin_scrollmap_range_set_state(range, plugin_scrollmap_range_state_done);
        return 0;
    case plugin_scrollmap_range_state_active:
        plugin_scrollmap_range_set_state(range, plugin_scrollmap_range_state_canceling);
        return 0;
    case plugin_scrollmap_range_state_canceling:
        return 0;
    case plugin_scrollmap_range_state_done:
        return 0;
    default:
        return -1;
    }
}

static plugin_scrollmap_range_t plugin_scrollmap_range_child_next(struct plugin_scrollmap_range_it * it) {
    plugin_scrollmap_range_t * data = (plugin_scrollmap_range_t *)(it->m_data);
    plugin_scrollmap_range_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_layer);

    return r;
}

void plugin_scrollmap_layer_active_ranges(plugin_scrollmap_range_it_t it, plugin_scrollmap_layer_t layer) {
    *(plugin_scrollmap_range_t *)(it->m_data) = TAILQ_FIRST(&layer->m_active_ranges);
    it->next = plugin_scrollmap_range_child_next;
}

void plugin_scrollmap_range_cancel_loop(plugin_scrollmap_range_t range) {
    range->m_loop_begin = 0.0f;
    range->m_loop_end = 0.0f;
    range->m_loop_blocks_begin = NULL;
    range->m_loop_scripts_begin = NULL;
    range->m_loop_count = 0;
    range->m_is_first_loop = 0;
}
