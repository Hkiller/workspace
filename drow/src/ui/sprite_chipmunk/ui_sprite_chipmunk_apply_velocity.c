#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
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
#include "ui_sprite_chipmunk_apply_velocity_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_apply_velocity_t ui_sprite_chipmunk_apply_velocity_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_NAME);
    return fsm_action ? (ui_sprite_chipmunk_apply_velocity_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_apply_velocity_free(ui_sprite_chipmunk_apply_velocity_t apply_velocity) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(apply_velocity);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_apply_velocity_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_apply_velocity_t apply_velocity = (ui_sprite_chipmunk_apply_velocity_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_body_t body;
    float ptm;
    float mass;
    float velocity;
    float velocity_angle;
    cpVect impulse;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk apply velocity: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (apply_velocity->m_unit == 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk apply velocity: unit not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (apply_velocity->m_velocity == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk apply velocity: velocity not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    if (apply_velocity->m_velocity_angle == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk apply velocity: velocity angle not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    if (apply_velocity->m_body_name[0]) {
        body = ui_sprite_chipmunk_obj_body_find(chipmunk_obj, apply_velocity->m_body_name);
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk apply velocity: body %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), apply_velocity->m_body_name);
            return -1;
        }
    }
    else {
        body = chipmunk_obj->m_main_body;
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk apply velocity: main body not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    if (apply_velocity->m_velocity[0] == ':') {
        if (ui_sprite_fsm_action_try_calc_float(
                &velocity, apply_velocity->m_velocity + 1,
                ui_sprite_fsm_action_from_data(apply_velocity), NULL,
                apply_velocity->m_module->m_em)
            != 0)
        {
            CPE_ERROR(
                apply_velocity->m_module->m_em, "chipmunk apply velocity: calc accel from %s fail!",
                apply_velocity->m_velocity + 1);
            return -1;
        }
    }
    else {
        velocity = atof(apply_velocity->m_velocity);
    }

    if (apply_velocity->m_velocity_angle[0] == ':') {
        if (ui_sprite_fsm_action_try_calc_float(
                &velocity_angle, apply_velocity->m_velocity_angle + 1,
                ui_sprite_fsm_action_from_data(apply_velocity), NULL,
                apply_velocity->m_module->m_em)
            != 0)
        {
            CPE_ERROR(
                apply_velocity->m_module->m_em, "chipmunk apply velocity: calc accel from %s fail!",
                apply_velocity->m_velocity_angle + 1);
            return -1;
        }
    }
    else {
        velocity_angle = atof(apply_velocity->m_velocity_angle);
    }

    ptm =
        apply_velocity->m_unit == ui_sprite_chipmunk_unit_logic
        ? plugin_chipmunk_env_ptm(body->m_obj->m_env->m_env)
        : 1.0f;
    
    mass = cpBodyGetMass(&body->m_body);

    impulse.x = velocity * cpe_cos_angle(velocity_angle) * ptm * mass;
    impulse.y = velocity * cpe_sin_angle(velocity_angle) * ptm * mass;

    cpBodyApplyImpulseAtLocalPoint(&body->m_body, impulse, cpv(0.0f, 0.0f));

    return 0;
}

static void ui_sprite_chipmunk_apply_velocity_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_chipmunk_apply_velocity_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_apply_velocity_t apply_velocity = (ui_sprite_chipmunk_apply_velocity_t)ui_sprite_fsm_action_data(fsm_action);
    apply_velocity->m_module = (ui_sprite_chipmunk_module_t)ctx;
    apply_velocity->m_body_name[0] = 0;
    apply_velocity->m_unit = ui_sprite_chipmunk_unit_unknown;
    apply_velocity->m_velocity = NULL;
    apply_velocity->m_velocity_angle = NULL;
    return 0;
}

static void ui_sprite_chipmunk_apply_velocity_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_apply_velocity_t apply_velocity = (ui_sprite_chipmunk_apply_velocity_t)ui_sprite_fsm_action_data(fsm_action);

    if (apply_velocity->m_velocity) {
        mem_free(module->m_alloc, apply_velocity->m_velocity);
        apply_velocity->m_velocity = NULL;
    }

    if (apply_velocity->m_velocity_angle) {
        mem_free(module->m_alloc, apply_velocity->m_velocity_angle);
        apply_velocity->m_velocity_angle = NULL;
    }
}

static int ui_sprite_chipmunk_apply_velocity_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_apply_velocity_t to_apply_velocity = (ui_sprite_chipmunk_apply_velocity_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_apply_velocity_t from_apply_velocity = (ui_sprite_chipmunk_apply_velocity_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_apply_velocity_init(to, ctx)) return -1;

    cpe_str_dup(to_apply_velocity->m_body_name, sizeof(to_apply_velocity->m_body_name), from_apply_velocity->m_body_name);
    to_apply_velocity->m_unit = from_apply_velocity->m_unit;

    if (from_apply_velocity->m_velocity) {
        to_apply_velocity->m_velocity = cpe_str_mem_dup(module->m_alloc, from_apply_velocity->m_velocity);
    }

    if (from_apply_velocity->m_velocity_angle) {
        to_apply_velocity->m_velocity_angle = cpe_str_mem_dup(module->m_alloc, from_apply_velocity->m_velocity_angle);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_apply_velocity_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_apply_velocity_t apply_velocity = (ui_sprite_chipmunk_apply_velocity_t)ui_sprite_chipmunk_apply_velocity_create(fsm_state, name);
    const char * str_value;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create apply_velocity action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (apply_velocity == NULL) {
        CPE_ERROR(module->m_em, "%s: create apply_velocity action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "unit", NULL))) {
        ui_sprite_chipmunk_unit_t unit = ui_sprite_chipmunk_unit_from_str(str_value);
        if (unit == 0) {
            CPE_ERROR(
                module->m_em, "%s: create apply_velocity action: read unit from %s fail!",
                ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }

        apply_velocity->m_unit = unit;
    }

    if ((str_value = cfg_get_string(cfg, "body-name", NULL))) {
        cpe_str_dup(apply_velocity->m_body_name, sizeof(apply_velocity->m_body_name), str_value);
    }

    if ((str_value = cfg_get_string(cfg, "velocity", NULL))) {
        apply_velocity->m_velocity = cpe_str_mem_dup(module->m_alloc, str_value);
    }
        
    if ((str_value = cfg_get_string(cfg, "angle", NULL))) {
        apply_velocity->m_velocity_angle = cpe_str_mem_dup(module->m_alloc, str_value);
    }

    return ui_sprite_fsm_action_from_data(apply_velocity);
}

    
int ui_sprite_chipmunk_apply_velocity_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_NAME, sizeof(struct ui_sprite_chipmunk_apply_velocity));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk apply velocity register: meta create fail",
            ui_sprite_chipmunk_module_name  (module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_apply_velocity_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_apply_velocity_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_apply_velocity_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_apply_velocity_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_apply_velocity_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_NAME, ui_sprite_chipmunk_apply_velocity_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_apply_velocity_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_NAME);
}

const char * UI_SPRITE_CHIPMUNK_APPLY_VELOCITY_NAME = "chipmunk-apply-velocity";

#ifdef __cplusplus
}
#endif
    
