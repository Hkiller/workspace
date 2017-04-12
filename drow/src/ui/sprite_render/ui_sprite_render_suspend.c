#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_render_suspend_i.h"
#include "ui_sprite_render_utils_i.h"

ui_sprite_render_suspend_t ui_sprite_render_suspend_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_RENDER_SUSPEND_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_render_suspend_free(ui_sprite_render_suspend_t suspend) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(suspend);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_render_suspend_set_anim_name(ui_sprite_render_suspend_t suspend, const char * anim_name) {
    assert(anim_name);

    if (suspend->m_cfg_anim_name) {
        mem_free(suspend->m_module->m_alloc, suspend->m_cfg_anim_name);
    }

    suspend->m_cfg_anim_name = cpe_str_mem_dup(suspend->m_module->m_alloc, anim_name);
    
    return 0;
}

static int ui_sprite_render_suspend_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_suspend_t suspend = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_runtime_render_obj_t render_obj;

    if (suspend->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_suspend: render not configured!");
        return -1;
    }

    render_obj = ui_sprite_render_find_render_obj(module, entity, suspend->m_cfg_anim_name);
    if (render_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_render_suspend: render obj %s not exist!",
            suspend->m_cfg_anim_name);
        return -1;
    }

    ui_runtime_render_obj_set_suspend(render_obj, 1);

    return 0;
}

static void ui_sprite_render_suspend_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_suspend_t suspend = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_runtime_render_obj_t render_obj;

    if (suspend->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_render_suspend: render not configured!");
        return;
    }

    render_obj = ui_sprite_render_find_render_obj(module, entity, suspend->m_cfg_anim_name);
    if (render_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_render_suspend: render obj %s not exist!",
            suspend->m_cfg_anim_name);
        return;
    }

    ui_runtime_render_obj_set_suspend(render_obj, 0);
}

static int ui_sprite_render_suspend_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_suspend_t suspend = ui_sprite_fsm_action_data(fsm_action);
	bzero(suspend, sizeof(*suspend));
    suspend->m_module = ctx;
    suspend->m_cfg_anim_name = NULL;
    return 0;
}

static void ui_sprite_render_suspend_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_suspend_t suspend = ui_sprite_fsm_action_data(fsm_action);

    if (suspend->m_cfg_anim_name) {
        mem_free(module->m_alloc, suspend->m_cfg_anim_name);
        suspend->m_cfg_anim_name = NULL;
    }
}

static int ui_sprite_render_suspend_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_suspend_t to_suspend = ui_sprite_fsm_action_data(to);
    ui_sprite_render_suspend_t from_suspend = ui_sprite_fsm_action_data(from);

    if (ui_sprite_render_suspend_init(to, ctx)) return -1;

    if (from_suspend->m_cfg_anim_name) {
        to_suspend->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_suspend->m_cfg_anim_name);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_render_suspend_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_render_module_t module = ctx;
    ui_sprite_render_suspend_t render_suspend = ui_sprite_render_suspend_create(fsm_state, name);
    const char * str_value;
    
    if (render_suspend == NULL) {
        CPE_ERROR(module->m_em, "%s: create render suspend action: create fail!", ui_sprite_render_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_render_suspend_set_anim_name(render_suspend, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render suspend action: set anim name %s fail!",
                ui_sprite_render_module_name(module), str_value);
            ui_sprite_render_suspend_free(render_suspend);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create render suspend: anim-name not configured!",
            ui_sprite_render_module_name(module));
        ui_sprite_render_suspend_free(render_suspend);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(render_suspend);
}

int ui_sprite_render_suspend_regist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_RENDER_SUSPEND_NAME, sizeof(struct ui_sprite_render_suspend));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_render_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_render_suspend_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_render_suspend_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_render_suspend_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_render_suspend_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_render_suspend_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_RENDER_SUSPEND_NAME, ui_sprite_render_suspend_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_render_suspend_unregist(ui_sprite_render_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_RENDER_SUSPEND_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_render_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_RENDER_SUSPEND_NAME = "render-suspend";

