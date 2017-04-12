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
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "ui/sprite_fsm/ui_sprite_fsm_ins.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui/sprite_render/ui_sprite_render_env.h"
#include "ui_sprite_chipmunk_with_boundary_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"
#include "ui_sprite_chipmunk_obj_shape_group_i.h"
#include "ui_sprite_chipmunk_obj_shape_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_with_boundary_t ui_sprite_chipmunk_with_boundary_create(ui_sprite_fsm_state_t fsm_state, const char * name) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_create(fsm_state, name, UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_NAME);
    return fsm_action ? (ui_sprite_chipmunk_with_boundary_t)ui_sprite_fsm_action_data(fsm_action) : NULL;
}

void ui_sprite_chipmunk_with_boundary_free(ui_sprite_chipmunk_with_boundary_t with_boundary) {
    ui_sprite_fsm_action_t fsm_action = ui_sprite_fsm_action_from_data(with_boundary);
    ui_sprite_fsm_action_free(fsm_action);
}

static int ui_sprite_chipmunk_with_boundary_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_with_boundary_t with_boundary = (ui_sprite_chipmunk_with_boundary_t)ui_sprite_fsm_action_data(fsm_action);
    ui_sprite_entity_t entity = ui_sprite_fsm_action_to_entity(fsm_action);
    cpSpace * space;
    ui_sprite_chipmunk_obj_t chipmunk_obj;
    ui_sprite_chipmunk_obj_shape_group_t shape_group;
    ui_rect source_rect;
    
    chipmunk_obj = ui_sprite_chipmunk_obj_find(entity);
    if (chipmunk_obj == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with boundary: not chipmunk obj!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    ui_sprite_2d_transform_t transform = ui_sprite_2d_transform_find(entity);
    if (transform == NULL) {
        CPE_ERROR(
                  module->m_em, "entity %d(%s): chipmunk with boundary: no transform!",
                  ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    if (with_boundary->m_source_rect == ui_sprite_chipmunk_with_boundary_source_rect_entity) {
        source_rect = ui_sprite_2d_transform_rect(transform);
    }
    else if (with_boundary->m_source_rect == ui_sprite_chipmunk_with_boundary_source_rect_camera) {
        ui_sprite_render_env_t render_env = ui_sprite_render_env_find(ui_sprite_entity_world(entity));
        ui_vector_2_t screen_size;
        if (render_env == NULL) {
            CPE_ERROR(
                module->m_em, "entity %d(%s): chipmunk with boundary: no transform!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
            return -1;
        }

        screen_size = ui_sprite_render_env_size(render_env);
        
        source_rect.lt = UI_VECTOR_2_ZERO;
        source_rect.rb.x = source_rect.lt.x + screen_size->x;
        source_rect.rb.y = source_rect.lt.y + screen_size->y;
    }
    else {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with boundary: unknown source rect type %d!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity), with_boundary->m_source_rect);
        return -1;
    }

    space = (cpSpace *)plugin_chipmunk_env_space(chipmunk_obj->m_env->m_env);
    assert(space);

    assert(with_boundary->m_body == NULL);

    with_boundary->m_body = ui_sprite_chipmunk_obj_body_create(chipmunk_obj, 0, "", 1);
    if (with_boundary->m_body == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with boundary: create body fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        return -1;
    }

    with_boundary->m_body->m_body_attrs.m_runing_mode = ui_sprite_chipmunk_runing_mode_passive;
    with_boundary->m_body->m_body_attrs.m_is_main = 0;
    with_boundary->m_body->m_body_attrs.m_is_free = 0;

    with_boundary->m_body->m_body_attrs.m_category = with_boundary->m_category;
    with_boundary->m_body->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_CATEGORY;

    with_boundary->m_body->m_body_attrs.m_mask = with_boundary->m_mask;
    with_boundary->m_body->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_MASK;

    with_boundary->m_body->m_body_attrs.m_type = chipmunk_obj_type_kinematic;
    with_boundary->m_body->m_body_attrs.m_attr_flags |= 1u << CHIPMUNK_BODY_ATTR_ID_TYPE;

    shape_group = ui_sprite_chipmunk_obj_shape_group_create(with_boundary->m_body);
    if (shape_group == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with boundary: create body group fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        goto ENTER_ERROR;
    }

    if (with_boundary->m_top.m_need_build) {
        ui_sprite_chipmunk_obj_shape_t shape = ui_sprite_chipmunk_obj_shape_create_managed(shape_group);
        CHIPMUNK_FIXTURE * fixture_data = shape->m_fixture_data;
        bzero(fixture_data, sizeof(*fixture_data));
        fixture_data->elasticity = with_boundary->m_top.m_elasticity;
        fixture_data->friction = with_boundary->m_top.m_friction;
        fixture_data->fixture_type = chipmunk_fixture_type_segment;
        fixture_data->collision_mask = with_boundary->m_top.m_mask;
        fixture_data->collision_category = with_boundary->m_top.m_category;
        fixture_data->fixture_data.segment.radius = with_boundary->m_top.m_radius;
        fixture_data->fixture_data.segment.a.x = source_rect.lt.x + with_boundary->m_left.m_adj;
        fixture_data->fixture_data.segment.a.y = source_rect.lt.y + with_boundary->m_top.m_adj;
        fixture_data->fixture_data.segment.b.x = source_rect.rb.x + with_boundary->m_right.m_adj;
        fixture_data->fixture_data.segment.b.y = source_rect.lt.y + with_boundary->m_top.m_adj;

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): chipmunk with boundary: top: (%f,%f)-(%f,%f)!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                fixture_data->fixture_data.segment.a.x, fixture_data->fixture_data.segment.a.y,
                fixture_data->fixture_data.segment.b.x, fixture_data->fixture_data.segment.b.y);
        }
    }

    if (with_boundary->m_bottom.m_need_build) {
        ui_sprite_chipmunk_obj_shape_t shape = ui_sprite_chipmunk_obj_shape_create_managed(shape_group);
        CHIPMUNK_FIXTURE * fixture_data = shape->m_fixture_data;
        bzero(fixture_data, sizeof(*fixture_data));
        fixture_data->elasticity = with_boundary->m_bottom.m_elasticity;
        fixture_data->friction = with_boundary->m_bottom.m_friction;
        fixture_data->fixture_type = chipmunk_fixture_type_segment;
        fixture_data->collision_mask = with_boundary->m_bottom.m_mask;
        fixture_data->collision_category = with_boundary->m_bottom.m_category;
        fixture_data->fixture_data.segment.radius = with_boundary->m_bottom.m_radius;
        fixture_data->fixture_data.segment.a.x = source_rect.lt.x + with_boundary->m_left.m_adj;
        fixture_data->fixture_data.segment.a.y = source_rect.rb.y + with_boundary->m_bottom.m_adj;
        fixture_data->fixture_data.segment.b.x = source_rect.rb.x + with_boundary->m_right.m_adj;
        fixture_data->fixture_data.segment.b.y = source_rect.rb.y + with_boundary->m_bottom.m_adj;

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): chipmunk with boundary: bottom: (%f,%f)-(%f,%f)!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                fixture_data->fixture_data.segment.a.x, fixture_data->fixture_data.segment.a.y,
                fixture_data->fixture_data.segment.b.x, fixture_data->fixture_data.segment.b.y);
        }
    }

    if (with_boundary->m_left.m_need_build) {
        ui_sprite_chipmunk_obj_shape_t shape = ui_sprite_chipmunk_obj_shape_create_managed(shape_group);
        CHIPMUNK_FIXTURE * fixture_data = shape->m_fixture_data;
        bzero(fixture_data, sizeof(*fixture_data));
        fixture_data->elasticity = with_boundary->m_left.m_elasticity;
        fixture_data->friction = with_boundary->m_left.m_friction;
        fixture_data->fixture_type = chipmunk_fixture_type_segment;
        fixture_data->collision_mask = with_boundary->m_left.m_mask;
        fixture_data->collision_category = with_boundary->m_left.m_category;
        fixture_data->fixture_data.segment.radius = with_boundary->m_left.m_radius;
        fixture_data->fixture_data.segment.a.x = source_rect.lt.x + with_boundary->m_left.m_adj;
        fixture_data->fixture_data.segment.a.y = source_rect.lt.y + with_boundary->m_top.m_adj;
        fixture_data->fixture_data.segment.b.x = source_rect.lt.x + with_boundary->m_left.m_adj;
        fixture_data->fixture_data.segment.b.y = source_rect.rb.y + with_boundary->m_bottom.m_adj;

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): chipmunk with boundary: left: (%f,%f)-(%f,%f)!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                fixture_data->fixture_data.segment.a.x, fixture_data->fixture_data.segment.a.y,
                fixture_data->fixture_data.segment.b.x, fixture_data->fixture_data.segment.b.y);
        }
    }

    if (with_boundary->m_right.m_need_build) {
        ui_sprite_chipmunk_obj_shape_t shape = ui_sprite_chipmunk_obj_shape_create_managed(shape_group);
        CHIPMUNK_FIXTURE * fixture_data = shape->m_fixture_data;
        bzero(fixture_data, sizeof(*fixture_data));
        fixture_data->elasticity = with_boundary->m_right.m_elasticity;
        fixture_data->friction = with_boundary->m_right.m_friction;
        fixture_data->fixture_type = chipmunk_fixture_type_segment;
        fixture_data->collision_mask = with_boundary->m_right.m_mask;
        fixture_data->collision_category = with_boundary->m_right.m_category;
        fixture_data->fixture_data.segment.radius = with_boundary->m_right.m_radius;
        fixture_data->fixture_data.segment.a.x = source_rect.rb.x + with_boundary->m_right.m_adj;
        fixture_data->fixture_data.segment.a.y = source_rect.lt.y + with_boundary->m_top.m_adj;
        fixture_data->fixture_data.segment.b.x = source_rect.rb.x + with_boundary->m_right.m_adj;
        fixture_data->fixture_data.segment.b.y = source_rect.rb.y + with_boundary->m_bottom.m_adj;

        if (ui_sprite_entity_debug(entity)) {
            CPE_INFO(
                module->m_em, "entity %d(%s): chipmunk with boundary: right: (%f,%f)-(%f,%f)!",
                ui_sprite_entity_id(entity), ui_sprite_entity_name(entity),
                fixture_data->fixture_data.segment.a.x, fixture_data->fixture_data.segment.a.y,
                fixture_data->fixture_data.segment.b.x, fixture_data->fixture_data.segment.b.y);
        }
    }

    if (ui_sprite_chipmunk_obj_body_add_to_space_i(with_boundary->m_body, space, transform) != 0) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): chipmunk with boundary: set main body fail!",
            ui_sprite_entity_id(entity), ui_sprite_entity_name(entity));
        ui_sprite_chipmunk_obj_body_remove_from_space(with_boundary->m_body);
        goto ENTER_ERROR;
    }
    
    return 0;

ENTER_ERROR:
    ui_sprite_chipmunk_obj_body_free(with_boundary->m_body);
    with_boundary->m_body = NULL;

    return -1;
}

static void ui_sprite_chipmunk_with_boundary_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_boundary_t with_boundary = (ui_sprite_chipmunk_with_boundary_t)ui_sprite_fsm_action_data(fsm_action);

    ui_sprite_chipmunk_obj_body_remove_from_space(with_boundary->m_body);
    ui_sprite_chipmunk_obj_body_free(with_boundary->m_body);
    with_boundary->m_body = NULL;
}

static int ui_sprite_chipmunk_with_boundary_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_boundary_t with_boundary = (ui_sprite_chipmunk_with_boundary_t)ui_sprite_fsm_action_data(fsm_action);
    with_boundary->m_module = (ui_sprite_chipmunk_module_t)ctx;

    bzero(with_boundary, sizeof(*with_boundary));

    with_boundary->m_source_rect = ui_sprite_chipmunk_with_boundary_source_rect_entity;

    with_boundary->m_top.m_radius = 1.0f;

    with_boundary->m_bottom.m_radius = 1.0f;

    with_boundary->m_left.m_radius = 1.0f;

    with_boundary->m_right.m_radius = 1.0f;

    with_boundary->m_body = NULL;

    return 0;
}

static void ui_sprite_chipmunk_with_boundary_clear(ui_sprite_fsm_action_t fsm_action, void * ctx) {
    ui_sprite_chipmunk_with_boundary_t with_boundary = (ui_sprite_chipmunk_with_boundary_t)ui_sprite_fsm_action_data(fsm_action);
    assert(with_boundary->m_body == NULL);
}

static int ui_sprite_chipmunk_with_boundary_copy(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
    ui_sprite_chipmunk_with_boundary_t to_with_boundary = (ui_sprite_chipmunk_with_boundary_t)ui_sprite_fsm_action_data(to);
    ui_sprite_chipmunk_with_boundary_t from_with_boundary = (ui_sprite_chipmunk_with_boundary_t)ui_sprite_fsm_action_data(from);

    if (ui_sprite_chipmunk_with_boundary_init(to, ctx)) return -1;

    to_with_boundary->m_category = from_with_boundary->m_category;
    to_with_boundary->m_mask = from_with_boundary->m_mask;
    to_with_boundary->m_source_rect = from_with_boundary->m_source_rect;
    to_with_boundary->m_left = from_with_boundary->m_left;
    to_with_boundary->m_right = from_with_boundary->m_right;
    to_with_boundary->m_top = from_with_boundary->m_top;    
    to_with_boundary->m_bottom = from_with_boundary->m_bottom;

    return 0;
}

static
int ui_sprite_chipmunk_with_boundary_load_side(
    ui_sprite_chipmunk_env_t env, struct ui_sprite_chipmunk_with_boundary_side * side, cfg_t child_cfg)
{
    ui_sprite_chipmunk_module_t module = env->m_module;
    const char * str_value;

    side->m_need_build = cfg_get_float(child_cfg, "need-build", 1);
    side->m_radius = cfg_get_float(child_cfg, "radius", side->m_radius);
    side->m_adj = cfg_get_float(child_cfg, "rect-adj", side->m_adj);
    side->m_elasticity = cfg_get_float(child_cfg, "elasticity", side->m_elasticity);
    side->m_friction = cfg_get_float(child_cfg, "friction", side->m_friction);

    if ((str_value = cfg_get_string(child_cfg, "category", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &side->m_category, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create with_boundary action: category %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }

    if ((str_value = cfg_get_string(child_cfg, "mask", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &side->m_mask, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create with_boundary action: mask %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return -1;
        }
    }

    return 0;
}

static ui_sprite_fsm_action_t
ui_sprite_chipmunk_with_boundary_load(
    void * ctx, ui_sprite_fsm_state_t fsm_state, const char * name, cfg_t cfg)
{
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)ctx;
    ui_sprite_chipmunk_env_t env;
    ui_sprite_world_t world = ui_sprite_fsm_to_world(ui_sprite_fsm_state_fsm(fsm_state));
    ui_sprite_chipmunk_with_boundary_t with_boundary = (ui_sprite_chipmunk_with_boundary_t)ui_sprite_chipmunk_with_boundary_create(fsm_state, name);
    const char * str_value;
    cfg_t child_cfg;
    
    env = ui_sprite_chipmunk_env_find(world);
    if (env == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_boundary action: env not exist!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }
    
    if (with_boundary == NULL) {
        CPE_ERROR(module->m_em, "%s: create with_boundary action: create fail!", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if ((str_value = cfg_get_string(cfg, "category", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &with_boundary->m_category, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create with_boundary action: category %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "mask", NULL))) {
        if (plugin_chipmunk_env_masks(env->m_env, &with_boundary->m_mask, str_value) != 0) {
            CPE_ERROR(module->m_em, "%s: create with_boundary action: mask %s load fail!", ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }
    }

    if ((str_value = cfg_get_string(cfg, "source-rect", NULL))) {
        if (strcmp(str_value, "entity") == 0) {
            with_boundary->m_source_rect = ui_sprite_chipmunk_with_boundary_source_rect_entity;
        }
        else if (strcmp(str_value, "camera") == 0) {
            with_boundary->m_source_rect = ui_sprite_chipmunk_with_boundary_source_rect_camera;
        }
        else {
            CPE_ERROR(module->m_em, "%s: create with_boundary action: source-rect %s unknown!", ui_sprite_chipmunk_module_name(module), str_value);
            return NULL;
        }
    }
    
    if ((child_cfg = cfg_find_cfg(cfg, "rect-adj"))) {
        float adj = cfg_as_float(child_cfg, 0.0f);
        with_boundary->m_top.m_adj = - adj;
        with_boundary->m_left.m_adj = - adj;
        with_boundary->m_right.m_adj = adj;
        with_boundary->m_bottom.m_adj =  adj;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "elasticity"))) {
        float elasticity = cfg_as_float(child_cfg, 0.0f);
        with_boundary->m_top.m_elasticity =
            with_boundary->m_left.m_elasticity =
            with_boundary->m_right.m_elasticity =
            with_boundary->m_bottom.m_elasticity = elasticity;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "friction"))) {
        float friction = cfg_as_float(child_cfg, 0.0f);
        with_boundary->m_top.m_friction =
            with_boundary->m_left.m_friction =
            with_boundary->m_right.m_friction =
            with_boundary->m_bottom.m_friction = friction;
    }
    
    if ((child_cfg = cfg_find_cfg(cfg, "top"))) {
        if (ui_sprite_chipmunk_with_boundary_load_side(env, &with_boundary->m_top, child_cfg) != 0) return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "left"))) {
        if (ui_sprite_chipmunk_with_boundary_load_side(env, &with_boundary->m_left, child_cfg) != 0) return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "right"))) {
        if (ui_sprite_chipmunk_with_boundary_load_side(env, &with_boundary->m_right, child_cfg) != 0) return NULL;
    }

    if ((child_cfg = cfg_find_cfg(cfg, "bottom"))) {
        if (ui_sprite_chipmunk_with_boundary_load_side(env, &with_boundary->m_bottom, child_cfg) != 0) return NULL;
    }

    if (with_boundary->m_top.m_need_build == 0
        && with_boundary->m_left.m_need_build == 0
        && with_boundary->m_right.m_need_build == 0
        && with_boundary->m_bottom.m_need_build == 0)
    {
        with_boundary->m_top.m_need_build = 1;
        with_boundary->m_left.m_need_build = 1;
        with_boundary->m_right.m_need_build = 1;
        with_boundary->m_bottom.m_need_build = 1;
    }

    return ui_sprite_fsm_action_from_data(with_boundary);
}

    
int ui_sprite_chipmunk_with_boundary_regist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_create(
        module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_NAME, sizeof(struct ui_sprite_chipmunk_with_boundary));
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: chipmunk with boundary register: meta create fail",
            ui_sprite_chipmunk_module_name(module));
        return -1;
    }

    ui_sprite_fsm_action_meta_set_enter_fun(meta, ui_sprite_chipmunk_with_boundary_enter, module);
    ui_sprite_fsm_action_meta_set_exit_fun(meta, ui_sprite_chipmunk_with_boundary_exit, module);
    ui_sprite_fsm_action_meta_set_init_fun(meta, ui_sprite_chipmunk_with_boundary_init, module);
    ui_sprite_fsm_action_meta_set_copy_fun(meta, ui_sprite_chipmunk_with_boundary_copy, module);
    ui_sprite_fsm_action_meta_set_free_fun(meta, ui_sprite_chipmunk_with_boundary_clear, module);

    if (ui_sprite_cfg_loader_add_action_loader(
            module->m_loader, UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_NAME, ui_sprite_chipmunk_with_boundary_load, module)
        != 0)
    {
        ui_sprite_fsm_action_meta_free(meta);
        return -1;
    }

    return 0;
}

void ui_sprite_chipmunk_with_boundary_unregist(ui_sprite_chipmunk_module_t module) {
    ui_sprite_fsm_action_meta_t meta;

    meta = ui_sprite_fsm_action_meta_find(module->m_fsm_module, UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_NAME);
    if (meta == NULL) {
        CPE_ERROR(
            module->m_em, "%s: send event unregister: meta not exist",
            ui_sprite_chipmunk_module_name(module));
        return;
    }

    ui_sprite_fsm_action_meta_free(meta);

    ui_sprite_cfg_loader_remove_action_loader(module->m_loader, UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_NAME);
}

const char * UI_SPRITE_CHIPMUNK_WITH_BOUNDARY_NAME = "chipmunk-with-boundary";


#ifdef __cplusplus
}
#endif
    
