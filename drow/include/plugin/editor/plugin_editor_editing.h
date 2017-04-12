#ifndef DROW_PLUGIN_EDITOR_EDITING_H
#define DROW_PLUGIN_EDITOR_EDITING_H
#include "plugin_editor_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_editor_editing_t
plugin_editor_editing_create(plugin_editor_module_t module, plugin_layout_render_t render);
    
void plugin_editor_editing_free(plugin_editor_editing_t editing);
plugin_editor_editing_t plugin_editor_editing_find_first(plugin_layout_render_t render);

uint32_t plugin_editor_editing_max_length(plugin_editor_editing_t editing);
void plugin_editor_editing_set_max_length(plugin_editor_editing_t editing, uint32_t max_len);

uint8_t plugin_editor_editing_number_only(plugin_editor_editing_t editing);
void plugin_editor_editing_set_number_only(plugin_editor_editing_t editing, uint8_t);

uint8_t plugin_editor_editing_is_passwd(plugin_editor_editing_t editing);
void plugin_editor_editing_set_is_passwd(plugin_editor_editing_t editing, uint8_t passwd);

uint8_t plugin_editor_editing_is_active(plugin_editor_editing_t editing);
void plugin_editor_editing_set_is_active(plugin_editor_editing_t editing, uint8_t active);

void plugin_editor_editing_selection_clear(plugin_editor_editing_t editing);
int plugin_editor_editing_selection_update(plugin_editor_editing_t editing, ui_vector_2_t begin_pt, ui_vector_2_t end_pt);

int plugin_editor_editing_copy_to_clipboard(plugin_editor_editing_t editing);
int plugin_editor_editing_past_from_clipboard(plugin_editor_editing_t editing);

#ifdef __cplusplus
}
#endif

#endif 
