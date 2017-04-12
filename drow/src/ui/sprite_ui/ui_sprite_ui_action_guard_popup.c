#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "plugin/ui/plugin_ui_popup.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_ui_action_guard_popup_i.h"
#include "ui_sprite_ui_env_i.h"

ui_sprite_ui_action_guard_popup_t ui_sprite_ui_action_guard_popup_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_UI_ACTION_GUARD_POPUP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_ui_action_guard_popup_free(ui_sprite_ui_action_guard_popup_t action_guard_popup) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(action_guard_popup);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_ui_action_guard_popup_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    return 0;
}

static void ui_sprite_ui_action_guard_popup_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_module_t module = ctx;
    ui_sprite_ui_action_guard_popup_t action_guard_popup = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_fsm_state_t cur_state;
    ui_sprite_fsm_ins_t cur_ins;
    ui_sprite_fsm_ins_t p_ins;
    ui_sprite_fsm_ins_t pp_ins;
    ui_sprite_fsm_action_t popup_action;
    uint32_t popup_id;
    plugin_ui_popup_t popup;

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
    if (sscanf(ui_sprite_fsm_action_name(popup_action), "P%d", &popup_id) != 1) return;

    popup = plugin_ui_popup_find_by_id(module->m_env->m_env, popup_id);
    if (popup == NULL) return;

    action_guard_popup->m_free_from_guard = 1;
    plugin_ui_popup_free(popup);
}

static int ui_sprite_ui_action_guard_popup_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_ui_action_guard_popup_t action_guard_popup = ui_sprite_fsm_action_data(fsm_action);
    action_guard_popup->m_module = ctx;
    action_guard_popup->m_free_from_guard = 0;
    return 0;
}

static void ui_sprite_ui_action_guard_popup_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_ui_action_guard_popup_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	if (ui_sprite_ui_action_guard_popup_init(to, ctx)) return -1;
    return 0;
}

int ui_sprite_ui_action_guard_popup_regist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_UI_ACTION_GUARD_POPUP_NAME, sizeof(struct ui_sprite_ui_action_guard_popup));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_ui_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_ui_action_guard_popup_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_ui_action_guard_popup_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_ui_action_guard_popup_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_ui_action_guard_popup_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_ui_action_guard_popup_clear, module);

    return 0;
}

void ui_sprite_ui_action_guard_popup_unregist(ui_sprite_ui_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_UI_ACTION_GUARD_POPUP_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_ui_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_UI_ACTION_GUARD_POPUP_NAME);
}

const char * UI_SPRITE_UI_ACTION_GUARD_POPUP_NAME = "ui-guard-popup";

