#include <assert.h>
#include <stdio.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "plugin/chipmunk/plugin_chipmunk_data_polygon_node.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "ui_sprite_chipmunk_obj_shape_group_i.h"
#include "ui_sprite_chipmunk_obj_body_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_obj_shape_group_t
ui_sprite_chipmunk_obj_shape_group_create(ui_sprite_chipmunk_obj_body_t body) {
    ui_sprite_chipmunk_module_t module = body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_shape_group_t shape_group;

    shape_group = (ui_sprite_chipmunk_obj_shape_group_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_shape_group));
    if (shape_group == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_group_create: alloc fail!");
        return NULL;
    }

    shape_group->m_body = body;
    shape_group->m_shape_count = 0;
    shape_group->m_bufs_begin = NULL;
    shape_group->m_bufs_last = NULL;

    TAILQ_INSERT_TAIL(&body->m_shape_groups, shape_group, m_next_for_body);

    return shape_group;
}

ui_sprite_chipmunk_obj_shape_group_t    
ui_sprite_chipmunk_obj_shape_group_create_from_data(ui_sprite_chipmunk_obj_body_t body, plugin_chipmunk_data_fixture_t fixture) {
    ui_sprite_chipmunk_module_t module = body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_shape_group_t group = ui_sprite_chipmunk_obj_shape_group_create(body);
    CHIPMUNK_FIXTURE * fixture_data = (CHIPMUNK_FIXTURE *)plugin_chipmunk_data_fixture_data(fixture);

    if (group == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_group_create_from_data: create group fail!");
        return NULL;
    }

    if (fixture_data->fixture_type == chipmunk_fixture_type_polygon) {
        plugin_chipmunk_data_polygon_node_t polygon_node;
        plugin_chipmunk_data_polygon_node_t polygon_head = NULL;
        uint32_t node_count;
        
        for(polygon_node = plugin_chipmunk_data_polygon_node_head(fixture);
            polygon_node;
            polygon_node = plugin_chipmunk_data_polygon_node_next(polygon_node))
        {
            if (polygon_head == NULL) {
                polygon_head = polygon_node;
                node_count = 1;
            }
            else {
                if (plugin_chipmunk_data_polygon_node_data(polygon_node)->group_id
                    == plugin_chipmunk_data_polygon_node_data(polygon_head)->group_id)
                {
                    ++node_count;
                }
                else {
                    if (node_count < 3) {
                        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_group_create_from_data: polygon node count error!");
                        ui_sprite_chipmunk_obj_shape_group_free(group);
                        return NULL;
                    }

                    if (ui_sprite_chipmunk_obj_shape_create_i(group, fixture_data, polygon_head, node_count, 0) == NULL) {
                        ui_sprite_chipmunk_obj_shape_group_free(group);
                        return NULL;
                    }

                    polygon_head = polygon_node;
                    node_count = 1;
                }
            }
        }

        if (polygon_head) {
            if (node_count < 3) {
                CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_group_create_from_data: polygon node count error!");
                ui_sprite_chipmunk_obj_shape_group_free(group);
                return NULL;
            }

            if (ui_sprite_chipmunk_obj_shape_create_i(group, fixture_data, polygon_head, node_count, 0) == NULL) {
                ui_sprite_chipmunk_obj_shape_group_free(group);
                return NULL;
            }
        }
    }
    else {
        if (ui_sprite_chipmunk_obj_shape_create_i(group, fixture_data, NULL, 0, 0) == NULL) {
            ui_sprite_chipmunk_obj_shape_group_free(group);
            return NULL;
        }
    }

    return group;
}

struct clone_ctx {
    ui_sprite_chipmunk_module_t m_module;
    ui_sprite_chipmunk_obj_body_t m_body;
    ui_sprite_chipmunk_obj_shape_group_t m_group;
    int m_error;
};
    
static void ui_sprite_chipmunk_obj_shape_do_clone(ui_sprite_chipmunk_obj_shape_t from_shape, void * i_ctx) {
    struct clone_ctx * ctx = (struct clone_ctx *)i_ctx;
    ui_sprite_chipmunk_obj_shape_t to_shape;

    if (from_shape->m_is_managed) {
        CHIPMUNK_FIXTURE * new_fixture_data = (CHIPMUNK_FIXTURE *)mem_alloc(ctx->m_module->m_alloc, sizeof(CHIPMUNK_FIXTURE));
        if (new_fixture_data == NULL) {
            ctx->m_error = 1;
            return;
        }

        memcpy(new_fixture_data, from_shape->m_fixture_data, sizeof(CHIPMUNK_FIXTURE));
        
        to_shape = ui_sprite_chipmunk_obj_shape_create_i(ctx->m_group, new_fixture_data, NULL, 0, 1);
        if (to_shape == NULL) {
            mem_free(ctx->m_module->m_alloc, new_fixture_data);
            ctx->m_error = 1;
            return;
        }
    }
    else {
        to_shape = ui_sprite_chipmunk_obj_shape_create_i(ctx->m_group, from_shape->m_fixture_data, from_shape->m_polygon_node, from_shape->m_polygon_node_count, 0);
        if (to_shape == NULL) {
            ctx->m_error = 1;
            return;
        }
    }
}
    
ui_sprite_chipmunk_obj_shape_group_t
ui_sprite_chipmunk_obj_shape_group_clone(ui_sprite_chipmunk_obj_body_t body, ui_sprite_chipmunk_obj_shape_group_t from_group) {
    struct clone_ctx ctx = {
        body->m_obj->m_env->m_module,
        body,
        ui_sprite_chipmunk_obj_shape_group_create(body),
        0
    };

    if (ctx.m_group == NULL) return NULL;

    ui_sprite_chipmunk_obj_shape_group_visit_shapes(from_group, ui_sprite_chipmunk_obj_shape_do_clone, &ctx);

    if (ctx.m_error) {
        CPE_ERROR(ctx.m_module->m_em, "ui_sprite_chipmunk_obj_shape_group_clone fail!");
        ui_sprite_chipmunk_obj_shape_group_free(ctx.m_group);
        return NULL;
    }

    return ctx.m_group;
}

static void ui_sprite_chipmunk_obj_shape_do_free(ui_sprite_chipmunk_obj_shape_t shape, void * i_ctx) {
    ui_sprite_chipmunk_obj_shape_free_i(shape);
}

void ui_sprite_chipmunk_obj_shape_group_free(ui_sprite_chipmunk_obj_shape_group_t group) {
    ui_sprite_chipmunk_module_t module = group->m_body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_shape_buf_t buf = group->m_bufs_begin;

    ui_sprite_chipmunk_obj_shape_group_visit_shapes(group, ui_sprite_chipmunk_obj_shape_do_free, module);
    
    while(buf) {
        ui_sprite_chipmunk_obj_shape_buf_t next_buf = buf->m_next;
        mem_free(module->m_alloc, buf);
        buf = next_buf;
    }

    TAILQ_REMOVE(&group->m_body->m_shape_groups, group, m_next_for_body);

    mem_free(module->m_alloc, group);
}

void ui_sprite_chipmunk_obj_shape_group_visit_shapes(
    ui_sprite_chipmunk_obj_shape_group_t group, ui_sprite_chipmunk_obj_shape_visit_fun_t fun, void * ctx)
{
    uint32_t count = group->m_shape_count;
    uint32_t block_count;
    ui_sprite_chipmunk_obj_shape_buf_t buf = group->m_bufs_begin;
    uint32_t i;

    block_count = count > CPE_ARRAY_SIZE(group->m_inline_shapes) ? CPE_ARRAY_SIZE(group->m_inline_shapes) : count;
    for(i = 0; i < block_count; ++i) {
        fun(&group->m_inline_shapes[i], ctx);
    }
    count -= block_count;

    while(count) {
        block_count =
            count > CPE_TYPE_ARRAY_SIZE(struct ui_sprite_chipmunk_obj_shape_buf, m_shapes)
            ? CPE_TYPE_ARRAY_SIZE(struct ui_sprite_chipmunk_obj_shape_buf, m_shapes)
            : count;
        for(i = 0; i < block_count; ++i) {
            fun(&buf->m_shapes[i], ctx);
        }

        buf = buf->m_next;
        count -= block_count;
    }
}

struct generate_shape_ctx {
    cpSpace * m_space;
    ui_sprite_2d_transform_t m_transform;
    int m_error;
};

static void ui_sprite_chipmunk_obj_shape_do_init_shape(ui_sprite_chipmunk_obj_shape_t obj_shape, void * i_ctx) {
    struct generate_shape_ctx * ctx = (struct generate_shape_ctx *)(i_ctx);
    if (ui_sprite_chipmunk_obj_shape_init(obj_shape, ctx->m_space, ctx->m_transform) != 0) ctx->m_error = -1;
}
    
int ui_sprite_chipmunk_obj_shape_group_init_shape(ui_sprite_chipmunk_obj_shape_group_t group, cpSpace * space, ui_sprite_2d_transform_t transform) {
    struct generate_shape_ctx ctx = { space, transform, 0 };
    ui_sprite_chipmunk_obj_shape_group_visit_shapes(group, ui_sprite_chipmunk_obj_shape_do_init_shape, (void*)&ctx);
    return ctx.m_error;
}

static void ui_sprite_chipmunk_obj_shape_do_fini_shape(ui_sprite_chipmunk_obj_shape_t obj_shape, void * i_ctx) {
    if (obj_shape->m_is_generated) ui_sprite_chipmunk_obj_shape_fini(obj_shape);
}

void ui_sprite_chipmunk_obj_shape_group_fini_shape(ui_sprite_chipmunk_obj_shape_group_t group) {
    ui_sprite_chipmunk_obj_shape_group_visit_shapes(group, ui_sprite_chipmunk_obj_shape_do_fini_shape, NULL);
}

#ifdef __cplusplus
}
#endif

