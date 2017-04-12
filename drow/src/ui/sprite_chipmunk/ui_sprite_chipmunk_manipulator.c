#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/random.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_manipulator_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "protocol/ui/sprite_chipmunk/ui_sprite_chipmunk_evt.h"

static void ui_sprite_chipmunk_manipulator_on_rand_lineear_velocity_angle(void * ctx, ui_sprite_event_t evt);

ui_sprite_chipmunk_manipulator_t ui_sprite_chipmunk_manipulator_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_MANIPULATOR_NAME);
    return fsm_action ? (ui_sprite_chipmunk_manipulator_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_manipulator_free(ui_sprite_chipmunk_manipulator_t manipulator) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(manipulator);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_manipulator_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_manipulator_t manipulator = (ui_sprite_chipmunk_manipulator_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_chipmunk_module_t module = manipulator->m_module;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (ui_sprite_fsm_action_add_event_handler(
            fsm_action, ui_sprite_event_scope_self,
            "ui_sprite_evt_chipmunk_rand_linear_velocity_angle",
            ui_sprite_chipmunk_manipulator_on_rand_lineear_velocity_angle, manipulator)
        != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk manipulator: enter: add event handler fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    return 0;
}

static void ui_sprite_chipmunk_manipulator_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_chipmunk_manipulator_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_manipulator_t manipulator = (ui_sprite_chipmunk_manipulator_t)ui_sprite_fsm_action_data(fsm_action);
    manipulator->m_module = (ui_sprite_chipmunk_module_t)ctx;
    return 0;
}

static void ui_sprite_chipmunk_manipulator_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    //ui_sprite_chipmunk_manipulator_t manipulator = (ui_sprite_chipmunk_manipulator_t)ui_sprite_fsm_action_data(fsm_action);
}

static int ui_sprite_chipmunk_manipulator_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_chipmunk_manipulator_init(to, ctx)) return -1;
   
    return 0;
}

void ui_sprite_chipmunk_manipulator_on_rand_lineear_velocity_angle(void * ctx, ui_sprite_event_t evt) {
    ui_sprite_chipmunk_manipulator_t manipulator = (ui_sprite_chipmunk_manipulator_t)ctx;
    ui_sprite_chipmunk_module_t module = manipulator->m_module;
    UI_SPRITE_EVT_CHIPMUNK_RAND_LINEAR_VELOCITY_ANGLE const * evt_data = (UI_SPRITE_EVT_CHIPMUNK_RAND_LINEAR_VELOCITY_ANGLE*)evt->data;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(ui_sprite_fsm_action_from_data(ctx));
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_body_t body;
    float angle;
    float velocity;

    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk manipulator: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    body = ui_sprite_chipmunk_obj_body_find(chipmunk_obj, evt_data->body);
    if (body == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk manipulator: body '%s' not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), evt_data->body);
        return;
    }

    angle = evt_data->angle_min + (evt_data->angle_max - evt_data->angle_min) * cpe_rand_dft(1000) / 1000.0f;
    velocity = evt_data->velocity_min + (evt_data->velocity_max - evt_data->velocity_min) * cpe_rand_dft(1000) / 1000.0f;

    switch(evt_data->base_policy) {
    case UI_SPRITE_EVT_CHIPMUNK_OP_BASE_OBJECT_FLIP: {
        ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
        if (transform == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk manipulator: base object flip: no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return;
        }
        
        angle = ui_sprite_2d_transform_adj_angle_by_flip(transform, angle);
        break;
    }
    case UI_SPRITE_EVT_CHIPMUNK_OP_BASE_OBJECT_ANGLE: {
        ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
        if (transform == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk manipulator: base object flip: no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return;
        }
        
        angle = cpe_math_angle_add(ui_sprite_2d_transform_angle(transform), angle);
        break;
    }
    case UI_SPRITE_EVT_CHIPMUNK_OP_BASE_OBJECT_MOVING:
        angle = cpe_math_angle_add(ui_sprite_chipmunk_obj_body_linear_velocity_angle(body), angle);
        break;
    }

    ui_sprite_chipmunk_obj_body_set_linear_velocity(body, angle, velocity);
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_manipulator_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_manipulator_t manipulator = (ui_sprite_chipmunk_manipulator_t)ui_sprite_chipmunk_manipulator_create(fsm_state, name);

    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create manipulator action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (manipulator == NULL) {
        CPE_ERROR(module->m_em, "%s: create manipulator action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(manipulator);
}

int ui_sprite_chipmunk_manipulator_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_MANIPULATOR_NAME, sizeof(struct ui_sprite_chipmunk_manipulator));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk on collision register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_manipulator_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_manipulator_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_manipulator_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_manipulator_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_manipulator_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_MANIPULATOR_NAME, ui_sprite_chipmunk_manipulator_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_manipulator_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_MANIPULATOR_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_MANIPULATOR_NAME);
}

const char * UI_SPRITE_CHIPMUNK_MANIPULATOR_NAME = "chipmunk-manipulator";
