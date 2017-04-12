#include <assert.h>
#include "spine/Skeleton.h"
#include "spine/Attachment.h"
#include "chipmunk/chipmunk.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_tri/ui_sprite_tri_obj.h"
#include "ui/sprite_tri/ui_sprite_tri_rule.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_chipmunk_with_tri_i.h"
#include "ui_sprite_spine_chipmunk_with_tri_binding_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_spine_chipmunk_with_tri_t ui_sprite_spine_chipmunk_with_tri_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_WITH_TRI_NAME);
    return (ui_sprite_spine_chipmunk_with_tri_t)(fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL);
}

void ui_sprite_spine_chipmunk_with_tri_free(ui_sprite_spine_chipmunk_with_tri_t with_tri) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_tri);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_chipmunk_with_tri_set_anim_name(ui_sprite_spine_chipmunk_with_tri_t with_tri, const char * anim_name) {
    assert(anim_name);

    if (with_tri->m_cfg_anim_name) {
        mem_free(with_tri->m_module->m_alloc, with_tri->m_cfg_anim_name);
    }

    with_tri->m_cfg_anim_name = cpe_str_mem_dup_trim(with_tri->m_module->m_alloc, anim_name);
    
    return 0;
}

int ui_sprite_spine_chipmunk_with_tri_set_prefix(ui_sprite_spine_chipmunk_with_tri_t with_tri, const char * prefix) {
    assert(prefix);

    if (with_tri->m_cfg_prefix) {
        mem_free(with_tri->m_module->m_alloc, with_tri->m_cfg_prefix);
    }

    with_tri->m_cfg_prefix = cpe_str_mem_dup_trim(with_tri->m_module->m_alloc, prefix);
    
    return 0;
}
    
static int ui_sprite_spine_chipmunk_with_tri_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_tri_t with_tri = (ui_sprite_spine_chipmunk_with_tri_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_tri_obj_t tri_obj;
    ui_sprite_render_anim_t render_anim;
    ui_sprite_render_sch_t render_sch;
    ui_runtime_render_obj_ref_t render_obj_ref;
    ui_runtime_render_obj_t render_obj;
    plugin_spine_obj_t spine_obj = NULL;
    struct spSkeleton* skeleton;
    char * anim_name = NULL;
    struct plugin_spine_attachment_it attachment_it;
    plugin_spine_data_attachment_t attachment;
    uint16_t prefix_len;

    /*获取tri_obj */
    tri_obj = ui_sprite_tri_obj_find(entity);
    if (tri_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: no tri obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    /*获取目标Spine */
    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: entity no anim sch!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    anim_name = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, with_tri->m_cfg_anim_name, fsm_action, NULL, module->m_em);
    if (anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: calc anim name from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_tri->m_cfg_anim_name);
        goto ENTER_FAIL;
    }
    
    render_anim = ui_sprite_render_anim_find_by_name(render_sch, anim_name);
    if (render_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: runing anim %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), anim_name);
        goto ENTER_FAIL;
    }

    render_obj_ref = ui_sprite_render_anim_obj(render_anim);
    assert(render_obj_ref);

    render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);
    assert(render_obj);
    
    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: render obj %s is not spine obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_tri->m_cfg_anim_name);
        goto ENTER_FAIL;
    }

    spine_obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
    if (spine_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: render obj %s no data!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_tri->m_cfg_anim_name);
        goto ENTER_FAIL;
    }
    
    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: spine obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), anim_name);
        goto ENTER_FAIL;
    }

	skeleton = plugin_spine_obj_skeleton(spine_obj);

    if (with_tri->m_cfg_prefix == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with tri: prefix not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    prefix_len = strlen(with_tri->m_cfg_prefix);
    
    /*遍历并绑定 */
    plugin_spine_skin_attachments(&attachment_it, skeleton->skin ? skeleton->skin : skeleton->data->defaultSkin);
    
    while((attachment = plugin_spine_attachment_it_next(&attachment_it))) {
        struct spAttachment * sp_attachment = plugin_spine_data_attachment_attachment(attachment);
		spSlot* slot;
        const char * str_info;
        const char * end;
        ui_sprite_spine_chipmunk_with_tri_binding_t binding;
        char buf[128];
        uint8_t is_first = 1;
        
        if (!cpe_str_start_with(sp_attachment->name, with_tri->m_cfg_prefix)) continue;

        str_info = sp_attachment->name + prefix_len;

        slot = skeleton->slots[plugin_spine_data_attachment_slot_index(attachment)];
        
        binding = ui_sprite_spine_chipmunk_with_tri_binding_create(with_tri, tri_obj, slot, sp_attachment);

        while(is_first || str_info[0] == '_') {
            if (is_first) {
                is_first = 0;
            }
            else {
                str_info++;
            }

            if ((end = strchr(str_info, ']')) == NULL || cpe_str_dup_range(buf, sizeof(buf), str_info, end) == NULL) {
                CPE_ERROR(
                    module->m_em, "entity %d(%s): spine with tri: config str %s format error!",
                    ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), str_info);
                goto ENTER_FAIL;
            }
            str_info = end + 1;

            if (cpe_str_start_with(buf, "C[")) {
                if (ui_sprite_spine_chipmunk_with_tri_build_condition(
                        module, entity, spine_obj, with_tri, slot, binding->m_rule, buf + 2) != 0) {
                    goto ENTER_FAIL;
                }
            }
            else if (cpe_str_start_with(buf, "A[")) {
                if (ui_sprite_spine_chipmunk_with_tri_build_action(
                        module, entity, spine_obj, with_tri, slot, binding->m_rule, ui_sprite_tri_action_on_active, buf + 2) != 0)
                {
                    goto ENTER_FAIL;
                }
            }
            else if (cpe_str_start_with(buf, "D[")) {
                if (ui_sprite_spine_chipmunk_with_tri_build_action(
                        module, entity, spine_obj, with_tri, slot, binding->m_rule, ui_sprite_tri_action_on_deactive, buf + 2)
                    != 0)
                {
                    goto ENTER_FAIL;
                }
            }
            else if (cpe_str_start_with(buf, "R[")) {
                if (ui_sprite_spine_chipmunk_with_tri_build_action(
                        module, entity, spine_obj, with_tri, slot, binding->m_rule, ui_sprite_tri_action_in_active, buf + 2)
                    != 0)
                {
                    goto ENTER_FAIL;
                }
            }
            else if (cpe_str_start_with(buf, "E[")) {
                if (ui_sprite_spine_chipmunk_with_tri_build_trigger_event(module, entity, binding->m_rule, buf + 2) != 0) {
                    goto ENTER_FAIL;
                }
            }
            else if (cpe_str_start_with(buf, "P[")) {
                if (ui_sprite_spine_chipmunk_with_tri_build_trigger_attr(module, entity, binding->m_rule, buf + 2) != 0) {
                    goto ENTER_FAIL;
                }
            }
            else {
                break;
            }
        }
    }

    assert(with_tri->m_render_anim == NULL);
    with_tri->m_render_anim = render_anim;

    if(!TAILQ_EMPTY(&with_tri->m_bindings)) {
        ui_sprite_fsm_action_start_update(fsm_action);
    }

    mem_free(module->m_alloc, anim_name);

    return 0;

ENTER_FAIL:
    while(!TAILQ_EMPTY(&with_tri->m_bindings)) {
        ui_sprite_spine_chipmunk_with_tri_binding_free(TAILQ_FIRST(&with_tri->m_bindings));
    }

    with_tri->m_render_anim = NULL;

    if (anim_name) {
        mem_free(module->m_alloc, anim_name);
        anim_name = NULL;
    }
    
    return -1;
}

static void ui_sprite_spine_chipmunk_with_tri_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_chipmunk_with_tri_t with_tri = (ui_sprite_spine_chipmunk_with_tri_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_spine_chipmunk_with_tri_binding_t binding;

    TAILQ_FOREACH(binding, &with_tri->m_bindings, m_next) {
        ui_sprite_spine_chipmunk_with_tri_binding_update(binding);
    }
}

static void ui_sprite_spine_chipmunk_with_tri_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_chipmunk_with_tri_t with_tri = (ui_sprite_spine_chipmunk_with_tri_t)ui_sprite_fsm_action_data(fsm_action);

    assert(with_tri->m_render_anim);

    while(!TAILQ_EMPTY(&with_tri->m_bindings)) {
        ui_sprite_spine_chipmunk_with_tri_binding_free(TAILQ_FIRST(&with_tri->m_bindings));
    }

    with_tri->m_render_anim = NULL;
}

static int ui_sprite_spine_chipmunk_with_tri_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_chipmunk_with_tri_t with_tri = (ui_sprite_spine_chipmunk_with_tri_t)ui_sprite_fsm_action_data(fsm_action);
	bzero(with_tri, sizeof(*with_tri));
    with_tri->m_module = (ui_sprite_spine_chipmunk_module_t)ctx;
    with_tri->m_cfg_anim_name = NULL;
    with_tri->m_cfg_prefix = NULL;
    with_tri->m_render_anim = NULL;
    TAILQ_INIT(&with_tri->m_bindings);
    
    return 0;
}

static void ui_sprite_spine_chipmunk_with_tri_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_tri_t with_tri = (ui_sprite_spine_chipmunk_with_tri_t)ui_sprite_fsm_action_data(fsm_action);

    assert(with_tri->m_render_anim == NULL);
    assert(TAILQ_EMPTY(&with_tri->m_bindings));

    if (with_tri->m_cfg_anim_name) {
        mem_free(module->m_alloc, with_tri->m_cfg_anim_name);
        with_tri->m_cfg_anim_name = NULL;
    }

    if (with_tri->m_cfg_prefix) {
        mem_free(module->m_alloc, with_tri->m_cfg_prefix);
        with_tri->m_cfg_prefix = NULL;
    }
}

static int ui_sprite_spine_chipmunk_with_tri_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_tri_t to_with_tri = (ui_sprite_spine_chipmunk_with_tri_t)ui_sprite_fsm_action_data(to);
    ui_sprite_spine_chipmunk_with_tri_t from_with_tri = (ui_sprite_spine_chipmunk_with_tri_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_spine_chipmunk_with_tri_init(to, ctx)) return -1;

    if (from_with_tri->m_cfg_anim_name) {
        to_with_tri->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_with_tri->m_cfg_anim_name);
    }
    
    if (from_with_tri->m_cfg_prefix) {
        to_with_tri->m_cfg_prefix = cpe_str_mem_dup(module->m_alloc, from_with_tri->m_cfg_prefix);
    }

    return 0;
}

static ui_sprite_fsm_action_t ui_sprite_spine_chipmunk_with_tri_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_tri_t spine_chipmunk_with_tri = ui_sprite_spine_chipmunk_with_tri_create(fsm_state, name);
    const char * str_value;
    
    if (spine_chipmunk_with_tri == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine-with-tri action: create fail!", ui_sprite_spine_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_spine_chipmunk_with_tri_set_anim_name(spine_chipmunk_with_tri, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine-with-tri action: set anim name %s fail!",
                ui_sprite_spine_chipmunk_module_name(module), str_value);
            ui_sprite_spine_chipmunk_with_tri_free(spine_chipmunk_with_tri);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create spine-with-tri: anim-name not configured!",
            ui_sprite_spine_chipmunk_module_name(module));
        ui_sprite_spine_chipmunk_with_tri_free(spine_chipmunk_with_tri);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_spine_chipmunk_with_tri_set_prefix(spine_chipmunk_with_tri, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine-with-tri action: set prefix %s fail!",
                ui_sprite_spine_chipmunk_module_name(module), str_value);
            ui_sprite_spine_chipmunk_with_tri_free(spine_chipmunk_with_tri);
            return NULL;
        }
    }

    return ui_sprite_fsm_action_from_data(spine_chipmunk_with_tri);
}

int ui_sprite_spine_chipmunk_with_tri_regist(ui_sprite_spine_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_WITH_TRI_NAME, sizeof(struct ui_sprite_spine_chipmunk_with_tri));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: spine-with-tri register: meta create fail",
            ui_sprite_spine_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_chipmunk_with_tri_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_chipmunk_with_tri_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_chipmunk_with_tri_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_chipmunk_with_tri_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_chipmunk_with_tri_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_chipmunk_with_tri_update, module);
    
    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_WITH_TRI_NAME, ui_sprite_spine_chipmunk_with_tri_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_chipmunk_with_tri_unregist(ui_sprite_spine_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_WITH_TRI_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: spine-with-tri unregister: meta not exist",
            ui_sprite_spine_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_SPINE_WITH_TRI_NAME = "spine-with-tri";

#ifdef __cplusplus
}
#endif
