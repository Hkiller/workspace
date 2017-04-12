#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_cfg/ui_sprite_cfg_loader.h"
#include "ui_sprite_basic_join_group_i.h"

ui_sprite_basic_join_group_t ui_sprite_basic_join_group_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_BASIC_JOIN_GROUP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_basic_join_group_free(ui_sprite_basic_join_group_t join_group) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(join_group);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_basic_join_group_set_name(ui_sprite_basic_join_group_t join_group, const char * name) {
    
    ui_sprite_basic_module_t module = join_group->m_module;

    if (join_group->m_group_name) mem_free(module->m_alloc, join_group->m_group_name);

    if (name) {
        join_group->m_group_name = cpe_str_mem_dup(module->m_alloc, name);
        return join_group->m_group_name == NULL ? -1 : 0;
    }
    else {
        join_group->m_group_name = NULL;
        return 0;
    }
}

const char * ui_sprite_basic_join_group_name(ui_sprite_basic_join_group_t join_group) {
    return join_group->m_group_name;
}

static int ui_sprite_basic_join_group_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_join_group_t join_group = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_group_t group;

    if (join_group->m_group_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): join group: group name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    group = ui_sprite_group_find_by_name(ui_sprite_entity_world(entity), join_group->m_group_name);
    if (group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): join group %s: group not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), join_group->m_group_name);
        return -1;
    }

    if (ui_sprite_group_add_entity(group, entity) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): join group %d(%s): join fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_group_id(group), ui_sprite_group_name(group));
        return -1;
    }

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): join group %d(%s)",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_group_id(group), ui_sprite_group_name(group));
    }

    return 0;
}

static void ui_sprite_basic_join_group_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_join_group_t join_group = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_group_t group;

    if (join_group->m_group_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): exit group: group name not set!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return;
    }

    group = ui_sprite_group_find_by_name(ui_sprite_entity_world(entity), join_group->m_group_name);
    if (group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): exit group %s: group not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), join_group->m_group_name);
        return;
    }

    ui_sprite_group_remove_entity(group, entity);

    if (ui_sprite_entity_debug(entity)) {
        CPE_INFO(
            module->m_em, "entity %d(%s): exit group %d(%s)",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
            ui_sprite_group_id(group), ui_sprite_group_name(group));
    }
}

static int ui_sprite_basic_join_group_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_join_group_t join_group = ui_sprite_fsm_action_data(fsm_action);
    join_group->m_module = ctx;
	join_group->m_group_name = NULL;
    return 0;
}

static void ui_sprite_basic_join_group_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_join_group_t join_group = ui_sprite_fsm_action_data(fsm_action);

    if (join_group->m_group_name) {
        mem_free(module->m_alloc, join_group->m_group_name);
        join_group->m_group_name = NULL;
    }
}

static int ui_sprite_basic_join_group_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_join_group_t to_join_group = ui_sprite_fsm_action_data(to);
    ui_sprite_basic_join_group_t from_join_group = ui_sprite_fsm_action_data(from);

    if (ui_sprite_basic_join_group_init(to, ctx)) return -1;

    if (from_join_group->m_group_name) {
        to_join_group->m_group_name = cpe_str_mem_dup(module->m_alloc, from_join_group->m_group_name);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_basic_join_group_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_basic_module_t module = ctx;
    ui_sprite_basic_join_group_t join_group = ui_sprite_basic_join_group_create(fsm_state, name);
    const char * group_name;

    group_name = cfg_get_string(cfg, "group-name", NULL);
    if (group_name == NULL) {
        CPE_ERROR(
            module->m_em, "%s: create join_group action: group-name not configured",
            ui_sprite_basic_module_name(module));
        return NULL;
    }

    if (join_group == NULL) {
        CPE_ERROR(module->m_em, "%s: create join_group action: create fail!", ui_sprite_basic_module_name(module));
        return NULL;
    }

    if (ui_sprite_basic_join_group_set_name(join_group, group_name) != 0) {
        CPE_ERROR(
            module->m_em, "%s: create join_group action: set group-name %s fail",
            ui_sprite_basic_module_name(module), group_name);
        ui_sprite_basic_join_group_free(join_group);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(join_group);
}

int ui_sprite_basic_join_group_regist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_BASIC_JOIN_GROUP_NAME, sizeof(struct ui_sprite_basic_join_group));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_basic_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_basic_join_group_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_basic_join_group_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_basic_join_group_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_basic_join_group_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_basic_join_group_clear, module);

    if (module->m_loader) {
        if (ui_sprite_cfg_loader_add_action_loader(module->m_loader, UI_SPRITE_BASIC_JOIN_GROUP_NAME, ui_sprite_basic_join_group_load, module) != 0) {
            ui_sprite_fsm_action_meta_free(meta);
            return -1;
        }
    }

    return 0;
}

void ui_sprite_basic_join_group_unregist(ui_sprite_basic_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_BASIC_JOIN_GROUP_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_basic_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    if (module->m_loader) {
        ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_BASIC_JOIN_GROUP_NAME);
    }
}

const char * UI_SPRITE_BASIC_JOIN_GROUP_NAME = "join-group";

