#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_destory_self_i.h"

ui_sprite_basic_destory_self_t ui_sprite_basic_destory_self_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
	ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_DESTORY_SELF_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_destory_self_free(ui_sprite_basic_destory_self_t destory_self) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(destory_self);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_basic_destory_self_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
	ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_entity_set_destory(entity);
    return 0;
}

static void ui_sprite_basic_destory_self_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_basic_destory_self_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_destory_self_t destory_self = ui_sprite_fsm_action_data(fsm_action);
	destory_self->m_module = ctx;
    return 0;
}

static void ui_sprite_basic_destory_self_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_basic_destory_self_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
	if (ui_sprite_basic_destory_self_init(to, ctx)) return -1;
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_basic_destory_self_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_destory_self_t check_entity = ui_sprite_basic_destory_self_create(fsm_state, name);
    return ui_sprite_fsm_action_from_data(check_entity);
}

int ui_sprite_basic_destory_self_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module,
        UI_SPRITE_BASIC_DESTORY_SELF_NAME,
        sizeof(struct ui_sprite_basic_destory_self));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_destory_self_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_destory_self_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_destory_self_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_destory_self_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_destory_self_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_DESTORY_SELF_NAME, ui_sprite_basic_destory_self_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_basic_destory_self_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_DESTORY_SELF_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_DESTORY_SELF_NAME);
    }
}

const char * UI_SPRITE_BASIC_DESTORY_SELF_NAME = "destory-self";

