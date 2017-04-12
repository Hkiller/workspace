#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_chipmunk_wait_collision_i.h"
#include "ui_sprite_chipmunk_obj_i.h"

ui_sprite_chipmunk_wait_collision_t ui_sprite_chipmunk_wait_collision_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WAIT_COLLISION_NAME);
    return fsm_action ? (ui_sprite_chipmunk_wait_collision_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_wait_collision_free(ui_sprite_chipmunk_wait_collision_t wait_collision) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(wait_collision);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_wait_collision_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    return ui_sprite_fsm_action_start_update(fsm_action);
}

static void ui_sprite_chipmunk_wait_collision_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static void ui_sprite_chipmunk_wait_collision_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_wait_collision_t wait_collision = (ui_sprite_chipmunk_wait_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;

    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk wait collision: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (ui_sprite_chipmunk_obj_is_colllision_with(chipmunk_obj, wait_collision->m_mask)) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static int ui_sprite_chipmunk_wait_collision_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_wait_collision_t wait_collision = (ui_sprite_chipmunk_wait_collision_t)ui_sprite_fsm_action_data(fsm_action);
    wait_collision->m_module = (ui_sprite_chipmunk_module_t)ctx;

    wait_collision->m_mask = 0;

    return 0;
}

static void ui_sprite_chipmunk_wait_collision_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_chipmunk_wait_collision_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_wait_collision_t to_wait_collision = (ui_sprite_chipmunk_wait_collision_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_wait_collision_t from_wait_collision = (ui_sprite_chipmunk_wait_collision_t)ui_sprite_fsm_action_data(from);
    
    if (ui_sprite_chipmunk_wait_collision_init(to, ctx)) return -1;

    to_wait_collision->m_mask = from_wait_collision->m_mask;
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_wait_collision_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_wait_collision_t wait_collision = (ui_sprite_chipmunk_wait_collision_t)ui_sprite_chipmunk_wait_collision_create(fsm_state, name);
    const char * str_value;

    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create wait_collision action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (wait_collision == NULL) {
        CPE_ERROR(module->m_em, "%s: create wait_collision action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    str_value = cfg_get_string(cfg, "mask", NULL);
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "%s: create wait_collision action: mask not configured!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (plugin_chipmunk_env_masks(env->m_env, &wait_collision->m_mask, str_value) != 0) {
        CPE_ERROR(module->m_em, "%s: create wait_collision action: mask %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
        return NULL;
    }
    
    return ui_sprite_fsm_action_from_data(wait_collision);
}

int ui_sprite_chipmunk_wait_collision_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WAIT_COLLISION_NAME, sizeof(struct ui_sprite_chipmunk_wait_collision));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk on collision register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_wait_collision_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_wait_collision_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_wait_collision_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_wait_collision_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_wait_collision_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_chipmunk_wait_collision_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WAIT_COLLISION_NAME, ui_sprite_chipmunk_wait_collision_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_wait_collision_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WAIT_COLLISION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WAIT_COLLISION_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WAIT_COLLISION_NAME = "chipmunk-wait-collision";
