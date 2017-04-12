#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "ui/sprite/ui_sprite_world.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui_sprite_chipmunk_with_collision_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_shape_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_collision_body_t
ui_sprite_chipmunk_with_collision_body_create(
    ui_sprite_entity_t entity, ui_sprite_chipmunk_obj_t chipmunk_obj,
    ui_sprite_chipmunk_with_collision_t with_collision,
    ui_sprite_2d_transform_t transform, cpSpace * space,
    plugin_chipmunk_data_body_t data_body, ui_sprite_chipmunk_with_collision_src_t src)
{
    ui_sprite_chipmunk_module_t module = with_collision->m_module;
    ui_sprite_chipmunk_obj_body_t body = NULL;
    ui_sprite_chipmunk_with_collision_body_t with_collision_body;
    ui_sprite_chipmunk_with_collision_shape_t with_collision_shape;

    if (data_body) {
        body = ui_sprite_chipmunk_obj_body_create_from_data(chipmunk_obj, data_body, 0);
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with collision: create body from data fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return NULL;
        }
    }
    else {
        body = ui_sprite_chipmunk_obj_body_create(chipmunk_obj, 0, src->m_name, 0);
        if (body == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with collision: create body fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return NULL;
        }
    }

    TAILQ_FOREACH(with_collision_shape, &src->m_shapes, m_next) {
        ui_sprite_chipmunk_obj_shape_t obj_shape = ui_sprite_chipmunk_obj_shape_create_mamaged_from_body(body);
        if (obj_shape == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with collision: create managed shape fail!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            ui_sprite_chipmunk_obj_body_free(body);
            return NULL;
        }

        if (ui_sprite_chipmunk_with_collision_shape_calc_data(with_collision_shape, obj_shape->m_fixture_data) != 0) {
            ui_sprite_chipmunk_obj_body_free(body);
            return NULL;
        }
    }
    
    body->m_body_attrs = src->m_body_attrs;

    if (ui_sprite_chipmunk_obj_body_add_to_space_i(body, space, transform) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with collision: set main body fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_chipmunk_obj_body_remove_from_space(body);
        ui_sprite_chipmunk_obj_body_free(body);
        return NULL;
    }

    with_collision_body = (ui_sprite_chipmunk_with_collision_body_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_with_collision_body));
    if (with_collision_body == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with collision: alloc collision body fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));

        if (src->m_is_main) ui_sprite_component_obj_set_main_body(chipmunk_obj, NULL);
        ui_sprite_chipmunk_obj_body_remove_from_space(body);
        ui_sprite_chipmunk_obj_body_free(body);

        return NULL;
    }

    TAILQ_INSERT_TAIL(&with_collision->m_bodies, with_collision_body, m_next);
    with_collision_body->m_loaded_body = body;

    return with_collision_body;
}

void ui_sprite_chipmunk_with_collision_body_free(
    ui_sprite_chipmunk_with_collision_t with_collision, ui_sprite_chipmunk_with_collision_body_t body)
{
    ui_sprite_chipmunk_obj_body_remove_from_space(body->m_loaded_body);
    ui_sprite_chipmunk_obj_body_free(body->m_loaded_body);
    TAILQ_REMOVE(&with_collision->m_bodies, body, m_next);
    mem_free(with_collision->m_module->m_alloc, body);
}

#ifdef __cplusplus
}
#endif
