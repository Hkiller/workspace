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
#include "ui_sprite_chipmunk_with_damping_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_damping_t ui_sprite_chipmunk_with_damping_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_DAMPING_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_damping_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_damping_free(ui_sprite_chipmunk_with_damping_t with_damping) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_damping);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_chipmunk_with_damping_do_update(
    ui_sprite_chipmunk_obj_updator_t updator, ui_sprite_chipmunk_obj_body_t body, UI_SPRITE_CHIPMUNK_PAIR * acc, float * damping)
{
    ui_sprite_chipmunk_with_damping_t with_damping =
        *(ui_sprite_chipmunk_with_damping_t *)ui_sprite_chipmunk_obj_updator_data(updator);
    *damping = with_damping->m_damping;
}

static int ui_sprite_chipmunk_with_damping_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_damping_t with_damping = (ui_sprite_chipmunk_with_damping_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with addition cfg_damping: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (with_damping->m_cfg_threshold) {
        if (ui_sprite_fsm_action_check_calc_float(
                &with_damping->m_threshold,
                with_damping->m_cfg_threshold, fsm_action, NULL, module->m_em)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with addition cfg_damping: calc threshold from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_damping->m_cfg_threshold);
            return -1;
        }
    }
    else {
        with_damping->m_threshold = 1.0f;
    }
    
    if (ui_sprite_fsm_action_check_calc_float(
            &with_damping->m_damping,
            with_damping->m_cfg_damping, fsm_action, NULL, module->m_em)
             != 0)
    {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with addition cfg_damping: calc damping from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_damping->m_cfg_damping);
        return -1;
    }

    if (with_damping->m_damping < 0.0f || with_damping->m_damping > 1.0f) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with addition cfg_damping: damping %f(%s) out of range!!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_damping->m_damping, with_damping->m_cfg_damping);
        return -1;
    }
    
    assert(with_damping->m_updator == NULL);

    with_damping->m_updator =
        ui_sprite_chipmunk_obj_updator_create(
            chipmunk_obj, ui_sprite_chipmunk_with_damping_do_update, NULL, sizeof(with_damping));
    if (with_damping->m_updator == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with addition cfg_damping: no cfg_damping configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    *(ui_sprite_chipmunk_with_damping_t *)ui_sprite_chipmunk_obj_updator_data(with_damping->m_updator) = with_damping;

    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }
    
    return 0;
}

static void ui_sprite_chipmunk_with_damping_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
	ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_damping_t with_damping = (ui_sprite_chipmunk_with_damping_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t obj;
    ui_sprite_chipmunk_obj_body_t body;
    cpVect cur_speed;

    obj = ui_sprite_chipmunk_obj_find(entity);
    if (obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with damping: entity is not chipmunk obj",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    body = ui_sprite_chipmunk_obj_main_body(obj);
    if (body == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with damping: no main body",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    cur_speed = cpBodyGetVelocity(&body->m_body);
    if (cpe_math_distance(0.0f, 0.0f, cur_speed.x, cur_speed.y) <= with_damping->m_threshold) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_chipmunk_with_damping_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_damping_t with_damping = (ui_sprite_chipmunk_with_damping_t)ui_sprite_fsm_action_data(fsm_action);
    assert(with_damping->m_updator);
    ui_sprite_chipmunk_obj_updator_free(with_damping->m_updator);
    with_damping->m_updator = NULL;
}

static int ui_sprite_chipmunk_with_damping_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_damping_t with_damping = (ui_sprite_chipmunk_with_damping_t)ui_sprite_fsm_action_data(fsm_action);
    with_damping->m_module = (ui_sprite_chipmunk_module_t)ctx;
    with_damping->m_updator = NULL;
    with_damping->m_cfg_damping = NULL;
    with_damping->m_cfg_threshold = NULL;
    return 0;
}

static void ui_sprite_chipmunk_with_damping_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_damping_t with_damping = (ui_sprite_chipmunk_with_damping_t)ui_sprite_fsm_action_data(fsm_action);
    assert(with_damping->m_updator == NULL);

    if (with_damping->m_cfg_damping) {
        mem_free(module->m_alloc, (void*)with_damping->m_cfg_damping);
        with_damping->m_cfg_damping = NULL;
    }

    if (with_damping->m_cfg_threshold) {
        mem_free(module->m_alloc, (void*)with_damping->m_cfg_threshold);
        with_damping->m_cfg_threshold = NULL;
    }
}

static int ui_sprite_chipmunk_with_damping_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_damping_t to_with_damping = (ui_sprite_chipmunk_with_damping_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_damping_t from_with_damping = (ui_sprite_chipmunk_with_damping_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_with_damping_init(to, ctx)) return -1;

    if (from_with_damping->m_cfg_damping) {
        to_with_damping->m_cfg_damping = cpe_str_mem_dup(module->m_alloc, from_with_damping->m_cfg_damping);
    }

    if (from_with_damping->m_cfg_threshold) {
        to_with_damping->m_cfg_threshold = cpe_str_mem_dup(module->m_alloc, from_with_damping->m_cfg_threshold);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_damping_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_with_damping_t with_damping = (ui_sprite_chipmunk_with_damping_t)ui_sprite_chipmunk_with_damping_create(fsm_state, name);
    const char * str_value;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_damping action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (with_damping == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_damping action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "damping", NULL))) {
        assert(with_damping->m_cfg_damping == NULL);
        with_damping->m_cfg_damping = cpe_str_mem_dup_trim(module->m_alloc, str_value);
    }
    else {
        CPE_ERROR(module->m_em, "%s: create with_damping action: damping not configured!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "threshold", NULL))) {
        assert(with_damping->m_cfg_threshold == NULL);
        with_damping->m_cfg_threshold = cpe_str_mem_dup_trim(module->m_alloc, str_value);
    }
    
    return ui_sprite_fsm_action_from_data(with_damping);
}

int ui_sprite_chipmunk_with_damping_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_DAMPING_NAME, sizeof(struct ui_sprite_chipmunk_with_damping));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with addition cfg_damping register: meta create fail",
            ui_sprite_chipmunk_module_name  (module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_damping_enter, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_chipmunk_with_damping_update, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_damping_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_damping_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_damping_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_damping_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_DAMPING_NAME, ui_sprite_chipmunk_with_damping_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_damping_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_DAMPING_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_DAMPING_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_DAMPING_NAME = "chipmunk-with-damping";

#ifdef __cplusplus
}
#endif
    
