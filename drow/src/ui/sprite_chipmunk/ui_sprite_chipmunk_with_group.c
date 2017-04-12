#include <assert.h>
#include <stdio.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_chipmunk_with_group_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_runtime_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_group_t ui_sprite_chipmunk_with_group_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_GROUP_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_group_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_group_free(ui_sprite_chipmunk_with_group_t with_group) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_group);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_with_group_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_group_t with_group = (ui_sprite_chipmunk_with_group_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_body_t body;
    uint32_t group_id;

    if (with_group->m_group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with group: group not configure!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with group: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (with_group->m_group[0] == ':') {
        if (ui_sprite_fsm_action_try_calc_uint32(&group_id, with_group->m_group + 1, fsm_action, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with group: calc grop id from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_group->m_group + 1);
            return -1;
        }
    }
    else {
        group_id = atoi(with_group->m_group);
    }

    TAILQ_FOREACH(body, &chipmunk_obj->m_bodies, m_next_for_obj) {
        if (ui_sprite_chipmunk_obj_runtime_group_create(&with_group->m_runtime_groups, body, group_id) == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with group: not chipmunk obj!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_ERROR;
        }
    }

    return 0;

ENTER_ERROR:
    while(!TAILQ_EMPTY(&with_group->m_runtime_groups)) {
        ui_sprite_chipmunk_obj_runtime_group_free(
            with_group->m_module, &with_group->m_runtime_groups, TAILQ_FIRST(&with_group->m_runtime_groups));
    }
    return -1;
}

static void ui_sprite_chipmunk_with_group_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_group_t with_group = (ui_sprite_chipmunk_with_group_t)ui_sprite_fsm_action_data(fsm_action);

    while(!TAILQ_EMPTY(&with_group->m_runtime_groups)) {
        ui_sprite_chipmunk_obj_runtime_group_free(
            with_group->m_module, &with_group->m_runtime_groups, TAILQ_FIRST(&with_group->m_runtime_groups));
    }
}

static int ui_sprite_chipmunk_with_group_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_group_t with_group = (ui_sprite_chipmunk_with_group_t)ui_sprite_fsm_action_data(fsm_action);
    with_group->m_module = (ui_sprite_chipmunk_module_t)ctx;
    with_group->m_group = NULL;
    TAILQ_INIT(&with_group->m_runtime_groups);
    return 0;
}

static void ui_sprite_chipmunk_with_group_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_group_t with_group = (ui_sprite_chipmunk_with_group_t)ui_sprite_fsm_action_data(fsm_action);

    assert(TAILQ_EMPTY(&with_group->m_runtime_groups));
    
    if (with_group->m_group) {
        mem_free(with_group->m_module->m_alloc, (void*)with_group->m_group);
        with_group->m_group = NULL;
    }
}

static int ui_sprite_chipmunk_with_group_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_with_group_t to_with_group = (ui_sprite_chipmunk_with_group_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_group_t from_with_group = (ui_sprite_chipmunk_with_group_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_with_group_init(to, ctx)) return -1;

    if (from_with_group->m_group) {
        to_with_group->m_group = cpe_str_mem_dup(to_with_group->m_module->m_alloc, from_with_group->m_group);
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_group_load(
    void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg)
{
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_with_group_t with_group = (ui_sprite_chipmunk_with_group_t)ui_sprite_chipmunk_with_group_create(fsm_state, name);
    const char * str_value;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_group action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (with_group == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_group action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "group", NULL))) {
        if (with_group->m_group) {
            mem_free(with_group->m_module->m_alloc, (void*)with_group->m_group);
        }

        with_group->m_group = cpe_str_mem_dup(with_group->m_module->m_alloc, str_value);
        if (with_group->m_group == NULL) {
            CPE_ERROR(module->m_em, "%s: create with_group action: category %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(with_group);
}

    
int ui_sprite_chipmunk_with_group_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_GROUP_NAME, sizeof(struct ui_sprite_chipmunk_with_group));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with group register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_group_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_group_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_group_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_group_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_group_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_GROUP_NAME, ui_sprite_chipmunk_with_group_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_group_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_GROUP_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_GROUP_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_GROUP_NAME = "chipmunk-with-group";


#ifdef __cplusplus
}
#endif
    
