#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "ui/sprite/ui_sprite_group.h"
#include "ui/sprite/ui_sprite_component.h"
#include "ui_sprite_chipmunk_tri_scope_i.h"
#include "ui_sprite_chipmunk_obj_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

ui_sprite_chipmunk_tri_scope_t
ui_sprite_chipmunk_tri_scope_create(ui_sprite_chipmunk_obj_t obj) {
    ui_sprite_chipmunk_module_t module = obj->m_env->m_module;
    ui_sprite_chipmunk_tri_scope_t scope;

    scope = TAILQ_FIRST(&module->m_free_tri_scopes);
    if (scope) {
        TAILQ_REMOVE(&module->m_free_tri_scopes, scope, m_next_for_obj);
    }
    else {
        scope = (ui_sprite_chipmunk_tri_scope_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_tri_scope));
        if (scope == NULL) {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_tri_scope_t: alloc fail!");
            return NULL;
        }
    }

    scope->m_env = obj->m_env;
    scope->m_obj = obj;
    scope->m_group = CP_NO_GROUP;
    scope->m_mask = CP_ALL_CATEGORIES;
    scope->m_type = ui_sprite_chipmunk_tri_scope_unknown;

    TAILQ_INSERT_TAIL(&obj->m_env->m_scopes, scope, m_next_for_env);
    TAILQ_INSERT_TAIL(&obj->m_scopes, scope, m_next_for_obj);
    
    return scope;
}
    
void ui_sprite_chipmunk_tri_scope_free(ui_sprite_chipmunk_tri_scope_t scope) {
    ui_sprite_chipmunk_module_t module = scope->m_env->m_module;

    if (scope->m_obj) {
        TAILQ_REMOVE(&scope->m_obj->m_env->m_scopes, scope, m_next_for_env);
        TAILQ_REMOVE(&scope->m_obj->m_scopes, scope, m_next_for_obj);
        scope->m_obj = NULL;
    }

    scope->m_env = (ui_sprite_chipmunk_env_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_tri_scopes, scope, m_next_for_obj);
}

void ui_sprite_chipmunk_tri_scope_real_free(ui_sprite_chipmunk_tri_scope_t scope) {
    ui_sprite_chipmunk_module_t module = (ui_sprite_chipmunk_module_t)scope->m_env;
    TAILQ_REMOVE(&module->m_free_tri_scopes, scope, m_next_for_obj);
    mem_free(module->m_alloc, scope);
}

uint32_t ui_sprite_chipmunk_tri_scope_group(ui_sprite_chipmunk_tri_scope_t scope) {
    return scope->m_group;
}

void ui_sprite_chipmunk_tri_scope_set_group(ui_sprite_chipmunk_tri_scope_t scope, uint32_t group) {
    scope->m_group = group;
}

uint32_t ui_sprite_chipmunk_tri_scope_mask(ui_sprite_chipmunk_tri_scope_t scope) {
    return scope->m_mask;
}

void ui_sprite_chipmunk_tri_scope_set_mask(ui_sprite_chipmunk_tri_scope_t scope, uint32_t mask) {
    scope->m_mask = mask;
}

int ui_sprite_chipmunk_tri_scope_set_dynamic(
    ui_sprite_chipmunk_tri_scope_t scope, ui_sprite_chipmunk_tri_scope_update_fun_t fun, void * ctx, size_t ctx_size)
{
    if (ctx && ctx_size > sizeof(scope->m_dynamic.m_ctx)) {
        CPE_ERROR(
            scope->m_env->m_module->m_em, "ui_sprite_chipmunk_tri_scope_query: ctx size %d overflow, capacity=%d!",
            (int)ctx_size, (int)(sizeof(scope->m_dynamic.m_ctx)));
        return -1;
    }

    scope->m_type = ui_sprite_chipmunk_tri_scope_dynamic;
    scope->m_dynamic.m_fun = fun;
    if (ctx) memcpy(scope->m_dynamic.m_ctx, ctx, ctx_size);

    return 0;
}

struct ui_sprite_chipmunk_tri_scope_query_ctx {
    ui_sprite_chipmunk_tri_scope_t m_scope;
    ui_sprite_chipmunk_tri_scope_visit_fun_t m_visit_fun;
    void * m_visit_ctx;
};
    
static void ui_sprite_chipmunk_tri_scope_query_cb(cpShape *shape, cpContactPointSet *points, void *data) {
    struct ui_sprite_chipmunk_tri_scope_query_ctx * query_ctx = (struct ui_sprite_chipmunk_tri_scope_query_ctx *)data;
    if (shape->type != query_ctx->m_scope->m_obj->m_env->m_collision_type) return;
    
    query_ctx->m_visit_fun(query_ctx->m_scope, (ui_sprite_chipmunk_obj_body_t)shape->userData, query_ctx->m_visit_ctx);
}

int ui_sprite_chipmunk_tri_scope_query(ui_sprite_chipmunk_tri_scope_t scope, ui_sprite_chipmunk_tri_scope_visit_fun_t visit, void * ctx) {
    struct ui_sprite_chipmunk_tri_scope_query_ctx query_ctx = { scope, visit, ctx };
    cpBody body;
    
    union {
        struct cpCircleShape m_circle;
        struct cpSegmentShape m_segment;
        struct cpPolyShape m_poly;
    } shape_data;
    
    if (scope->m_obj == NULL) {
        CPE_ERROR(scope->m_env->m_module->m_em, "ui_sprite_chipmunk_tri_scope_query: no associate obj");
        return -1;
    }

    bzero(&body, sizeof(body));
    cpBodyInit(&body, 0.0f, 0.0f);
    
    switch(scope->m_type) {
    case ui_sprite_chipmunk_tri_scope_dynamic:
        assert(scope->m_dynamic.m_fun);
        switch(scope->m_dynamic.m_fun(scope->m_dynamic.m_ctx, &shape_data, &body)) {
        case ui_sprite_chipmunk_tri_scope_update_success:
            break;
        case ui_sprite_chipmunk_tri_scope_update_fail:
            CPE_ERROR(scope->m_env->m_module->m_em, "ui_sprite_chipmunk_tri_scope_query: create shape fail");
            cpBodyDestroy(&body);
            return -1;
        case ui_sprite_chipmunk_tri_scope_update_skip:
        default:
            cpBodyDestroy(&body);
            return 0;
        }
        break;
    case ui_sprite_chipmunk_tri_scope_fix:
        CPE_ERROR(scope->m_env->m_module->m_em, "ui_sprite_chipmunk_tri_scope_query: not support fix yet");
        cpBodyDestroy(&body);
        return -1;
    case ui_sprite_chipmunk_tri_scope_unknown:
    default:
        CPE_ERROR(scope->m_env->m_module->m_em, "ui_sprite_chipmunk_tri_scope_query: scope type unknown");
        cpBodyDestroy(&body);
        return -1;
    }

    cpShapeSetFilter((cpShape *)&shape_data, cpShapeFilterNew(0, CP_ALL_CATEGORIES, CP_ALL_CATEGORIES));

    cpShapeCacheBB((cpShape *)&shape_data);
    
    cpSpaceShapeQuery(
        (cpSpace *)plugin_chipmunk_env_space(scope->m_obj->m_env->m_env),
        (cpShape *)&shape_data,
        ui_sprite_chipmunk_tri_scope_query_cb, &query_ctx);

    cpShapeDestroy((cpShape *)&shape_data);
    cpBodyDestroy(&body);

    return 0;
}

static void ui_sprite_chipmunk_tri_scope_collect_entity(ui_sprite_chipmunk_tri_scope_t scope, ui_sprite_chipmunk_obj_body_t body, void * ctx) {
    ui_sprite_group_add_entity((ui_sprite_group_t)ctx, ui_sprite_component_entity(ui_sprite_component_from_data(body->m_obj)));
}

int ui_sprite_chipmunk_tri_scope_query_entities(ui_sprite_chipmunk_tri_scope_t scope, ui_sprite_group_t group) {
    return ui_sprite_chipmunk_tri_scope_query(scope, ui_sprite_chipmunk_tri_scope_collect_entity, group);
}
