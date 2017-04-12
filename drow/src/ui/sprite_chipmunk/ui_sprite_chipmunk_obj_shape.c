#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "ui/sprite_2d/ui_sprite_2d_transform.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "plugin/chipmunk/plugin_chipmunk_data_polygon_node.h"
#include "ui_sprite_chipmunk_obj_shape_i.h"
#include "ui_sprite_chipmunk_obj_shape_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_chipmunk_obj_shape_t
ui_sprite_chipmunk_obj_shape_create_i(
    ui_sprite_chipmunk_obj_shape_group_t group, CHIPMUNK_FIXTURE * fixture_data,
    plugin_chipmunk_data_polygon_node_t polygon_node, uint16_t polygon_node_count, uint8_t is_managed)
{
    ui_sprite_chipmunk_module_t module = group->m_body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_shape_t r;

    uint32_t pos = group->m_shape_count;
    if (pos < CPE_ARRAY_SIZE(group->m_inline_shapes)) {
        group->m_shape_count++;
        r = &group->m_inline_shapes[pos];
    }
    else {
        pos -= CPE_ARRAY_SIZE(group->m_inline_shapes);
        pos = pos % CPE_TYPE_ARRAY_SIZE(struct ui_sprite_chipmunk_obj_shape_buf, m_shapes);

        if (pos == 0) {
            ui_sprite_chipmunk_obj_shape_buf_t new_buf =
                (ui_sprite_chipmunk_obj_shape_buf_t)mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_shape_buf));
            if (new_buf == NULL) {
                CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_create: alloc shape buf fail!");
                return NULL;
            }
            new_buf->m_next = NULL;

            if (group->m_bufs_last == NULL) {
                assert(group->m_bufs_begin == NULL);
                group->m_bufs_begin = group->m_bufs_last = new_buf;
            }
            else {
                assert(group->m_bufs_begin);
                
                group->m_bufs_last->m_next = new_buf;
                group->m_bufs_last = new_buf;
            }
        }

        group->m_shape_count++;
        r = &group->m_bufs_last->m_shapes[pos];
    }

    bzero(r, sizeof(*r));
    r->m_group = group;
    r->m_is_managed = is_managed;
    r->m_is_generated = 0;
    r->m_fixture_data = fixture_data;
    r->m_polygon_node = polygon_node;
    r->m_polygon_node_count = polygon_node_count;
    r->m_inline_nodes = NULL;

    assert(r->m_fixture_data);

    return r;
}

ui_sprite_chipmunk_obj_shape_t
ui_sprite_chipmunk_obj_shape_create_managed(ui_sprite_chipmunk_obj_shape_group_t group) {
    ui_sprite_chipmunk_module_t module = group->m_body->m_obj->m_env->m_module;
    CHIPMUNK_FIXTURE * fixture_data;
    ui_sprite_chipmunk_obj_shape_t shape;

    fixture_data = (CHIPMUNK_FIXTURE*)mem_calloc(module->m_alloc, sizeof(CHIPMUNK_FIXTURE));
    if (fixture_data == NULL) return NULL;
    
    shape = ui_sprite_chipmunk_obj_shape_create_i(group, fixture_data, NULL, 0, 1);
    if (shape == NULL) {
        mem_free(module->m_alloc, fixture_data);
        return NULL;
    }

    return shape;
}
    
ui_sprite_chipmunk_obj_shape_t
ui_sprite_chipmunk_obj_shape_create_mamaged_from_body(ui_sprite_chipmunk_obj_body_t body) {
    ui_sprite_chipmunk_module_t module = body->m_obj->m_env->m_module;
    ui_sprite_chipmunk_obj_shape_group_t group;
    CHIPMUNK_FIXTURE * fixture_data;
    ui_sprite_chipmunk_obj_shape_t shape;

    fixture_data = (CHIPMUNK_FIXTURE*)mem_calloc(module->m_alloc, sizeof(CHIPMUNK_FIXTURE));
    if (fixture_data == NULL) return NULL;
    
    group = ui_sprite_chipmunk_obj_shape_group_create(body);
    if (group == NULL) {
        mem_free(module->m_alloc, fixture_data);
        return NULL;
    }

    shape = ui_sprite_chipmunk_obj_shape_create_i(group, fixture_data, NULL, 0, 1);
    if (shape == NULL) {
        ui_sprite_chipmunk_obj_shape_group_free(group);
        mem_free(module->m_alloc, fixture_data);
        return NULL;
    }

    return shape;
}

void ui_sprite_chipmunk_obj_shape_free_i(ui_sprite_chipmunk_obj_shape_t shape) {
    ui_sprite_chipmunk_module_t module = shape->m_group->m_body->m_obj->m_env->m_module;
    
    if (shape->m_is_generated) {
        ui_sprite_chipmunk_obj_shape_fini(shape);
        assert(shape->m_is_generated == 0);
    }

    if (shape->m_is_managed) {
        mem_free(module->m_alloc, shape->m_fixture_data);
        shape->m_fixture_data = NULL;

        if (shape->m_inline_nodes) {
            mem_free(module->m_alloc, shape->m_inline_nodes);
            shape->m_inline_nodes = NULL;
        }

        shape->m_is_managed = 0;
    }
}

CHIPMUNK_FIXTURE * ui_sprite_chipmunk_obj_shape_fixture_data(ui_sprite_chipmunk_obj_shape_t obj_shape) {
    return obj_shape->m_fixture_data;
}

uint32_t ui_sprite_chipmunk_obj_shape_node_count(ui_sprite_chipmunk_obj_shape_t obj_shape) {
    if (obj_shape->m_inline_nodes) {
        return obj_shape->m_inline_nodes->m_count;
    }
    else if (obj_shape->m_polygon_node) {
        uint32_t r;
        plugin_chipmunk_data_polygon_node_t node;

        r = 1;
        for(node = plugin_chipmunk_data_polygon_node_next(obj_shape->m_polygon_node);
            node && (plugin_chipmunk_data_polygon_node_data(node)->group_id
                     == plugin_chipmunk_data_polygon_node_data(obj_shape->m_polygon_node)->group_id);
            node = plugin_chipmunk_data_polygon_node_next(node))
        {
            r++;
        }

        return r;
    }
    else {
        return 0;
    }
}

void * ui_sprite_chipmunk_obj_shape_cp_shape(ui_sprite_chipmunk_obj_shape_t obj_shape) {
    return obj_shape;
}

ui_sprite_chipmunk_obj_shape_node_buf_t
ui_sprite_chipmunk_obj_shape_alloc_node_buf(ui_sprite_chipmunk_obj_shape_t shape, uint32_t node_capacity) {
    ui_sprite_chipmunk_module_t module = shape->m_group->m_body->m_obj->m_env->m_module;

    if (!shape->m_is_managed) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_alloc_node_buf: shape is not managed, can`t alloc node");
        return NULL;
    }

    if (shape->m_group->m_body->m_is_in_space) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_alloc_node_buf: shape already in space, can`t alloc node");
        return NULL;
    }

    if (shape->m_inline_nodes) {
        mem_free(module->m_alloc, shape->m_inline_nodes);
    }

    shape->m_inline_nodes =
        (struct ui_sprite_chipmunk_obj_shape_node_buf *)
        mem_alloc(module->m_alloc, sizeof(struct ui_sprite_chipmunk_obj_shape_node_buf) + sizeof(CHIPMUNK_PAIR) * node_capacity);
    if (shape->m_inline_nodes == NULL) {
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_alloc_node_buf: all node buf fail, capacity=%d", node_capacity);
        return NULL;
    }

    shape->m_inline_nodes->m_capacity = node_capacity;
    shape->m_inline_nodes->m_count = 0;

    return shape->m_inline_nodes;
}

ui_sprite_chipmunk_obj_shape_node_buf_t ui_sprite_chipmunk_obj_shape_get_node_buf(ui_sprite_chipmunk_obj_shape_t shape) {
    return shape->m_inline_nodes;
}

uint32_t ui_sprite_chipmunk_obj_shape_node_buf_capacity(ui_sprite_chipmunk_obj_shape_node_buf_t node_buf) {
    return node_buf->m_capacity;
}

uint32_t ui_sprite_chipmunk_obj_shape_node_buf_count(ui_sprite_chipmunk_obj_shape_node_buf_t node_buf) {
    return node_buf->m_count;
}

void ui_sprite_chipmunk_obj_shape_node_buf_set_count(ui_sprite_chipmunk_obj_shape_node_buf_t node_buf, uint32_t count) {
    assert(count <= node_buf->m_capacity);
    node_buf->m_count = count;
}

CHIPMUNK_PAIR * ui_sprite_chipmunk_obj_shape_node_buf_data(ui_sprite_chipmunk_obj_shape_node_buf_t node_buf) {
    return node_buf->m_poss;
}

int ui_sprite_chipmunk_obj_shape_init(ui_sprite_chipmunk_obj_shape_t obj_shape, cpSpace* space, ui_sprite_2d_transform_t transform) {
    cpShape* shape = (cpShape*)obj_shape;
    ui_sprite_chipmunk_obj_body_t body = obj_shape->m_group->m_body;
    ui_sprite_chipmunk_module_t module = body->m_obj->m_env->m_module;
    uint8_t pos_adj_type = UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_FLIP | UI_SPRITE_2D_TRANSFORM_POS_ADJ_BY_SCALE;
    CHIPMUNK_FIXTURE const * fixture_data = obj_shape->m_fixture_data;
    uint32_t collision_category;
    uint32_t collision_mask;
    uint32_t collision_group;
    ui_vector_2 entity_scale = ui_sprite_2d_transform_scale_pair(transform);

    assert(obj_shape->m_is_generated == 0);

    switch(fixture_data->fixture_type) {
    case chipmunk_fixture_type_circle: {
        ui_vector_2 pos = UI_VECTOR_2_INITLIZER(fixture_data->fixture_data.circle.position.x, fixture_data->fixture_data.circle.position.y);
        pos = ui_sprite_2d_transform_adj_local_pos(transform, pos, pos_adj_type);

        cpCircleShapeInit(
            &obj_shape->m_circle, &body->m_body,
            fixture_data->fixture_data.circle.radius * cpe_min(entity_scale.x, entity_scale.y),
            cpv(pos.x, pos.y));

        break;
    }
    case chipmunk_fixture_type_polygon: {
        cpVect inline_buf[32];
        cpVect * buf;
        uint32_t point_count = ui_sprite_chipmunk_obj_shape_node_count(obj_shape);
        uint32_t i;
        uint8_t polygon_revert =
            ((ui_sprite_2d_transform_flip_x(transform) ? 1 : 0) + (ui_sprite_2d_transform_flip_y(transform) ? 1 : 0)) == 1;

        if (point_count > CPE_ARRAY_SIZE(inline_buf)) {
            buf = (cpVect*)mem_alloc(module->m_alloc, sizeof(cpVect) * point_count);
            if (buf == NULL) {
                CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_group_create_from_data: alloc polygon node buf fail!");
                return -1;
            }
        }
        else {
            buf = inline_buf;
        }

        i = 0;
        if (obj_shape->m_inline_nodes) {
            for(; i < obj_shape->m_inline_nodes->m_count; ++i) {
                ui_vector_2 pos = UI_VECTOR_2_INITLIZER(obj_shape->m_inline_nodes->m_poss[i].x, obj_shape->m_inline_nodes->m_poss[i].y );
                uint32_t buf_pos = polygon_revert ? (obj_shape->m_inline_nodes->m_count - i - 1) : i;
                
                pos = ui_sprite_2d_transform_adj_local_pos(transform, pos, pos_adj_type);

                buf[buf_pos].x = pos.x;
                buf[buf_pos].y = pos.y;
            }
        }
        else {
            plugin_chipmunk_data_polygon_node_t polygon_node;
            uint32_t total_node_count = ui_sprite_chipmunk_obj_shape_node_count(obj_shape);

            for(polygon_node = obj_shape->m_polygon_node;
                polygon_node && (plugin_chipmunk_data_polygon_node_data(polygon_node)->group_id
                                 == plugin_chipmunk_data_polygon_node_data(obj_shape->m_polygon_node)->group_id);
                polygon_node = plugin_chipmunk_data_polygon_node_next(polygon_node))
            {
                CHIPMUNK_POLYGON_NODE const * polygon_node_data = plugin_chipmunk_data_polygon_node_data(polygon_node);
                ui_vector_2 pos = UI_VECTOR_2_INITLIZER(polygon_node_data->pos.x, polygon_node_data->pos.y);
                uint32_t buf_pos = polygon_revert ? (total_node_count - i - 1) : i;
                assert(i < total_node_count);
                
                pos = ui_sprite_2d_transform_adj_local_pos(transform, pos, pos_adj_type);

                buf[buf_pos].x = pos.x;
                buf[buf_pos].y = pos.y;
                ++i;
            }
        }

        cpPolyShapeInitRaw(&obj_shape->m_poly, &body->m_body, i, buf, 0);

        if (point_count > CPE_ARRAY_SIZE(inline_buf)) {
            mem_free(module->m_alloc, buf);
        }

        break;
    }
    case chipmunk_fixture_type_entity_rect: {
        ui_vector_2 lt = ui_sprite_2d_transform_local_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_F_TOP_LEFT, pos_adj_type);
        ui_vector_2 rb = ui_sprite_2d_transform_local_pos(transform, UI_SPRITE_2D_TRANSFORM_POS_F_BOTTOM_RIGHT, pos_adj_type);
        cpVect points[4];

        points[0].x = lt.x - fixture_data->fixture_data.entity_rect.adj.x;
        points[0].y = lt.y - fixture_data->fixture_data.entity_rect.adj.y;
        points[1].x = rb.x + fixture_data->fixture_data.entity_rect.adj.x;
        points[1].y = lt.y - fixture_data->fixture_data.entity_rect.adj.y;
        points[2].x = rb.x + fixture_data->fixture_data.entity_rect.adj.x;
        points[2].y = rb.y + fixture_data->fixture_data.entity_rect.adj.y;
        points[3].x = lt.x - fixture_data->fixture_data.entity_rect.adj.x;
        points[3].y = rb.y + fixture_data->fixture_data.entity_rect.adj.y;
        
        cpPolyShapeInitRaw(&obj_shape->m_poly, &body->m_body, 4, points, 0);
        break;
    }
    case chipmunk_fixture_type_box: {
        ui_vector_2 lt = UI_VECTOR_2_INITLIZER(fixture_data->fixture_data.box.lt.x, fixture_data->fixture_data.box.lt.y);
        ui_vector_2 rb = UI_VECTOR_2_INITLIZER(fixture_data->fixture_data.box.rb.x, fixture_data->fixture_data.box.rb.y);
        cpVect points[4];

        lt = ui_sprite_2d_transform_adj_local_pos(transform, lt, pos_adj_type);
        rb = ui_sprite_2d_transform_adj_local_pos(transform, rb, pos_adj_type);

        points[0].x = lt.x;
        points[0].y = lt.y;
        points[1].x = rb.x;
        points[1].y = lt.y;
        points[2].x = rb.x;
        points[2].y = rb.y;
        points[3].x = lt.x;
        points[3].y = rb.y;
        
        cpPolyShapeInitRaw(&obj_shape->m_poly, &body->m_body, 4, points, 0);
        break;
    }
    case chipmunk_fixture_type_sector: {
        cpVect inline_buf[32];
        cpVect * buf;
        uint32_t point_count;
        float angle, angle_end;
        uint32_t i;
        uint8_t angle_way = fixture_data->fixture_data.sector.angle_step > 0.0f ? 1.0f : -1.0f;
        float angle_step = fabs(fixture_data->fixture_data.sector.angle_step);
        
        if (fixture_data->fixture_data.sector.radius <= 0.0f) {
            CPE_ERROR(
                module->m_em, "ui_sprite_chipmunk_obj_shape_group_create_from_data: sector radius %f error!",
                fixture_data->fixture_data.sector.radius);
            return -1;
        }

        if (fixture_data->fixture_data.sector.angle_step == 0.0f) {
            CPE_ERROR(
                module->m_em, "ui_sprite_chipmunk_obj_shape_group_create_from_data: sector range step %f error!",
                fixture_data->fixture_data.sector.angle_step);
            return -1;
        }

        if (fixture_data->fixture_data.sector.angle_range <= 0.0f || fixture_data->fixture_data.sector.angle_range > 360.0f) {
            CPE_ERROR(
                module->m_em, "ui_sprite_chipmunk_obj_shape_group_create_from_data: sector range %f error, should (0.0f ~ 360.0f]!",
                fixture_data->fixture_data.sector.angle_range);
            return -1;
        }

        point_count = (uint32_t)(fixture_data->fixture_data.sector.angle_range / fabs(fixture_data->fixture_data.sector.angle_step));
        if (cpe_float_cmp(fixture_data->fixture_data.sector.angle_range, 360.0f, UI_FLOAT_PRECISION) != 0) {
            point_count++;
            if ((point_count * fabs(fixture_data->fixture_data.sector.angle_step) - fixture_data->fixture_data.sector.angle_range) > UI_FLOAT_PRECISION) {
                point_count++;
            }
        }

        if (point_count > CPE_ARRAY_SIZE(inline_buf)) {
            buf = (cpVect*)mem_alloc(module->m_alloc, sizeof(cpVect) * point_count);
            if (buf == NULL) {
                CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_group_create_from_data: alloc polygon node buf fail!");
                return -1;
            }
        }
        else {
            buf = inline_buf;
        }

        i = 0;
        
        if (cpe_float_cmp(fixture_data->fixture_data.sector.angle_range, 360.0f, UI_FLOAT_PRECISION) != 0) {
            buf[i].x = fixture_data->fixture_data.sector.center.x;
            buf[i].y = fixture_data->fixture_data.sector.center.y;
            i++;
        }
        
        for(angle = fixture_data->fixture_data.sector.angle_start, angle_end = angle + fixture_data->fixture_data.sector.angle_range * angle_way;
            i < point_count;
            )
        {
            buf[i].x = fixture_data->fixture_data.sector.center.x + fixture_data->fixture_data.sector.radius * cpe_cos_angle(angle);
            buf[i].y = fixture_data->fixture_data.sector.center.y + fixture_data->fixture_data.sector.radius * cpe_sin_angle(angle);
            i++;

            angle += fixture_data->fixture_data.sector.angle_step * angle_way;
            if (cpe_float_cmp(angle, angle_end, UI_FLOAT_PRECISION) == 0
                || (angle_way > 0.0f && angle > angle_end)
                || (angle_way < 0.0f && angle < angle_end))
            {
                angle = angle_end;
            }
        }

        cpPolyShapeInitRaw(&obj_shape->m_poly, &body->m_body, i, buf, 0);

        if (point_count > CPE_ARRAY_SIZE(inline_buf)) {
            mem_free(module->m_alloc, buf);
        }
            
        break;
    }
    case chipmunk_fixture_type_segment: {
        ui_vector_2 pos_a = UI_VECTOR_2_INITLIZER(fixture_data->fixture_data.segment.a.x, fixture_data->fixture_data.segment.a.y);
        ui_vector_2 pos_b = UI_VECTOR_2_INITLIZER(fixture_data->fixture_data.segment.b.x, fixture_data->fixture_data.segment.b.y);
        pos_a = ui_sprite_2d_transform_adj_local_pos(transform, pos_a, pos_adj_type);
        pos_b = ui_sprite_2d_transform_adj_local_pos(transform, pos_b, pos_adj_type);

        cpSegmentShapeInit(
            &obj_shape->m_segment, &body->m_body,
            cpv(pos_a.x, pos_a.y), cpv(pos_b.x, pos_b.y),
            fixture_data->fixture_data.segment.radius);
        
        break;
    }
    default:
        CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_generate_shape: unkonwn shape type %d!", fixture_data->fixture_type);
        return -1;
    }

    ui_sprite_chipmunk_obj_body_collision_info(body, &collision_category, &collision_mask, &collision_group);

    cpShapeSetCollisionType(shape, body->m_obj->m_env->m_collision_type);

    if (cpBodyGetType(&body->m_body) == CP_BODY_TYPE_DYNAMIC) {
        if (fixture_data->density > 0.0f) {
            cpShapeSetDensity(shape, fixture_data->density);
        }
        else if (fixture_data->mass > 0.0f) {
            cpShapeSetMass(shape, fixture_data->mass);
        }
        else {
            CPE_ERROR(module->m_em, "ui_sprite_chipmunk_obj_shape_generate_shape: dynamic object no mass!");
            cpShapeDestroy((cpShape *)obj_shape);
            return -1;
        }
    }

    cpShapeSetElasticity(shape, fixture_data->elasticity);
    cpShapeSetFriction(shape, fixture_data->friction);
    cpShapeSetSurfaceVelocity(shape, cpv(fixture_data->surface_velocity.x, fixture_data->surface_velocity.y));
    cpShapeSetFilter(
        shape, cpShapeFilterNew(
            collision_group ? collision_group : fixture_data->collision_group,
            fixture_data->collision_category | collision_category,
            fixture_data->collision_mask | collision_mask));
    cpShapeSetSensor(shape, fixture_data->is_sensor);
    cpShapeSetUserData(shape, body);

    cpSpaceAddShape(space, shape);
    
    obj_shape->m_is_generated = 1;
    
    /* printf("xxxxxxx: body %s add collision: is-sensor=%d, elasticity=%f, fricition=%f, category=%u,%u, mask=%u,%u\n", */
    /*        body->m_name, */
    /*        fixture_data->is_sensor, */
    /*        fixture_data->elasticity, fixture_data->friction, */
    /*        fixture_data->collision_category, collision_category, */
    /*        fixture_data->collision_mask, collision_mask); */
    return 0;
}

void ui_sprite_chipmunk_obj_shape_fini(ui_sprite_chipmunk_obj_shape_t obj_shape) {
    cpShape* shape = (cpShape*)obj_shape;
    
    assert(obj_shape->m_is_generated);
    
    cpSpaceRemoveShape(shape->space, shape);
    cpShapeDestroy(shape);
    
    obj_shape->m_is_generated = 0;
}
    
#ifdef __cplusplus
}
#endif
