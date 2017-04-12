#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui_sprite_barrage_mask_i.h"
#include "ui_sprite_barrage_obj_i.h"
#include "ui_sprite_barrage_barrage_i.h"

ui_sprite_barrage_mask_t ui_sprite_barrage_mask_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BARRAGE_OBJ_MASK_NAME);
    return fsm_action ? (ui_sprite_barrage_mask_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_barrage_mask_free(ui_sprite_barrage_mask_t pause_emitter) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(pause_emitter);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_barrage_mask_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_barrage_mask_t barrage_mask = (ui_sprite_barrage_mask_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_chipmunk_env_t sprite_chipmunk_env = ui_sprite_chipmunk_env_find(world);
    plugin_chipmunk_env_t chipmunk_env;
    ui_sprite_barrage_obj_t barrage_obj;
    uint32_t mask;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage pause emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    barrage_mask->m_old_mask = ui_sprite_barrage_obj_mask(barrage_obj);

    if (barrage_mask->m_new_mask[0] == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage obj mask: new mask can not be 0!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    chipmunk_env = ui_sprite_chipmunk_env_env(sprite_chipmunk_env);
    if (plugin_chipmunk_env_masks(chipmunk_env, &mask, barrage_mask->m_new_mask) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage obj mask create: load mask from '%s' fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), barrage_mask->m_new_mask);
        return -1;
    }

    ui_sprite_barrage_obj_set_mask(barrage_obj, barrage_mask->m_emitter_perfix, mask);
   
    return 0;
}

static void ui_sprite_barrage_mask_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_barrage_mask_t barrage_mask = (ui_sprite_barrage_mask_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_barrage_obj_t barrage_obj;

    barrage_obj = ui_sprite_barrage_obj_find(entity);
    if (barrage_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): barrage pause emitter: not barrage obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_barrage_obj_set_mask(barrage_obj, barrage_mask->m_emitter_perfix, barrage_mask->m_old_mask);
}

static int ui_sprite_barrage_mask_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_barrage_mask_t barrage_mask = (ui_sprite_barrage_mask_t)ui_sprite_fsm_action_data(fsm_action);
    barrage_mask->m_module = (ui_sprite_barrage_module_t)ctx;
    barrage_mask->m_old_mask = 0;
    barrage_mask->m_new_mask[0] = 0;
    barrage_mask->m_emitter_perfix[0] = 0;
    return 0;
}

static void ui_sprite_barrage_mask_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_barrage_mask_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_barrage_mask_t to_barrage_mask = (ui_sprite_barrage_mask_t)ui_sprite_fsm_action_data(to);
    ui_sprite_barrage_mask_t from_barrage_mask = (ui_sprite_barrage_mask_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_barrage_mask_init(to, ctx)) return -1;

    cpe_str_dup(to_barrage_mask->m_new_mask, sizeof(to_barrage_mask->m_new_mask), from_barrage_mask->m_new_mask);
    cpe_str_dup(to_barrage_mask->m_emitter_perfix, sizeof(to_barrage_mask->m_emitter_perfix), from_barrage_mask->m_emitter_perfix);
    to_barrage_mask->m_old_mask = from_barrage_mask->m_old_mask;

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_barrage_mask_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_barrage_module_t module = (ui_sprite_barrage_module_t)ctx;
    ui_sprite_barrage_mask_t barrage_mask = ui_sprite_barrage_mask_create(fsm_state, name);
    const char * arg_value;

    if (barrage_mask == NULL) {
        CPE_ERROR(module->m_em, "%s: create barrage obj mask action: create fail!", ui_sprite_barrage_module_name(module));
        return NULL;
    }

    if ((arg_value = cfg_get_string(cfg, "mask", NULL))) {
        cpe_str_dup(barrage_mask->m_new_mask, sizeof(barrage_mask->m_new_mask), arg_value);
    }

    if ((arg_value = cfg_get_string(cfg, "perfix", NULL))) {
        cpe_str_dup(barrage_mask->m_emitter_perfix, sizeof(barrage_mask->m_emitter_perfix), arg_value);
    }

    return ui_sprite_fsm_action_from_data(barrage_mask);
}

int ui_sprite_barrage_mask_regist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BARRAGE_OBJ_MASK_NAME, sizeof(struct ui_sprite_barrage_mask));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: barrage pause emitter register: meta create fail",
            ui_sprite_barrage_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_barrage_mask_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_barrage_mask_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_barrage_mask_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_barrage_mask_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_barrage_mask_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_BARRAGE_OBJ_MASK_NAME, ui_sprite_barrage_mask_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_barrage_mask_unregist(ui_sprite_barrage_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BARRAGE_OBJ_MASK_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_barrage_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BARRAGE_OBJ_MASK_NAME);
}

const char * UI_SPRITE_BARRAGE_OBJ_MASK_NAME = "barrage-obj-mask";
