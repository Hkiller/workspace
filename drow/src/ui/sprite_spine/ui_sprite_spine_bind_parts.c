#include <assert.h>
#include "spine/Skeleton.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_render/ui_sprite_render_utils.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_bind_parts_i.h"

static void ui_sprite_spine_bind_parts_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s);

ui_sprite_spine_bind_parts_t ui_sprite_spine_bind_parts_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_BIND_PARTS_NAME);
    return fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_spine_bind_parts_free(ui_sprite_spine_bind_parts_t bind_parts) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(bind_parts);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_bind_parts_set_obj_name(ui_sprite_spine_bind_parts_t bind_parts, const char * obj_name) {
    assert(obj_name);

    if (bind_parts->m_cfg_obj_name) {
        mem_free(bind_parts->m_module->m_alloc, bind_parts->m_cfg_obj_name);
        bind_parts->m_cfg_obj_name = NULL;
    }

    bind_parts->m_cfg_obj_name = cpe_str_mem_dup(bind_parts->m_module->m_alloc, obj_name);
    
    return 0;
}

int ui_sprite_spine_bind_parts_set_prefix(ui_sprite_spine_bind_parts_t bind_parts, const char * prefix) {
    assert(prefix);

    if (bind_parts->m_cfg_prefix) {
        mem_free(bind_parts->m_module->m_alloc, bind_parts->m_cfg_prefix);
        bind_parts->m_cfg_prefix = NULL;
    }

    bind_parts->m_cfg_prefix = cpe_str_mem_dup(bind_parts->m_module->m_alloc, prefix);
     
    return 0;
}

static int ui_sprite_spine_bind_parts_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_bind_parts_t bind_parts = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_2d_transform_t transform;
    plugin_spine_obj_t spine_obj;
    uint16_t prefix_len;
    struct spSkeleton* skeleton;

    assert(TAILQ_EMPTY(&bind_parts->m_bindings));
    
    transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind parts: obj no transform!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }
    
    spine_obj = ui_sprite_spine_find_obj_on_entity(entity, bind_parts->m_cfg_obj_name, module->m_em);
    if (spine_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind parts: spine-obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_parts->m_cfg_obj_name);
        goto ENTER_FAIL;
    }

	skeleton = plugin_spine_obj_skeleton(spine_obj);

    if (bind_parts->m_cfg_prefix == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind parts: prefix not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    prefix_len = strlen(bind_parts->m_cfg_prefix);

    if (bind_parts->m_cfg_debug) {
        CPE_INFO(
            module->m_em, "spine_bin_parts: anim %s: prefix=%s, target=%d",
            bind_parts->m_cfg_obj_name, bind_parts->m_cfg_prefix, bind_parts->m_bind_target);
    }

    if (bind_parts->m_bind_target == ui_sprite_spine_bind_target_skeleton) {
        int bone_pos;
        
        for(bone_pos = 0; bone_pos < skeleton->bonesCount; ++bone_pos) {
            spBone* bone = skeleton->bones[bone_pos];
            const char * part_name;
            ui_sprite_2d_part_t part;
            char * args;
            
            if (!cpe_str_start_with(bone->data->name, bind_parts->m_cfg_prefix)) continue;

            part_name = ui_sprite_spine_module_analize_name(module, bone->data->name + prefix_len, &args);
            if (part_name == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): bind parts: analize part name %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bone->data->name + prefix_len);
                goto ENTER_FAIL;
            }
            
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

            if (ui_sprite_spine_bind_parts_binding_create(bind_parts, part, bone, NULL) == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): bind parts: create binding fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                goto ENTER_FAIL;
            }
            
            if (args) ui_sprite_2d_part_bulk_set_values_mutable(part, args, ',', '=');

            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): spine bind part: %s ==> %s!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    bone->data->name, ui_sprite_2d_part_name(part));
            }
        }
    }
    else if (bind_parts->m_bind_target == ui_sprite_spine_bind_target_slot) {
        struct plugin_spine_attachment_it attachment_it;
        plugin_spine_data_attachment_t attachment;

        plugin_spine_skin_attachments(&attachment_it, skeleton->skin ? skeleton->skin : skeleton->data->defaultSkin);
    
        while((attachment = plugin_spine_attachment_it_next(&attachment_it))) {
            struct spAttachment * sp_attachment = plugin_spine_data_attachment_attachment(attachment);
            spSlot* slot;
            const char * name;
            ui_sprite_2d_part_t part;
            char * args;

            if (sp_attachment->type != SP_ATTACHMENT_BOUNDING_BOX) continue;

            slot = skeleton->slots[plugin_spine_data_attachment_slot_index(attachment)];
            assert(slot);
            
            if (!cpe_str_start_with(slot->data->name, bind_parts->m_cfg_prefix)) continue;

            name = ui_sprite_spine_module_analize_name(module, slot->data->name + prefix_len, &args);
            if (name == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): bind parts: analize part name %s error!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->data->name + prefix_len);
                goto ENTER_FAIL;
            }

            part = ui_sprite_2d_part_find(transform, name);
            if (part == NULL) {
                part = ui_sprite_2d_part_create(transform, name);
                if (part == NULL) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): bind parts: create part %s fail!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
                    goto ENTER_FAIL;
                }
            }

            if (ui_sprite_spine_bind_parts_binding_create(bind_parts, part, slot->bone, slot) == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): bind parts: create binding fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
                goto ENTER_FAIL;
            }

            if (args) ui_sprite_2d_part_bulk_set_values_mutable(part, args, ',', '=');

            name = ui_sprite_spine_module_analize_name(module, slot->bone->data->name, &args);
            if (name == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): bind parts: analize slot name %s fail!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), slot->bone->data->name);
                goto ENTER_FAIL;
            }
            ui_sprite_2d_part_set_value(part, "bone", name);
            
            if (args) ui_sprite_2d_part_bulk_set_values_mutable(part, args, ',', '=');

            if (ui_sprite_entity_debug(entity)) {
                CPE_INFO(
                    module->m_em, "entity %d(%s): spine bind part: %s.%s ==> %s!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                    slot->bone->data->name, slot->data->name, ui_sprite_2d_part_name(part));
            }
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind parts: unknown bind target %d!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_parts->m_bind_target);
        goto ENTER_FAIL;
    }
        
    ui_sprite_fsm_action_start_update(fsm_action);
    ui_sprite_spine_bind_parts_update(fsm_action, ctx, 0.0f);
    
    return 0;

ENTER_FAIL:
    while(!TAILQ_EMPTY(&bind_parts->m_bindings)) {
        ui_sprite_spine_bind_parts_binding_free(TAILQ_FIRST(&bind_parts->m_bindings));
    }

    return -1;
}

static void ui_sprite_spine_bind_parts_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_bind_parts_t bind_parts = ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_spine_bind_parts_binding_t binding;
    ui_transform anim_transform;
    ui_transform_t use_transform;
    ui_sprite_render_anim_t render_anim;

    render_anim = ui_sprite_render_anim_find_on_entity_by_name(entity, bind_parts->m_cfg_obj_name);
    if (render_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): bind parts: update: spine-obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), bind_parts->m_cfg_obj_name);
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    if (ui_sprite_render_anim_calc_obj_local_transform(render_anim, &anim_transform) != 0) return;

    /* struct mem_buffer buff; */
    /* mem_buffer_init(&buff, NULL); */
    /* printf("   spine anim trans=%s\n", ui_transform_dump_2d(&buff, &anim_transform)); */
    /* mem_buffer_clear(&buff); */
    
    if (ui_transform_cmp(&anim_transform, &UI_TRANSFORM_IDENTITY) == 0) {
        use_transform = NULL;
    }
    else {
        use_transform = &anim_transform;
    }

    TAILQ_FOREACH(binding, &bind_parts->m_bindings, m_next) {
        ui_sprite_spine_bind_parts_binding_update(binding, use_transform);
    }
}

static void ui_sprite_spine_bind_parts_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_bind_parts_t bind_parts = ui_sprite_fsm_action_data(fsm_action);

    while(!TAILQ_EMPTY(&bind_parts->m_bindings)) {
        ui_sprite_spine_bind_parts_binding_free(TAILQ_FIRST(&bind_parts->m_bindings));
    }
}

static int ui_sprite_spine_bind_parts_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_bind_parts_t bind_parts = ui_sprite_fsm_action_data(fsm_action);
    bind_parts->m_module = ctx;
    bind_parts->m_cfg_obj_name = NULL;
    bind_parts->m_cfg_prefix = NULL;
    bind_parts->m_cfg_debug = 0;
    bind_parts->m_bind_target = ui_sprite_spine_bind_target_skeleton;
    TAILQ_INIT(&bind_parts->m_bindings);
    return 0;
}

static void ui_sprite_spine_bind_parts_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;
    ui_sprite_spine_bind_parts_t bind_parts = ui_sprite_fsm_action_data(fsm_action);

    assert(TAILQ_EMPTY(&bind_parts->m_bindings));

    if (bind_parts->m_cfg_obj_name) {
        mem_free(modue->m_alloc, bind_parts->m_cfg_obj_name);
        bind_parts->m_cfg_obj_name = NULL;
    }

    if (bind_parts->m_cfg_prefix) {
        mem_free(modue->m_alloc, bind_parts->m_cfg_prefix);
        bind_parts->m_cfg_prefix = NULL;
    }
}

static int ui_sprite_spine_bind_parts_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_module_t modue = ctx;    
	ui_sprite_spine_bind_parts_t to_bind_parts = ui_sprite_fsm_action_data(to);
	ui_sprite_spine_bind_parts_t from_bind_parts = ui_sprite_fsm_action_data(from);

	if (ui_sprite_spine_bind_parts_init(to, ctx)) return -1;

    to_bind_parts->m_cfg_debug = from_bind_parts->m_cfg_debug;
    to_bind_parts->m_bind_target = from_bind_parts->m_bind_target;

    if (from_bind_parts->m_cfg_obj_name) {
        to_bind_parts->m_cfg_obj_name = cpe_str_mem_dup(modue->m_alloc, from_bind_parts->m_cfg_obj_name);
    }

    if (from_bind_parts->m_cfg_prefix) {
        to_bind_parts->m_cfg_prefix = cpe_str_mem_dup(modue->m_alloc, from_bind_parts->m_cfg_prefix);
    }
    
    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_spine_bind_parts_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_module_t module = ctx;
    ui_sprite_spine_bind_parts_t bind_parts = ui_sprite_spine_bind_parts_create(fsm_state, name);
    const char * str_value;
    
    if (bind_parts == NULL) {
        CPE_ERROR(module->m_em, "%s: create bind_parts action: create fail!", ui_sprite_spine_module_name(module));
        return NULL;
    }

    bind_parts->m_cfg_debug = cfg_get_uint8( cfg, "debug", bind_parts->m_cfg_debug);

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_spine_bind_parts_set_obj_name(bind_parts, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create bind_parts action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_bind_parts_free(bind_parts);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create bind_parts action: anim-name not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_bind_parts_free(bind_parts);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_spine_bind_parts_set_prefix(bind_parts, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create bind_parts action: set obj name %s fail!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_bind_parts_free(bind_parts);
            return NULL;
        }
    }
    else {
        CPE_ERROR(module->m_em, "%s: create bind_parts action: prefix not configured!", ui_sprite_spine_module_name(module));
        ui_sprite_spine_bind_parts_free(bind_parts);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "bind-target", NULL))) {
        if (strcmp(str_value, "skeleton") == 0) {
            bind_parts->m_bind_target = ui_sprite_spine_bind_target_skeleton;
        }
        else if (strcmp(str_value, "slot") == 0) {
            bind_parts->m_bind_target = ui_sprite_spine_bind_target_slot;
        }
        else {
            CPE_ERROR(
                module->m_em, "%s: create bind_parts action: bind-target %s unknown!",
                ui_sprite_spine_module_name(module), str_value);
            ui_sprite_spine_bind_parts_free(bind_parts);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(bind_parts);
}

int ui_sprite_spine_bind_parts_regist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_BIND_PARTS_NAME, sizeof(struct ui_sprite_spine_bind_parts));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: ui enable emitter register: meta create fail",
            ui_sprite_spine_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_bind_parts_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_bind_parts_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_bind_parts_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_bind_parts_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_bind_parts_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_bind_parts_update, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_BIND_PARTS_NAME, ui_sprite_spine_bind_parts_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_bind_parts_unregist(ui_sprite_spine_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_BIND_PARTS_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_spine_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_SPINE_BIND_PARTS_NAME);
}

const char * UI_SPRITE_SPINE_BIND_PARTS_NAME = "spine-bind-parts";
