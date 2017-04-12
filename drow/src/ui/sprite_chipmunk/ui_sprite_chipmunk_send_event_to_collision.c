#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_chipmunk_send_event_to_collision_i.h"
#include "ui_sprite_chipmunk_obj_i.h"

ui_sprite_chipmunk_send_event_to_collision_t ui_sprite_chipmunk_send_event_to_collision_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_SEND_EVENT_TO_COLLISION_NAME);
    return fsm_action ? (ui_sprite_chipmunk_send_event_to_collision_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_send_event_to_collision_free(ui_sprite_chipmunk_send_event_to_collision_t send_event_to_collision) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(send_event_to_collision);
    ui_sprite_fsm_action_free(fsm_action);
}

struct visit_ctx {
    ui_sprite_chipmunk_module_t m_module;
    ui_sprite_fsm_action_t m_action;
    const char * m_event;
    int m_rv;
};
    
static void ui_sprite_chipmunk_send_event_to_collision_on_entity(ui_sprite_group_t g, ui_sprite_entity_t entity, void * i_ctx) {
    struct visit_ctx * ctx = (struct visit_ctx *)i_ctx;
    ui_sprite_event_t event;

    event = ui_sprite_fsm_action_build_event(ctx->m_action, ctx->m_module->m_alloc, ctx->m_event, NULL);
    if (event == NULL) {
        CPE_ERROR(
            ctx->m_module->m_em, "entity %d(%s): chipmunk-send-event-to-collision: calc event from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), ctx->m_event);
        ctx->m_rv = -1;
        return;
    }

    ui_sprite_entity_send_event(entity, event->meta, event->data, event->size);

    mem_free(ctx->m_module->m_alloc, event);
}

static int ui_sprite_chipmunk_send_event_to_collision_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_send_event_to_collision_t send_event_to_collision =
        (ui_sprite_chipmunk_send_event_to_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    uint32_t mask = CP_ALL_CATEGORIES;
    ui_sprite_group_t collision_entity_group;
    struct visit_ctx process_ctx;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-send-event-to-collision: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (send_event_to_collision->m_cfg_event == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-send-event-to-collision: no event configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (send_event_to_collision->m_cfg_mask) {
        if (plugin_chipmunk_env_masks(chipmunk_obj->m_env->m_env, &mask, send_event_to_collision->m_cfg_mask) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk-send-event-to-collision: read mask from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), send_event_to_collision->m_cfg_mask);
            return -1;
        }
    }

    collision_entity_group = ui_sprite_chipmunk_obj_find_collision_entities(chipmunk_obj, mask);
    if (collision_entity_group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk-send-event-to-collision: find collision entities faild!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    process_ctx.m_module = module;
    process_ctx.m_action = fsm_action;
    process_ctx.m_event = send_event_to_collision->m_cfg_event;
    process_ctx.m_rv = 0;
    ui_sprite_group_visit(collision_entity_group, ui_sprite_chipmunk_send_event_to_collision_on_entity, &process_ctx);

    ui_sprite_group_free(collision_entity_group);

    return process_ctx.m_rv;
}

static void ui_sprite_chipmunk_send_event_to_collision_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_chipmunk_send_event_to_collision_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_send_event_to_collision_t send_event_to_collision = (ui_sprite_chipmunk_send_event_to_collision_t)ui_sprite_fsm_action_data(fsm_action);

    send_event_to_collision->m_module = (ui_sprite_chipmunk_module_t)ctx;
    send_event_to_collision->m_cfg_event = NULL;
    send_event_to_collision->m_cfg_mask = NULL;

    return 0;
}

static void ui_sprite_chipmunk_send_event_to_collision_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_send_event_to_collision_t send_event_to_collision = (ui_sprite_chipmunk_send_event_to_collision_t)ui_sprite_fsm_action_data(fsm_action);

    if (send_event_to_collision->m_cfg_event) {
        mem_free(module->m_alloc, send_event_to_collision->m_cfg_event);
        send_event_to_collision->m_cfg_event = NULL;
    }

    if (send_event_to_collision->m_cfg_mask) {
        mem_free(module->m_alloc, send_event_to_collision->m_cfg_mask);
        send_event_to_collision->m_cfg_mask = NULL;
    }
}

static int ui_sprite_chipmunk_send_event_to_collision_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(to);
    ui_sprite_chipmunk_send_event_to_collision_t to_send_event_to_collision = (ui_sprite_chipmunk_send_event_to_collision_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_send_event_to_collision_t from_send_event_to_collision = (ui_sprite_chipmunk_send_event_to_collision_t)ui_sprite_fsm_action_data(from);
    
    if (ui_sprite_chipmunk_send_event_to_collision_init(to, ctx)) return -1;

    if (from_send_event_to_collision->m_cfg_event) {
        to_send_event_to_collision->m_cfg_event = cpe_str_mem_dup(module->m_alloc, from_send_event_to_collision->m_cfg_event);
        if (to_send_event_to_collision->m_cfg_event == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk-send-event-to-collision: copy on event fail !",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_chipmunk_send_event_to_collision_free(to_send_event_to_collision);
            return -1;
        }
    }

    if (from_send_event_to_collision->m_cfg_mask) {
        to_send_event_to_collision->m_cfg_mask = cpe_str_mem_dup(module->m_alloc, from_send_event_to_collision->m_cfg_mask);
        if (to_send_event_to_collision->m_cfg_mask == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk-send-event-to-collision: copy mask fail !",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_chipmunk_send_event_to_collision_free(to_send_event_to_collision);
            return -1;
        }
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_send_event_to_collision_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_send_event_to_collision_t send_event_to_collision = (ui_sprite_chipmunk_send_event_to_collision_t)ui_sprite_chipmunk_send_event_to_collision_create(fsm_state, name);
    const char * str_value;

    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create send_event_to_collision action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (send_event_to_collision == NULL) {
        CPE_ERROR(module->m_em, "%s: create send_event_to_collision action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "event", NULL))) {
        send_event_to_collision->m_cfg_event = cpe_str_mem_dup(module->m_alloc, str_value);
        if (send_event_to_collision->m_cfg_event == NULL) {
            CPE_ERROR(module->m_em, "%s: create send_event_to_collision action: set event %s fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "masks", NULL))) {
        send_event_to_collision->m_cfg_mask = cpe_str_mem_dup(module->m_alloc, str_value);
        if (send_event_to_collision->m_cfg_mask == NULL) {
            CPE_ERROR(module->m_em, "%s: create send_event_to_collision action: set mask %s fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(send_event_to_collision);
}

int ui_sprite_chipmunk_send_event_to_collision_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_SEND_EVENT_TO_COLLISION_NAME, sizeof(struct ui_sprite_chipmunk_send_event_to_collision));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk-send-event-to-collision register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_send_event_to_collision_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_send_event_to_collision_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_send_event_to_collision_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_send_event_to_collision_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_send_event_to_collision_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_SEND_EVENT_TO_COLLISION_NAME, ui_sprite_chipmunk_send_event_to_collision_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_send_event_to_collision_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_SEND_EVENT_TO_COLLISION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_SEND_EVENT_TO_COLLISION_NAME);
}

const char * UI_SPRITE_CHIPMUNK_SEND_EVENT_TO_COLLISION_NAME = "chipmunk-send-event-to-collision";
