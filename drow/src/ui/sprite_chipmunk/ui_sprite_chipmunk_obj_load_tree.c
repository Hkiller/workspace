#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_load_ctx.h"
#include "ui_sprite_chipmunk_obj_constraint_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_obj_load_body_tree_from_res(
    ui_sprite_chipmunk_obj_t chipmunk_obj,
    const char * res, const char * body_name, ui_vector_2 const * pos, uint8_t is_runtime)
{
    ui_sprite_chipmunk_module_t module = chipmunk_obj->m_env->m_module;
    ui_sprite_component_t component = ui_sprite_component_from_data(chipmunk_obj);
    ui_sprite_entity_t entity = ui_sprite_component_entity(component);
    ui_data_src_t chipmunk_src;
    plugin_chipmunk_data_scene_t scene;
    
    chipmunk_src = ui_data_src_find_by_path(
        plugin_chipmunk_module_data_mgr(module->m_chipmunk_module), res, ui_data_src_type_chipmunk_scene);
    if (chipmunk_src == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk obj: find chipmunk scene %s fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
        return NULL;
    }

    scene = (plugin_chipmunk_data_scene_t)ui_data_src_product(chipmunk_src);
    if (scene == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk obj: chipmunk scene %s not loaded!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), res);
        return NULL;
    }

    return ui_sprite_chipmunk_obj_load_body_tree(chipmunk_obj, scene, body_name, pos, is_runtime);
}

ui_sprite_chipmunk_obj_body_t
ui_sprite_chipmunk_obj_load_body_tree(
    ui_sprite_chipmunk_obj_t chipmunk_obj,
    plugin_chipmunk_data_scene_t scene,
    const char * body_name, ui_vector_2 const * pos, uint8_t is_runtime)
{
    struct ui_sprite_chipmunk_obj_load_ctx build_ctx;
    ui_sprite_chipmunk_obj_body_t root_body = NULL;
    int rv = -1;
    ui_sprite_chipmunk_obj_body_load_info_t body_load_info;
    ui_sprite_chipmunk_obj_constraint_info_t constraint_info;
    struct ui_sprite_chipmunk_obj_body_load_info * main_body_load_info;
    CHIPMUNK_BODY const * main_body_data;
    ui_vector_2 adj_pos;

    ui_sprite_chipmunk_obj_load_ctx_init(&build_ctx, chipmunk_obj);

    if (ui_sprite_chipmunk_obj_load_ctx_load_loaded_bodies(&build_ctx) != 0
        || ui_sprite_chipmunk_obj_load_ctx_load_scene_bodies(&build_ctx, scene) != 0
        || ui_sprite_chipmunk_obj_load_ctx_load_scene_joins(&build_ctx, scene) != 0
        || ui_sprite_chipmunk_obj_load_ctx_load_tag_load_state(&build_ctx, body_name) != 0)
    {
        goto COMPLETE;
    }

    main_body_load_info = ui_sprite_chipmunk_obj_load_ctx_find_body_load_info_by_name(&build_ctx, body_name);
    assert(main_body_load_info);

    main_body_data = plugin_chipmunk_data_body_data(main_body_load_info->m_body_data);
    adj_pos = *pos;
    adj_pos.x -= main_body_data->anchorpoint.x;
    adj_pos.y -= main_body_data->anchorpoint.y;
        
    /*创建Body */
    TAILQ_FOREACH(body_load_info, &build_ctx.m_body_load_infos, m_next) {
        if (body_load_info->m_state != ui_sprite_chipmunk_obj_body_load_state_need_load) continue;

        body_load_info->m_body = ui_sprite_chipmunk_obj_body_create_from_data(chipmunk_obj, body_load_info->m_body_data, is_runtime);
        if (body_load_info->m_body == NULL) {
            CPE_ERROR(
                build_ctx.m_module->m_em, "entity %d(%s): chipmunk obj: chipmunk body %s(%d) create fail!",
                ui_sprite_entity_id(build_ctx.m_entity), ui_sprite_entity_name(build_ctx.m_entity),
                body_load_info->m_body_name, body_load_info->m_body_id);
            goto COMPLETE;
        }

        body_load_info->m_body->m_body_attrs.m_position.x += adj_pos.x;
        body_load_info->m_body->m_body_attrs.m_position.y += adj_pos.y;
        
        if (strcmp(body_name, body_load_info->m_body_name) == 0) root_body = body_load_info->m_body;
    }

    /*创建Joint */
    TAILQ_FOREACH(constraint_info, &build_ctx.m_constraint_infos, m_next_for_ctx) {
        CHIPMUNK_CONSTRAINT const * constraint_data;
        
        if (!constraint_info->m_need_process) continue;

        constraint_data = plugin_chipmunk_data_constraint_data(constraint_info->m_constraint_data);
        
        if (ui_sprite_chipmunk_obj_constraint_create(
                constraint_info->body_a->m_body, constraint_info->body_b->m_body,
                constraint_data->name, constraint_data->constraint_type, &constraint_data->constraint_data, is_runtime)
            == NULL)
        {
            CPE_ERROR(
                build_ctx.m_module->m_em, "entity %d(%s): chipmunk obj: chipmunk constraint create fail!",
                ui_sprite_entity_id(build_ctx.m_entity), ui_sprite_entity_name(build_ctx.m_entity));
            goto COMPLETE;
        }
    }

    rv = 0;

COMPLETE:
    ui_sprite_chipmunk_obj_load_ctx_fini(&build_ctx);

    return rv == 0 ? root_body : NULL;
}

#ifdef __cplusplus
}
#endif
