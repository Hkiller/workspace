#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_2d/ui_sprite_2d_part.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_render/ui_sprite_render_utils.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_particle/ui_sprite_particle_utils.h"
#include "ui_sprite_particle_bind_emitter_part_binding_i.h"
#include "ui_sprite_particle_utils_i.h"

ui_sprite_particle_bind_emitter_part_t ui_sprite_particle_bind_emitter_part_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_PARTICLE_BIND_EMITTER_PART_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_particle_bind_emitter_part_free(ui_sprite_particle_bind_emitter_part_t bind_emitter_part) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(bind_emitter_part);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_particle_bind_emitter_part_set_anim_name(ui_sprite_particle_bind_emitter_part_t bind_emitter_part, const char * anim_name) {
    assert(anim_name);

    if (bind_emitter_part->m_cfg_anim_name) {
        mem_free(bind_emitter_part->m_module->m_alloc, bind_emitter_part->m_cfg_anim_name);
    }

    bind_emitter_part->m_cfg_anim_name = cpe_str_mem_dup_trim(bind_emitter_part->m_module->m_alloc, anim_name);
    
    return 0;
}

int ui_sprite_particle_bind_emitter_part_set_prefix(ui_sprite_particle_bind_emitter_part_t bind_emitter_part, const char * prefix) {
    assert(prefix);

    if (bind_emitter_part->m_cfg_prefix) {
        mem_free(bind_emitter_part->m_module->m_alloc, bind_emitter_part->m_cfg_prefix);
    }

    bind_emitter_part->m_cfg_prefix = cpe_str_mem_dup_trim(bind_emitter_part->m_module->m_alloc, prefix);
    
    return 0;
}

int ui_sprite_particle_bind_emitter_part_set_scope(ui_sprite_particle_bind_emitter_part_t bind_emitter_part, const char * scope) {
    assert(scope);

    if (bind_emitter_part->m_cfg_scope) {
        mem_free(bind_emitter_part->m_module->m_alloc, bind_emitter_part->m_cfg_scope);
    }

    bind_emitter_part->m_cfg_scope = cpe_str_mem_dup_trim(bind_emitter_part->m_module->m_alloc, scope);
    
    return 0;
}

int ui_sprite_particle_bind_emitter_part_set_angle_adj(ui_sprite_particle_bind_emitter_part_t bind_emitter_part, const char * angle_adj) {
    assert(angle_adj);

    if (bind_emitter_part->m_cfg_angle_adj) {
        mem_free(bind_emitter_part->m_module->m_alloc, bind_emitter_part->m_cfg_angle_adj);
    }

    bind_emitter_part->m_cfg_angle_adj = cpe_str_mem_dup_trim(bind_emitter_part->m_module->m_alloc, angle_adj);
    
    return 0;
}

static int ui_sprite_particle_bind_emitter_part_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_bind_emitter_part_t bind_emitter_part = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);  
    plugin_particle_obj_t particle_obj;
    plugin_particle_obj_emitter_t particle_emitter;
    struct plugin_particle_obj_emitter_it emitter_it;
    ui_sprite_2d_transform_t transform;
    size_t prefix_len;
    
    if (bind_emitter_part->m_cfg_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind emitter part: render not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind emitter part: no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    bind_emitter_part->m_anim_name = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, bind_emitter_part->m_cfg_anim_name, fsm_action, NULL, module->m_em);
    if (bind_emitter_part->m_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind emitter part: calc anim from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_emitter_part->m_cfg_anim_name);
        goto ENTER_FAIL;
    }
    
    particle_obj = ui_sprite_particle_find_obj_on_entity(entity, bind_emitter_part->m_anim_name, module->m_em);
    if (particle_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind emitter part: particle obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_emitter_part->m_anim_name);
        goto ENTER_FAIL;
    }

    if (bind_emitter_part->m_cfg_angle_adj) {
        if (ui_sprite_fsm_action_check_calc_float(
                &bind_emitter_part->m_angle_adj_rad, bind_emitter_part->m_cfg_angle_adj, fsm_action, NULL, module->m_em)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): bind emitter part: calc angle-adj from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_emitter_part->m_cfg_angle_adj);
            goto ENTER_FAIL;
        }

        bind_emitter_part->m_angle_adj_rad = cpe_math_angle_to_radians(bind_emitter_part->m_angle_adj_rad);
    }
    
    prefix_len = bind_emitter_part->m_cfg_prefix ? strlen(bind_emitter_part->m_cfg_prefix) : 0;

    plugin_particle_obj_emitters(&emitter_it, particle_obj);
    while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
        const char * emitter_name = plugin_particle_obj_emitter_name(particle_emitter);
        const char * args_begin;
        ui_sprite_2d_part_t part = NULL;
        
        if (bind_emitter_part->m_cfg_prefix && !cpe_str_start_with(emitter_name, bind_emitter_part->m_cfg_prefix)) continue;

        emitter_name = emitter_name + prefix_len;

        if (bind_emitter_part->m_cfg_scope && !cpe_str_start_with(emitter_name, bind_emitter_part->m_cfg_scope)) continue;
        
        if ((args_begin = strchr(emitter_name, '['))) {
            char buf[64];
            const char * args_end;

            args_end = strchr(args_begin + 1, ']');
            if (args_end == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): bind emitter part: emitter %s name format error!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), emitter_name);
                goto ENTER_FAIL;
            }

            if (cpe_str_read_arg_range(buf, sizeof(buf), args_begin + 1, args_end, "bind-to", ',', '=') == 0) {
                part = ui_sprite_2d_part_check_create(transform, buf);
                if (part == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): bind emitter part: emitter %s create bind to part %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), emitter_name, buf);
                    goto ENTER_FAIL;
                }
            }
        }

        if (part == NULL) {
            ui_sprite_2d_part_t check_part;
            struct ui_sprite_2d_part_it part_it;

            ui_sprite_2d_transform_parts(&part_it, transform);
            while((check_part = ui_sprite_2d_part_it_next(&part_it))) {
                const char * part_name = ui_sprite_2d_part_name(check_part);

                if (!cpe_str_start_with(emitter_name, part_name)) continue;

                if (part == NULL || strlen(ui_sprite_2d_part_name(part)) < strlen(part_name)) {
                    part = check_part;
                }
            }
        }

        if (part) {
            ui_sprite_particle_bind_emitter_part_binding_t binding;

            binding = ui_sprite_particle_bind_emitter_part_binding_create(bind_emitter_part, part, particle_emitter);
            if (binding == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): bind emitter part: create binding fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                goto ENTER_FAIL;
            }

            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): bind emitter part: %s ==> %s!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), emitter_name, ui_sprite_2d_part_name(part));
            }
        }
    }

    if (!TAILQ_EMPTY(&bind_emitter_part->m_bindings)) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }
    
    return 0;

ENTER_FAIL:
    while(!TAILQ_EMPTY(&bind_emitter_part->m_bindings)) {
        ui_sprite_particle_bind_emitter_part_binding_free(TAILQ_FIRST(&bind_emitter_part->m_bindings));
    }
    bind_emitter_part->m_angle_adj_rad = 0.0f;

    if (bind_emitter_part->m_anim_name) {
        mem_free(module->m_alloc, bind_emitter_part->m_anim_name);
        bind_emitter_part->m_anim_name = NULL;
    }
    
    return -1;
}

static void ui_sprite_particle_bind_emitter_part_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_bind_emitter_part_t bind_emitter_part = ui_sprite_fsm_action_data(fsm_action);

    assert(bind_emitter_part->m_anim_name);
    mem_free(module->m_alloc, bind_emitter_part->m_anim_name);
    bind_emitter_part->m_anim_name = NULL;

    bind_emitter_part->m_angle_adj_rad = 0.0f;

    while(!TAILQ_EMPTY(&bind_emitter_part->m_bindings)) {
        ui_sprite_particle_bind_emitter_part_binding_free(TAILQ_FIRST(&bind_emitter_part->m_bindings));
    }
}

static void ui_sprite_particle_bind_emitter_part_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_bind_emitter_part_t bind_emitter_part = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_2d_transform_t entity_transform = ui_sprite_2d_transform_find(entity);
    ui_sprite_particle_bind_emitter_part_binding_t binding;
    ui_sprite_render_anim_t render_anim;
    ui_transform entity_w;
    ui_transform anim_w;
    ui_transform anim_l_r;

    render_anim = ui_sprite_render_anim_find_on_entity_by_name(entity, bind_emitter_part->m_anim_name);
    if (render_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind emitter part: update: particle-obj calc from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_emitter_part->m_anim_name);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    /*计算anim的本地绘制坐标 */
    if (ui_sprite_2d_transform_calc_trans(entity_transform, &entity_w) != 0) return;
    if (ui_sprite_render_anim_calc_obj_world_transform(render_anim, &anim_w) != 0) return;

    ui_transform_inline_reverse(&anim_w);
    ui_transform_cross_product(&anim_l_r, &anim_w, &entity_w);
    
    TAILQ_FOREACH(binding, &bind_emitter_part->m_bindings, m_next) {
        ui_sprite_paritcle_bind_emitter_part_binding_update(binding, &anim_l_r);
    }
}

static int ui_sprite_particle_bind_emitter_part_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_bind_emitter_part_t bind_emitter_part = ui_sprite_fsm_action_data(fsm_action);
	bzero(bind_emitter_part, sizeof(*bind_emitter_part));
    bind_emitter_part->m_module = ctx;
    bind_emitter_part->m_cfg_anim_name = NULL;
    bind_emitter_part->m_cfg_prefix = NULL;
    bind_emitter_part->m_cfg_scope = NULL;
    bind_emitter_part->m_cfg_angle_adj = NULL;
    bind_emitter_part->m_anim_name = NULL;
    TAILQ_INIT(&bind_emitter_part->m_bindings);
    return 0;
}

static void ui_sprite_particle_bind_emitter_part_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_bind_emitter_part_t bind_emitter_part = ui_sprite_fsm_action_data(fsm_action);

    assert(TAILQ_EMPTY(&bind_emitter_part->m_bindings));

    if (bind_emitter_part->m_cfg_anim_name) {
        mem_free(module->m_alloc, bind_emitter_part->m_cfg_anim_name);
        bind_emitter_part->m_cfg_anim_name = NULL;
    }

    if (bind_emitter_part->m_cfg_prefix) {
        mem_free(module->m_alloc, bind_emitter_part->m_cfg_prefix);
        bind_emitter_part->m_cfg_prefix = NULL;
    }

    if (bind_emitter_part->m_cfg_scope) {
        mem_free(module->m_alloc, bind_emitter_part->m_cfg_scope);
        bind_emitter_part->m_cfg_scope = NULL;
    }

    if (bind_emitter_part->m_cfg_angle_adj) {
        mem_free(module->m_alloc, bind_emitter_part->m_cfg_angle_adj);
        bind_emitter_part->m_cfg_angle_adj = NULL;
    }
}

static int ui_sprite_particle_bind_emitter_part_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_bind_emitter_part_t to_bind_emitter_part = ui_sprite_fsm_action_data(to);
    ui_sprite_particle_bind_emitter_part_t from_bind_emitter_part = ui_sprite_fsm_action_data(from);

    if (ui_sprite_particle_bind_emitter_part_init(to, ctx)) return -1;

    if (from_bind_emitter_part->m_cfg_anim_name) {
        to_bind_emitter_part->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_bind_emitter_part->m_cfg_anim_name);
    }
    
    if (from_bind_emitter_part->m_cfg_prefix) {
        to_bind_emitter_part->m_cfg_prefix = cpe_str_mem_dup(module->m_alloc, from_bind_emitter_part->m_cfg_prefix);
    }

    if (from_bind_emitter_part->m_cfg_scope) {
        to_bind_emitter_part->m_cfg_scope = cpe_str_mem_dup(module->m_alloc, from_bind_emitter_part->m_cfg_scope);
    }

    if (from_bind_emitter_part->m_cfg_angle_adj) {
        to_bind_emitter_part->m_cfg_angle_adj = cpe_str_mem_dup(module->m_alloc, from_bind_emitter_part->m_cfg_angle_adj);
    }

    to_bind_emitter_part->m_cfg_accept_flip = from_bind_emitter_part->m_cfg_accept_flip;
    to_bind_emitter_part->m_cfg_accept_scale = from_bind_emitter_part->m_cfg_accept_scale;
    to_bind_emitter_part->m_cfg_accept_angle = from_bind_emitter_part->m_cfg_accept_angle;

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_particle_bind_emitter_part_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_particle_module_t module = ctx;
    ui_sprite_particle_bind_emitter_part_t particle_bind_emitter_part = ui_sprite_particle_bind_emitter_part_create(fsm_state, name);
    const char * str_value;
    
    if (particle_bind_emitter_part == NULL) {
        CPE_ERROR(module->m_em, "%s: create render bind_emitter_part action: create fail!", ui_sprite_particle_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_particle_bind_emitter_part_set_anim_name(particle_bind_emitter_part, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render bind_emitter_part action: set anim name %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_bind_emitter_part_free(particle_bind_emitter_part);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create particle-bind_emitter_part: anim-name not configured!",
            ui_sprite_particle_module_name(module));
        ui_sprite_particle_bind_emitter_part_free(particle_bind_emitter_part);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_particle_bind_emitter_part_set_prefix(particle_bind_emitter_part, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render bind_emitter_part action: set prefix %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_bind_emitter_part_free(particle_bind_emitter_part);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "scope", NULL))) {
        if (ui_sprite_particle_bind_emitter_part_set_scope(particle_bind_emitter_part, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render bind_emitter_part action: set scope %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_bind_emitter_part_free(particle_bind_emitter_part);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "angle-adj", NULL))) {
        if (ui_sprite_particle_bind_emitter_part_set_angle_adj(particle_bind_emitter_part, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create render bind_emitter_part action: set angle adj %s fail!",
                ui_sprite_particle_module_name(module), str_value);
            ui_sprite_particle_bind_emitter_part_free(particle_bind_emitter_part);
            return NULL;
        }
    }
    
    particle_bind_emitter_part->m_cfg_accept_flip = cfg_get_uint8(cfg, "accept-flip", particle_bind_emitter_part->m_cfg_accept_flip);
    particle_bind_emitter_part->m_cfg_accept_scale = cfg_get_uint8(cfg, "accept-scale", particle_bind_emitter_part->m_cfg_accept_scale);
    particle_bind_emitter_part->m_cfg_accept_angle = cfg_get_uint8(cfg, "accept-angle", particle_bind_emitter_part->m_cfg_accept_angle);
    
    return ui_sprite_fsm_action_from_data(particle_bind_emitter_part);
}

int ui_sprite_particle_bind_emitter_part_regist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_PARTICLE_BIND_EMITTER_PART_NAME, sizeof(struct ui_sprite_particle_bind_emitter_part));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-bind_emitter_part register: meta create fail",
            ui_sprite_particle_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_particle_bind_emitter_part_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_particle_bind_emitter_part_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_particle_bind_emitter_part_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_particle_bind_emitter_part_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_particle_bind_emitter_part_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_particle_bind_emitter_part_update, module);
    
    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_PARTICLE_BIND_EMITTER_PART_NAME, ui_sprite_particle_bind_emitter_part_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_particle_bind_emitter_part_unregist(ui_sprite_particle_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_PARTICLE_BIND_EMITTER_PART_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: particle-bind_emitter_part unregister: meta not exist",
            ui_sprite_particle_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_PARTICLE_BIND_EMITTER_PART_NAME = "particle-bind-emitter-part";

