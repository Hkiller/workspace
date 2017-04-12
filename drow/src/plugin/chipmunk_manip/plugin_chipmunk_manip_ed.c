#include <assert.h>
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "plugin/chipmunk/plugin_chipmunk_data_polygon_node.h"
#include "plugin/chipmunk/plugin_chipmunk_data_constraint.h"
#include "plugin_chipmunk_manip_i.h"

ui_ed_obj_t plugin_chipmunk_data_body_ed_obj_create(ui_ed_obj_t parent) {
    plugin_chipmunk_data_scene_t scene;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_chipmunk_data_body_t body;
    ui_ed_obj_t obj;

    scene = ui_ed_obj_product(parent);
    assert(scene);

    body = plugin_chipmunk_data_body_create(scene);
    if (body == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_chipmunk_body,
        body, plugin_chipmunk_data_body_data(body), sizeof(*plugin_chipmunk_data_body_data(body)));
    if (obj == NULL) {
        plugin_chipmunk_data_body_free(body);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_chipmunk_data_fixture_ed_obj_create(ui_ed_obj_t parent) {
    plugin_chipmunk_data_body_t body;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_chipmunk_data_fixture_t fixture;
    ui_ed_obj_t obj;

    body = ui_ed_obj_product(parent);
    assert(body);

    fixture = plugin_chipmunk_data_fixture_create(body);
    if (fixture == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_chipmunk_fixture,
        fixture, plugin_chipmunk_data_fixture_data(fixture), sizeof(*plugin_chipmunk_data_fixture_data(fixture)));
    if (obj == NULL) {
        plugin_chipmunk_data_fixture_free(fixture);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_chipmunk_data_polygon_node_ed_obj_create(ui_ed_obj_t parent) {
    plugin_chipmunk_data_fixture_t fixture;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_chipmunk_data_polygon_node_t polygon_node;
    ui_ed_obj_t obj;

    fixture = ui_ed_obj_product(parent);
    assert(fixture);

    polygon_node = plugin_chipmunk_data_polygon_node_create(fixture);
    if (polygon_node == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_chipmunk_polygon_node,
        polygon_node, plugin_chipmunk_data_polygon_node_data(polygon_node), sizeof(*plugin_chipmunk_data_polygon_node_data(polygon_node)));
    if (obj == NULL) {
        plugin_chipmunk_data_polygon_node_free(polygon_node);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_chipmunk_data_constraint_ed_obj_create(ui_ed_obj_t parent) {
    plugin_chipmunk_data_scene_t scene;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_chipmunk_data_constraint_t constraint;
    ui_ed_obj_t obj;

    scene = ui_ed_obj_product(parent);
    assert(scene);

    constraint = plugin_chipmunk_data_constraint_create(scene);
    if (constraint == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_chipmunk_constraint,
        constraint, plugin_chipmunk_data_constraint_data(constraint), sizeof(*plugin_chipmunk_data_constraint_data(constraint)));
    if (obj == NULL) {
        plugin_chipmunk_data_constraint_free(constraint);
        return NULL;
    }

    return obj;
}

int plugin_chipmunk_scene_ed_src_load(ui_ed_src_t src) {
    ui_ed_obj_t src_obj = ui_ed_src_root_obj(src);
    plugin_chipmunk_data_scene_t scene = ui_ed_src_product(src);
    ui_ed_obj_t scene_obj;
    struct plugin_chipmunk_data_body_it bodies_it;
    plugin_chipmunk_data_body_t body;
    ui_ed_obj_t body_obj;
    struct plugin_chipmunk_data_fixture_it fixtures_it;
    plugin_chipmunk_data_fixture_t fixture;
    ui_ed_obj_t fixture_obj;
    plugin_chipmunk_data_polygon_node_t polygon_node;
    ui_ed_obj_t polygon_node_obj;
    struct plugin_chipmunk_data_constraint_it constraints_it;
    plugin_chipmunk_data_constraint_t constraint;
    ui_ed_obj_t constraint_obj;

    assert(scene);

    scene_obj = ui_ed_obj_create_i(
            src, src_obj,
            ui_ed_obj_type_chipmunk_scene,
            scene, plugin_chipmunk_data_scene_data(scene), sizeof(*plugin_chipmunk_data_scene_data(scene)));
    if (scene_obj == NULL) {
        return -1;
    }

    plugin_chipmunk_data_scene_bodies(&bodies_it, scene);
    while((body = plugin_chipmunk_data_body_it_next(&bodies_it))) {
        body_obj = ui_ed_obj_create_i(
            src, scene_obj,
            ui_ed_obj_type_chipmunk_body,
            body, plugin_chipmunk_data_body_data(body), sizeof(*plugin_chipmunk_data_body_data(body)));
        if (body_obj == NULL) {
            return -1;
        }

        plugin_chipmunk_data_body_fixtures(&fixtures_it, body);
        while((fixture = plugin_chipmunk_data_fixture_it_next(&fixtures_it))) {
            fixture_obj = ui_ed_obj_create_i(
                src, body_obj,
                ui_ed_obj_type_chipmunk_fixture,
                fixture, plugin_chipmunk_data_fixture_data(fixture), sizeof(*plugin_chipmunk_data_fixture_data(fixture)));
            if (fixture_obj == NULL) {
                return -1;
            }

            for(polygon_node = plugin_chipmunk_data_polygon_node_head(fixture);
                polygon_node;
                polygon_node = plugin_chipmunk_data_polygon_node_next(polygon_node))
            {
                polygon_node_obj = ui_ed_obj_create_i(
                    src, fixture_obj,
                    ui_ed_obj_type_chipmunk_polygon_node,
                    polygon_node, plugin_chipmunk_data_polygon_node_data(polygon_node), sizeof(*plugin_chipmunk_data_polygon_node_data(polygon_node)));
                if (polygon_node_obj == NULL) {
                    return -1;
                }
            }
        }
    }

    plugin_chipmunk_data_scene_constraints(&constraints_it, scene);
    while((constraint = plugin_chipmunk_data_constraint_it_next(&constraints_it))) {
        constraint_obj = ui_ed_obj_create_i(
            src, scene_obj,
            ui_ed_obj_type_chipmunk_constraint,
            constraint, plugin_chipmunk_data_constraint_data(constraint), sizeof(*plugin_chipmunk_data_constraint_data(constraint)));
        if (constraint_obj == NULL) {
            return -1;
        }
    }

    return 0;
}
