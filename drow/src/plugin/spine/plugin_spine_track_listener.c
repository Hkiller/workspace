#include "cpe/pal/pal_strings.h"
#include "plugin_spine_track_listener_i.h"

plugin_spine_track_listener_t
plugin_spine_track_listener_create(plugin_spine_module_t module, plugin_spine_anim_event_fun_t fun, void * ctx) {
    plugin_spine_track_listener_t listener;

    listener = module->m_free_listeners;
    if (listener == NULL) {
        listener = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_track_listener));
        if (listener == NULL) return NULL;
        listener->m_next = NULL;
    }
    else {
        module->m_free_listeners = listener->m_next;
        listener->m_next = NULL;
    }

    listener->m_func = fun;
    listener->m_func_ctx = ctx;
    return listener;
}

void plugin_spine_track_listener_real_free_all(plugin_spine_module_t module) {
    while(module->m_free_listeners) {
        plugin_spine_track_listener_t r = module->m_free_listeners;
        module->m_free_listeners = r->m_next;
        mem_free(module->m_alloc, r);
    }
}

void plugin_spine_track_listener_free_list(plugin_spine_module_t module, plugin_spine_track_listener_t * list) {
    if (*list == NULL) return;

    if (module->m_free_listeners) {
        plugin_spine_track_listener_t last = *list;
        while(last->m_next) last = last->m_next;
        last->m_next = module->m_free_listeners;
    }

    module->m_free_listeners = *list;
    *list = NULL;
}

void plugin_spine_track_listener_free_list_by_ctx(plugin_spine_module_t module, plugin_spine_track_listener_t * list, void * ctx) {
    while(*list) {
        plugin_spine_track_listener_t cur = *list;

        if (cur->m_func_ctx == ctx) {
            *list = cur->m_next;
            cur->m_next = module->m_free_listeners;
            module->m_free_listeners = cur;
        }
        else {
            list = &cur->m_next;
        }
    }
}
