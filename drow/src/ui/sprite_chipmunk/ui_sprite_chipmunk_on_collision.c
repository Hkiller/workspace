#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_entity_attr.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_chipmunk_on_collision_i.h"
#include "ui_sprite_chipmunk_obj_i.h"

ui_sprite_chipmunk_on_collision_t ui_sprite_chipmunk_on_collision_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_ON_COLLISION_NAME);
    return fsm_action ? (ui_sprite_chipmunk_on_collision_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_on_collision_free(ui_sprite_chipmunk_on_collision_t on_collision) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(on_collision);
    ui_sprite_fsm_action_free(fsm_action);
}

static void ui_sprite_chipmunk_on_collision_on_collision(
    void * ctx, UI_SPRITE_CHIPMUNK_COLLISION_DATA const * collision_data,
    ui_sprite_entity_t self_entity, ui_sprite_chipmunk_obj_body_t self_body,
    ui_sprite_entity_t other_entity, ui_sprite_chipmunk_obj_body_t other_body)
{
    ui_sprite_chipmunk_on_collision_t on_collision = (ui_sprite_chipmunk_on_collision_t)ctx;
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(on_collision);
    struct dr_data_source data_source_buf[64];
    dr_data_source_t data_source = data_source_buf;
    const char * event_def = NULL;

    if (collision_data->collision_state == UI_SPRITE_CHIPMUNK_COLLISION_STATE_BEGIN) {
        event_def = on_collision->m_on_collision_begin;
    }
    else if (collision_data->collision_state == UI_SPRITE_CHIPMUNK_COLLISION_STATE_END) {
        event_def = on_collision->m_on_collision_end;
    }

    if (event_def) {
        data_source->m_data.m_meta = on_collision->m_module->m_meta_chipmunk_collision_data;
        data_source->m_data.m_data = (void*)collision_data;
        data_source->m_data.m_size = sizeof(*collision_data);
        data_source->m_next =
            ui_sprite_entity_build_data_source(
                other_entity, data_source_buf + 1, CPE_ARRAY_SIZE(data_source_buf) - 1);

        ui_sprite_fsm_action_build_and_send_event(fsm_action, event_def, data_source);
    }
}

static int ui_sprite_chipmunk_on_collision_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_on_collision_t on_collision = (ui_sprite_chipmunk_on_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;

    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk on collision: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (ui_sprite_chipmunk_monitor_enter(chipmunk_obj, &on_collision->m_monitor) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk on collision:  obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    on_collision->m_monitor.m_ctx = on_collision;
    on_collision->m_monitor.m_on_collision = ui_sprite_chipmunk_on_collision_on_collision;
    
    return 0;
}

static void ui_sprite_chipmunk_on_collision_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_on_collision_t on_collision = (ui_sprite_chipmunk_on_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;

    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk on collision: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    ui_sprite_chipmunk_monitor_exit(&on_collision->m_monitor);
}

static int ui_sprite_chipmunk_on_collision_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_on_collision_t on_collision = (ui_sprite_chipmunk_on_collision_t)ui_sprite_fsm_action_data(fsm_action);
    on_collision->m_module = (ui_sprite_chipmunk_module_t)ctx;

    ui_sprite_chipmunk_monitor_init(module, &on_collision->m_monitor);
    on_collision->m_on_collision_begin = NULL;
    on_collision->m_on_collision_end = NULL;

    return 0;
}

static void ui_sprite_chipmunk_on_collision_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_on_collision_t on_collision = (ui_sprite_chipmunk_on_collision_t)ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_chipmunk_monitor_fini(module, &on_collision->m_monitor);
    
    if (on_collision->m_on_collision_begin) {
        mem_free(module->m_alloc, on_collision->m_on_collision_begin);
        on_collision->m_on_collision_begin = NULL;
    }

    if (on_collision->m_on_collision_end) {
        mem_free(module->m_alloc, on_collision->m_on_collision_end);
        on_collision->m_on_collision_end = NULL;
    }
}

static int ui_sprite_chipmunk_on_collision_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(to);
    ui_sprite_chipmunk_on_collision_t to_on_collision = (ui_sprite_chipmunk_on_collision_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_on_collision_t from_on_collision = (ui_sprite_chipmunk_on_collision_t)ui_sprite_fsm_action_data(from);
    
    if (ui_sprite_chipmunk_on_collision_init(to, ctx)) return -1;

    if (ui_sprite_chipmunk_monitor_copy(module, &to_on_collision->m_monitor, &from_on_collision->m_monitor) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk on collision: copy monitor fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_chipmunk_on_collision_free(to_on_collision);
        return -1;
    }
    
    if (from_on_collision->m_on_collision_begin) {
        to_on_collision->m_on_collision_begin = cpe_str_mem_dup(module->m_alloc, from_on_collision->m_on_collision_begin);
        if (to_on_collision->m_on_collision_begin == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk on collision: copy on collision begin fail !",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_chipmunk_on_collision_free(to_on_collision);
            return -1;
        }
    }

    if (from_on_collision->m_on_collision_end) {
        to_on_collision->m_on_collision_end = cpe_str_mem_dup(module->m_alloc, from_on_collision->m_on_collision_end);
        if (to_on_collision->m_on_collision_end == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk on collision: copy on collision end fail !",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_chipmunk_on_collision_free(to_on_collision);
            return -1;
        }
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_on_collision_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_on_collision_t on_collision = (ui_sprite_chipmunk_on_collision_t)ui_sprite_chipmunk_on_collision_create(fsm_state, name);
    const char * str_value;

    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create on_collision action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (on_collision == NULL) {
        CPE_ERROR(module->m_em, "%s: create on_collision action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if (ui_sprite_chipmunk_monitor_load(env, &on_collision->m_monitor, cfg) != 0) {
        CPE_ERROR(module->m_em, "%s: create on_collision action: monitor load fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "on-begin", NULL))) {
        on_collision->m_on_collision_begin = cpe_str_mem_dup(module->m_alloc, str_value);
        if (on_collision->m_on_collision_begin == NULL) {
            CPE_ERROR(module->m_em, "%s: create on_collision action: set on-begin %s fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "on-end", NULL))) {
        on_collision->m_on_collision_end = cpe_str_mem_dup(module->m_alloc, str_value);
        if (on_collision->m_on_collision_end == NULL) {
            CPE_ERROR(module->m_em, "%s: create on_collision action: set on-end %s fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(on_collision);
}

int ui_sprite_chipmunk_on_collision_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_ON_COLLISION_NAME, sizeof(struct ui_sprite_chipmunk_on_collision));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk on collision register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_on_collision_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_on_collision_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_on_collision_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_on_collision_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_on_collision_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_ON_COLLISION_NAME, ui_sprite_chipmunk_on_collision_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_on_collision_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_ON_COLLISION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_ON_COLLISION_NAME);
}

const char * UI_SPRITE_CHIPMUNK_ON_COLLISION_NAME = "chipmunk-on-collision";
