#ifndef DROW_PLUGIN_UI_PAGE_SLOT_H
#define DROW_PLUGIN_UI_PAGE_SLOT_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page_slot_it {
    plugin_ui_page_slot_t (*next)(struct plugin_ui_page_slot_it * it);
    char m_data[64];
};
    
plugin_ui_page_slot_t plugin_ui_page_slot_create(plugin_ui_page_t page, const char * name);
void plugin_ui_page_slot_free(plugin_ui_page_slot_t slot);
plugin_ui_page_slot_t plugin_ui_page_slot_find(plugin_ui_page_t page, const char * name);

const char * plugin_ui_page_slot_name(plugin_ui_page_slot_t slot);
plugin_ui_page_t plugin_ui_page_slot_page(plugin_ui_page_slot_t slot);

dr_value_t plugin_ui_page_slot_value(plugin_ui_page_slot_t slot);
int plugin_ui_page_slot_set_by_str(plugin_ui_page_slot_t slot, const char * str_value);

void plugin_ui_page_slot_sync_page_value(plugin_ui_page_slot_t slot);
void plugin_ui_page_slot_set_binding_need_process(plugin_ui_page_slot_t slot);
    
#define plugin_ui_page_slot_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

