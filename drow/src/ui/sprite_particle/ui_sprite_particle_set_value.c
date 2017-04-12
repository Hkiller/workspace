#include <assert.h>
#include "gd/app/app_log.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/pal/pal_strings.h"
#include "plugin/particle/plugin_particle_types.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_particle_set_value_i.h"

ui_sprite_particle_set_value_t ui_sprite_particle_set_value_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_PARTICLE_SET_VALUE_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_particle_set_value_free(ui_sprite_particle_set_value_t set_value) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(set_value);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_particle_set_value_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_set_value_t set_value = ui_sprite_fsm_action_data(fsm_action);
    //ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);

    if (set_value->m_particle == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_set_value: particle not configured!");
        return -1;
    }
    
    if (set_value->m_value == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_particle_set_value: value not configured!");
        return -1;
    }

    return 0;
}

static void ui_sprite_particle_set_value_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
}

static int ui_sprite_particle_set_value_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_set_value_t set_value = ui_sprite_fsm_action_data(fsm_action);
	bzero(set_value, sizeof(*set_value));
    set_value->m_module = ctx;
    set_value->m_particle = NULL;
    set_value->m_value = NULL;
    return 0;
}

static void ui_sprite_particle_set_value_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_set_value_t set_value = ui_sprite_fsm_action_data(fsm_action);

    if (set_value->m_particle) {
        mem_free(module->m_alloc, set_value->m_particle);
        set_value->m_particle = NULL;
    }

    if (set_value->m_value) {
        mem_free(module->m_alloc, set_value->m_value);
        set_value->m_value = NULL;
    }
}

static int ui_sprite_particle_set_value_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_set_value_t to_set_value = ui_sprite_fsm_action_data(to);
    ui_sprite_particle_set_value_t from_set_value = ui_sprite_fsm_action_data(from);

    if (ui_sprite_particle_set_value_init(to, ctx)) return -1;

    if (from_set_value->m_particle) {
        to_set_value->m_particle = cpe_str_mem_dup(module->m_alloc, from_set_value->m_particle);
    }

    if (from_set_value->m_value) {
        to_set_value->m_value = cpe_str_mem_dup(module->m_alloc, from_set_value->m_value);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_particle_set_value_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_set_value_t particle_set_value = ui_sprite_particle_set_value_create(fsm_state, name);

    if (particle_set_value == NULL) {
        CPE_ERROR(module->m_em, "%s: create particle_set_value action: create fail!", ui_sprite_particle_module_name(module));
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(particle_set_value);
}

int ui_sprite_particle_set_value_regist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_PARTICLE_SET_VALUE_NAME, sizeof(struct ui_sprite_particle_set_value));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: anim send event register: meta create fail",
            ui_sprite_particle_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_particle_set_value_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_particle_set_value_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_particle_set_value_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_particle_set_value_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_particle_set_value_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_PARTICLE_SET_VALUE_NAME, ui_sprite_particle_set_value_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_set_value_unregist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_PARTICLE_SET_VALUE_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_particle_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_PARTICLE_SET_VALUE_NAME = "particle-set-value";

