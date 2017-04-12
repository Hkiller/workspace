#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/math_ex.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_utils.h"
#include "ui/sprite_render/ui_sprite_render_group.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_follow_parts_i.h"

ui_sprite_spine_follow_parts_t ui_sprite_spine_follow_parts_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_FOLLOW_PARTS_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_follow_parts_free(ui_sprite_spine_follow_parts_t follow_parts) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(follow_parts);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_follow_parts_set_obj_name(ui_sprite_spine_follow_parts_t follow_parts, const char * obj_name) {
    assert(obj_name);

    if (follow_parts->m_cfg_obj_name) {
        mem_free(follow_parts->m_module->m_alloc, follow_parts->m_cfg_obj_name);
        follow_parts->m_cfg_obj_name = NULL;
    }

    follow_parts->m_cfg_obj_name = cpe_str_mem_dup(follow_parts->m_module->m_alloc, obj_name);
    
    return 0;
}

int ui_sprite_spine_follow_parts_set_prefix(ui_sprite_spine_follow_parts_t follow_parts, const char * prefix) {
    assert(prefix);

    if (follow_parts->m_cfg_prefix) {
        mem_free(follow_parts->m_module->m_alloc, follow_parts->m_cfg_prefix);
        follow_parts->m_cfg_prefix = NULL;
    }

    follow_parts->m_cfg_prefix = cpe_str_mem_dup(follow_parts->m_module->m_alloc, prefix);
     
    return 0;
}

int ui_sprite_spine_follow_parts_set_scope(ui_sprite_spine_follow_parts_t follow_parts, const char * scope) {
    assert(scope);

    if (follow_parts->m_cfg_scope) {
        mem_free(follow_parts->m_module->m_alloc, follow_parts->m_cfg_scope);
        follow_parts->m_cfg_scope = NULL;
    }

    follow_parts->m_cfg_scope = cpe_str_mem_dup(follow_parts->m_module->m_alloc, scope);
     
    return 0;
}

static int ui_sprite_spine_follow_parts_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_follow_parts_t follow_parts = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_transform_t transform;
    plugin_spine_obj_t spine_obj;
    struct plugin_spine_obj_ik_it ik_it;
    plugin_spine_obj_ik_t ik;
    uint16_t prefix_len;
        
    assert(TAILQ_EMPTY(&follow_parts->m_bindings));
    
    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): follow parts: obj no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    spine_obj = ui_sprite_spine_find_obj_on_entity(entity, follow_parts->m_cfg_obj_name, module->m_em);
    if (spine_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): follow parts: spine-obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), follow_parts->m_cfg_obj_name);
        goto ENTER_FAIL;
    }

    if (follow_parts->m_cfg_prefix == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): follow parts: prefix not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    prefix_len = strlen(follow_parts->m_cfg_prefix);

    plugin_spine_obj_iks(spine_obj, &ik_it);
    while((ik = plugin_spine_obj_ik_it_next(&ik_it))) {
        ui_sprite_2d_part_t part;
        const char * part_name;
        
        if (!cpe_str_start_with(plugin_spine_obj_ik_name(ik), follow_parts->m_cfg_prefix)) continue;

        part_name = plugin_spine_obj_ik_name(ik) + prefix_len;

        if (follow_parts->m_cfg_scope && !cpe_str_start_with(part_name, follow_parts->m_cfg_scope)) continue;
        
        part = ui_sprite_2d_part_find(transform, part_name);
        if (part == NULL) {
            part = ui_sprite_2d_part_create(transform, part_name);
            if (part == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): bind parts: create part %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), part_name);
                goto ENTER_FAIL;
            }
        }

        if (ui_sprite_spine_follow_parts_binding_create(follow_parts, part, ik) == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): bind parts: create binding fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_FAIL;
        }
    }
    
    ui_sprite_fsm_action_start_update(fsm_action);
    
    return 0;

ENTER_FAIL:
    while(!TAILQ_EMPTY(&follow_parts->m_bindings)) {
        ui_sprite_spine_follow_parts_binding_free(TAILQ_FIRST(&follow_parts->m_bindings));
    }

    return -1;
}

static void ui_sprite_spine_follow_parts_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_follow_parts_t follow_parts = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_follow_parts_binding_t binding;
    ui_transform anim_transform;
    ui_transform_t anim_transform_r;
    ui_vector_2 pos_adj;
    ui_vector_2_t use_pos_adj;
    ui_sprite_render_anim_t render_anim;
    ui_sprite_render_group_t group;

    render_anim = ui_sprite_render_anim_find_on_entity_by_name(entity, follow_parts->m_cfg_obj_name);
    if (render_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): follow parts: update: spine-obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), follow_parts->m_cfg_obj_name);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (ui_sprite_render_anim_calc_obj_local_transform(render_anim, &anim_transform) != 0) return;

    if (ui_transform_cmp(&anim_transform, &UI_TRANSFORM_IDENTITY) == 0) {
        anim_transform_r = NULL;
    }
    else {
        ui_transform_inline_reverse(&anim_transform);
        anim_transform_r = &anim_transform;
    }

    group = ui_sprite_render_anim_group(render_anim);
    if (group && !ui_sprite_render_group_adj_accept_scale(group)) {
        ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(ui_sprite_fsm_action_to_entity(fsm_action));
        pos_adj = ui_sprite_2d_transform_scale_pair(transform);
        use_pos_adj = &pos_adj;
    }
    else {
        use_pos_adj = NULL;
    }
    
    TAILQ_FOREACH(binding, &follow_parts->m_bindings, m_next) {
        ui_sprite_spine_follow_parts_binding_update(binding, anim_transform_r, use_pos_adj);
    }
}

static void ui_sprite_spine_follow_parts_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_follow_parts_t follow_parts = ui_sprite_fsm_action_data(fsm_action);

    if (follow_parts->m_cfg_restore) {
        ui_sprite_spine_follow_parts_binding_t binding;
        TAILQ_FOREACH(binding, &follow_parts->m_bindings, m_next) {
            plugin_spine_obj_ik_restore(binding->m_ik);
        }
    }
        
    while(!TAILQ_EMPTY(&follow_parts->m_bindings)) {
        ui_sprite_spine_follow_parts_binding_free(TAILQ_FIRST(&follow_parts->m_bindings));
    }
}

static int ui_sprite_spine_follow_parts_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_follow_parts_t follow_parts = ui_sprite_fsm_action_data(fsm_action);
    follow_parts->m_module = ctx;
    follow_parts->m_cfg_obj_name = NULL;
    follow_parts->m_cfg_prefix = NULL;
    follow_parts->m_cfg_scope = NULL;
    follow_parts->m_cfg_restore = 1;
    TAILQ_INIT(&follow_parts->m_bindings);
    return 0;
}

static void ui_sprite_spine_follow_parts_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;
    ui_sprite_spine_follow_parts_t follow_parts = ui_sprite_fsm_action_data(fsm_action);

    assert(TAILQ_EMPTY(&follow_parts->m_bindings));

    if (follow_parts->m_cfg_obj_name) {
        mem_free(modue->m_alloc, follow_parts->m_cfg_obj_name);
        follow_parts->m_cfg_obj_name = NULL;
    }

    if (follow_parts->m_cfg_prefix) {
        mem_free(modue->m_alloc, follow_parts->m_cfg_prefix);
        follow_parts->m_cfg_prefix = NULL;
    }

    if (follow_parts->m_cfg_scope) {
        mem_free(modue->m_alloc, follow_parts->m_cfg_scope);
        follow_parts->m_cfg_scope = NULL;
    }
}

static int ui_sprite_spine_follow_parts_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;    
	ui_sprite_spine_follow_parts_t to_follow_parts = ui_sprite_fsm_action_data(to);
	ui_sprite_spine_follow_parts_t from_follow_parts = ui_sprite_fsm_action_data(from);

	if (ui_sprite_spine_follow_parts_init(to, ctx)) return -1;

    to_follow_parts->m_cfg_restore = from_follow_parts->m_cfg_restore;
    
    if (from_follow_parts->m_cfg_obj_name) {
        to_follow_parts->m_cfg_obj_name = cpe_str_mem_dup(modue->m_alloc, from_follow_parts->m_cfg_obj_name);
    }

    if (from_follow_parts->m_cfg_prefix) {
        to_follow_parts->m_cfg_prefix = cpe_str_mem_dup(modue->m_alloc, from_follow_parts->m_cfg_prefix);
    }

    if (from_follow_parts->m_cfg_scope) {
        to_follow_parts->m_cfg_scope = cpe_str_mem_dup(modue->m_alloc, from_follow_parts->m_cfg_scope);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_spine_follow_parts_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_follow_parts_t follow_parts = ui_sprite_spine_follow_parts_create(fsm_state, name);
    const char * str_value;
    
    if (follow_parts == NULL) {
        CPE_ERROR(module->m_em, "%s: create follow_parts action: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_spine_follow_parts_set_obj_name(follow_parts, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create follow_parts action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_follow_parts_free(follow_parts);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create follow_parts action: anim-name not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_follow_parts_free(follow_parts);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_spine_follow_parts_set_prefix(follow_parts, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create follow_parts action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_follow_parts_free(follow_parts);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create follow_parts action: prefix not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_follow_parts_free(follow_parts);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "scope", NULL))) {
        if (ui_sprite_spine_follow_parts_set_scope(follow_parts, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create follow_parts action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_follow_parts_free(follow_parts);
            return NULL;
        }
    }
    
    follow_parts->m_cfg_restore = cfg_get_uint8(cfg, "restore", follow_parts->m_cfg_restore);
        
    return ui_sprite_fsm_action_from_data(follow_parts);
}

int ui_sprite_spine_follow_parts_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_FOLLOW_PARTS_NAME, sizeof(struct ui_sprite_spine_follow_parts));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_follow_parts_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_follow_parts_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_follow_parts_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_follow_parts_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_follow_parts_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_follow_parts_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_FOLLOW_PARTS_NAME, ui_sprite_spine_follow_parts_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_follow_parts_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_FOLLOW_PARTS_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SPINE_FOLLOW_PARTS_NAME);
}

const char * UI_SPRITE_SPINE_FOLLOW_PARTS_NAME = "spine-follow-parts";
