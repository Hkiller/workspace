#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_render_lock_on_screen_i.h"

ui_sprite_render_lock_on_screen_t ui_sprite_render_lock_on_screen_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CAMERA_LOCK_ON_SCREEN_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_lock_on_screen_free(ui_sprite_render_lock_on_screen_t lock_on_screen) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(lock_on_screen);
    ui_sprite_fsm_action_free(fsm_action);
}

float ui_sprite_render_lock_on_screen_max_speed(ui_sprite_render_lock_on_screen_t lock_on_screen) {
    return lock_on_screen->m_max_speed;
}

void ui_sprite_render_lock_on_screen_set_max_speed(ui_sprite_render_lock_on_screen_t lock_on_screen, float max_speed) {
    lock_on_screen->m_max_speed = max_speed;
}

int ui_sprite_render_lock_on_screen_set_decorator(ui_sprite_render_lock_on_screen_t lock_on_screen, const char * decorator) {
    return ui_percent_decorator_setup(&lock_on_screen->m_decorator, decorator, lock_on_screen->m_module->m_em);
}

static int ui_sprite_render_lock_on_screen_update_pos(
    ui_sprite_render_lock_on_screen_t lock_on_screen, ui_sprite_2d_transform_t transform, ui_sprite_render_env_t render)
{
    ui_vector_2 entity_pos;
    ui_vector_2 target_pos;

    if (lock_on_screen->m_duration <= 0.0f || lock_on_screen->m_runing_time >= lock_on_screen->m_duration) {
        target_pos = lock_on_screen->m_target_pos_on_screen;
    }
    else {
        float percent = lock_on_screen->m_runing_time / lock_on_screen->m_duration;
        percent = ui_percent_decorator_decorate(&lock_on_screen->m_decorator, percent);

        if (lock_on_screen->m_cfg_x) {
            target_pos.x =
                lock_on_screen->m_init_pos_on_screen.x
                + (lock_on_screen->m_target_pos_on_screen.x - lock_on_screen->m_init_pos_on_screen.x) * percent;
        }
        else {
            target_pos.x = 0.0f;
        }
        
        if (lock_on_screen->m_cfg_y) {
            target_pos.y =
                lock_on_screen->m_init_pos_on_screen.y
                + (lock_on_screen->m_target_pos_on_screen.y - lock_on_screen->m_init_pos_on_screen.y) * percent;
        }
        else {
            target_pos.y = 0.0f;
        }
    }

    target_pos = ui_sprite_render_env_screen_to_world(render, &target_pos);
    
    entity_pos = ui_sprite_2d_transform_origin_pos(transform);

    if (lock_on_screen->m_cfg_x) entity_pos.x = target_pos.x;
    if (lock_on_screen->m_cfg_y) {
        entity_pos.y = target_pos.y;
    }
    
    ui_sprite_2d_transform_set_origin_pos(transform, entity_pos);

    return 0;
}

static int ui_sprite_render_lock_on_screen_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_lock_on_screen_t lock_on_screen = ui_sprite_fsm_action_data(fsm_action); 
    ui_sprite_render_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_render_env_t render = ui_sprite_render_env_find(world);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            lock_on_screen->m_module->m_em, "entity %d(%s): lock_on_screen: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (render == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): lock_on_screen: no render!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (lock_on_screen->m_cfg_x) {
        if (ui_sprite_fsm_action_check_calc_float(&lock_on_screen->m_target_pos_on_screen.x, lock_on_screen->m_cfg_x, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): lock_on_screen: calc target-x from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), lock_on_screen->m_cfg_x);
            return -1;
        }
    }
    else {
        lock_on_screen->m_target_pos_on_screen.x = 0.0f;
    }

    if (lock_on_screen->m_cfg_y) {
        if (ui_sprite_fsm_action_check_calc_float(&lock_on_screen->m_target_pos_on_screen.y, lock_on_screen->m_cfg_y, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): lock_on_screen: calc target-y from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), lock_on_screen->m_cfg_y);
            return -1;
        }
    }
    else {
        lock_on_screen->m_target_pos_on_screen.y = 0.0f;
    }
    
    if (lock_on_screen->m_max_speed > 0.0f) {
        lock_on_screen->m_init_pos_on_screen = ui_sprite_2d_transform_origin_pos(transform);
        lock_on_screen->m_init_pos_on_screen =
            ui_sprite_render_env_world_to_screen(render, &lock_on_screen->m_init_pos_on_screen);
        if (lock_on_screen->m_cfg_x == NULL) lock_on_screen->m_init_pos_on_screen.x = 0.0f;
        if (lock_on_screen->m_cfg_y == NULL) lock_on_screen->m_init_pos_on_screen.y = 0.0f;
        
        lock_on_screen->m_duration =
            cpe_math_distance(
                lock_on_screen->m_init_pos_on_screen.x, lock_on_screen->m_init_pos_on_screen.y, 
                lock_on_screen->m_target_pos_on_screen.x, lock_on_screen->m_target_pos_on_screen.y)
            / lock_on_screen->m_max_speed;
    }
    else {
        lock_on_screen->m_duration = 0.0f;
        if (ui_sprite_render_lock_on_screen_update_pos(lock_on_screen, transform, render) != 0) return -1;
    }

    lock_on_screen->m_runing_time = 0.0f;
    ui_sprite_fsm_action_start_update(fsm_action);

    return 0;
}

static void ui_sprite_render_lock_on_screen_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_render_lock_on_screen_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_lock_on_screen_t lock_on_screen = ui_sprite_fsm_action_data(fsm_action);

    bzero(lock_on_screen, sizeof(*lock_on_screen));

    lock_on_screen->m_module = ctx;
    
    return 0;
}

static void ui_sprite_render_lock_on_screen_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_lock_on_screen_t lock_on_screen = ui_sprite_fsm_action_data(fsm_action);

    if (lock_on_screen->m_cfg_x) {
        mem_free(module->m_alloc, lock_on_screen->m_cfg_x);
        lock_on_screen->m_cfg_x = NULL;
    }

    if (lock_on_screen->m_cfg_y) {
        mem_free(module->m_alloc, lock_on_screen->m_cfg_y);
        lock_on_screen->m_cfg_y = NULL;
    }
}

static int ui_sprite_render_lock_on_screen_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_lock_on_screen_t to_on_screen = ui_sprite_fsm_action_data(to);
    ui_sprite_render_lock_on_screen_t from_on_screen = ui_sprite_fsm_action_data(from);

    if (ui_sprite_render_lock_on_screen_init(to, ctx)) return -1;

    to_on_screen->m_max_speed = from_on_screen->m_max_speed;
    memcpy(&to_on_screen->m_decorator, &from_on_screen->m_decorator, sizeof(to_on_screen->m_decorator));

    if (from_on_screen->m_cfg_x) to_on_screen->m_cfg_x = cpe_str_mem_dup(module->m_alloc, from_on_screen->m_cfg_x);
    if (from_on_screen->m_cfg_y) to_on_screen->m_cfg_y = cpe_str_mem_dup(module->m_alloc, from_on_screen->m_cfg_y);

    return 0;
}

static void ui_sprite_render_lock_on_screen_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_render_lock_on_screen_t lock_on_screen = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_render_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_render_env_t render = ui_sprite_render_env_find(world);
    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);

    if (transform == NULL) {
        CPE_ERROR(
            lock_on_screen->m_module->m_em, "entity %d(%s): lock_on_screen: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    if (render == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): lock_on_screen: no render!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    lock_on_screen->m_runing_time += delta;

    ui_sprite_render_lock_on_screen_update_pos(lock_on_screen, transform, render);
}

static ui_sprite_fsm_action_t ui_sprite_render_lock_on_screen_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_lock_on_screen_t lock_on_screen = ui_sprite_render_lock_on_screen_create(fsm_state, name);
    const char * str_value;

    if (lock_on_screen == NULL) {
        CPE_ERROR(module->m_em, "%s: create lock_on_screen action: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "pos.x", NULL))) {
        lock_on_screen->m_cfg_x = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        if (lock_on_screen->m_cfg_x == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create lock-on-screen: set x %s fail!",
                ui_sprite_render_module_name(module), str_value);
            ui_sprite_render_lock_on_screen_free(lock_on_screen);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "pos.y", NULL))) {
        lock_on_screen->m_cfg_y = cpe_str_mem_dup_trim(module->m_alloc, str_value);
        if (lock_on_screen->m_cfg_y == NULL) {
            CPE_ERROR(
                module->m_em, "%s: create lock-on-screen: set y %s fail!",
                ui_sprite_render_module_name(module), str_value);
            ui_sprite_render_lock_on_screen_free(lock_on_screen);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "decorator", NULL))) {
        if (ui_sprite_render_lock_on_screen_set_decorator(lock_on_screen, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create lock_on_screen action: set decorator %s fail!",
                ui_sprite_render_module_name(module), str_value);
            ui_sprite_render_lock_on_screen_free(lock_on_screen);
            return NULL;
        }
    }

    ui_sprite_render_lock_on_screen_set_max_speed(
        lock_on_screen,
        cfg_get_float(cfg, "max-speed", ui_sprite_render_lock_on_screen_max_speed(lock_on_screen)));
    
    return ui_sprite_fsm_action_from_data(lock_on_screen);
}

int ui_sprite_render_lock_on_screen_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(module->m_fsm_module, UI_SPRITE_CAMERA_LOCK_ON_SCREEN_NAME, sizeof(struct ui_sprite_render_lock_on_screen));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: render lock_on_screen register: meta create fail",
            ui_sprite_render_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_lock_on_screen_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_lock_on_screen_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_lock_on_screen_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_lock_on_screen_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_lock_on_screen_clear, module);
	ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_render_lock_on_screen_update, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_CAMERA_LOCK_ON_SCREEN_NAME, ui_sprite_render_lock_on_screen_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }
    
    return 0;
}

void ui_sprite_render_lock_on_screen_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CAMERA_LOCK_ON_SCREEN_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: render lock_on_screen unregister: meta not exist",
            ui_sprite_render_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CAMERA_LOCK_ON_SCREEN_NAME);
    }
}

const char * UI_SPRITE_CAMERA_LOCK_ON_SCREEN_NAME = "lock-on-screen";

