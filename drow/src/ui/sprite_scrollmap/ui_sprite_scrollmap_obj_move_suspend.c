#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/scrollmap/plugin_scrollmap_obj.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_scrollmap_obj_move_suspend_i.h"
#include "ui_sprite_scrollmap_obj_i.h"

ui_sprite_scrollmap_obj_move_suspend_t ui_sprite_scrollmap_obj_move_suspend_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SCROLLMAP_OBJ_MOVE_SUSPEND_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_scrollmap_obj_move_suspend_free(ui_sprite_scrollmap_obj_move_suspend_t move_suspend) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(move_suspend);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_scrollmap_obj_move_suspend_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_scrollmap_obj_t scrollmap_obj;

    scrollmap_obj = ui_sprite_scrollmap_obj_find(entity);
    if (scrollmap_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): move-suspend: no scrollmap_obj obj",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_scrollmap_obj_set_move_suspend(scrollmap_obj, 1);
    
    return 0;
}

static void ui_sprite_scrollmap_obj_move_suspend_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_scrollmap_obj_t scrollmap_obj;

    scrollmap_obj = ui_sprite_scrollmap_obj_find(entity);
    if (scrollmap_obj) {
        ui_sprite_scrollmap_obj_set_move_suspend(scrollmap_obj, 0);
    }
}

static int ui_sprite_scrollmap_obj_move_suspend_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_obj_move_suspend_t move_suspend = ui_sprite_fsm_action_data(fsm_action);
    move_suspend->m_module = ctx;
    return 0;
}

static void ui_sprite_scrollmap_obj_move_suspend_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_scrollmap_obj_move_suspend_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	if (ui_sprite_scrollmap_obj_move_suspend_init(to, ctx)) return -1;

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_scrollmap_obj_move_suspend_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_obj_move_suspend_t move_suspend = ui_sprite_scrollmap_obj_move_suspend_create(fsm_state, name);
    
    if (move_suspend == NULL) {
        CPE_ERROR(module->m_em, "%s: create move_suspend action: create fail!", ui_sprite_scrollmap_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(move_suspend);
}

int ui_sprite_scrollmap_obj_move_suspend_regist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SCROLLMAP_OBJ_MOVE_SUSPEND_NAME, sizeof(struct ui_sprite_scrollmap_obj_move_suspend));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: scrollmap_obj enable emitter register: meta create fail",
            ui_sprite_scrollmap_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_scrollmap_obj_move_suspend_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_scrollmap_obj_move_suspend_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_scrollmap_obj_move_suspend_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_scrollmap_obj_move_suspend_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_scrollmap_obj_move_suspend_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SCROLLMAP_OBJ_MOVE_SUSPEND_NAME, ui_sprite_scrollmap_obj_move_suspend_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_scrollmap_obj_move_suspend_unregist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SCROLLMAP_OBJ_MOVE_SUSPEND_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_scrollmap_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SCROLLMAP_OBJ_MOVE_SUSPEND_NAME);
}

const char * UI_SPRITE_SCROLLMAP_OBJ_MOVE_SUSPEND_NAME = "scrollmap-obj-move-suspend";
