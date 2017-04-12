#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_particle_disable_emitter_i.h"
#include "ui_sprite_particle_utils_i.h"

ui_sprite_particle_disable_emitter_t ui_sprite_particle_disable_emitter_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_PARTICLE_DISABLE_EMITTER_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_particle_disable_emitter_free(ui_sprite_particle_disable_emitter_t disable_emitter) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(disable_emitter);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_particle_disable_emitter_set_anim_name(ui_sprite_particle_disable_emitter_t disable_emitter, const char * anim_name) {
    assert(anim_name);

    if (disable_emitter->m_cfg_anim_name) {
        mem_free(disable_emitter->m_module->m_alloc, disable_emitter->m_cfg_anim_name);
    }

    disable_emitter->m_cfg_anim_name = cpe_str_mem_dup(disable_emitter->m_module->m_alloc, anim_name);
    
    return 0;
}

int ui_sprite_particle_disable_emitter_set_prefix(ui_sprite_particle_disable_emitter_t disable_emitter, const char * prefix) {
    assert(prefix);

    if (disable_emitter->m_cfg_prefix) {
        mem_free(disable_emitter->m_module->m_alloc, disable_emitter->m_cfg_prefix);
    }

    disable_emitter->m_cfg_prefix = cpe_str_mem_dup(disable_emitter->m_module->m_alloc, prefix);
    
    return 0;
}

static int ui_sprite_particle_disable_emitter_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_disable_emitter_t disable_emitter = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_particle_obj_t particle_obj;
    plugin_particle_obj_emitter_t particle_emitter;
    struct plugin_particle_obj_emitter_it emitter_it;

    if (disable_emitter->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_disable_emitter: render not configured!");
        return -1;
    }

    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, disable_emitter->m_cfg_anim_name, module->m_em);
    if (particle_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_particle_disable_emitter: particle obj %s not exist!",
            disable_emitter->m_cfg_anim_name);
        return -1;
    }

    plugin_particle_obj_emitters(&emitter_it, particle_obj);

    if (disable_emitter->m_cfg_prefix) {
        while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
            if (!cpe_str_start_with(plugin_particle_obj_emitter_name(particle_emitter), disable_emitter->m_cfg_prefix)) continue;
                                                                                                                            
            if (plugin_particle_obj_emitter_use_state(particle_emitter) == plugin_particle_obj_emitter_use_state_active) {
                if (disable_emitter->m_cfg_force) {
                    plugin_particle_obj_emitter_set_use_state(particle_emitter, plugin_particle_obj_emitter_use_state_suspend);
                }
                else {
                    plugin_particle_obj_emitter_set_close(particle_emitter, 1);
                }
            }
        }
    }
    else {
        while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
            if (plugin_particle_obj_emitter_use_state(particle_emitter) == plugin_particle_obj_emitter_use_state_active) {
                if (disable_emitter->m_cfg_force) {
                    plugin_particle_obj_emitter_set_use_state(particle_emitter, plugin_particle_obj_emitter_use_state_suspend);
                }
                else {
                    plugin_particle_obj_emitter_set_close(particle_emitter, 1);
                }
            }
        }
    }

    return 0;
}

static void ui_sprite_particle_disable_emitter_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_disable_emitter_t disable_emitter = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_particle_obj_t particle_obj;
    plugin_particle_obj_emitter_t particle_emitter;
    struct plugin_particle_obj_emitter_it emitter_it;

    if (!disable_emitter->m_cfg_resume) return;
    
    if (disable_emitter->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_disable_emitter: render not configured!");
        return;
    }

    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, disable_emitter->m_cfg_anim_name, module->m_em);
    if (particle_obj == NULL) {
        CPE_ERROR(
            module->m_em, "ui_sprite_particle_disable_emitter: particle obj %s not exist!",
            disable_emitter->m_cfg_anim_name);
        return;
    }

    plugin_particle_obj_emitters(&emitter_it, particle_obj);

    if (disable_emitter->m_cfg_prefix) {
        while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
            if (!cpe_str_start_with(plugin_particle_obj_emitter_name(particle_emitter), disable_emitter->m_cfg_prefix)) continue;

            if(plugin_particle_obj_emitter_use_state(particle_emitter) == plugin_particle_obj_emitter_use_state_suspend) {
                plugin_particle_obj_emitter_set_use_state(particle_emitter, plugin_particle_obj_emitter_use_state_active);
            }
            else if(plugin_particle_obj_emitter_use_state(particle_emitter) == plugin_particle_obj_emitter_use_state_active) {
                plugin_particle_obj_emitter_set_close(particle_emitter, 0);
            }
        }
    }
    else {
        while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
            if(plugin_particle_obj_emitter_use_state(particle_emitter) == plugin_particle_obj_emitter_use_state_suspend) {
                plugin_particle_obj_emitter_set_use_state(particle_emitter, plugin_particle_obj_emitter_use_state_active);
            }
            else if(plugin_particle_obj_emitter_use_state(particle_emitter) == plugin_particle_obj_emitter_use_state_active) {
                plugin_particle_obj_emitter_set_close(particle_emitter, 0);
            }
        }
    }
}

static int ui_sprite_particle_disable_emitter_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_disable_emitter_t disable_emitter = ui_sprite_fsm_action_data(fsm_action);
	bzero(disable_emitter, sizeof(*disable_emitter));
    disable_emitter->m_module = ctx;
    disable_emitter->m_cfg_anim_name = NULL;
    disable_emitter->m_cfg_prefix = NULL;
    disable_emitter->m_cfg_resume = 1;
    disable_emitter->m_cfg_force = 0;
    return 0;
}

static void ui_sprite_particle_disable_emitter_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_disable_emitter_t disable_emitter = ui_sprite_fsm_action_data(fsm_action);

    if (disable_emitter->m_cfg_anim_name) {
        mem_free(module->m_alloc, disable_emitter->m_cfg_anim_name);
        disable_emitter->m_cfg_anim_name = NULL;
    }

    if (disable_emitter->m_cfg_prefix) {
        mem_free(module->m_alloc, disable_emitter->m_cfg_prefix);
        disable_emitter->m_cfg_prefix = NULL;
    }
}

static int ui_sprite_particle_disable_emitter_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_disable_emitter_t to_disable_emitter = ui_sprite_fsm_action_data(to);
    ui_sprite_particle_disable_emitter_t from_disable_emitter = ui_sprite_fsm_action_data(from);

    if (ui_sprite_particle_disable_emitter_init(to, ctx)) return -1;

    to_disable_emitter->m_cfg_resume = from_disable_emitter->m_cfg_resume;
    
    if (from_disable_emitter->m_cfg_anim_name) {
        to_disable_emitter->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_disable_emitter->m_cfg_anim_name);
    }
    
    if (from_disable_emitter->m_cfg_prefix) {
        to_disable_emitter->m_cfg_prefix = cpe_str_mem_dup(module->m_alloc, from_disable_emitter->m_cfg_prefix);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_particle_disable_emitter_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_disable_emitter_t particle_disable_emitter = ui_sprite_particle_disable_emitter_create(fsm_state, name);
    const char * str_value;
    
    if (particle_disable_emitter == NULL) {
        CPE_ERROR(module->m_em, "%s: create render disable_emitter action: create fail!", ui_sprite_particle_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_particle_disable_emitter_set_anim_name(particle_disable_emitter, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render disable_emitter action: set anim name %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_disable_emitter_free(particle_disable_emitter);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle-disable_emitter: anim-name not configured!",
            ui_sprite_particle_module_name(module));
        ui_sprite_particle_disable_emitter_free(particle_disable_emitter);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_particle_disable_emitter_set_prefix(particle_disable_emitter, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render disable_emitter action: set prefix %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_disable_emitter_free(particle_disable_emitter);
            return NULL;
        }
    }

    particle_disable_emitter->m_cfg_resume = cfg_get_uint8(cfg, "resume", particle_disable_emitter->m_cfg_resume);
    particle_disable_emitter->m_cfg_force = cfg_get_uint8(cfg, "force", particle_disable_emitter->m_cfg_force);
    
    return ui_sprite_fsm_action_from_data(particle_disable_emitter);
}

int ui_sprite_particle_disable_emitter_regist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_PARTICLE_DISABLE_EMITTER_NAME, sizeof(struct ui_sprite_particle_disable_emitter));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-disable_emitter register: meta create fail",
            ui_sprite_particle_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_particle_disable_emitter_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_particle_disable_emitter_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_particle_disable_emitter_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_particle_disable_emitter_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_particle_disable_emitter_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_PARTICLE_DISABLE_EMITTER_NAME, ui_sprite_particle_disable_emitter_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_disable_emitter_unregist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_PARTICLE_DISABLE_EMITTER_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-disable_emitter unregister: meta not exist",
            ui_sprite_particle_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_PARTICLE_DISABLE_EMITTER_NAME = "particle-disable-emitter";

