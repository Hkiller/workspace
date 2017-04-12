#ifndef PLUGIN_EDITOR_BACKEND_I_H
#define PLUGIN_EDITOR_BACKEND_I_H
#include "../plugin_editor_module_i.h"
#include "plugin_editor_delegate.h"

struct plugin_editor_backend {
    plugin_editor_delegate * m_delegate;
};

void plugin_editor_backend_update_caret(
    plugin_editor_module_t module, plugin_editor_editing_t editing);

void plugin_editor_backend_commit_text(plugin_editor_module_t module);
void plugin_editor_backend_commit_selection(plugin_editor_module_t module);

#endif
