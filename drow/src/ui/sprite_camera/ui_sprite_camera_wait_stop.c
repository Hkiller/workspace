#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_camera_wait_stop_i.h"
#include "ui_sprite_camera_env_i.h"

ui_sprite_camera_wait_stop_t ui_sprite_camera_wait_stop_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_WAIT_STOP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_camera_wait_stop_free(ui_sprite_camera_wait_stop_t wait_stop) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(wait_stop);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_camera_wait_stop_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_fsm_action_start_update(fsm_action);
    return 0;
}

static void ui_sprite_camera_wait_stop_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_camera_wait_stop_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_camera_wait_stop_t wait_stop = ui_sprite_fsm_action_data(fsm_action);
    wait_stop->m_module = ctx;
    return 0;
}

static void ui_sprite_camera_wait_stop_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_camera_wait_stop_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_camera_wait_stop_init(to, ctx)) return -1;
    return 0;
}

static void ui_sprite_camera_wait_stop_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_camera_env_t camera = ui_sprite_camera_env_find(world);

    if (camera == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): camera-wait-stop: update: world no camera!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (camera->m_curent_op_id == 0) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static ui_sprite_fsm_action_t ui_sprite_camera_wait_stop_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_camera_module_t module = ctx;
    ui_sprite_camera_wait_stop_t camera_camera_wait_stop = ui_sprite_camera_wait_stop_create(fsm_state, name);

    if (camera_camera_wait_stop == NULL) {
        CPE_ERROR(module->m_em, "%s: create camera_camera_wait_stop action: create fail!", ui_sprite_camera_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(camera_camera_wait_stop);
}

int ui_sprite_camera_wait_stop_regist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_WAIT_STOP_NAME, sizeof(struct ui_sprite_camera_wait_stop));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera wait_stop register: meta create fail",
            ui_sprite_camera_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_camera_wait_stop_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_camera_wait_stop_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_camera_wait_stop_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_camera_wait_stop_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_camera_wait_stop_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_camera_wait_stop_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_WAIT_STOP_NAME, ui_sprite_camera_wait_stop_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_camera_wait_stop_unregist(ui_sprite_camera_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_WAIT_STOP_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: camera camera wait_stop unregister: meta not exist",
            ui_sprite_camera_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_WAIT_STOP_NAME);
    }
}

const char * UI_SPRITE_CAMERA_WAIT_STOP_NAME = "camera-wait-stop";

