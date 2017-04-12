#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_with_time_scale_i.h"
#include "ui_sprite_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_time_scale_t ui_sprite_chipmunk_with_time_scale_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_TIME_SCALE_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_time_scale_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_time_scale_free(ui_sprite_chipmunk_with_time_scale_t with_time_scale) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_time_scale);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_with_time_scale_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_time_scale_t with_time_scale = (ui_sprite_chipmunk_with_time_scale_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_chipmunk_env_t env;
    float new_time_scale;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-with-time-scale: chipmunk env not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (with_time_scale->m_cfg_time_scale == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-with-time-scale: time scale not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_fsm_action_try_calc_float(
            &new_time_scale, with_time_scale->m_cfg_time_scale, fsm_action, NULL, module->m_em)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-with-time-scale: calc time scale from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_time_scale->m_cfg_time_scale);
        return -1;
    }
    
    with_time_scale->m_saved_time_scale = plugin_chipmunk_env_time_scale(env->m_env);
    plugin_chipmunk_env_set_time_scale(env->m_env, new_time_scale);
    
    return 0;
}

static void ui_sprite_chipmunk_with_time_scale_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_time_scale_t with_time_scale = (ui_sprite_chipmunk_with_time_scale_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_chipmunk_env_t env;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env) {
        plugin_chipmunk_env_set_time_scale(env->m_env, with_time_scale->m_saved_time_scale);
    }
}

static int ui_sprite_chipmunk_with_time_scale_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_time_scale_t with_time_scale = (ui_sprite_chipmunk_with_time_scale_t)ui_sprite_fsm_action_data(fsm_action);
    with_time_scale->m_module = (ui_sprite_chipmunk_module_t)ctx;
    with_time_scale->m_cfg_time_scale;
    with_time_scale->m_saved_time_scale = 0.0f;
    return 0;
}

static void ui_sprite_chipmunk_with_time_scale_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_time_scale_t with_time_scale = (ui_sprite_chipmunk_with_time_scale_t)ui_sprite_fsm_action_data(fsm_action);

    if (with_time_scale->m_cfg_time_scale) {
        mem_free(module->m_alloc, with_time_scale->m_cfg_time_scale);
        with_time_scale->m_cfg_time_scale = NULL;
    }
}

static int ui_sprite_chipmunk_with_time_scale_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_time_scale_t to_with_time_scale = (ui_sprite_chipmunk_with_time_scale_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_time_scale_t from_with_time_scale = (ui_sprite_chipmunk_with_time_scale_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_with_time_scale_init(to, ctx)) return -1;

    if (from_with_time_scale->m_cfg_time_scale) {
        to_with_time_scale->m_cfg_time_scale = cpe_str_mem_dup(module->m_alloc, from_with_time_scale->m_cfg_time_scale);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_time_scale_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_time_scale_t with_time_scale =
        (ui_sprite_chipmunk_with_time_scale_t)ui_sprite_chipmunk_with_time_scale_create(fsm_state, name);
    const char * str_value;

    str_value = cfg_get_string(cfg, "time-scale", NULL);
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "%s: chipmunk-with-time-scale: time-scale not configured!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (with_time_scale == NULL) {
        CPE_ERROR(module->m_em, "%s: chipmunk-with-time-scale: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    with_time_scale->m_cfg_time_scale = cpe_str_mem_dup_trim(module->m_alloc, str_value);
    if (with_time_scale->m_cfg_time_scale == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk-with-time-scale: dup time scale %s fial!",
            ui_sprite_chipmunk_module_name(module), str_value);
        ui_sprite_chipmunk_with_time_scale_free(with_time_scale);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(with_time_scale);
}
    
int ui_sprite_chipmunk_with_time_scale_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_TIME_SCALE_NAME, sizeof(struct ui_sprite_chipmunk_with_time_scale));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with time_scale register: meta create fail",
            ui_sprite_chipmunk_module_name  (module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_time_scale_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_time_scale_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_time_scale_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_time_scale_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_time_scale_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_TIME_SCALE_NAME, ui_sprite_chipmunk_with_time_scale_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_time_scale_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_TIME_SCALE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_TIME_SCALE_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_TIME_SCALE_NAME = "chipmunk-with-time-scale";

#ifdef __cplusplus
}
#endif
    
