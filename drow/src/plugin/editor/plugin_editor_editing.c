#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_vector_2.h"
#include "plugin/layout/plugin_layout_module.h"
#include "plugin/layout/plugin_layout_render.h"
#include "plugin/layout/plugin_layout_animation.h"
#include "plugin/layout/plugin_layout_animation_meta.h"
#include "plugin/layout/plugin_layout_animation_caret.h"
#include "plugin/layout/plugin_layout_animation_selection.h"
#include "plugin_editor_editing_i.h"

plugin_editor_editing_t
plugin_editor_editing_create(plugin_editor_module_t module, plugin_layout_render_t render) {
    plugin_layout_animation_t animation;

    animation = plugin_layout_animation_create(render, module->m_animation_meta_editing);
    if (animation == NULL) return NULL;

    return plugin_layout_animation_data(animation);
}

void plugin_editor_editing_free(plugin_editor_editing_t editing) {
    plugin_layout_animation_t animation = plugin_layout_animation_from_data(editing);
    plugin_layout_animation_free(animation);
}

plugin_editor_editing_t plugin_editor_editing_find_first(plugin_layout_render_t render) {
    plugin_layout_animation_t animation;

    animation = plugin_layout_animation_find_first_by_type(render, "editor");

    return animation ? plugin_layout_animation_data(animation) : NULL;
}

static int plugin_editor_editing_init(plugin_layout_animation_t animation, void * ctx) {
    plugin_editor_module_t module = ctx;
    plugin_editor_editing_t editing = plugin_layout_animation_data(animation);
    plugin_layout_animation_selection_t selection;
    plugin_layout_animation_caret_t caret;
    plugin_layout_render_t render = plugin_layout_animation_render(animation);
    
    editing->m_module = module;
    editing->m_max_len = 0;
    editing->m_is_passwd = 0;
    editing->m_number_only = 0;

    caret = plugin_layout_animation_caret_find_first(render);
    if (caret == NULL) {
        caret = plugin_layout_animation_caret_create(render);
        if (caret == NULL) {
            CPE_ERROR(module->m_em, "plugin_editor_editing_create: create caret fail!");
            return -1;
        }
    }

    selection = plugin_layout_animation_selection_find_first(render);
    if (selection && plugin_layout_animation_selection_length(selection) == 0) {
        plugin_layout_animation_selection_free(selection);
        selection = NULL;
    }
    
    plugin_layout_animation_caret_set_visiable(caret, selection ? 0 : 1);
    
    TAILQ_INSERT_TAIL(&module->m_editings, editing, m_next);
    
    return 0;
}
    
static void plugin_editor_editing_fini(plugin_layout_animation_t animation, void * ctx) {
    plugin_editor_module_t module = ctx;
    plugin_layout_render_t render = plugin_layout_animation_render(animation);
    plugin_editor_editing_t editing = plugin_layout_animation_data(animation);
    plugin_layout_animation_caret_t caret;
    plugin_layout_animation_selection_t selection;

    if (editing == module->m_active_editing) {
        plugin_editor_module_set_active_editing(module, NULL);
    }

    if ((caret = plugin_layout_animation_caret_find_first(render))) {
        plugin_layout_animation_caret_free(caret);
    }

    if ((selection = plugin_layout_animation_selection_find_first(render))) {
        plugin_layout_animation_selection_free(selection);
    }
    
    TAILQ_REMOVE(&editing->m_module->m_editings, editing, m_next);
}

uint8_t plugin_editor_editing_is_active(plugin_editor_editing_t editing) {
    return editing->m_module->m_active_editing == editing ? 1 : 0;
}

void plugin_editor_editing_set_is_active(plugin_editor_editing_t editing, uint8_t active) {
    if (active) {
        plugin_editor_module_set_active_editing(editing->m_module, editing);
    }
    else {
        if (editing->m_module->m_active_editing == editing) {
            plugin_editor_module_set_active_editing(editing->m_module, NULL);
        }
    }
}

uint32_t plugin_editor_editing_max_length(plugin_editor_editing_t editing) {
    return editing->m_max_len;
}

void plugin_editor_editing_set_max_length(plugin_editor_editing_t editing, uint32_t max_len) {
    editing->m_max_len = max_len;
}

uint8_t plugin_editor_editing_number_only(plugin_editor_editing_t editing) {
    return editing->m_number_only;
}

void plugin_editor_editing_set_number_only(plugin_editor_editing_t editing, uint8_t number_only) {
    if (number_only) number_only = 1;

    if (editing->m_number_only != number_only) {
        editing->m_number_only = number_only;

        if (editing == editing->m_module->m_active_editing) {
            plugin_editor_backend_update_content(
                editing->m_module,
                plugin_layout_animation_render(plugin_layout_animation_from_data(editing)));
        }
    }
}

uint8_t plugin_editor_editing_is_passwd(plugin_editor_editing_t editing) {
    return editing->m_is_passwd;
}

void plugin_editor_editing_set_is_passwd(plugin_editor_editing_t editing, uint8_t passwd) {
    if (passwd) passwd = 1;

    if (editing->m_is_passwd != passwd) {
        editing->m_is_passwd = passwd;

        if (editing == editing->m_module->m_active_editing) {
            plugin_editor_backend_update_content(
                editing->m_module,
                plugin_layout_animation_render(plugin_layout_animation_from_data(editing)));
        }
    }
}

int plugin_editor_editing_caret(plugin_editor_editing_t editing) {
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    plugin_layout_animation_caret_t caret;

    if ((caret = plugin_layout_animation_caret_find_first(render))) {
        return plugin_layout_animation_caret_pos(caret);
    }
    else {
        return -1;
    }
}

void plugin_editor_editing_set_caret(plugin_editor_editing_t editing, int index) {
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    plugin_layout_animation_caret_t caret;

    if ((caret = plugin_layout_animation_caret_find_first(render))) {
        plugin_layout_animation_caret_set_pos(caret, index);
    }
}

void plugin_editor_editing_set_caret_by_pt(plugin_editor_editing_t editing, ui_vector_2_t pt) {
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    plugin_layout_animation_caret_t caret;

    if ((caret = plugin_layout_animation_caret_find_first(render))) {
        plugin_layout_animation_caret_set_pos_by_pt(caret, pt);
    }
}

void plugin_editor_editing_selection_clear(plugin_editor_editing_t editing) {
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    plugin_layout_animation_selection_t selection = plugin_layout_animation_selection_find_first(render);
    plugin_layout_animation_caret_t caret = plugin_layout_animation_caret_find_first(render);

    if (selection) {
        plugin_layout_animation_selection_free(selection);
        selection = NULL;
    }

    if (caret) {
        plugin_layout_animation_caret_set_visiable(caret, selection ? 0 : 0);
    }
    
    if (editing == editing->m_module->m_active_editing) {
        plugin_editor_backend_update_selection(editing->m_module, render);
    }
}

int plugin_editor_editing_selection_update(plugin_editor_editing_t editing, ui_vector_2_t begin_pt, ui_vector_2_t end_pt) {
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    plugin_layout_animation_selection_t selection = plugin_layout_animation_selection_find_first(render);
    plugin_layout_animation_caret_t caret = plugin_layout_animation_caret_find_first(render);

    if (fabs(begin_pt->x - end_pt->x) > 5) {
        if (selection == NULL) {
            selection = plugin_layout_animation_selection_create(render);
            if (selection == NULL) {
                CPE_ERROR(editing->m_module->m_em, "plugin_editor_editing_selection_update: create selection fail!");
                return -1;
            }
        }

        plugin_layout_animation_selection_set_range_by_pt(selection, begin_pt, end_pt);
        if (plugin_layout_animation_selection_length(selection) <= 0) {
            plugin_layout_animation_selection_free(selection);
            selection = NULL;
        }
    }
    else {
        if (selection) {
            plugin_layout_animation_selection_free(selection);
            selection = NULL;
        }
    }

    if (caret) {
        plugin_layout_animation_caret_set_pos_by_pt(caret, end_pt);
        plugin_layout_animation_caret_set_visiable(caret, selection ? 0 : 1);
    }        


    if (editing == editing->m_module->m_active_editing) {
        plugin_editor_backend_update_selection(editing->m_module, render);
    }
    
    return 0;
}

int plugin_editor_editing_copy_to_clipboard(plugin_editor_editing_t editing) {
    plugin_editor_module_t module = editing->m_module;
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    plugin_layout_animation_selection_t selection;
    char * text_utf8;
    int rv;

    if ((selection = plugin_layout_animation_selection_find_first(render))) {
        text_utf8 = plugin_layout_animation_selection_text_utf8(module->m_alloc, selection);
    }
    else {
        text_utf8 = plugin_layout_render_text_utf8(module->m_alloc, render);
    }
    
    if (text_utf8 == NULL) return -1;

    rv = plugin_editor_backend_clipboard_put(module, text_utf8);
        
    mem_free(module->m_alloc, text_utf8);

    return rv == 0 ? 0 : -1;
}

int plugin_editor_editing_past_from_clipboard(plugin_editor_editing_t editing) {
    plugin_editor_module_t module = editing->m_module;
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    plugin_layout_animation_selection_t selection;
    char * text_utf8;
    int rv;
        
    text_utf8 = plugin_editor_backend_clipboard_get(module);
    if (text_utf8 == NULL) return -1;

    if ((selection = plugin_layout_animation_selection_find_first(render))) {
        rv = plugin_layout_animation_selection_set_text_utf8(selection, text_utf8);
    }
    else {
        rv = plugin_layout_render_set_data(render, text_utf8);
    }
        
    mem_free(module->m_alloc, text_utf8);

    return rv == 0 ? 0 : -1;
}

int plugin_editor_editing_commit_data_ucs4(
    plugin_editor_editing_t editing, uint32_t const * text, size_t text_len)
{
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    int rv;

    rv = plugin_layout_render_update_text_ucs4(render, -1, -1, text, text_len);

    return rv;
}

int plugin_editor_editing_commit_data_utf8(
    plugin_editor_editing_t editing, const char * c_str)
{
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    int rv;

    rv = plugin_layout_render_update_text_utf8(render, -1, -1, c_str);

    return rv;
}

int plugin_editor_editing_commit_selection(plugin_editor_editing_t editing, int begin_pos, int end_pos) {
    plugin_layout_render_t render = plugin_layout_animation_render(plugin_layout_animation_from_data(editing));
    plugin_layout_animation_selection_t selection = plugin_layout_animation_selection_find_first(render);
    plugin_layout_animation_caret_t caret = plugin_layout_animation_caret_find_first(render);

    if (end_pos > begin_pos) {
        if (selection == NULL) {
            selection = plugin_layout_animation_selection_create(render);
            if (selection == NULL) {
                CPE_ERROR(editing->m_module->m_em, "plugin_editor_editing_commit_selection: create selection fail!");
                return -1;
            }
        }

        plugin_layout_animation_selection_set_range(selection, begin_pos, end_pos);
    }
    else {
        if (selection) {
            plugin_layout_animation_selection_free(selection);
            selection = NULL;
        }
    }

    if (caret) {
        plugin_layout_animation_caret_set_pos(caret, end_pos);
        plugin_layout_animation_caret_set_visiable(caret, selection ? 0 : 1);
    }
    
    return 0;
}

int plugin_editor_editing_register(plugin_editor_module_t module) {
    plugin_layout_module_t layout_module;

    assert(module->m_animation_meta_editing == NULL);

    layout_module = plugin_layout_module_find_nc(module->m_app, NULL);
    if (layout_module == NULL) {
        CPE_ERROR(module->m_em, "plugin_editor_editing_register: no layout module");
        return -1;
    }

    module->m_animation_meta_editing =
        plugin_layout_animation_meta_create(
            layout_module, "editor", module,
            sizeof(struct plugin_editor_editing),
            plugin_editor_editing_init,
            plugin_editor_editing_fini,
            NULL, /*layout*/
            NULL, /*update*/
            NULL /*render*/);
    
    return module->m_animation_meta_editing ? 0 : -1;
}

void plugin_editor_editing_unregister(plugin_editor_module_t module) {
    assert(module->m_animation_meta_editing);
    plugin_layout_animation_meta_free(module->m_animation_meta_editing);
    module->m_animation_meta_editing = NULL;
}
