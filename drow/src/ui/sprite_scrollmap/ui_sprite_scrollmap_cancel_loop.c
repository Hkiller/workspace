#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "plugin/scrollmap/plugin_scrollmap_env.h"
#include "plugin/scrollmap/plugin_scrollmap_layer.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_scrollmap_cancel_loop_i.h"
#include "ui_sprite_scrollmap_env_i.h"

ui_sprite_scrollmap_cancel_loop_t ui_sprite_scrollmap_cancel_loop_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SCROLLMAP_CANCEL_LOOP_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_scrollmap_cancel_loop_free(ui_sprite_scrollmap_cancel_loop_t cancel_loop) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(cancel_loop);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_scrollmap_cancel_loop_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_cancel_loop_t cancel_loop = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_world_t world = ui_sprite_entity_world(entity);
    ui_sprite_scrollmap_env_t scene_env;
    struct plugin_scrollmap_layer_it layer_it;
    plugin_scrollmap_layer_t scene_layer;

    scene_env = ui_sprite_scrollmap_env_find(world);
    if (scene_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): cancel loop: no scene env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    plugin_scrollmap_env_layers(&layer_it, scene_env->m_env);
    while((scene_layer = plugin_scrollmap_layer_it_next(&layer_it))) {
        if (cpe_str_start_with(plugin_scrollmap_layer_name(scene_layer), cancel_loop->m_layer_prefix)) {
            plugin_scrollmap_layer_cancel_loop(scene_layer);
        }
    }

    return 0;
}

static void ui_sprite_scrollmap_cancel_loop_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_scrollmap_cancel_loop_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_scrollmap_cancel_loop_t cancel_loop = ui_sprite_fsm_action_data(fsm_action);
    cancel_loop->m_module = ctx;
	cancel_loop->m_layer_prefix[0] = 0;
    return 0;
}

static void ui_sprite_scrollmap_cancel_loop_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_scrollmap_cancel_loop_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_scrollmap_cancel_loop_t to_cancel_loop = ui_sprite_fsm_action_data(to);
    ui_sprite_scrollmap_cancel_loop_t from_cancel_loop = ui_sprite_fsm_action_data(from);

    if (ui_sprite_scrollmap_cancel_loop_init(to, ctx)) return -1;

    cpe_str_dup(to_cancel_loop->m_layer_prefix, sizeof(to_cancel_loop->m_layer_prefix), from_cancel_loop->m_layer_prefix);

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_scrollmap_cancel_loop_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_scrollmap_module_t module = ctx;
    ui_sprite_scrollmap_cancel_loop_t cancel_loop = ui_sprite_scrollmap_cancel_loop_create(fsm_state, name);
    const char * layer_prefix;

    if (cancel_loop == NULL) {
        CPE_ERROR(module->m_em, "%s: create cancel_loop action: create fail!", ui_sprite_scrollmap_module_name(module));
        return NULL;
    }

    layer_prefix = cfg_get_string(cfg, "layer-prefix", NULL);
    if (layer_prefix == NULL) {
        CPE_ERROR(module->m_em, "%s: create cancel_loop action: layer-prefix not configured!", ui_sprite_scrollmap_module_name(module));
        return NULL;
    }
    cpe_str_dup(cancel_loop->m_layer_prefix, sizeof(cancel_loop->m_layer_prefix), layer_prefix);
 
    return ui_sprite_fsm_action_from_data(cancel_loop);
}

int ui_sprite_scrollmap_cancel_loop_regist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SCROLLMAP_CANCEL_LOOP_NAME, sizeof(struct ui_sprite_scrollmap_cancel_loop));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: moving enable emitter register: meta create fail",
            ui_sprite_scrollmap_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_scrollmap_cancel_loop_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_scrollmap_cancel_loop_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_scrollmap_cancel_loop_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_scrollmap_cancel_loop_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_scrollmap_cancel_loop_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SCROLLMAP_CANCEL_LOOP_NAME, ui_sprite_scrollmap_cancel_loop_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_scrollmap_cancel_loop_unregist(ui_sprite_scrollmap_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SCROLLMAP_CANCEL_LOOP_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_scrollmap_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SCROLLMAP_CANCEL_LOOP_NAME);
}

const char * UI_SPRITE_SCROLLMAP_CANCEL_LOOP_NAME = "scrollmap-cancel-loop";
