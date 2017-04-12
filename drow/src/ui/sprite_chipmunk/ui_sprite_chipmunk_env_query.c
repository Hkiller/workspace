#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "gd/app/app_context.h"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui/sprite/ui_sprite_world_res.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_env_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct shape_query_ctx {
    ui_sprite_chipmunk_env_t m_env;
    ui_sprite_chipmunk_obj_body_visit_fun_t m_fun;
    void * m_ctx;
};

static void ui_sprite_chipmunk_visit_shape_by_point(cpShape *shape, cpVect point, cpFloat distance, cpVect gradient, void *data) {
    struct shape_query_ctx * ctx = (struct shape_query_ctx *)data;
    if (shape->type != ctx->m_env->m_collision_type) return;
    
    ctx->m_fun(ctx->m_env, (ui_sprite_chipmunk_obj_body_t)shape->userData, ctx->m_ctx);
}

static void ui_sprite_chipmunk_visit_shape_by_bb(cpShape *shape, void *data) {
    struct shape_query_ctx * ctx = (struct shape_query_ctx *)data;
    if (shape->type != ctx->m_env->m_collision_type) return;
    
    ctx->m_fun(ctx->m_env, (ui_sprite_chipmunk_obj_body_t)shape->userData, ctx->m_ctx);
}
    
static void ui_sprite_chipmunk_visit_shape_by_shape(cpShape *shape, cpContactPointSet *points, void *data) {
    assert(0);
}

int ui_sprite_chipmunk_env_query_bodies_by_shape(
    ui_sprite_chipmunk_env_t env, ui_sprite_entity_t from_entity,
    ui_sprite_chipmunk_obj_body_visit_fun_t fun, void * ctx,
    CHIPMUNK_SHAPE const * shape, uint32_t category, uint32_t mask, uint32_t group)
{
    ui_sprite_chipmunk_module_t module = env->m_module;
    ui_sprite_2d_transform_t transform;
    uint8_t pos_adj_type = UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP | UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE;    
    cpShapeFilter filter;
    ui_vector_2 entity_scale;
    struct shape_query_ctx query_ctx = { env, fun, ctx };
    
    if (category == 0) category = CP_ALL_CATEGORIES;
    if (mask == 0) mask = CP_ALL_CATEGORIES;

    filter = cpShapeFilterNew(group, category, mask);
    
    transform = ui_sprite_2d_transform_find(from_entity);
    if (transform == NULL) {
        CPE_ERROR(
            module->m_em, "entity %d(%s): query by shape: entity no transform!",
            ui_sprite_entity_id(from_entity), ui_sprite_entity_name(from_entity));
        return -1;
    }
    entity_scale = ui_sprite_2d_transform_scale_pair(transform);
        
    switch(shape->shape_type) {
    case chipmunk_fixture_type_circle: {
        ui_vector_2 entity_pos = ui_sprite_2d_transform_origin_pos(transform);
        ui_vector_2 pos = UI_VECTOR_2_INITLIZER( shape->shape_data.circle.position.x, shape->shape_data.circle.position.y );
        pos = ui_sprite_2d_transform_adj_local_pos(transform, pos, pos_adj_type);
        
        cpSpacePointQuery(
            (cpSpace *)plugin_chipmunk_env_space(env->m_env),
            cpv(entity_pos.x + pos.x, entity_pos.y + pos.y),
            shape->shape_data.circle.radius * cpe_min(entity_scale.x, entity_scale.y),
            filter,
            ui_sprite_chipmunk_visit_shape_by_point, &query_ctx);

        break;
    }
    case chipmunk_fixture_type_polygon: {
        CPE_ERROR(
            module->m_em, "entity %d(%s): query by shape: not support polygon!",
            ui_sprite_entity_id(from_entity), ui_sprite_entity_name(from_entity));
        break;
    }
    case chipmunk_fixture_type_entity_rect: {
        ui_vector_2 lt = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_F_TOP_LEFT, pos_adj_type);
        ui_vector_2 rb = ui_sprite_2d_transform_world_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_RIGHT, pos_adj_type);

        cpSpaceBBQuery(
            (cpSpace *)plugin_chipmunk_env_space(env->m_env),
            cpBBNew(
                lt.x - shape->shape_data.entity_rect.adj.x * entity_scale.x,
                lt.y - shape->shape_data.entity_rect.adj.y * entity_scale.y,
                rb.x + shape->shape_data.entity_rect.adj.x * entity_scale.x,
                rb.y + shape->shape_data.entity_rect.adj.y * entity_scale.y),
            filter,
            ui_sprite_chipmunk_visit_shape_by_bb, &query_ctx);
        
        break;
    }
    case chipmunk_fixture_type_box: {
        /* ui_vector_2 lt = { shape->shape.box.lt.x, shape->shape.box.lt.y }; */
        /* ui_vector_2 rb = { shape->shape.box.rb.x, shape->shape.box.rb.y }; */
        /* cpVect points[4]; */

        /* lt = ui_sprite_2d_transform_adj_local_pos(ctx->m_transform, lt, pos_adj_type); */
        /* rb = ui_sprite_2d_transform_adj_local_pos(ctx->m_transform, rb, pos_adj_type); */

        /* points[0].x = lt.x; */
        /* points[0].y = lt.y; */
        /* points[1].x = rb.x; */
        /* points[1].y = lt.y; */
        /* points[2].x = rb.x; */
        /* points[2].y = rb.y; */
        /* points[3].x = lt.x; */
        /* points[3].y = rb.y; */
        
        /* cpPolyShapeInitRaw(&obj_shape->m_poly, &ctx->m_body->m_body, 4, points, 0); */
        assert(0);
        break;
    }
    case chipmunk_fixture_type_sector: {
        assert(0);
        break;
    }
    case chipmunk_fixture_type_segment: {
        /* ui_vector_2 pos_a = { shape->shape.segment.a.x, shape->shape.segment.a.y }; */
        /* ui_vector_2 pos_b = { shape->shape.segment.b.x, shape->shape.segment.b.y }; */
        /* pos_a = ui_sprite_2d_transform_adj_local_pos(ctx->m_transform, pos_a, pos_adj_type); */
        /* pos_b = ui_sprite_2d_transform_adj_local_pos(ctx->m_transform, pos_b, pos_adj_type); */

        /* cpSegmentShapeInit( */
        /*     &obj_shape->m_segment, &ctx->m_body->m_body, */
        /*     cpv(pos_a.x, pos_a.y), cpv(pos_b.x, pos_b.y), */
        /*     shape->shape.segment.radius); */
        assert(0);
        break;
    }
    default:
        assert(0);
        CPE_ERROR(
            module->m_em, "entity %d(%s): query shapes: unknown shape type %d!",
            ui_sprite_entity_id(from_entity), ui_sprite_entity_name(from_entity), shape->shape_type);
        return -1;
    }

    return 0;
}

int ui_sprite_chipmunk_env_query_bodies_by_point(
    ui_sprite_chipmunk_env_t env, 
    ui_sprite_chipmunk_obj_body_visit_fun_t fun, void * ctx,
    CHIPMUNK_PAIR const * pos, float radius, uint32_t category, uint32_t mask, uint32_t group)
{
    cpShapeFilter filter = { group, category, mask };
    struct shape_query_ctx query_ctx = { env, fun, ctx };

    cpSpacePointQuery(
        (cpSpace *)plugin_chipmunk_env_space(env->m_env),
        cpv(pos->x, pos->y), radius,
        filter, ui_sprite_chipmunk_visit_shape_by_point, &query_ctx);

    return 0;
}

struct ui_sprite_chipmunk_env_collect_env {
    ui_sprite_entity_t m_from_entity;
    ui_sprite_group_t m_group;
};

static void ui_sprite_chipmunk_env_collect_body(ui_sprite_chipmunk_env_t env, ui_sprite_chipmunk_obj_body_t body, void * i_ctx) {
    struct ui_sprite_chipmunk_env_collect_env * ctx = (struct ui_sprite_chipmunk_env_collect_env *)i_ctx;
    ui_sprite_entity_t chipmunk_entity;

    chipmunk_entity = ui_sprite_component_entity(ui_sprite_component_from_data(body->m_obj));
    if (chipmunk_entity != ctx->m_from_entity) {
        ui_sprite_group_add_entity(ctx->m_group, chipmunk_entity);
    }
}

ui_sprite_group_t
ui_sprite_chipmunk_env_query_entities_by_shape(
    ui_sprite_chipmunk_env_t env,
    ui_sprite_entity_t from_entity, CHIPMUNK_SHAPE const * from_shape,
    uint32_t category, uint32_t mask, uint32_t gruop)
{
    ui_sprite_chipmunk_module_t module = env->m_module;
    ui_sprite_world_t world = ui_sprite_world_res_world(ui_sprite_world_res_from_data(env));
    struct ui_sprite_chipmunk_env_collect_env ctx;
    ctx.m_from_entity = from_entity;
    
    ctx.m_group = ui_sprite_group_create(world, NULL);
    if (ctx.m_group == NULL) {
        CPE_ERROR(module->m_em, "%s: query entities: create group fail", ui_sprite_chipmunk_module_name(module));
        return NULL;
    }

    if (ui_sprite_chipmunk_env_query_bodies_by_shape(
            env, from_entity, ui_sprite_chipmunk_env_collect_body, &ctx, from_shape, category, mask, gruop) != 0)
    {
        CPE_ERROR(module->m_em, "%s: query entities: do query fail", ui_sprite_chipmunk_module_name(module));
        ui_sprite_group_free(ctx.m_group);
        return NULL;
    }
    
    return ctx.m_group;
}

#ifdef __cplusplus
}
#endif
