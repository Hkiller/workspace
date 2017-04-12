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
#include "ui_sprite_particle_enable_i.h"
#include "ui_sprite_particle_utils_i.h"

ui_sprite_particle_enable_t ui_sprite_particle_enable_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_PARTICLE_ENABLE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_particle_enable_free(ui_sprite_particle_enable_t enable) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(enable);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_particle_enable_set_anim_name(ui_sprite_particle_enable_t enable, const char * anim_name) {
    assert(anim_name);

    if (enable->m_cfg_anim_name) {
        mem_free(enable->m_module->m_alloc, enable->m_cfg_anim_name);
    }

    enable->m_cfg_anim_name = cpe_str_mem_dup(enable->m_module->m_alloc, anim_name);
    
    return 0;
}

static int ui_sprite_particle_enable_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_enable_t enable = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_particle_obj_t particle_obj;

    if (enable->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_enable: render not configured!");
        return -1;
    }

    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, enable->m_cfg_anim_name, module->m_em);
    if (particle_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_particle_enable: particle obj %s not exist!",
            enable->m_cfg_anim_name);
        return -1;
    }

    return 0;
}

static void ui_sprite_particle_enable_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_enable_t enable = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_particle_obj_t particle_obj;

    if (enable->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_enable: render not configured!");
        return;
    }

    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, enable->m_cfg_anim_name, module->m_em);
    if (particle_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_particle_enable: particle obj %s not exist!",
            enable->m_cfg_anim_name);
        return;
    }
}

static int ui_sprite_particle_enable_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_enable_t enable = ui_sprite_fsm_action_data(fsm_action);
	bzero(enable, sizeof(*enable));
    enable->m_module = ctx;
    enable->m_cfg_anim_name = NULL;
    return 0;
}

static void ui_sprite_particle_enable_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_enable_t enable = ui_sprite_fsm_action_data(fsm_action);

    if (enable->m_cfg_anim_name) {
        mem_free(module->m_alloc, enable->m_cfg_anim_name);
        enable->m_cfg_anim_name = NULL;
    }
}

static int ui_sprite_particle_enable_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_enable_t to_enable = ui_sprite_fsm_action_data(to);
    ui_sprite_particle_enable_t from_enable = ui_sprite_fsm_action_data(from);

    if (ui_sprite_particle_enable_init(to, ctx)) return -1;

    if (from_enable->m_cfg_anim_name) {
        to_enable->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_enable->m_cfg_anim_name);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_particle_enable_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_enable_t particle_enable = ui_sprite_particle_enable_create(fsm_state, name);
    const char * str_value;
    
    if (particle_enable == NULL) {
        CPE_ERROR(module->m_em, "%s: create render enable action: create fail!", ui_sprite_particle_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_particle_enable_set_anim_name(particle_enable, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render enable action: set anim name %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_enable_free(particle_enable);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle-enable: anim-name not configured!",
            ui_sprite_particle_module_name(module));
        ui_sprite_particle_enable_free(particle_enable);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(particle_enable);
}

int ui_sprite_particle_enable_regist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_PARTICLE_ENABLE_NAME, sizeof(struct ui_sprite_particle_enable));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-enable register: meta create fail",
            ui_sprite_particle_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_particle_enable_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_particle_enable_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_particle_enable_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_particle_enable_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_particle_enable_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_PARTICLE_ENABLE_NAME, ui_sprite_particle_enable_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_enable_unregist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_PARTICLE_ENABLE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-enable unregister: meta not exist",
            ui_sprite_particle_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_PARTICLE_ENABLE_NAME = "particle-enable";

