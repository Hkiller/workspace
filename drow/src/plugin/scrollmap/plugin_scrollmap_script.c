#include "cpe/dr/dr_json.h"
#include "cpe/utils/stream_buffer.h"
#include "plugin_scrollmap_script_i.h"
#include "plugin_scrollmap_range_i.h"

plugin_scrollmap_script_t
plugin_scrollmap_script_create(
    plugin_scrollmap_layer_t layer, SCROLLMAP_SCRIPT const * script_data, ui_vector_2_t pos)
{
    plugin_scrollmap_env_t env = layer->m_env;
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_script_t script;

    script = TAILQ_FIRST(&env->m_free_scripts);
    if (script) {
        TAILQ_REMOVE(&env->m_free_scripts, script, m_next_for_layer);
    }
    else {
        script = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_script));
        if (script == NULL) return NULL;
    }
    
    script->m_layer = layer;
    script->m_script = *script_data;
    script->m_state = plugin_scrollmap_script_state_wait;
    script->m_pos = *pos;
    script->m_range = NULL;
    
    TAILQ_INSERT_TAIL(&layer->m_scripts, script, m_next_for_layer);

    return script;
}

void plugin_scrollmap_script_free(plugin_scrollmap_script_t script) {
    plugin_scrollmap_layer_t layer = script->m_layer;

    TAILQ_REMOVE(&layer->m_scripts, script, m_next_for_layer);

    if (script->m_range) {
        TAILQ_REMOVE(&script->m_range->m_scripts, script, m_next_for_range);
        script->m_range = NULL;
    }

    script->m_layer = (void*)layer->m_env;
    TAILQ_INSERT_HEAD(&layer->m_env->m_free_scripts, script, m_next_for_layer);
}

void plugin_scrollmap_script_real_free(plugin_scrollmap_script_t script) {
    plugin_scrollmap_env_t env = (void*)script->m_layer;

    TAILQ_REMOVE(&env->m_free_scripts, script, m_next_for_layer);

    mem_free(env->m_module->m_alloc, script);
}

plugin_scrollmap_script_state_t plugin_scrollmap_script_state(plugin_scrollmap_script_t script) {
    return script->m_state;
}

plugin_scrollmap_layer_t plugin_scrollmap_script_layer(plugin_scrollmap_script_t script) {
    return script->m_layer;
}

SCROLLMAP_SCRIPT const * plugin_scrollmap_script_data(plugin_scrollmap_script_t script) {
	return &script->m_script;
}

ui_vector_2_t plugin_scrollmap_script_pos(plugin_scrollmap_script_t script) {
    return &script->m_pos;
}

void plugin_scrollmap_script_set_pos(plugin_scrollmap_script_t script, ui_vector_2_t pos) {
    script->m_pos = *pos;
}

plugin_scrollmap_range_t plugin_scrollmap_script_range(plugin_scrollmap_script_t script) {
    return script->m_range;
}

void plugin_scrollmap_script_set_range(plugin_scrollmap_script_t script, plugin_scrollmap_range_t range) {
    if (script->m_range) {
        TAILQ_REMOVE(&script->m_range->m_scripts, script, m_next_for_range);
    }

    script->m_range = range;

    if (script->m_range) {
        TAILQ_INSERT_TAIL(&script->m_range->m_scripts, script, m_next_for_range);
    }
}

uint8_t plugin_scrollmap_script_need_process(plugin_scrollmap_script_t script) {
    plugin_scrollmap_env_t env = script->m_layer->m_env;
    return env->m_script_check_fun
        ? env->m_script_check_fun(env->m_script_check_ctx, script->m_layer, &script->m_script)
        : 1;
}

void plugin_scrollmap_script_print(write_stream_t s, plugin_scrollmap_script_t script) {
    dr_json_print(
        s, &script->m_script, sizeof(script->m_script),
        script->m_layer->m_env->m_module->m_meta_script, DR_JSON_PRINT_MINIMIZE, NULL);
}

const char * plugin_scrollmap_script_dump(plugin_scrollmap_script_t script) {
    plugin_scrollmap_env_t env = script->m_layer->m_env;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&env->m_dump_buffer);

    mem_buffer_clear_data(&env->m_dump_buffer);

    plugin_scrollmap_script_print((write_stream_t)&stream, script);
    
    stream_putc((write_stream_t)&stream, 0);

    return (const char *)mem_buffer_make_continuous(&env->m_dump_buffer, 0);
}

