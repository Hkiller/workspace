#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/scrollmap/plugin_scrollmap_env.h"
#include "plugin/scrollmap/plugin_scrollmap_layer.h"
#include "plugin/scrollmap/plugin_scrollmap_team.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_scrollmap_gen_team_i.h"
#include "ui_sprite_scrollmap_env_i.h"

ui_sprite_scrollmap_gen_team_t ui_sprite_scrollmap_gen_team_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SCROLLMAP_GEN_TEAM_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_scrollmap_gen_team_free(ui_sprite_scrollmap_gen_team_t gen_team) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(gen_team);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_scrollmap_gen_team_set_res(ui_sprite_scrollmap_gen_team_t gen_team, const char * res) {
    if (gen_team->m_cfg_res) {
        mem_free(gen_team->m_module->m_alloc, gen_team->m_cfg_res);
    }

    if (res) {
        gen_team->m_cfg_res = cpe_str_mem_dup_trim(gen_team->m_module->m_alloc, res);
    }
    else {
        gen_team->m_cfg_res = NULL;
    }
    
    return 0;
}

const char * ui_sprite_scrollmap_gen_team_res(ui_sprite_scrollmap_gen_team_t gen_team) {
    return gen_team->m_cfg_res;
}

int ui_sprite_scrollmap_gen_team_set_layer(ui_sprite_scrollmap_gen_team_t gen_team, const char * layer) {
    if (gen_team->m_cfg_layer) {
        mem_free(gen_team->m_module->m_alloc, gen_team->m_cfg_layer);
    }

    if (layer) {
        gen_team->m_cfg_layer = cpe_str_mem_dup_trim(gen_team->m_module->m_alloc, layer);
    }
    else {
        gen_team->m_cfg_layer = NULL;
    }
    
    return 0;
}

const char * ui_sprite_scrollmap_gen_team_layer(ui_sprite_scrollmap_gen_team_t gen_team) {
    return gen_team->m_cfg_layer;
}

static int ui_sprite_scrollmap_gen_team_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_scrollmap_gen_team_t gen_team = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_scrollmap_env_t scene_env;
    plugin_scrollmap_layer_t layer;
    plugin_scrollmap_team_t team;
    char * res = NULL;
    
    assert(gen_team->m_team_id == 0);
    
    scene_env = ui_sprite_scrollmap_env_find(world);
    if (scene_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen_team: no scene env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (gen_team->m_cfg_res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen_team: team res not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (gen_team->m_cfg_layer == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen_team: layer not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }
    
    gen_team->m_layer = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, gen_team->m_cfg_layer, fsm_action, NULL, module->m_em);
    if (gen_team->m_layer == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen_team: calc layer from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), gen_team->m_cfg_layer);
        goto ENTER_FAIL;
    }

    layer = plugin_scrollmap_layer_find(scene_env->m_env, gen_team->m_layer);
    if (layer == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen_team: layer %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), gen_team->m_layer);
        goto ENTER_FAIL;
    }
    
    res = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, gen_team->m_cfg_res, fsm_action, NULL, module->m_em);
    if (res == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen_team: calc team res from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), gen_team->m_cfg_res);
        goto ENTER_FAIL;
    }

    team = plugin_scrollmap_team_create_from_res(scene_env->m_env, layer, res);
    if (team == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen_team: gen team from res %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
        goto ENTER_FAIL;
    }

    gen_team->m_team_id = plugin_scrollmap_team_id(team);
    assert(gen_team->m_team_id > 0);
    
    if (ui_sprite_fsm_action_life_circle(fsm_action) == ui_sprite_fsm_action_life_circle_working) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    return 0;

ENTER_FAIL:
    if (gen_team->m_layer) {
        mem_free(module->m_alloc, gen_team->m_layer);
        gen_team->m_layer = NULL;
    }

    if (res) {
        mem_free(module->m_alloc, res);
    }
    
    return -1;
}

static void ui_sprite_scrollmap_gen_team_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_scrollmap_gen_team_t gen_team = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_scrollmap_env_t scene_env;

    scene_env = ui_sprite_scrollmap_env_find(world);
    if (scene_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): gen_team: no scrollmap env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    assert(gen_team->m_team_id > 0);
    if (plugin_scrollmap_team_find_by_id(scene_env->m_env, gen_team->m_team_id) == NULL) {
        ui_sprite_fsm_action_stop_update(fsm_action);
    }
}

static void ui_sprite_scrollmap_gen_team_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_gen_team_t gen_team = ui_sprite_fsm_action_data(fsm_action);

    assert(gen_team->m_team_id);
    assert(gen_team->m_layer);

    mem_free(module->m_alloc, gen_team->m_layer);
    gen_team->m_layer = NULL;
    
    gen_team->m_team_id = 0;
}

static int ui_sprite_scrollmap_gen_team_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_gen_team_t gen_team = ui_sprite_fsm_action_data(fsm_action);
    gen_team->m_module = ctx;
    gen_team->m_cfg_res = NULL;
    gen_team->m_cfg_layer = NULL;
    gen_team->m_layer = NULL;
    gen_team->m_team_id = 0;
    return 0;
}

static void ui_sprite_scrollmap_gen_team_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_gen_team_t gen_team = ui_sprite_fsm_action_data(fsm_action);

    assert(gen_team->m_team_id == 0);
    assert(gen_team->m_layer == NULL);
    
    if (gen_team->m_cfg_res) {
        mem_free(module->m_alloc, gen_team->m_cfg_res);
        gen_team->m_cfg_res = NULL;
    }

    if (gen_team->m_cfg_layer) {
        mem_free(module->m_alloc, gen_team->m_cfg_layer);
        gen_team->m_cfg_layer = NULL;
    }
}

static int ui_sprite_scrollmap_gen_team_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_gen_team_t to_gen_team = ui_sprite_fsm_action_data(to);
    ui_sprite_scrollmap_gen_team_t from_gen_team = ui_sprite_fsm_action_data(from);
    
    if (ui_sprite_scrollmap_gen_team_init(to, ctx)) return -1;

    if (from_gen_team->m_cfg_res) {
        to_gen_team->m_cfg_res = cpe_str_mem_dup(module->m_alloc, from_gen_team->m_cfg_res);
    }

    if (from_gen_team->m_cfg_layer) {
        to_gen_team->m_cfg_layer = cpe_str_mem_dup(module->m_alloc, from_gen_team->m_cfg_layer);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_scrollmap_gen_team_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_gen_team_t gen_team = ui_sprite_scrollmap_gen_team_create(fsm_state, name);
    const char * str_value;
    
    if (gen_team == NULL) {
        CPE_ERROR(module->m_em, "%s: create gen_team action: create fail!", ui_sprite_scrollmap_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "res", NULL))) {
        if (ui_sprite_scrollmap_gen_team_set_res(gen_team, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create gen_team action: set res fail!", ui_sprite_scrollmap_module_name(module));
            ui_sprite_scrollmap_gen_team_free(gen_team);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create gen_team action: res not configured!", ui_sprite_scrollmap_module_name(module));
        ui_sprite_scrollmap_gen_team_free(gen_team);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "layer", NULL))) {
        if (ui_sprite_scrollmap_gen_team_set_layer(gen_team, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create gen_team action: set layer fail!", ui_sprite_scrollmap_module_name(module));
            ui_sprite_scrollmap_gen_team_free(gen_team);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create gen_team action: layer not configured!", ui_sprite_scrollmap_module_name(module));
        ui_sprite_scrollmap_gen_team_free(gen_team);
        return NULL;
    }
    
    return ui_sprite_fsm_action_from_data(gen_team);
}

int ui_sprite_scrollmap_gen_team_regist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SCROLLMAP_GEN_TEAM_NAME, sizeof(struct ui_sprite_scrollmap_gen_team));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: moving enable emitter register: meta create fail",
            ui_sprite_scrollmap_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_scrollmap_gen_team_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_scrollmap_gen_team_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_scrollmap_gen_team_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_scrollmap_gen_team_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_scrollmap_gen_team_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_scrollmap_gen_team_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SCROLLMAP_GEN_TEAM_NAME, ui_sprite_scrollmap_gen_team_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_scrollmap_gen_team_unregist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SCROLLMAP_GEN_TEAM_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_scrollmap_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SCROLLMAP_GEN_TEAM_NAME);
}

const char * UI_SPRITE_SCROLLMAP_GEN_TEAM_NAME = "scrollmap-gen-team";
