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
#include "ui_sprite_chipmunk_track_angle_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_track_angle_t ui_sprite_chipmunk_track_angle_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_TRACK_ANGLE_NAME);
    return fsm_action ? (ui_sprite_chipmunk_track_angle_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_track_angle_free(ui_sprite_chipmunk_track_angle_t track_angle) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(track_angle);
    ui_sprite_fsm_action_free(fsm_action);
}

struct ui_sprite_chipmunk_track_angle_data {
    float m_step_duration;
};
    
static void ui_sprite_chipmunk_track_angle_do_update(
    ui_sprite_chipmunk_obj_updator_t updator, ui_sprite_chipmunk_obj_body_t body, UI_SPRITE_CHIPMUNK_PAIR * acc, float * damping)
{
    if (body == body->m_obj->m_main_body) {
        cpVect velocity = cpBodyGetVelocityAtLocalPoint(&body->m_body, cpv(0.0f, 0.0f));
        float cur_radians = cpBodyGetAngle(&body->m_body);
        float velocity_radians = cpe_math_radians(0.0f, 0.0f, velocity.x, velocity.y);

        if (velocity_radians != cur_radians) {
            struct ui_sprite_chipmunk_track_angle_data * updator_data =
                (struct ui_sprite_chipmunk_track_angle_data *)ui_sprite_chipmunk_obj_updator_data(updator);
            float v_adj = (velocity_radians - cur_radians) / updator_data->m_step_duration;
            body->m_body.w_bias += v_adj;
        }
    }
}

static int ui_sprite_chipmunk_track_angle_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_track_angle_t track_angle = (ui_sprite_chipmunk_track_angle_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_chipmunk_env_t env;
    struct ui_sprite_chipmunk_track_angle_data * updator_data;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk track angle: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    assert(track_angle->m_updator == NULL);

    track_angle->m_updator =
        ui_sprite_chipmunk_obj_updator_create(
            chipmunk_obj, ui_sprite_chipmunk_track_angle_do_update, NULL, sizeof(UI_SPRITE_CHIPMUNK_PAIR));
    if (track_angle->m_updator == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk track angle: no accel configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk track angle: no accel configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    updator_data = (struct ui_sprite_chipmunk_track_angle_data *)ui_sprite_chipmunk_obj_updator_data(track_angle->m_updator);
    updator_data->m_step_duration = plugin_chipmunk_env_step_duration(env->m_env);

    return 0;
}

static void ui_sprite_chipmunk_track_angle_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_track_angle_t track_angle = (ui_sprite_chipmunk_track_angle_t)ui_sprite_fsm_action_data(fsm_action);
    assert(track_angle->m_updator);
    ui_sprite_chipmunk_obj_updator_free(track_angle->m_updator);
    track_angle->m_updator = NULL;
}

static int ui_sprite_chipmunk_track_angle_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_track_angle_t track_angle = (ui_sprite_chipmunk_track_angle_t)ui_sprite_fsm_action_data(fsm_action);
    track_angle->m_module = (ui_sprite_chipmunk_module_t)ctx;
    track_angle->m_updator = NULL;
    return 0;
}

static void ui_sprite_chipmunk_track_angle_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_track_angle_t track_angle = (ui_sprite_chipmunk_track_angle_t)ui_sprite_fsm_action_data(fsm_action);
    assert(track_angle->m_updator == NULL);
}

static int ui_sprite_chipmunk_track_angle_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    if (ui_sprite_chipmunk_track_angle_init(to, ctx)) return -1;

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_track_angle_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_track_angle_t track_angle = (ui_sprite_chipmunk_track_angle_t)ui_sprite_chipmunk_track_angle_create(fsm_state, name);
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create track_angle action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (track_angle == NULL) {
        CPE_ERROR(module->m_em, "%s: create track_angle action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(track_angle);
}

int ui_sprite_chipmunk_track_angle_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_TRACK_ANGLE_NAME, sizeof(struct ui_sprite_chipmunk_track_angle));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk track angle register: meta create fail",
            ui_sprite_chipmunk_module_name  (module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_track_angle_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_track_angle_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_track_angle_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_track_angle_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_track_angle_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_TRACK_ANGLE_NAME, ui_sprite_chipmunk_track_angle_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_track_angle_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_TRACK_ANGLE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_TRACK_ANGLE_NAME);
}

const char * UI_SPRITE_CHIPMUNK_TRACK_ANGLE_NAME = "chipmunk-track-angle";

#ifdef __cplusplus
}
#endif
    
