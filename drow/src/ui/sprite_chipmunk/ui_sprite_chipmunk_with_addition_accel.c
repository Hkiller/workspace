#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_with_addition_accel_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_addition_accel_t ui_sprite_chipmunk_with_addition_accel_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_addition_accel_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_addition_accel_free(ui_sprite_chipmunk_with_addition_accel_t with_addition_accel) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_addition_accel);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_chipmunk_with_addition_accel_do_update(
    ui_sprite_chipmunk_obj_updator_t updator, ui_sprite_chipmunk_obj_body_t body, UI_SPRITE_CHIPMUNK_PAIR * acc, float * damping)
{
    UI_SPRITE_CHIPMUNK_PAIR const * data = (UI_SPRITE_CHIPMUNK_PAIR*)ui_sprite_chipmunk_obj_updator_data(updator);
    *acc = *data;
}
    
static int ui_sprite_chipmunk_with_addition_accel_update_data(
    ui_sprite_chipmunk_with_addition_accel_t with_addition_accel, ui_sprite_world_t world)
{
    UI_SPRITE_CHIPMUNK_PAIR * data = (UI_SPRITE_CHIPMUNK_PAIR*)ui_sprite_chipmunk_obj_updator_data(with_addition_accel->m_updator);
    ui_sprite_chipmunk_env_t env = ui_sprite_chipmunk_env_find(world);
    float ptm;
    float accel;
    float angle;
    
    assert(env);
    ptm = plugin_chipmunk_env_ptm(env->m_env);

    if (with_addition_accel->m_accel == NULL) {
        CPE_ERROR(with_addition_accel->m_module->m_em, "chipmunk with addition accel: accle not configured!");
        return -1;
    }
    
    if (with_addition_accel->m_angle == NULL) {
        CPE_ERROR(with_addition_accel->m_module->m_em, "chipmunk with addition accel: angle not configured!");
        return -1;
    }
    
    if (with_addition_accel->m_accel[0] == ':') {
        if (ui_sprite_fsm_action_try_calc_float(
                &accel, with_addition_accel->m_accel + 1,
                ui_sprite_fsm_action_from_data(with_addition_accel), NULL,
                with_addition_accel->m_module->m_em)
            != 0)
        {
            CPE_ERROR(
                with_addition_accel->m_module->m_em, "chipmunk with addition accel: calc accel from %s fail!",
                with_addition_accel->m_accel + 1);
        }
    }
    else {
        accel = atof(with_addition_accel->m_accel);
    }

    if (with_addition_accel->m_angle[0] == ':') {
        if (ui_sprite_fsm_action_try_calc_float(
                &angle, with_addition_accel->m_angle + 1,
                ui_sprite_fsm_action_from_data(with_addition_accel), NULL,
                with_addition_accel->m_module->m_em)
            != 0)
        {
            CPE_ERROR(
                with_addition_accel->m_module->m_em, "chipmunk with addition accel: calc angle from %s fail!",
                with_addition_accel->m_angle + 1);
        }
    }
    else {
        angle = atof(with_addition_accel->m_angle);
    }

    accel *= ptm;
    
    data->x = cpe_cos_angle(angle) * accel;
    data->y = cpe_sin_angle(angle) * accel;

    return 0;
}

static int ui_sprite_chipmunk_with_addition_accel_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_addition_accel_t with_addition_accel = (ui_sprite_chipmunk_with_addition_accel_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with addition accel: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (with_addition_accel->m_accel == NULL && with_addition_accel->m_angle == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with addition accel: no accel configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    assert(with_addition_accel->m_updator == NULL);

    with_addition_accel->m_updator =
        ui_sprite_chipmunk_obj_updator_create(
            chipmunk_obj, ui_sprite_chipmunk_with_addition_accel_do_update, NULL, sizeof(UI_SPRITE_CHIPMUNK_PAIR));
    if (with_addition_accel->m_updator == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with addition accel: no accel configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_chipmunk_with_addition_accel_update_data(with_addition_accel, world) != 0) {
        ui_sprite_chipmunk_obj_updator_free(with_addition_accel->m_updator);
        with_addition_accel->m_updator = NULL;
        return -1;
    }

    return 0;
}

static void ui_sprite_chipmunk_with_addition_accel_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_addition_accel_t with_addition_accel = (ui_sprite_chipmunk_with_addition_accel_t)ui_sprite_fsm_action_data(fsm_action);
    assert(with_addition_accel->m_updator);
    ui_sprite_chipmunk_obj_updator_free(with_addition_accel->m_updator);
    with_addition_accel->m_updator = NULL;
}

static int ui_sprite_chipmunk_with_addition_accel_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_addition_accel_t with_addition_accel = (ui_sprite_chipmunk_with_addition_accel_t)ui_sprite_fsm_action_data(fsm_action);
    with_addition_accel->m_module = (ui_sprite_chipmunk_module_t)ctx;
    with_addition_accel->m_updator = NULL;
    with_addition_accel->m_accel = NULL;
    with_addition_accel->m_angle = NULL;
    return 0;
}

static void ui_sprite_chipmunk_with_addition_accel_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_addition_accel_t with_addition_accel = (ui_sprite_chipmunk_with_addition_accel_t)ui_sprite_fsm_action_data(fsm_action);
    assert(with_addition_accel->m_updator == NULL);

    if (with_addition_accel->m_accel) {
        mem_free(module->m_alloc, (void*)with_addition_accel->m_accel);
        with_addition_accel->m_accel = NULL;
    }

    if (with_addition_accel->m_angle) {
        mem_free(module->m_alloc, (void*)with_addition_accel->m_angle);
        with_addition_accel->m_angle = NULL;
    }
}

static int ui_sprite_chipmunk_with_addition_accel_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_addition_accel_t to_with_addition_accel = (ui_sprite_chipmunk_with_addition_accel_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_addition_accel_t from_with_addition_accel = (ui_sprite_chipmunk_with_addition_accel_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_with_addition_accel_init(to, ctx)) return -1;

    if (from_with_addition_accel->m_accel) {
        to_with_addition_accel->m_accel = cpe_str_mem_dup(module->m_alloc, from_with_addition_accel->m_accel);
    }

    if (from_with_addition_accel->m_angle) {
        to_with_addition_accel->m_angle = cpe_str_mem_dup(module->m_alloc, from_with_addition_accel->m_angle);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_addition_accel_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_with_addition_accel_t with_addition_accel = (ui_sprite_chipmunk_with_addition_accel_t)ui_sprite_chipmunk_with_addition_accel_create(fsm_state, name);
    const char * str_value;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_addition_accel action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (with_addition_accel == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_addition_accel action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "accel", NULL))) {
        if (with_addition_accel->m_accel) {
            mem_free(module->m_alloc, (void*)with_addition_accel->m_accel);
        }

        with_addition_accel->m_accel = cpe_str_mem_dup(module->m_alloc, str_value);
    }

    if ((str_value = cfg_get_string(cfg, "angle", NULL))) {
        if (with_addition_accel->m_angle) {
            mem_free(module->m_alloc, (void*)with_addition_accel->m_angle);
        }

        with_addition_accel->m_angle = cpe_str_mem_dup(module->m_alloc, str_value);
    }
    
    return ui_sprite_fsm_action_from_data(with_addition_accel);
}

int ui_sprite_chipmunk_with_addition_accel_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_NAME, sizeof(struct ui_sprite_chipmunk_with_addition_accel));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with addition accel register: meta create fail",
            ui_sprite_chipmunk_module_name  (module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_addition_accel_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_addition_accel_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_addition_accel_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_addition_accel_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_addition_accel_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_NAME, ui_sprite_chipmunk_with_addition_accel_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_addition_accel_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_ADDITION_ACCEL_NAME = "chipmunk-with-addition-accel";

#ifdef __cplusplus
}
#endif
    
