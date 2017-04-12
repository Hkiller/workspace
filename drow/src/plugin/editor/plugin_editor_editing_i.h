#ifndef PLUGIN_EDITOR_EDITING_I_H
#define PLUGIN_EDITOR_EDITING_I_H
#include "plugin/editor/plugin_editor_editing.h"
#include "plugin_editor_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_editor_editing {
    plugin_editor_module_t m_module;
    TAILQ_ENTRY(plugin_editor_editing) m_next;
    uint32_t m_max_len;
    uint8_t m_is_passwd;
    uint8_t m_number_only;
};

int plugin_editor_editing_register(plugin_editor_module_t module);
void plugin_editor_editing_unregister(plugin_editor_module_t module);


int plugin_editor_editing_commit_data_ucs4(
    plugin_editor_editing_t editing, uint32_t const * text, size_t text_len);

int plugin_editor_editing_commit_data_utf8(
    plugin_editor_editing_t editing, const char * c_str);

int plugin_editor_editing_commit_selection(
    plugin_editor_editing_t editing, int begin_pos, int end_pos);
    
#ifdef __cplusplus
}
#endif

#endif 
