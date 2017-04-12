#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui_sprite_render_action_obj_bind_value_i.h"
#include "ui_sprite_render_utils_i.h"

static void ui_sprite_render_action_obj_bind_value_setup(void * ctx);

ui_sprite_render_action_obj_bind_value_t ui_sprite_render_action_obj_bind_value_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_RENDER_ACTION_OBJ_BIND_VALUE);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_action_obj_bind_value_free(ui_sprite_render_action_obj_bind_value_t bind_value) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(bind_value);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_render_chvalue_action_obj_bind_value_set_anim_name(ui_sprite_render_action_obj_bind_value_t bind_value, const char * anim_name) {
    assert(anim_name);

    if (bind_value->m_cfg_anim_name) {
        mem_free(bind_value->m_module->m_alloc, bind_value->m_cfg_anim_name);
        bind_value->m_cfg_anim_name = NULL;
    }

    bind_value->m_cfg_anim_name = cpe_str_mem_dup(bind_value->m_module->m_alloc, anim_name);
    
    return 0;
}

int ui_sprite_render_chvalue_action_obj_bind_value_set_setup(ui_sprite_render_action_obj_bind_value_t bind_value, const char * setup) {
    assert(setup);

    if (bind_value->m_cfg_setup) {
        mem_free(bind_value->m_module->m_alloc, bind_value->m_cfg_setup);
        bind_value->m_cfg_setup = NULL;
    }

    bind_value->m_cfg_setup = cpe_str_mem_dup(bind_value->m_module->m_alloc, setup);
    
    return 0;
}

static int ui_sprite_render_action_obj_bind_value_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_obj_bind_value_t bind_value = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (bind_value->m_cfg_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render-obj-bind-value: anim name not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (bind_value->m_cfg_setup == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render-obj-bind-value: setup not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (bind_value->m_cfg_setup[0] == ':') {
        if (ui_sprite_fsm_action_add_attr_monitor_by_def(
                fsm_action, bind_value->m_cfg_setup + 1, ui_sprite_render_action_obj_bind_value_setup, bind_value)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): render-obj-bind-value: bind monitor for setup %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_value->m_cfg_setup);
            return -1;
        }
    }
    
    ui_sprite_render_action_obj_bind_value_setup(bind_value);
    
    return 0;
}

static void ui_sprite_render_action_obj_bind_value_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_render_action_obj_bind_value_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_action_obj_bind_value_t bind_value = ui_sprite_fsm_action_data(fsm_action);

    bind_value->m_module = ctx;
    bind_value->m_cfg_anim_name = NULL;
    bind_value->m_cfg_setup = NULL;
    
    return 0;
}

static void ui_sprite_render_action_obj_bind_value_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_obj_bind_value_t bind_value = ui_sprite_fsm_action_data(fsm_action);

    if (bind_value->m_cfg_anim_name) {
        mem_free(module->m_alloc, bind_value->m_cfg_anim_name);
        bind_value->m_cfg_anim_name = NULL;
    }

    if (bind_value->m_cfg_setup) {
        mem_free(module->m_alloc, bind_value->m_cfg_setup);
        bind_value->m_cfg_setup = NULL;
    }
}

static void ui_sprite_render_action_obj_bind_value_setup(void * ctx) {
    ui_sprite_render_action_obj_bind_value_t bind_value = ctx;
    ui_sprite_render_module_t module = bind_value->m_module;
    ui_sprite_fsm_action_t action = ui_sprite_fsm_action_from_data(bind_value);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(action);
    ui_runtime_render_obj_ref_t render_obj_ref;
    char * setup;

    render_obj_ref = ui_sprite_render_find_entity_render_obj(module, entity, bind_value->m_cfg_anim_name);
    if (render_obj_ref == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render-obj-bind-value: find anim %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_value->m_cfg_anim_name);
        return;
    }

    setup = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, bind_value->m_cfg_setup, action, NULL, module->m_em);
    if (setup == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): render-obj-bind-value: calc setup from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_value->m_cfg_setup);
        return;
    }

    ui_runtime_render_obj_ref_setup(render_obj_ref, setup);

    mem_free(module->m_alloc, setup);
}

static int ui_sprite_render_action_obj_bind_value_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_obj_bind_value_t to_bind_value = ui_sprite_fsm_action_data(to);
    ui_sprite_render_action_obj_bind_value_t from_bind_value = ui_sprite_fsm_action_data(from);

    if (ui_sprite_render_action_obj_bind_value_init(to, ctx)) return -1;

    if (from_bind_value->m_cfg_anim_name) {
        to_bind_value->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_bind_value->m_cfg_anim_name);
    }

    if (from_bind_value->m_cfg_setup) {
        to_bind_value->m_cfg_setup = cpe_str_mem_dup(module->m_alloc, from_bind_value->m_cfg_setup);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_render_action_obj_bind_value_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_action_obj_bind_value_t bind_value = ui_sprite_render_action_obj_bind_value_create(fsm_state, name);
    const char * str_value;
    
    if (bind_value == NULL) {
        CPE_ERROR(module->m_em, "%s: create render-obj-bind-value: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_render_chvalue_action_obj_bind_value_set_anim_name(bind_value, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create render-obj-bind-value: set anim-name fail!", ui_sprite_render_module_name(module));
            ui_sprite_render_action_obj_bind_value_free(bind_value);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create render-obj-bind-value: anim-name not configured!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "setup", NULL))) {
        if (ui_sprite_render_chvalue_action_obj_bind_value_set_setup(bind_value, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create render-obj-bind-value: set setup fail!", ui_sprite_render_module_name(module));
            ui_sprite_render_action_obj_bind_value_free(bind_value);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create render-obj-bind-value: setup not configured!", ui_sprite_render_module_name(module));
        return NULL;
    }
    
    return ui_sprite_fsm_action_from_data(bind_value);
}

int ui_sprite_render_action_obj_bind_value_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_RENDER_ACTION_OBJ_BIND_VALUE, sizeof(struct ui_sprite_render_action_obj_bind_value));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_render_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_action_obj_bind_value_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_action_obj_bind_value_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_action_obj_bind_value_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_action_obj_bind_value_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_action_obj_bind_value_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_RENDER_ACTION_OBJ_BIND_VALUE, ui_sprite_render_action_obj_bind_value_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_render_action_obj_bind_value_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_RENDER_ACTION_OBJ_BIND_VALUE);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_render_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_RENDER_ACTION_OBJ_BIND_VALUE = "render-obj-bind-value";

