#ifndef DROW_PLUGIN_UI_PAGE_EH_H
#define DROW_PLUGIN_UI_PAGE_EH_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page_eh_it {
    plugin_ui_page_eh_t (*next)(struct plugin_ui_page_eh_it * it);
    char m_data[64];
};
    
plugin_ui_page_eh_t
plugin_ui_page_eh_create(
    plugin_ui_page_t page,
    const char * event, plugin_ui_page_eh_scope_t scope,
    plugin_ui_page_eh_fun_t fun, void * ctx);

void plugin_ui_page_eh_free(plugin_ui_page_eh_t eh);

const char * plugin_ui_page_eh_event(plugin_ui_page_eh_t eh);

void * plugin_ui_page_eh_data(plugin_ui_page_eh_t eh);

void plugin_ui_page_eh_call(
    plugin_ui_page_eh_t eh,
    LPDRMETA data_meta, void const * data, size_t data_size);

#define plugin_ui_page_eh_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

