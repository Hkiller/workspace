#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/particle/plugin_particle_obj.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "plugin/particle/plugin_particle_obj_plugin.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_particle/ui_sprite_particle_utils.h"
#include "ui_sprite_particle_chipmunk_with_collision_i.h"
#include "ui_sprite_particle_chipmunk_body_i.h"
#include "ui_sprite_particle_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_particle_chipmunk_with_collision_t ui_sprite_particle_chipmunk_with_collision_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_PARTICLE_WITH_COLLISION_NAME);
    return (ui_sprite_particle_chipmunk_with_collision_t)(fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL);
}

void ui_sprite_particle_chipmunk_with_collision_free(ui_sprite_particle_chipmunk_with_collision_t with_collision) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_collision);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_particle_chipmunk_with_collision_set_anim_name(ui_sprite_particle_chipmunk_with_collision_t with_collision, const char * anim_name) {
    assert(anim_name);

    if (with_collision->m_cfg_anim_name) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_anim_name);
    }

    with_collision->m_cfg_anim_name = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, anim_name);
    
    return 0;
}

int ui_sprite_particle_chipmunk_with_collision_set_prefix(ui_sprite_particle_chipmunk_with_collision_t with_collision, const char * prefix) {
    assert(prefix);

    if (with_collision->m_cfg_prefix) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_prefix);
    }

    with_collision->m_cfg_prefix = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, prefix);
    
    return 0;
}

int ui_sprite_particle_chipmunk_with_collision_set_collision_mask(
    ui_sprite_particle_chipmunk_with_collision_t with_collision, const char * collision_mask)
{
    assert(collision_mask);

    if (with_collision->m_cfg_collision_mask) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_collision_mask);
    }

    with_collision->m_cfg_collision_mask = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, collision_mask);
    
    return 0;
}

int ui_sprite_particle_chipmunk_with_collision_set_collision_category(
    ui_sprite_particle_chipmunk_with_collision_t with_collision, const char * collision_category)
{
    assert(collision_category);

    if (with_collision->m_cfg_collision_category) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_collision_category);
    }

    with_collision->m_cfg_collision_category = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, collision_category);
    
    return 0;
}

int ui_sprite_particle_chipmunk_with_collision_set_collision_group(
    ui_sprite_particle_chipmunk_with_collision_t with_collision, const char * collision_group)
{
    assert(collision_group);

    if (with_collision->m_cfg_collision_group) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_collision_group);
    }

    with_collision->m_cfg_collision_group = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, collision_group);
    
    return 0;
}

int ui_sprite_particle_chipmunk_with_collision_set_collision_event(
    ui_sprite_particle_chipmunk_with_collision_t with_collision, const char * collision_event)
{
    assert(collision_event);

    if (with_collision->m_cfg_collision_event) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_collision_event);
    }

    with_collision->m_cfg_collision_event = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, collision_event);
    
    return 0;
}

static int ui_sprite_particle_chipmunk_with_collision_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_chipmunk_module_t module = (ui_sprite_particle_chipmunk_module_t)ctx;
    ui_sprite_particle_chipmunk_with_collision_t with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_particle_chipmunk_env_t chipmunk_env;
    plugin_particle_obj_t particle_obj = NULL;
    plugin_particle_obj_emitter_t particle_emitter;
    struct plugin_particle_obj_emitter_it emitter_it;

    assert(with_collision->m_anim_name == NULL);
    assert(with_collision->m_chipmunk_env == NULL);

    /*获取碰撞系统环境 */
    chipmunk_env = ui_sprite_particle_chipmunk_env_find(ui_sprite_entity_world(entity));
    if (chipmunk_env == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle with collision: no chipmunk env!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    with_collision->m_chipmunk_env = chipmunk_env;

    /*计算碰撞事件 */
    if(with_collision->m_cfg_collision_event == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle with collision: collision.event not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    /*计算碰撞信息 */
    if (with_collision->m_cfg_collision_category) {
        if (plugin_chipmunk_env_masks(chipmunk_env->m_env, &with_collision->m_collision_category, with_collision->m_cfg_collision_category) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): particle with collision: load category from '%s' fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_collision_category);
            goto ENTER_FAIL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle with collision: collision.category not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    if (with_collision->m_cfg_collision_mask) {
        if (plugin_chipmunk_env_masks(chipmunk_env->m_env, &with_collision->m_collision_mask, with_collision->m_cfg_collision_mask) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): particle with collision: load mask from '%s' fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_collision_mask);
            goto ENTER_FAIL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle with collision: collision.mask not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    if (with_collision->m_cfg_collision_group) {
        if (ui_sprite_fsm_action_check_calc_uint32(
                &with_collision->m_collision_group, with_collision->m_cfg_collision_group, fsm_action, NULL, module->m_em)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): particle with collision: calc collision.group from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_collision_group);
            goto ENTER_FAIL;
        }
    }
    
    /*获取目标Particle */
    if (with_collision->m_cfg_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle with collision: anim-name not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    with_collision->m_anim_name = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, with_collision->m_cfg_anim_name, fsm_action, NULL, module->m_em);
    if (with_collision->m_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle with collision: calc anim name from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_anim_name);
        goto ENTER_FAIL;
    }
    
    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, with_collision->m_anim_name, module->m_em);
    if (particle_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): particle with collision: particle obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_anim_name);
        goto ENTER_FAIL;
    }

    /*给发射器添加碰撞 */
    plugin_particle_obj_emitters(&emitter_it, particle_obj);
    while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
        plugin_particle_obj_plugin_t plugin;
        UI_PARTICLE_EMITTER const * emitter_data;
        
        if (with_collision->m_cfg_prefix
            && !cpe_str_start_with(plugin_particle_obj_emitter_name(particle_emitter), with_collision->m_cfg_prefix))
        {
            continue;
        }

        emitter_data = plugin_particle_obj_emitter_data_r(particle_emitter);
        if(emitter_data->bound_mod == UI_PARTICLE_BOUND_NONE) {
            continue;
        }

        plugin =
            plugin_particle_obj_plugin_create(
                particle_emitter,
                with_collision,
                sizeof(struct ui_sprite_particle_chipmunk_body),
                ui_sprite_particle_chipmunk_body_init,
                ui_sprite_particle_chipmunk_body_fini,
                ui_sprite_particle_chipmunk_body_update);
        if (plugin == NULL) goto ENTER_FAIL;
    }
    
    return 0;

ENTER_FAIL:
    if (particle_obj) {
        plugin_particle_obj_emitters(&emitter_it, particle_obj);
        while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
            plugin_particle_obj_plugin_t plugin = plugin_particle_obj_plugin_find_by_ctx(particle_emitter, with_collision);
            if (plugin) {
                plugin_particle_obj_plugin_free(plugin);
            }
        }
    }

    with_collision->m_chipmunk_env = NULL;
    
    if (with_collision->m_anim_name) {
        mem_free(module->m_alloc, with_collision->m_anim_name);
        with_collision->m_anim_name = NULL;
    }
    
    return -1;
}

static void ui_sprite_particle_chipmunk_with_collision_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_chipmunk_module_t module = (ui_sprite_particle_chipmunk_module_t)ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_particle_chipmunk_with_collision_t with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
    plugin_particle_obj_t particle_obj;

    assert(with_collision->m_anim_name);
    particle_obj = ui_sprite_particle_find_obj(module->m_sprite_render, entity, with_collision->m_anim_name, module->m_em);
    if (particle_obj) {
        struct plugin_particle_obj_emitter_it emitter_it;
        plugin_particle_obj_emitter_t particle_emitter;
        plugin_particle_obj_emitters(&emitter_it, particle_obj);
        while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
            plugin_particle_obj_plugin_t plugin = plugin_particle_obj_plugin_find_by_ctx(particle_emitter, with_collision);
            if (plugin) {
                plugin_particle_obj_plugin_free(plugin);
            }
        }
    }

    with_collision->m_chipmunk_env = NULL;

    if (with_collision->m_anim_name) {
        mem_free(module->m_alloc, with_collision->m_anim_name);
        with_collision->m_anim_name = NULL;
    }
}

static int ui_sprite_particle_chipmunk_with_collision_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_chipmunk_with_collision_t with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
	bzero(with_collision, sizeof(*with_collision));
    with_collision->m_module = (ui_sprite_particle_chipmunk_module_t)ctx;
    with_collision->m_cfg_anim_name = NULL;
    with_collision->m_cfg_prefix = NULL;
    with_collision->m_cfg_collision_group = NULL;
    with_collision->m_cfg_collision_category = NULL;
    with_collision->m_cfg_collision_mask = NULL;
    with_collision->m_chipmunk_env = NULL;
    with_collision->m_anim_name = NULL;
    return 0;
}

static void ui_sprite_particle_chipmunk_with_collision_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_chipmunk_module_t module = (ui_sprite_particle_chipmunk_module_t)ctx;
    ui_sprite_particle_chipmunk_with_collision_t with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);

    assert(with_collision->m_chipmunk_env == NULL);
    assert(with_collision->m_anim_name == NULL);
    
    if (with_collision->m_cfg_anim_name) {
        mem_free(module->m_alloc, with_collision->m_cfg_anim_name);
        with_collision->m_cfg_anim_name = NULL;
    }

    if (with_collision->m_cfg_prefix) {
        mem_free(module->m_alloc, with_collision->m_cfg_prefix);
        with_collision->m_cfg_prefix = NULL;
    }

    if (with_collision->m_cfg_collision_category) {
        mem_free(module->m_alloc, with_collision->m_cfg_collision_category);
        with_collision->m_cfg_collision_category = NULL;
    }

    if (with_collision->m_cfg_collision_mask) {
        mem_free(module->m_alloc, with_collision->m_cfg_collision_mask);
        with_collision->m_cfg_collision_mask = NULL;
    }

    if (with_collision->m_cfg_collision_group) {
        mem_free(module->m_alloc, with_collision->m_cfg_collision_group);
        with_collision->m_cfg_collision_group = NULL;
    }

    if (with_collision->m_cfg_collision_event) {
        mem_free(module->m_alloc, with_collision->m_cfg_collision_event);
        with_collision->m_cfg_collision_event = NULL;
    }
}

static int ui_sprite_particle_chipmunk_with_collision_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_particle_chipmunk_module_t module = (ui_sprite_particle_chipmunk_module_t)ctx;
    ui_sprite_particle_chipmunk_with_collision_t to_with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ui_sprite_fsm_action_data(to);
    ui_sprite_particle_chipmunk_with_collision_t from_with_collision = (ui_sprite_particle_chipmunk_with_collision_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_particle_chipmunk_with_collision_init(to, ctx)) return -1;

    if (from_with_collision->m_cfg_anim_name) {
        to_with_collision->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_anim_name);
    }
    
    if (from_with_collision->m_cfg_prefix) {
        to_with_collision->m_cfg_prefix = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_prefix);
    }

    if (from_with_collision->m_cfg_collision_category) {
        to_with_collision->m_cfg_collision_category = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_collision_category);
    }

    if (from_with_collision->m_cfg_collision_mask) {
        to_with_collision->m_cfg_collision_mask = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_collision_mask);
    }

    if (from_with_collision->m_cfg_collision_group) {
        to_with_collision->m_cfg_collision_group = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_collision_group);
    }

    if (from_with_collision->m_cfg_collision_event) {
        to_with_collision->m_cfg_collision_event = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_collision_event);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_particle_chipmunk_with_collision_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_particle_chipmunk_module_t module = (ui_sprite_particle_chipmunk_module_t)ctx;
    ui_sprite_particle_chipmunk_with_collision_t particle_chipmunk_with_collision = ui_sprite_particle_chipmunk_with_collision_create(fsm_state, name);
    const char * str_value;
    
    if (particle_chipmunk_with_collision == NULL) {
        CPE_ERROR(module->m_em, "%s: create particle-with-collision action: create fail!", ui_sprite_particle_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_particle_chipmunk_with_collision_set_anim_name(particle_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create particle-with-collision action: set anim name %s fail!",
                ui_sprite_particle_chipmunk_module_name(module), str_value);
            ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle-with-collision: anim-name not configured!",
            ui_sprite_particle_chipmunk_module_name(module));
        ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_particle_chipmunk_with_collision_set_prefix(particle_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create particle-with-collision action: set anim name %s fail!",
                ui_sprite_particle_chipmunk_module_name(module), str_value);
            ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "collision.category", NULL))) {
        if (ui_sprite_particle_chipmunk_with_collision_set_collision_category(particle_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create particle with collision action: set collision category %s fail!",
                ui_sprite_particle_chipmunk_module_name(module), str_value);
            ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle-with-collision: collision.category not configured!",
            ui_sprite_particle_chipmunk_module_name(module));
        ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "collision.mask", NULL))) {
        if (ui_sprite_particle_chipmunk_with_collision_set_collision_mask(particle_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create particle with collision action: set collision mask %s fail!",
                ui_sprite_particle_chipmunk_module_name(module), str_value);
            ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle-with-collision: collision.mask not configured!",
            ui_sprite_particle_chipmunk_module_name(module));
        ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "collision.group", NULL))) {
        if (ui_sprite_particle_chipmunk_with_collision_set_collision_group(particle_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create particle with collision action: set collision group %s fail!",
                ui_sprite_particle_chipmunk_module_name(module), str_value);
            ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
            return NULL;
        }
    }
    
    if ((str_value = cfg_get_string(cfg, "collision.event", NULL))) {
        if (ui_sprite_particle_chipmunk_with_collision_set_collision_event(particle_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create particle with collision action: set collision event %s fail!",
                ui_sprite_particle_chipmunk_module_name(module), str_value);
            ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle-with-collision: collision.event not configured!",
            ui_sprite_particle_chipmunk_module_name(module));
        ui_sprite_particle_chipmunk_with_collision_free(particle_chipmunk_with_collision);
        return NULL;
    }

    return ui_sprite_fsm_action_from_data(particle_chipmunk_with_collision);
}

int ui_sprite_particle_chipmunk_with_collision_regist(ui_sprite_particle_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_PARTICLE_WITH_COLLISION_NAME, sizeof(struct ui_sprite_particle_chipmunk_with_collision));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-with-collision register: meta create fail",
            ui_sprite_particle_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_particle_chipmunk_with_collision_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_particle_chipmunk_with_collision_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_particle_chipmunk_with_collision_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_particle_chipmunk_with_collision_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_particle_chipmunk_with_collision_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_PARTICLE_WITH_COLLISION_NAME, ui_sprite_particle_chipmunk_with_collision_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_chipmunk_with_collision_unregist(ui_sprite_particle_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_PARTICLE_WITH_COLLISION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-with-collision unregister: meta not exist",
            ui_sprite_particle_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_PARTICLE_WITH_COLLISION_NAME = "particle-with-collision";

#ifdef __cplusplus
}
#endif
