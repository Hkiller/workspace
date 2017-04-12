#ifndef PLUGIN_EDITOR_MODULE_I_H
#define PLUGIN_EDITOR_MODULE_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "gd/app/app_context.h"
#include "plugin/editor/plugin_editor_module.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_editor_backend * plugin_editor_backend_t;

typedef TAILQ_HEAD(plugin_editor_editing_list, plugin_editor_editing) plugin_editor_editing_list_t;
    
struct plugin_editor_module {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    plugin_editor_backend_t m_backend;

    plugin_layout_animation_meta_t m_animation_meta_editing;
    
    plugin_editor_editing_list_t m_editings;
    plugin_editor_editing_t m_active_editing;
};

int plugin_editor_backend_init(plugin_editor_module_t module);
void plugin_editor_backend_fini(plugin_editor_module_t module);

void plugin_editor_backend_update_content(plugin_editor_module_t module, plugin_layout_render_t render);
void plugin_editor_backend_update_selection(plugin_editor_module_t module, plugin_layout_render_t render);    
void plugin_editor_backend_close(plugin_editor_module_t module);

char * plugin_editor_backend_clipboard_get(plugin_editor_module_t module);
int plugin_editor_backend_clipboard_put(plugin_editor_module_t module, const char * data);
    
#ifdef __cplusplus
}
#endif

#endif 
