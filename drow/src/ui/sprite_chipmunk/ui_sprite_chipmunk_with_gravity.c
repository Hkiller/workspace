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
#include "ui_sprite_chipmunk_with_gravity_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_gravity_t ui_sprite_chipmunk_with_gravity_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_GRAVITY_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_gravity_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_gravity_free(ui_sprite_chipmunk_with_gravity_t with_gravity) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_gravity);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_with_gravity_calc_gravity(
    ui_sprite_chipmunk_module_t module, ui_sprite_chipmunk_with_gravity_t with_gravity, ui_sprite_entity_t entity,
    UI_SPRITE_CHIPMUNK_GRAVITY * gravity)
{
    if (with_gravity->m_gravity) {
        float ptm;
        float accel;
        ui_sprite_chipmunk_env_t env;
        
        if (with_gravity->m_gravity[0] == ':') {
            if (ui_sprite_fsm_action_try_calc_float(
                    &accel, with_gravity->m_gravity + 1,
                    ui_sprite_fsm_action_from_data(with_gravity), NULL,

                    with_gravity->m_module->m_em)
                != 0)
            {
                CPE_ERROR(
                    with_gravity->m_module->m_em, "chipmunk with gravity: calc accel from %s fail!",
                    with_gravity->m_gravity + 1);
                return -1;
            }
        }
        else {
            accel = atof(with_gravity->m_gravity);
        }

        env = ui_sprite_chipmunk_env_find(ui_sprite_entity_world(entity));
        if (env == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_gravity action: env not exist!", ui_sprite_chipmunk_module_name(module));
            return -1;
        }
        
        ptm = plugin_chipmunk_env_ptm(env->m_env);

        accel *= ptm;
    
        if (with_gravity->m_gravity_angle) {
            float angle;
            if (with_gravity->m_gravity_angle[0] == ':') {
                if (ui_sprite_fsm_action_try_calc_float(
                        &angle, with_gravity->m_gravity_angle + 1,
                        ui_sprite_fsm_action_from_data(with_gravity), NULL,
                        with_gravity->m_module->m_em)
                    != 0)
                {
                    CPE_ERROR(
                        with_gravity->m_module->m_em, "chipmunk with gravity: calc angle from %s fail!",
                        with_gravity->m_gravity_angle + 1);
                    return -1;
                }
            }
            else {
                angle = atof(with_gravity->m_gravity_angle);
            }
            gravity->type = UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_FIX_VALUE;
            gravity->data.fix_value.gravity.x = cpe_cos_angle(angle) * accel;
            gravity->data.fix_value.gravity.y = cpe_sin_angle(angle) * accel;
        }
        else {
            gravity->type = UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_FIX_SIZE_VALUE;
            gravity->data.fix_size_value.gravity = accel;
        }
    }
    else if (with_gravity->m_gravity_adj) {
        float adj_value;
        
        if (with_gravity->m_gravity_angle[0] == ':') {
            if (ui_sprite_fsm_action_try_calc_float(
                    &adj_value, with_gravity->m_gravity_adj + 1,
                    ui_sprite_fsm_action_from_data(with_gravity), NULL,
                    with_gravity->m_module->m_em)
                != 0)
            {
                CPE_ERROR(
                    with_gravity->m_module->m_em, "chipmunk with gravity: calc adj value from %s fail!",
                    with_gravity->m_gravity_adj + 1);
                return -1;
            }
        }
        else {
            adj_value = atof(with_gravity->m_gravity_adj);
        }

        gravity->type = UI_SPRITE_CHIPMUNK_GRAVITY_TYPE_ADJ_VALUE;
        gravity->data.adj_value.adj_value = adj_value;
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with gravity: body %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_gravity->m_body_name);
        return -1;
    }

    return 0;
}
    
static int ui_sprite_chipmunk_with_gravity_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_gravity_t with_gravity = (ui_sprite_chipmunk_with_gravity_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_body_t body;
    UI_SPRITE_CHIPMUNK_GRAVITY gravity;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with gravity: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (with_gravity->m_body_name[0]) {
        body = ui_sprite_chipmunk_obj_body_find(chipmunk_obj, with_gravity->m_body_name);
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with gravity: body %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_gravity->m_body_name);
            return -1;
        }
    }
    else {
        body = chipmunk_obj->m_main_body;
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with gravity: main body not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }
    }

    if (ui_sprite_chipmunk_with_gravity_calc_gravity(module, with_gravity, entity, &gravity) != 0) {
        return -1;
    }

    with_gravity->m_saved_gravity = body->m_body_attrs.m_gravity;
    ui_sprite_chipmunk_obj_body_set_gravity(body, &gravity);
    
    return 0;
}

static void ui_sprite_chipmunk_with_gravity_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_gravity_t with_gravity = (ui_sprite_chipmunk_with_gravity_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_body_t body;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with gravity: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (with_gravity->m_body_name[0]) {
        body = ui_sprite_chipmunk_obj_body_find(chipmunk_obj, with_gravity->m_body_name);
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with gravity: body %s not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_gravity->m_body_name);
            return;
        }
    }
    else {
        body = chipmunk_obj->m_main_body;
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with gravity: main body not exist!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return;
        }
    }
    
    ui_sprite_chipmunk_obj_body_set_gravity(body, &with_gravity->m_saved_gravity);
}

static int ui_sprite_chipmunk_with_gravity_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_gravity_t with_gravity = (ui_sprite_chipmunk_with_gravity_t)ui_sprite_fsm_action_data(fsm_action);
    with_gravity->m_module = (ui_sprite_chipmunk_module_t)ctx;
    with_gravity->m_body_name[0] = 0;
    with_gravity->m_gravity_adj = NULL;
    with_gravity->m_gravity = NULL;
    with_gravity->m_gravity_angle = NULL;
    return 0;
}

static void ui_sprite_chipmunk_with_gravity_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_gravity_t with_gravity = (ui_sprite_chipmunk_with_gravity_t)ui_sprite_fsm_action_data(fsm_action);

    if (with_gravity->m_gravity_adj) {
        mem_free(module->m_alloc, with_gravity->m_gravity_adj);
        with_gravity->m_gravity_adj = NULL;
    }

    if (with_gravity->m_gravity) {
        mem_free(module->m_alloc, with_gravity->m_gravity);
        with_gravity->m_gravity = NULL;
    }

    if (with_gravity->m_gravity_angle) {
        mem_free(module->m_alloc, with_gravity->m_gravity_angle);
        with_gravity->m_gravity_angle = NULL;
    }
}

static int ui_sprite_chipmunk_with_gravity_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_gravity_t to_with_gravity = (ui_sprite_chipmunk_with_gravity_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_gravity_t from_with_gravity = (ui_sprite_chipmunk_with_gravity_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_with_gravity_init(to, ctx)) return -1;

    cpe_str_dup(to_with_gravity->m_body_name, sizeof(to_with_gravity->m_body_name), from_with_gravity->m_body_name);

    if (from_with_gravity->m_gravity) {
        to_with_gravity->m_gravity = cpe_str_mem_dup(module->m_alloc, from_with_gravity->m_gravity);
    }

    if (from_with_gravity->m_gravity_angle) {
        to_with_gravity->m_gravity_angle = cpe_str_mem_dup(module->m_alloc, from_with_gravity->m_gravity_angle);
    }

    if (from_with_gravity->m_gravity_adj) {
        to_with_gravity->m_gravity_adj = cpe_str_mem_dup(module->m_alloc, from_with_gravity->m_gravity_adj);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_gravity_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_with_gravity_t with_gravity = (ui_sprite_chipmunk_with_gravity_t)ui_sprite_chipmunk_with_gravity_create(fsm_state, name);
    const char * str_value;
    cfg_t child_cfg;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_gravity action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (with_gravity == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_gravity action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "body-name", NULL))) {
        cpe_str_dup(with_gravity->m_body_name, sizeof(with_gravity->m_body_name), str_value);
    }

    if ((child_cfg = cfg_find_cfg(cfg, "fix-value"))) {
        const char * gravity = cfg_get_string(child_cfg, "gravity", NULL);
        const char * gravity_angle = cfg_get_string(child_cfg, "angle", NULL);

        if (gravity == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create with_gravity action: fix-value.gravity not configured!",
                ui_sprite_chipmunk_module_name(module));
            return NULL;
        }
        
        with_gravity->m_gravity = cpe_str_mem_dup(module->m_alloc, gravity);
        with_gravity->m_gravity_angle = gravity_angle ? cpe_str_mem_dup(module->m_alloc, gravity_angle) : NULL;
    }
    else if ((child_cfg = cfg_find_cfg(cfg, "adj-value"))) {
        const char * gravity_adj = cfg_as_string(child_cfg, NULL);

        if (gravity_adj == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create with_gravity action: adj-value type error!",
                ui_sprite_chipmunk_module_name(module));
            return NULL;
        }

        with_gravity->m_gravity_adj = cpe_str_mem_dup(module->m_alloc, gravity_adj);
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create with_gravity action: unknown type!",
            ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(with_gravity);
}

    
int ui_sprite_chipmunk_with_gravity_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_GRAVITY_NAME, sizeof(struct ui_sprite_chipmunk_with_gravity));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with gravity register: meta create fail",
            ui_sprite_chipmunk_module_name  (module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_gravity_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_gravity_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_gravity_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_gravity_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_gravity_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_GRAVITY_NAME, ui_sprite_chipmunk_with_gravity_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_gravity_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_GRAVITY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_GRAVITY_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_GRAVITY_NAME = "chipmunk-with-gravity";

#ifdef __cplusplus
}
#endif
    
