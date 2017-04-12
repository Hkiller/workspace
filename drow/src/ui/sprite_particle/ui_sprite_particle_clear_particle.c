#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_particle_clear_particle_i.h"
#include "ui_sprite_particle_utils_i.h"

ui_sprite_particle_clear_particle_t ui_sprite_particle_clear_particle_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_PARTICLE_CLEAR_PARTICLE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_particle_clear_particle_free(ui_sprite_particle_clear_particle_t clear_particle) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(clear_particle);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_particle_clear_particle_set_anim_name(ui_sprite_particle_clear_particle_t clear_particle, const char * anim_name) {
    assert(anim_name);

    if (clear_particle->m_cfg_anim_name) {
        mem_free(clear_particle->m_module->m_alloc, clear_particle->m_cfg_anim_name);
    }

    clear_particle->m_cfg_anim_name = cpe_str_mem_dup_trim(clear_particle->m_module->m_alloc, anim_name);
    
    return 0;
}

int ui_sprite_particle_clear_particle_set_prefix(ui_sprite_particle_clear_particle_t clear_particle, const char * prefix) {
    assert(prefix);

    if (clear_particle->m_cfg_prefix) {
        mem_free(clear_particle->m_module->m_alloc, clear_particle->m_cfg_prefix);
    }

    clear_particle->m_cfg_prefix = cpe_str_mem_dup_trim(clear_particle->m_module->m_alloc, prefix);
    
    return 0;
}

int ui_sprite_particle_clear_particle_set_show_dead_anim(ui_sprite_particle_clear_particle_t clear_particle, const char * show_dead_anim) {
    assert(show_dead_anim);

    if (clear_particle->m_cfg_show_dead_anim) {
        mem_free(clear_particle->m_module->m_alloc, clear_particle->m_cfg_show_dead_anim);
    }

    clear_particle->m_cfg_show_dead_anim = cpe_str_mem_dup_trim(clear_particle->m_module->m_alloc, show_dead_anim);
    
    return 0;
}

static int ui_sprite_particle_clear_particle_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_clear_particle_t clear_particle = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_particle_obj_t particle_obj;
    plugin_particle_obj_emitter_t particle_emitter;
    struct plugin_particle_obj_emitter_it emitter_it;
    const char * str_value;
    mem_buffer_t tmp_buffer = gd_app_tmp_buffer(module->m_app);
    uint8_t show_dead_anim = 1;
    
    if (clear_particle->m_cfg_anim_name == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_clear_particle: render not configured!");
        return -1;
    }

    str_value = ui_sprite_fsm_action_check_calc_str(tmp_buffer, clear_particle->m_cfg_anim_name, fsm_action, NULL, module->m_em);
    if (str_value == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_clear_particle: calc anim name from %s fail!", clear_particle->m_cfg_anim_name);
        return -1;
    }
    
    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, str_value, module->m_em);
    if (particle_obj == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_clear_particle: particle obj %s not exist!", str_value);
        return -1;
    }

    if (clear_particle->m_cfg_show_dead_anim) {
        if (ui_sprite_fsm_action_check_calc_bool(
                &show_dead_anim, clear_particle->m_cfg_show_dead_anim, fsm_action, NULL, module->m_em) != 0)
        {
            CPE_ERROR(
                module->m_em, "ui_sprite_particle_clear_particle: calc show dead anim from %s fail!",
                clear_particle->m_cfg_show_dead_anim);
            return -1;
        }
    }
    
    str_value = NULL;
    if (clear_particle->m_cfg_prefix) {
        str_value = ui_sprite_fsm_action_check_calc_str(tmp_buffer, clear_particle->m_cfg_prefix, fsm_action, NULL, module->m_em);
        if (str_value == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_particle_clear_particle: calc prefix from %s fail!", clear_particle->m_cfg_prefix);
            return -1;
        }
    }
    
    plugin_particle_obj_emitters(&emitter_it, particle_obj);
    while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
        if (str_value && !cpe_str_start_with(plugin_particle_obj_emitter_name(particle_emitter), str_value)) continue;

        if (plugin_particle_obj_emitter_use_state(particle_emitter) == plugin_particle_obj_emitter_use_state_active) {
            plugin_particle_obj_emitter_clear_particles(particle_emitter, show_dead_anim);
        }
    }

    return 0;
}

static void ui_sprite_particle_clear_particle_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_particle_clear_particle_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_clear_particle_t clear_particle = ui_sprite_fsm_action_data(fsm_action);
	bzero(clear_particle, sizeof(*clear_particle));
    clear_particle->m_module = ctx;
    clear_particle->m_cfg_anim_name = NULL;
    clear_particle->m_cfg_prefix = NULL;
    clear_particle->m_cfg_show_dead_anim = NULL;
    return 0;
}

static void ui_sprite_particle_clear_particle_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_clear_particle_t clear_particle = ui_sprite_fsm_action_data(fsm_action);

    if (clear_particle->m_cfg_anim_name) {
        mem_free(module->m_alloc, clear_particle->m_cfg_anim_name);
        clear_particle->m_cfg_anim_name = NULL;
    }

    if (clear_particle->m_cfg_prefix) {
        mem_free(module->m_alloc, clear_particle->m_cfg_prefix);
        clear_particle->m_cfg_prefix = NULL;
    }

    if (clear_particle->m_cfg_show_dead_anim) {
        mem_free(module->m_alloc, clear_particle->m_cfg_show_dead_anim);
        clear_particle->m_cfg_show_dead_anim = NULL;
    }
}

static int ui_sprite_particle_clear_particle_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_clear_particle_t to_clear_particle = ui_sprite_fsm_action_data(to);
    ui_sprite_particle_clear_particle_t from_clear_particle = ui_sprite_fsm_action_data(from);

    if (ui_sprite_particle_clear_particle_init(to, ctx)) return -1;

    if (from_clear_particle->m_cfg_anim_name) {
        to_clear_particle->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_clear_particle->m_cfg_anim_name);
    }
    
    if (from_clear_particle->m_cfg_prefix) {
        to_clear_particle->m_cfg_prefix = cpe_str_mem_dup(module->m_alloc, from_clear_particle->m_cfg_prefix);
    }
    
    if (from_clear_particle->m_cfg_show_dead_anim) {
        to_clear_particle->m_cfg_show_dead_anim = cpe_str_mem_dup(module->m_alloc, from_clear_particle->m_cfg_show_dead_anim);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_particle_clear_particle_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_clear_particle_t particle_clear_particle = ui_sprite_particle_clear_particle_create(fsm_state, name);
    const char * str_value;
    
    if (particle_clear_particle == NULL) {
        CPE_ERROR(module->m_em, "%s: create render clear_particle action: create fail!", ui_sprite_particle_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_particle_clear_particle_set_anim_name(particle_clear_particle, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render clear_particle action: set anim name %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_clear_particle_free(particle_clear_particle);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle-clear_particle: anim-name not configured!",
            ui_sprite_particle_module_name(module));
        ui_sprite_particle_clear_particle_free(particle_clear_particle);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_particle_clear_particle_set_prefix(particle_clear_particle, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render clear_particle action: set prefix %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_clear_particle_free(particle_clear_particle);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "show-dead-anim", NULL))) {
        if (ui_sprite_particle_clear_particle_set_show_dead_anim(particle_clear_particle, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render clear_particle action: set show-dead-anim %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_clear_particle_free(particle_clear_particle);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(particle_clear_particle);
}

int ui_sprite_particle_clear_particle_regist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_PARTICLE_CLEAR_PARTICLE_NAME, sizeof(struct ui_sprite_particle_clear_particle));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-clear_particle register: meta create fail",
            ui_sprite_particle_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_particle_clear_particle_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_particle_clear_particle_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_particle_clear_particle_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_particle_clear_particle_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_particle_clear_particle_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_PARTICLE_CLEAR_PARTICLE_NAME, ui_sprite_particle_clear_particle_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_clear_particle_unregist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_PARTICLE_CLEAR_PARTICLE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-clear_particle unregister: meta not exist",
            ui_sprite_particle_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_PARTICLE_CLEAR_PARTICLE_NAME = "particle-clear-particle";

