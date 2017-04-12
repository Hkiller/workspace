#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/particle/plugin_particle_obj.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "plugin/particle/plugin_particle_obj_plugin.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_particle/ui_sprite_particle_utils.h"
#include "ui_sprite_particle_gen_entity_i.h"
#include "ui_sprite_particle_gen_entity_slot_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_particle_gen_entity_t ui_sprite_particle_gen_entity_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_PARTICLE_GEN_ENTITY_NAME);
    return (ui_sprite_particle_gen_entity_t)(fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL);
}

void ui_sprite_particle_gen_entity_free(ui_sprite_particle_gen_entity_t gen_entity) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(gen_entity);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_particle_gen_entity_set_anim_name(ui_sprite_particle_gen_entity_t gen_entity, const char * anim_name) {
    assert(anim_name);

    if (gen_entity->m_cfg_anim_name) {
        mem_free(gen_entity->m_module->m_alloc, gen_entity->m_cfg_anim_name);
    }

    gen_entity->m_cfg_anim_name = cpe_str_mem_dup_trim(gen_entity->m_module->m_alloc, anim_name);
    
    return 0;
}

int ui_sprite_particle_gen_entity_set_prefix(ui_sprite_particle_gen_entity_t gen_entity, const char * prefix) {
    assert(prefix);

    if (gen_entity->m_cfg_prefix) {
        mem_free(gen_entity->m_module->m_alloc, gen_entity->m_cfg_prefix);
    }

    gen_entity->m_cfg_prefix = cpe_str_mem_dup_trim(gen_entity->m_module->m_alloc, prefix);
    
    return 0;
}


static int ui_sprite_particle_gen_entity_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = (ui_sprite_particle_module_t)ctx;
    ui_sprite_particle_gen_entity_t gen_entity = (ui_sprite_particle_gen_entity_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    plugin_particle_obj_t particle_obj = NULL;
    plugin_particle_obj_emitter_t particle_emitter;
    struct plugin_particle_obj_emitter_it emitter_it;

    assert(gen_entity->m_anim_name == NULL);

    /*获取目标Particle */
    if (gen_entity->m_cfg_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle gen entity: anim-name not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    gen_entity->m_anim_name = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, gen_entity->m_cfg_anim_name, fsm_action, NULL, module->m_em);
    if (gen_entity->m_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle gen entity: calc anim name from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), gen_entity->m_cfg_anim_name);
        goto ENTER_FAIL;
    }
    
    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, gen_entity->m_anim_name, module->m_em);
    if (particle_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle gen entity: particle obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), gen_entity->m_anim_name);
        goto ENTER_FAIL;
    }

    /*给发射器添加碰撞 */
    plugin_particle_obj_emitters(&emitter_it, particle_obj);
    while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
        plugin_particle_obj_plugin_t plugin;
        const char * emitter_name = plugin_particle_obj_emitter_name(particle_emitter);
        
        if (gen_entity->m_cfg_prefix && !cpe_str_start_with(emitter_name, gen_entity->m_cfg_prefix)) continue;

        plugin =
            plugin_particle_obj_plugin_create(
                particle_emitter,
                gen_entity,
                sizeof(struct ui_sprite_particle_gen_entity_slot),
                ui_sprite_particle_gen_entity_slot_init,
                ui_sprite_particle_gen_entity_slot_fini,
                ui_sprite_particle_gen_entity_slot_update);
        
        if (plugin == NULL) goto ENTER_FAIL;
    }
    
    return 0;

ENTER_FAIL:
    if (particle_obj) {
        plugin_particle_obj_emitters(&emitter_it, particle_obj);
        while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
            plugin_particle_obj_plugin_t plugin = plugin_particle_obj_plugin_find_by_ctx(particle_emitter, gen_entity);
            if (plugin) {
                plugin_particle_obj_plugin_free(plugin);
            }
        }
    }

    if (gen_entity->m_anim_name) {
        mem_free(module->m_alloc, gen_entity->m_anim_name);
        gen_entity->m_anim_name = NULL;
    }
    
    return -1;
}

static void ui_sprite_particle_gen_entity_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = (ui_sprite_particle_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_particle_gen_entity_t gen_entity = (ui_sprite_particle_gen_entity_t)ui_sprite_fsm_action_data(fsm_action);
    plugin_particle_obj_t particle_obj;

    assert(gen_entity->m_anim_name);
    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, gen_entity->m_anim_name, module->m_em);
    if (particle_obj) {
        struct plugin_particle_obj_emitter_it emitter_it;
        plugin_particle_obj_emitter_t particle_emitter;
        plugin_particle_obj_emitters(&emitter_it, particle_obj);
        while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
            plugin_particle_obj_plugin_t plugin = plugin_particle_obj_plugin_find_by_ctx(particle_emitter, gen_entity);
            if (plugin) {
                plugin_particle_obj_plugin_free(plugin);
            }
        }
    }

    if (gen_entity->m_anim_name) {
        mem_free(module->m_alloc, gen_entity->m_anim_name);
        gen_entity->m_anim_name = NULL;
    }
}

static int ui_sprite_particle_gen_entity_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_gen_entity_t gen_entity = (ui_sprite_particle_gen_entity_t)ui_sprite_fsm_action_data(fsm_action);
	bzero(gen_entity, sizeof(*gen_entity));
    gen_entity->m_module = (ui_sprite_particle_module_t)ctx;
    gen_entity->m_cfg_anim_name = NULL;
    gen_entity->m_cfg_prefix = NULL;
    gen_entity->m_anim_name = NULL;
    return 0;
}

static void ui_sprite_particle_gen_entity_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = (ui_sprite_particle_module_t)ctx;
    ui_sprite_particle_gen_entity_t gen_entity = (ui_sprite_particle_gen_entity_t)ui_sprite_fsm_action_data(fsm_action);

    assert(gen_entity->m_anim_name == NULL);
    
    if (gen_entity->m_cfg_anim_name) {
        mem_free(module->m_alloc, gen_entity->m_cfg_anim_name);
        gen_entity->m_cfg_anim_name = NULL;
    }

    if (gen_entity->m_cfg_prefix) {
        mem_free(module->m_alloc, gen_entity->m_cfg_prefix);
        gen_entity->m_cfg_prefix = NULL;
    }
}

static int ui_sprite_particle_gen_entity_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_particle_module_t module = (ui_sprite_particle_module_t)ctx;
    ui_sprite_particle_gen_entity_t to_gen_entity = (ui_sprite_particle_gen_entity_t)ui_sprite_fsm_action_data(to);
    ui_sprite_particle_gen_entity_t from_gen_entity = (ui_sprite_particle_gen_entity_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_particle_gen_entity_init(to, ctx)) return -1;

    if (from_gen_entity->m_cfg_anim_name) {
        to_gen_entity->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_gen_entity->m_cfg_anim_name);
    }
    
    if (from_gen_entity->m_cfg_prefix) {
        to_gen_entity->m_cfg_prefix = cpe_str_mem_dup(module->m_alloc, from_gen_entity->m_cfg_prefix);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_particle_gen_entity_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_particle_module_t module = (ui_sprite_particle_module_t)ctx;
    ui_sprite_particle_gen_entity_t particle_gen_entity = ui_sprite_particle_gen_entity_create(fsm_state, name);
    const char * str_value;
    
    if (particle_gen_entity == NULL) {
        CPE_ERROR(module->m_em, "%s: create particle gen entity action: create fail!", ui_sprite_particle_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_particle_gen_entity_set_anim_name(particle_gen_entity, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create particle gen entity action: set anim name %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_gen_entity_free(particle_gen_entity);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle gen entity: anim-name not configured!",
            ui_sprite_particle_module_name(module));
        ui_sprite_particle_gen_entity_free(particle_gen_entity);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_particle_gen_entity_set_prefix(particle_gen_entity, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create particle gen entity action: set anim name %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_gen_entity_free(particle_gen_entity);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(particle_gen_entity);
}

int ui_sprite_particle_gen_entity_regist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_PARTICLE_GEN_ENTITY_NAME, sizeof(struct ui_sprite_particle_gen_entity));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle gen entity register: meta create fail",
            ui_sprite_particle_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_particle_gen_entity_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_particle_gen_entity_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_particle_gen_entity_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_particle_gen_entity_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_particle_gen_entity_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_PARTICLE_GEN_ENTITY_NAME, ui_sprite_particle_gen_entity_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_gen_entity_unregist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_PARTICLE_GEN_ENTITY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle gen entity unregister: meta not exist",
            ui_sprite_particle_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_PARTICLE_GEN_ENTITY_NAME = "particle-gen-entity";

#ifdef __cplusplus
}
#endif
