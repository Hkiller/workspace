#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/tailq_sort.h"
#include "plugin_scrollmap_data_script_i.h"

plugin_scrollmap_data_script_t
plugin_scrollmap_data_script_create(plugin_scrollmap_data_layer_t layer) {
    plugin_scrollmap_module_t module = layer->m_scene->m_module;
    plugin_scrollmap_data_script_t script;

    script = TAILQ_FIRST(&module->m_free_data_scripts);
    if (script) {
        TAILQ_REMOVE(&module->m_free_data_scripts, script, m_next);
    }
    else {
        script = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_data_script));
        if (script == NULL) {
            CPE_ERROR(module->m_em, "plugin_scrollmap_data_script_create: alloc fail!");
            return NULL;
        }
    }

    bzero(&script->m_data, sizeof(script->m_data));
    script->m_layer = layer;

    layer->m_script_count++;    
    TAILQ_INSERT_TAIL(&layer->m_scripts, script, m_next);

    return script;
}

void plugin_scrollmap_data_script_free(plugin_scrollmap_data_script_t script) {
    plugin_scrollmap_module_t module = script->m_layer->m_scene->m_module;

    assert(script->m_layer->m_script_count > 0);
    script->m_layer->m_script_count--;
    TAILQ_REMOVE(&script->m_layer->m_scripts, script, m_next);

    script->m_layer = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_data_scripts, script, m_next);
}

void plugin_scrollmap_data_script_real_free(plugin_scrollmap_data_script_t script) {
    plugin_scrollmap_module_t module = (void*)script->m_layer;
    TAILQ_REMOVE(&module->m_free_data_scripts, script, m_next);
    mem_free(module->m_alloc, script);
}

SCROLLMAP_SCRIPT * plugin_scrollmap_data_script_data(plugin_scrollmap_data_script_t script) {
    return &script->m_data;
}

static int plugin_scrollmap_manip_script_cmp(plugin_scrollmap_data_script_t l, plugin_scrollmap_data_script_t r, void * p) {
	return (int)((l->m_data.logic_pos.y + l->m_data.trigger_screen_pos_y) - (r->m_data.logic_pos.y + r->m_data.trigger_screen_pos_y));
}

void plugin_scrollmap_data_layer_sort_scripts(plugin_scrollmap_data_layer_t layer) {
    TAILQ_SORT(
        &layer->m_scripts,
        plugin_scrollmap_data_script,
        plugin_scrollmap_data_script_list,
        m_next,
        plugin_scrollmap_manip_script_cmp, NULL);
}

static plugin_scrollmap_data_script_t plugin_scrollmap_data_script_next(struct plugin_scrollmap_data_script_it * it) {
    plugin_scrollmap_data_script_t * data = (plugin_scrollmap_data_script_t *)(it->m_data);
    plugin_scrollmap_data_script_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_scrollmap_data_layer_scripts(plugin_scrollmap_data_layer_t layer, plugin_scrollmap_data_script_it_t it) {
    *(plugin_scrollmap_data_script_t *)(it->m_data) = TAILQ_FIRST(&layer->m_scripts);
    it->next = plugin_scrollmap_data_script_next;
}
