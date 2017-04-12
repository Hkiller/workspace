#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_popup.h"
#include "plugin/ui/plugin_ui_control.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_ui/ui_sprite_ui_utils.h"
#include "ui_sprite_ui_env_i.h"

plugin_ui_page_t
ui_sprite_ui_find_page_from_action(
    ui_sprite_ui_module_t module, ui_sprite_fsm_action_t fsm_action, const char * name)
{
    plugin_ui_page_t page;
    ui_sprite_fsm_state_t cur_state;
    ui_sprite_fsm_ins_t cur_ins;
    ui_sprite_fsm_ins_t p_ins;
    ui_sprite_fsm_ins_t pp_ins;
    ui_sprite_fsm_action_t popup_action;
    uint32_t popup_id;
    plugin_ui_popup_t popup;
    
    page = plugin_ui_page_find(module->m_env->m_env, name);
    if (page) return page;

    if (ui_sprite_fsm_action_to_entity(fsm_action) != module->m_env->m_entity) return NULL;

    cur_state = ui_sprite_fsm_action_state(fsm_action);
    cur_ins = ui_sprite_fsm_state_fsm(cur_state);
    p_ins = ui_sprite_fsm_parent(cur_ins);
    pp_ins = p_ins ? ui_sprite_fsm_parent(p_ins) : NULL;

    while(pp_ins) {
        cur_ins = p_ins;
        p_ins = pp_ins;
        pp_ins = ui_sprite_fsm_parent(p_ins);
    }

    popup_action = ui_sprite_fsm_action_from_data(cur_ins);
    if (sscanf(ui_sprite_fsm_action_name(popup_action), "P%d", &popup_id) != 1) return NULL;

    popup = plugin_ui_popup_find_by_id(module->m_env->m_env, popup_id);
    if (popup == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_ui_find_page_from_action: popup %d not exist", popup_id);
        return NULL;
    }

    return strcmp(plugin_ui_popup_name(popup), name) == 0 ? plugin_ui_popup_page(popup) : NULL;
}

plugin_ui_control_t
ui_sprite_ui_find_control_from_action(
    ui_sprite_ui_module_t module, ui_sprite_fsm_action_t fsm_action, const char * path)
{
    plugin_ui_page_t page;
    const char * p;
    
    p = strchr(path, '.');
    if (p == NULL) {
        page = ui_sprite_ui_find_page_from_action(module, fsm_action, path);
        if (page == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_find_control_from_action: page %s not exist", path);
            return NULL;
        }
        return plugin_ui_page_root_control(page);
    }
    else {
        char page_name[64];
        size_t len;

        len = p - path;
        if (len + 1 > CPE_ARRAY_SIZE(page_name)) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_find_control_from_action: page name len overflow, path=%s", path);
            return NULL;
        }
    
        memcpy(page_name, path, len);
        page_name[len] = 0;

        page = ui_sprite_ui_find_page_from_action(module, fsm_action, page_name);
        if (page == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_ui_find_control_from_action: page %s not exist", page_name);
            return NULL;
        }

        return plugin_ui_control_find_child_by_path(plugin_ui_page_root_control(page), p + 1);
    }
}
