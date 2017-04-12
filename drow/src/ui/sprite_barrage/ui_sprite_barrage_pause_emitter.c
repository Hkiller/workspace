#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_barrage_pause_emitter_i.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_barrage_i.h"

ui_sprite_barrage_pause_emitter_t ui_sprite_barrage_pause_emitter_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BARRAGE_PAUSE_EMITTER_NAME);
    return fsm_action ? (ui_sprite_barrage_pause_emitter_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_barrage_pause_emitter_free(ui_sprite_barrage_pause_emitter_t pause_emitter) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(pause_emitter);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_barrage_pause_emitter_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_barrage_pause_emitter_t pause_emitter = (ui_sprite_barrage_pause_emitter_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage pause emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (pause_emitter->m_group_name[0]) {
        ui_sprite_barrage_obj_pause_barrages(barrage_obj, pause_emitter->m_group_name);
    }
    else {
        ui_sprite_barrage_obj_pause_barrages(barrage_obj, NULL);
    }

    return 0;
}

static void ui_sprite_barrage_pause_emitter_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_barrage_pause_emitter_t pause_emitter = (ui_sprite_barrage_pause_emitter_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage pause emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (pause_emitter->m_group_name[0]) {
        ui_sprite_barrage_obj_resume_barrages(barrage_obj, pause_emitter->m_group_name);
    }
    else {
        ui_sprite_barrage_obj_resume_barrages(barrage_obj, NULL);
    }
}

static int ui_sprite_barrage_pause_emitter_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_pause_emitter_t pause_emitter = (ui_sprite_barrage_pause_emitter_t)ui_sprite_fsm_action_data(fsm_action);
    pause_emitter->m_module = (ui_sprite_barrage_module_t)ctx;
	pause_emitter->m_group_name[0] = 0;
    return 0;
}

static void ui_sprite_barrage_pause_emitter_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_barrage_pause_emitter_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_barrage_pause_emitter_t to_pause_emitter = (ui_sprite_barrage_pause_emitter_t)ui_sprite_fsm_action_data(to);
    ui_sprite_barrage_pause_emitter_t from_pause_emitter = (ui_sprite_barrage_pause_emitter_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_barrage_pause_emitter_init(to, ctx)) return -1;

    cpe_str_dup(to_pause_emitter->m_group_name, sizeof(to_pause_emitter->m_group_name), from_pause_emitter->m_group_name);

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_barrage_pause_emitter_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_barrage_pause_emitter_t pause_emitter = ui_sprite_barrage_pause_emitter_create(fsm_state, name);
    const char * group_name;

    if (pause_emitter == NULL) {
        CPE_ERROR(module->m_em, "%s: create pause_emitter action: create fail!", ui_sprite_barrage_module_name(module));
        return NULL;
    }

    if ((group_name = cfg_get_string(cfg, "group-name", NULL))) {
        cpe_str_dup(pause_emitter->m_group_name, sizeof(pause_emitter->m_group_name), group_name);
    }

    return ui_sprite_fsm_action_from_data(pause_emitter);
}

int ui_sprite_barrage_pause_emitter_regist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BARRAGE_PAUSE_EMITTER_NAME, sizeof(struct ui_sprite_barrage_pause_emitter));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: barrage pause emitter register: meta create fail",
            ui_sprite_barrage_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_barrage_pause_emitter_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_barrage_pause_emitter_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_barrage_pause_emitter_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_barrage_pause_emitter_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_barrage_pause_emitter_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_BARRAGE_PAUSE_EMITTER_NAME, ui_sprite_barrage_pause_emitter_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_pause_emitter_unregist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BARRAGE_PAUSE_EMITTER_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_barrage_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BARRAGE_PAUSE_EMITTER_NAME);
}

const char * UI_SPRITE_BARRAGE_PAUSE_EMITTER_NAME = "pause-barrage-emitter";
