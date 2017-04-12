#include <assert.h>
#include "spine/Skeleton.h"
#include "spine/BoundingBoxAttachment.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "render/utils/ui_transform.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/spine/plugin_spine_obj.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_sch.h"
#include "ui/sprite_render/ui_sprite_render_anim.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_env.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_obj.h"
#include "ui/sprite_spine/ui_sprite_spine_utils.h"
#include "ui_sprite_spine_chipmunk_with_collision_i.h"
#include "ui_sprite_spine_chipmunk_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

static void ui_sprite_spine_chipmunk_with_collision_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s);

ui_sprite_spine_chipmunk_with_collision_t ui_sprite_spine_chipmunk_with_collision_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_SPINE_WITH_COLLISION_NAME);
    return (ui_sprite_spine_chipmunk_with_collision_t)(fsm_action ? ui_sprite_fsm_action_data(fsm_action) : NULL);
}

void ui_sprite_spine_chipmunk_with_collision_free(ui_sprite_spine_chipmunk_with_collision_t with_collision) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_collision);
    ui_sprite_fsm_action_free(fsm_action);
}

int ui_sprite_spine_chipmunk_with_collision_set_anim_name(ui_sprite_spine_chipmunk_with_collision_t with_collision, const char * anim_name) {
    assert(anim_name);

    if (with_collision->m_cfg_anim_name) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_anim_name);
    }

    with_collision->m_cfg_anim_name = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, anim_name);
    
    return 0;
}

int ui_sprite_spine_chipmunk_with_collision_set_prefix(ui_sprite_spine_chipmunk_with_collision_t with_collision, const char * prefix) {
    assert(prefix);

    if (with_collision->m_cfg_prefix) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_prefix);
    }

    with_collision->m_cfg_prefix = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, prefix);
    
    return 0;
}

int ui_sprite_spine_chipmunk_with_collision_set_name_sep(ui_sprite_spine_chipmunk_with_collision_t with_collision, const char * name_sep) {
    assert(name_sep);

    if (with_collision->m_cfg_name_sep) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_name_sep);
    }

    with_collision->m_cfg_name_sep = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, name_sep);
    
    return 0;
}

int ui_sprite_spine_chipmunk_with_collision_set_collision_mask(
    ui_sprite_spine_chipmunk_with_collision_t with_collision, const char * collision_mask)
{
    assert(collision_mask);

    if (with_collision->m_cfg_collision_mask) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_collision_mask);
    }

    with_collision->m_cfg_collision_mask = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, collision_mask);
    
    return 0;
}

int ui_sprite_spine_chipmunk_with_collision_set_collision_category(
    ui_sprite_spine_chipmunk_with_collision_t with_collision, const char * collision_category)
{
    assert(collision_category);

    if (with_collision->m_cfg_collision_category) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_collision_category);
    }

    with_collision->m_cfg_collision_category = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, collision_category);
    
    return 0;
}

int ui_sprite_spine_chipmunk_with_collision_set_collision_group(
    ui_sprite_spine_chipmunk_with_collision_t with_collision, const char * collision_group)
{
    assert(collision_group);

    if (with_collision->m_cfg_collision_group) {
        mem_free(with_collision->m_module->m_alloc, with_collision->m_cfg_collision_group);
    }

    with_collision->m_cfg_collision_group = cpe_str_mem_dup_trim(with_collision->m_module->m_alloc, collision_group);
    
    return 0;
}

static int ui_sprite_spine_chipmunk_with_collision_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_collision_t with_collision = (ui_sprite_spine_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    plugin_chipmunk_env_t chipmunk_env;
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

    render_sch = ui_sprite_render_sch_find(entity);
    if (render_sch == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: entity no anim sch!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    anim_name = ui_sprite_fsm_action_check_calc_str_dup(module->m_alloc, with_collision->m_cfg_anim_name, fsm_action, NULL, module->m_em);
    if (anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: calc anim name from %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_anim_name);
        goto ENTER_FAIL;
    }
    
    render_anim = ui_sprite_render_anim_find_by_name(render_sch, anim_name);
    if (render_anim == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: runing anim %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), anim_name);
        goto ENTER_FAIL;
    }

    render_obj_ref = ui_sprite_render_anim_obj(render_anim);
    assert(render_obj_ref);

    render_obj = ui_runtime_render_obj_ref_obj(render_obj_ref);
    assert(render_obj);
    
    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: render obj %s is not spine obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_anim_name);
        goto ENTER_FAIL;
    }

    spine_obj = (plugin_spine_obj_t)ui_runtime_render_obj_data(render_obj);
    if (spine_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: render obj %s no data!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_anim_name);
        goto ENTER_FAIL;
    }
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: no chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    chipmunk_env = ui_sprite_chipmunk_env_env(ui_sprite_chipmunk_obj_env(chipmunk_obj));
    assert(chipmunk_env);
    
    /*计算碰撞信息 */
    if (with_collision->m_cfg_collision_category) {
        if (plugin_chipmunk_env_masks(chipmunk_env, &with_collision->m_collision_category, with_collision->m_cfg_collision_category) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with collision: load category from '%s' fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_collision_category);
            goto ENTER_FAIL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: collision.category not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    if (with_collision->m_cfg_collision_mask) {
        if (plugin_chipmunk_env_masks(chipmunk_env, &with_collision->m_collision_mask, with_collision->m_cfg_collision_mask) != 0) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with collision: load mask from '%s' fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_collision_mask);
            goto ENTER_FAIL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: collision.mask not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    if (with_collision->m_cfg_collision_group) {
        if (ui_sprite_fsm_action_check_calc_uint32(
                &with_collision->m_collision_group, with_collision->m_cfg_collision_group, fsm_action, NULL, module->m_em)
            != 0)
        {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with collision: calc collision.group from %s fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_collision_group);
            goto ENTER_FAIL;
        }
    }
    
    /*获取目标Spine */
    if (with_collision->m_cfg_anim_name == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: anim-name not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    if (ui_runtime_render_obj_type_id(render_obj) != UI_OBJECT_TYPE_SKELETON) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: spine obj %s not exist!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_collision->m_cfg_anim_name);
        goto ENTER_FAIL;
    }

	skeleton = plugin_spine_obj_skeleton(spine_obj);

    if (with_collision->m_cfg_prefix == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: prefix not configured!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_FAIL;
    }

    prefix_len = strlen(with_collision->m_cfg_prefix);
    
    plugin_spine_skin_attachments(&attachment_it, skeleton->skin ? skeleton->skin : skeleton->data->defaultSkin);
    
    while((attachment = plugin_spine_attachment_it_next(&attachment_it))) {
        struct spAttachment * sp_attachment = plugin_spine_data_attachment_attachment(attachment);
		spSlot* slot;
        ui_sprite_spine_chipmunk_body_t body;
        const char * name;
        char name_buf[64];
        
		if (sp_attachment->type != SP_ATTACHMENT_BOUNDING_BOX) continue;
        if (!cpe_str_start_with(sp_attachment->name, with_collision->m_cfg_prefix)) continue;

        slot = skeleton->slots[plugin_spine_data_attachment_slot_index(attachment)];
        
        name = sp_attachment->name + prefix_len;
        if (with_collision->m_cfg_name_sep) {
            const char * sep = strstr(name, with_collision->m_cfg_name_sep);
            if (sep) {
                size_t len = sep - name;
                
                if (len + 1 > CPE_ARRAY_SIZE(name_buf)) {
                    CPE_ERROR(
                        module->m_em, "entity %d(%s): spine with collision: binding body name %s len overflow!",
                        ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), name);
                    goto ENTER_FAIL;
                }

                memcpy(name_buf, name, len);
                name_buf[len] = 0;
                name = name_buf;
            }
        }

        body = ui_sprite_spine_chipmunk_body_create(with_collision, name, slot, sp_attachment);
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): spine with collision: create binding body fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            goto ENTER_FAIL;
        }
    }

    assert(with_collision->m_render_anim == NULL);
    with_collision->m_render_anim = render_anim;

    ui_sprite_fsm_action_start_update(fsm_action);

    ui_sprite_spine_chipmunk_with_collision_update(fsm_action, ctx, 0.0f);

    mem_free(module->m_alloc, anim_name);
    
    return 0;

ENTER_FAIL:
    while(!TAILQ_EMPTY(&with_collision->m_bodies)) {
        ui_sprite_spine_chipmunk_body_free(TAILQ_FIRST(&with_collision->m_bodies));
    }
    with_collision->m_render_anim = NULL;

    if (anim_name) {
        mem_free(module->m_alloc, anim_name);
        anim_name = NULL;
    }
    
    return -1;
}

static void ui_sprite_spine_chipmunk_with_collision_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_chipmunk_with_collision_t with_collision = (ui_sprite_spine_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);

    assert(with_collision->m_render_anim);

    while(!TAILQ_EMPTY(&with_collision->m_bodies)) {
        ui_sprite_spine_chipmunk_body_free(TAILQ_FIRST(&with_collision->m_bodies));
    }

    while(!TAILQ_EMPTY(&with_collision->m_collided_bodies)) {
        ui_sprite_spine_chipmunk_body_free(TAILQ_FIRST(&with_collision->m_collided_bodies));
    }
    
    with_collision->m_render_anim = NULL;
}

static int ui_sprite_spine_chipmunk_with_collision_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_chipmunk_with_collision_t with_collision = (ui_sprite_spine_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
	bzero(with_collision, sizeof(*with_collision));
    with_collision->m_module = (ui_sprite_spine_chipmunk_module_t)ctx;
    with_collision->m_cfg_anim_name = NULL;
    with_collision->m_cfg_prefix = NULL;
    with_collision->m_cfg_name_sep = NULL;
    with_collision->m_cfg_collision_group = NULL;
    with_collision->m_cfg_collision_category = NULL;
    with_collision->m_cfg_collision_mask = NULL;
    with_collision->m_render_anim = NULL;
    TAILQ_INIT(&with_collision->m_bodies);
    TAILQ_INIT(&with_collision->m_collided_bodies);
    return 0;
}

static void ui_sprite_spine_chipmunk_with_collision_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_collision_t with_collision = (ui_sprite_spine_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);

    assert(with_collision->m_render_anim == NULL);
    assert(TAILQ_EMPTY(&with_collision->m_bodies));
    assert(TAILQ_EMPTY(&with_collision->m_collided_bodies));
    
    if (with_collision->m_cfg_anim_name) {
        mem_free(module->m_alloc, with_collision->m_cfg_anim_name);
        with_collision->m_cfg_anim_name = NULL;
    }

    if (with_collision->m_cfg_prefix) {
        mem_free(module->m_alloc, with_collision->m_cfg_prefix);
        with_collision->m_cfg_prefix = NULL;
    }

    if (with_collision->m_cfg_name_sep) {
        mem_free(module->m_alloc, with_collision->m_cfg_name_sep);
        with_collision->m_cfg_name_sep = NULL;
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
}

static int ui_sprite_spine_chipmunk_with_collision_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_collision_t to_with_collision = (ui_sprite_spine_chipmunk_with_collision_t)ui_sprite_fsm_action_data(to);
    ui_sprite_spine_chipmunk_with_collision_t from_with_collision = (ui_sprite_spine_chipmunk_with_collision_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_spine_chipmunk_with_collision_init(to, ctx)) return -1;

    if (from_with_collision->m_cfg_anim_name) {
        to_with_collision->m_cfg_anim_name = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_anim_name);
    }
    
    if (from_with_collision->m_cfg_prefix) {
        to_with_collision->m_cfg_prefix = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_prefix);
    }

    if (from_with_collision->m_cfg_name_sep) {
        to_with_collision->m_cfg_name_sep = cpe_str_mem_dup(module->m_alloc, from_with_collision->m_cfg_name_sep);
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
    
    return 0;
}

static void ui_sprite_spine_chipmunk_with_collision_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta_s) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_collision_t with_collision = (ui_sprite_spine_chipmunk_with_collision_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_spine_chipmunk_body_t body;
    ui_transform entity_trans;
    ui_transform local_transform;

    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): spine with collision: no chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_fsm_action_stop_update(fsm_action);
        return;
    }

    local_transform = *ui_runtime_render_obj_ref_transform(ui_sprite_render_anim_obj(with_collision->m_render_anim));
    ui_transform_adj_by_parent(&local_transform, ui_sprite_render_anim_transform(with_collision->m_render_anim));

    if (ui_sprite_2d_transform_calc_trans(ui_sprite_2d_transform_find(entity), &entity_trans) != 0) {
        return;
    }
    
    ui_transform_inline_reverse(&entity_trans);
    ui_transform_adj_by_parent(&local_transform, &entity_trans);

    TAILQ_FOREACH(body, &with_collision->m_bodies, m_next) {
        ui_sprite_spine_chipmunk_body_update(chipmunk_obj, body, &local_transform);
    }
    
    while(!TAILQ_EMPTY(&with_collision->m_collided_bodies)) {
        ui_sprite_spine_chipmunk_body_free(TAILQ_FIRST(&with_collision->m_collided_bodies));
    }
}
    
static ui_sprite_fsm_action_t ui_sprite_spine_chipmunk_with_collision_load(void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg) {
    ui_sprite_spine_chipmunk_module_t module = (ui_sprite_spine_chipmunk_module_t)ctx;
    ui_sprite_spine_chipmunk_with_collision_t spine_chipmunk_with_collision = ui_sprite_spine_chipmunk_with_collision_create(fsm_state, name);
    const char * str_value;
    
    if (spine_chipmunk_with_collision == NULL) {
        CPE_ERROR(module->m_em, "%s: create spine-with-collision action: create fail!", ui_sprite_spine_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "anim-name", NULL))) {
        if (ui_sprite_spine_chipmunk_with_collision_set_anim_name(spine_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine-with-collision action: set anim name %s fail!",
                ui_sprite_spine_chipmunk_module_name(module), str_value);
            ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create spine-with-collision: anim-name not configured!",
            ui_sprite_spine_chipmunk_module_name(module));
        ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "prefix", NULL))) {
        if (ui_sprite_spine_chipmunk_with_collision_set_prefix(spine_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine-with-collision action: set prefix %s fail!",
                ui_sprite_spine_chipmunk_module_name(module), str_value);
            ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "name-sep", NULL))) {
        if (ui_sprite_spine_chipmunk_with_collision_set_name_sep(spine_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine-with-collision action: set name sep %s fail!",
                ui_sprite_spine_chipmunk_module_name(module), str_value);
            ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
            return NULL;
        }
    }
    
    if ((str_value = cfg_get_string(cfg, "collision.category", NULL))) {
        if (ui_sprite_spine_chipmunk_with_collision_set_collision_category(spine_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine with collision action: set collision category %s fail!",
                ui_sprite_spine_chipmunk_module_name(module), str_value);
            ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create spine-with-collision: collision.category not configured!",
            ui_sprite_spine_chipmunk_module_name(module));
        ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "collision.mask", NULL))) {
        if (ui_sprite_spine_chipmunk_with_collision_set_collision_mask(spine_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine with collision action: set collision mask %s fail!",
                ui_sprite_spine_chipmunk_module_name(module), str_value);
            ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
            return NULL;
        }
    }
    else {
        CPE_ERROR(
            module->m_em, "%s: create spine-with-collision: collision.mask not configured!",
            ui_sprite_spine_chipmunk_module_name(module));
        ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "collision.group", NULL))) {
        if (ui_sprite_spine_chipmunk_with_collision_set_collision_group(spine_chipmunk_with_collision, str_value) != 0) {
            CPE_ERROR(
                module->m_em, "%s: create spine with collision action: set collision group %s fail!",
                ui_sprite_spine_chipmunk_module_name(module), str_value);
            ui_sprite_spine_chipmunk_with_collision_free(spine_chipmunk_with_collision);
            return NULL;
        }
    }
    
    return ui_sprite_fsm_action_from_data(spine_chipmunk_with_collision);
}

int ui_sprite_spine_chipmunk_with_collision_regist(ui_sprite_spine_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_SPINE_WITH_COLLISION_NAME, sizeof(struct ui_sprite_spine_chipmunk_with_collision));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: spine-with-collision register: meta create fail",
            ui_sprite_spine_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_spine_chipmunk_with_collision_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_spine_chipmunk_with_collision_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_spine_chipmunk_with_collision_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_spine_chipmunk_with_collision_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_spine_chipmunk_with_collision_clear, module);
    ui_sprite_fsm_action_meta_set_update_fun(meta, ui_sprite_spine_chipmunk_with_collision_update, module);
    
    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_SPINE_WITH_COLLISION_NAME, ui_sprite_spine_chipmunk_with_collision_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_spine_chipmunk_with_collision_unregist(ui_sprite_spine_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_SPINE_WITH_COLLISION_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: spine-with-collision unregister: meta not exist",
            ui_sprite_spine_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);
}

const char * UI_SPRITE_SPINE_WITH_COLLISION_NAME = "spine-with-collision";

#ifdef __cplusplus
}
#endif
