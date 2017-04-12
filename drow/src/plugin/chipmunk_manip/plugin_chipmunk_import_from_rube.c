#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/chipmunk_manip/plugin_chipmunk_rube.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"

static ui_ed_obj_t plugin_chipmunk_import_from_rube_create_fixture(ui_ed_obj_t body_obj, cfg_t rube_fixture, error_monitor_t em);
static int plugin_chipmunk_import_from_rube_create_bodies(ui_ed_obj_t scene_obj, cfg_t rube_root, float ptm, error_monitor_t em);
static int plugin_chipmunk_import_from_rube_create_constraints(ui_ed_obj_t scene_obj, cfg_t rube_root, float ptm, error_monitor_t em);

int plugin_chipmunk_import_from_rube(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr,
    const char * rube_path, const char * to_chipmunk_path, float ptm, error_monitor_t em)
{
    ui_ed_src_t chipmunk_ed_src;
    ui_ed_obj_t scene_obj;
    cfg_t rube_root = NULL;
    int rv = -1;

    rube_root = cfg_create(NULL);
    if (rube_root == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_rube: create cfg root fail!");
        goto COMPLETE;
    }

    if (cfg_json_read_file(rube_root, gd_app_vfs_mgr(ui_data_mgr_app(data_mgr)), rube_path, cfg_replace, em) != 0) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_rube: load input rube file %s faile!", rube_path);
        goto COMPLETE;
    }
    
    chipmunk_ed_src =  ui_ed_src_check_create(ed_mgr, to_chipmunk_path, ui_data_src_type_chipmunk_scene);
    if (chipmunk_ed_src == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_rube: create output chipmunk src %s faile!", to_chipmunk_path);
        goto COMPLETE;
    }

    scene_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(chipmunk_ed_src));
    assert(scene_obj);
    ui_ed_obj_remove_childs(scene_obj);

    if (plugin_chipmunk_import_from_rube_create_bodies(scene_obj, rube_root, ptm, em) != 0) {
        goto COMPLETE;
    }

    if (plugin_chipmunk_import_from_rube_create_constraints(scene_obj, rube_root, ptm, em) != 0) {
        goto COMPLETE;
    }
    
    rv = 0;
    
COMPLETE:
    if (rube_root) cfg_free(rube_root);

    return rv;
}

static int plugin_chipmunk_import_from_rube_add_constraint(
    ui_ed_obj_t scene_obj, CHIPMUNK_CONSTRAINT const * constraint_data, error_monitor_t em)
{
    ui_ed_obj_t constraint_obj;

    constraint_obj = ui_ed_obj_new(scene_obj, ui_ed_obj_type_chipmunk_constraint);
    if (constraint_obj == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_rube: create constraint obj faile!");
        return -1;
    }
    
    memcpy(ui_ed_obj_data(constraint_obj), constraint_data, sizeof(*constraint_data));

    return 0;
}

static cfg_t plugin_chipmunk_import_from_rube_find_body(cfg_t body_list_cfg, uint16_t body_id) {
    struct cfg_it body_it;
    cfg_t r;
    
    cfg_it_init(&body_it, body_list_cfg);

    while((r = cfg_it_next(&body_it))) {
        if (cfg_get_uint16(r, "id", 0) == body_id) return r;
    }

    return NULL;
}

static int plugin_chipmunk_import_from_rube_create_constraints(ui_ed_obj_t scene_obj, cfg_t rube_root, float ptm, error_monitor_t em) {
    struct cfg_it constraints_it;
    cfg_t rube_constraint;
    cfg_t body_list_cfg;

    body_list_cfg = cfg_find_cfg(rube_root, "metaworld.metabody");
    
    cfg_it_init(&constraints_it, cfg_find_cfg(rube_root, "metaworld.metajoint"));
    while((rube_constraint = cfg_it_next(&constraints_it))) {
        CHIPMUNK_CONSTRAINT constraint_data;
        cfg_t body_cfg;
        const char * constraint_type;

        body_cfg = plugin_chipmunk_import_from_rube_find_body(body_list_cfg, cfg_get_uint16(rube_constraint, "bodyA", 0));
        if (body_cfg == NULL) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_rube: find body a[id=%d] fail!", cfg_get_uint16(rube_constraint, "bodyA", 0));
            return -1;
        }
        constraint_data.body_a = cfg_get_uint32(body_cfg, "id", 0);

        body_cfg = plugin_chipmunk_import_from_rube_find_body(body_list_cfg, cfg_get_uint16(rube_constraint, "bodyB", 0));
        if (body_cfg == NULL) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_rube: find body b[id=%d] faile!", cfg_get_uint16(rube_constraint, "bodyB", 0));
            return -1;
        }
        constraint_data.body_b = cfg_get_uint32(body_cfg, "id", 0);

        cpe_str_dup(constraint_data.name, sizeof(constraint_data.name), cfg_get_string(rube_constraint, "name", ""));
        //constraint_data.error_bias = cfg_get_float(rube_constraint, "springDampingRatio", 0.0f);

        constraint_type = cfg_get_string(rube_constraint, "type", NULL);
        if (constraint_type == NULL) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_rube: constraint type not configured!");
            return -1;
        }

        if (strcmp(constraint_type, "wheel") == 0) {
            constraint_data.constraint_type = chipmunk_constraint_type_pin_joint;
            constraint_data.constraint_data.pin_joint.anchor_a.x =
                cfg_get_float(rube_constraint, "anchorA.x", cfg_get_float(rube_constraint, "anchorA", 0.0f)) * ptm;
            constraint_data.constraint_data.pin_joint.anchor_a.y =
                - cfg_get_float(rube_constraint, "anchorA.y", cfg_get_float(rube_constraint, "anchorA", 0.0f)) * ptm;
            constraint_data.constraint_data.pin_joint.anchor_b.x =
                cfg_get_float(rube_constraint, "anchorB.x", cfg_get_float(rube_constraint, "anchorB", 0.0f)) * ptm;
            constraint_data.constraint_data.pin_joint.anchor_b.y =
                - cfg_get_float(rube_constraint, "anchorB.y", cfg_get_float(rube_constraint, "anchorB", 0.0f)) * ptm;

            if (plugin_chipmunk_import_from_rube_add_constraint(scene_obj, &constraint_data, em) != 0) return -1;

            if (cfg_get_uint8(rube_constraint, "enableMotor", 0)) {
                snprintf(constraint_data.name, sizeof(constraint_data.name), "*motor-%s", cfg_get_string(rube_constraint, "name", ""));
                constraint_data.constraint_type = chipmunk_constraint_type_simple_motor;
                constraint_data.constraint_data.simple_motor.rate = cfg_get_float(rube_constraint, "springDampingRatio", 0.0f);
                if (plugin_chipmunk_import_from_rube_add_constraint(scene_obj, &constraint_data, em) != 0) return -1;
            }
        }
        else if (strcmp(constraint_type, "revolute") == 0) {
            constraint_data.constraint_type = chipmunk_constraint_type_pin_joint;
            constraint_data.constraint_data.pin_joint.anchor_a.x =
                cfg_get_float(rube_constraint, "anchorA.x", cfg_get_float(rube_constraint, "anchorA", 0.0f)) * ptm;
            constraint_data.constraint_data.pin_joint.anchor_a.y =
                - cfg_get_float(rube_constraint, "anchorA.y", cfg_get_float(rube_constraint, "anchorA", 0.0f)) * ptm;
            constraint_data.constraint_data.pin_joint.anchor_b.x =
                cfg_get_float(rube_constraint, "anchorB.x", cfg_get_float(rube_constraint, "anchorB", 0.0f)) * ptm;
            constraint_data.constraint_data.pin_joint.anchor_b.y =
                - cfg_get_float(rube_constraint, "anchorB.y", cfg_get_float(rube_constraint, "anchorB", 0.0f)) * ptm;

            if (plugin_chipmunk_import_from_rube_add_constraint(scene_obj, &constraint_data, em) != 0) return -1;
        }
        else {
            CPE_ERROR(em, "plugin_chipmunk_import_from_rube: not support constraint type %s!", constraint_type);
            return -1;
        }
    }

    return 0;
}

static int plugin_chipmunk_import_from_rube_create_bodies(ui_ed_obj_t scene_obj, cfg_t rube_root, float ptm, error_monitor_t em) {
    struct cfg_it bodies_it;
    cfg_t rube_body;

    cfg_it_init(&bodies_it, cfg_find_cfg(rube_root, "metaworld.metabody"));
    while((rube_body = cfg_it_next(&bodies_it))) {
        ui_ed_obj_t body_obj;
        CHIPMUNK_BODY * body_data;
        struct cfg_it fixtures_it;
        cfg_t rube_fixture;
        const char * rube_body_type;

        body_obj = ui_ed_obj_new(scene_obj, ui_ed_obj_type_chipmunk_body);
        if (body_obj == NULL) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_rube: create body obj faile!");
            return -1;
        }
        body_data = ui_ed_obj_data(body_obj);

        body_data->id = cfg_get_uint32(rube_body, "id", 0);
        cpe_str_dup(body_data->name, sizeof(body_data->name), cfg_get_string(rube_body, "name", ""));
        body_data->anchorpoint.x = cfg_get_float(rube_body, "position.x", 0.0f) * ptm;
        body_data->anchorpoint.y = - cfg_get_float(rube_body, "position.y", 0.0f) * ptm;

        rube_body_type = cfg_get_string(rube_body, "type", "");

        if (strcmp(rube_body_type, "static") == 0) {
            body_data->type = CHIPMUNK_OBJ_TYPE_STATIC;
        }
        else if (strcmp(rube_body_type, "dynamic") == 0) {
            body_data->type = CHIPMUNK_OBJ_TYPE_DYNAMIC;
        }
        else if (strcmp(rube_body_type, "kinematic") == 0) {
            body_data->type = CHIPMUNK_OBJ_TYPE_KINEMATIC;
        }
        else {
            CPE_ERROR(em, "plugin_chipmunk_import_from_rube: unknown type %s!", rube_body_type);
            return -1;
        }

        cfg_it_init(&fixtures_it, cfg_find_cfg(rube_body, "fixture"));
        while((rube_fixture = cfg_it_next(&fixtures_it))) {
            struct cfg_it shapes_it;
            cfg_t rube_shape;

            cfg_it_init(&shapes_it, cfg_find_cfg(rube_fixture, "shapes"));
            while((rube_shape = cfg_it_next(&shapes_it))) {
                const char * shape_type = cfg_get_string(rube_shape, "type", "");

                if (strcmp(shape_type, "polygon") == 0) {
                    ui_ed_obj_t fixture_obj;
                    CHIPMUNK_FIXTURE * fixture_data;
                    struct cfg_it x_it;
                    struct cfg_it y_it;
                    uint32_t point_count = 0;
                    uint32_t i;
                    CHIPMUNK_PAIR * points = NULL;

                    point_count = cfg_seq_count(cfg_find_cfg(rube_fixture, "vertices.x"));
                    if (point_count < 3) {
                        CPE_ERROR(em, "plugin_chipmunk_import_from_rube: fixture %s: polygon not enouth point!", body_data->name);
                        return -1;
                    }
                    
                    points = mem_alloc(NULL, sizeof(CHIPMUNK_PAIR) * point_count);
                    if (points == NULL) {
                        CPE_ERROR(em, "plugin_chipmunk_import_from_rube: alloc point buff fail, count=%d!", point_count);
                        return -1;
                    }
                    
                    fixture_obj = plugin_chipmunk_import_from_rube_create_fixture(body_obj, rube_fixture, em);
                    if (fixture_obj == NULL) {
                        mem_free(NULL, points);
                        return -1;
                    }
                    fixture_data = ui_ed_obj_data(fixture_obj);

                    fixture_data->fixture_type = chipmunk_fixture_type_polygon;

                    cfg_it_init(&x_it, cfg_find_cfg(rube_fixture, "vertices.x"));
                    cfg_it_init(&y_it, cfg_find_cfg(rube_fixture, "vertices.y"));

                    i = point_count - 1;
                    do {
                        cfg_t rube_x = cfg_it_next(&x_it);
                        cfg_t rube_y = cfg_it_next(&y_it);

                        if (rube_x == NULL) {
                            if (rube_y != NULL) {
                                CPE_ERROR(
                                    em, "plugin_chipmunk_import_from_rube: fixture %s.%s: polygon point count mismatch!",
                                    body_data->name, fixture_data->name);
                                mem_free(NULL, points);
                                return -1;
                            }
                            else {
                                break;
                            }
                        }

                        points[i].x = cfg_as_float(rube_x, 0.0f) * ptm;
                        points[i].y = - cfg_as_float(rube_y, 0.0f) * ptm;
                        i--;
                    } while(1);

                    for(i = 0; i < point_count; ++i) {
                        ui_ed_obj_t polygon_node_obj;
                        CHIPMUNK_POLYGON_NODE * polygon_node_data;
                        
                        polygon_node_obj = ui_ed_obj_new(fixture_obj, ui_ed_obj_type_chipmunk_polygon_node);
                        if (polygon_node_obj == NULL) {
                            CPE_ERROR(
                                em, "plugin_chipmunk_import_from_rube: fixture %s.%s: create polygon_node obj faile!",
                                body_data->name, fixture_data->name);
                            mem_free(NULL, points);
                            return -1;
                        }
                        polygon_node_data = ui_ed_obj_data(polygon_node_obj);

                        polygon_node_data->group_id = 0;
                        polygon_node_data->pos = points[i];
                    }

                    mem_free(NULL, points);
                }
                else if (strcmp(shape_type, "circle") == 0) {
                    ui_ed_obj_t fixture_obj;
                    CHIPMUNK_FIXTURE * fixture_data;

                    fixture_obj = plugin_chipmunk_import_from_rube_create_fixture(body_obj, rube_fixture, em);
                    if (fixture_obj == NULL) return -1;
                    fixture_data = ui_ed_obj_data(fixture_obj);
                
                    fixture_data->fixture_type = chipmunk_fixture_type_circle;
                    fixture_data->fixture_data.circle.radius = cfg_get_float(rube_shape, "radius", 0.0f) * ptm;
                    fixture_data->fixture_data.circle.position.x = cfg_get_float(rube_fixture, "vertices.x[0]", 0.0f) * ptm;
                    fixture_data->fixture_data.circle.position.y = - cfg_get_float(rube_fixture, "vertices.y[0]", 0.0f) * ptm;
                }
                else if (strcmp(shape_type, "line") == 0) {
                    struct cfg_it x_it;
                    struct cfg_it y_it;
                    cfg_t rube_x;
                    cfg_t rube_y;
                    float pre_x;
                    float pre_y;

                    cfg_it_init(&x_it, cfg_find_cfg(rube_fixture, "vertices.x"));
                    cfg_it_init(&y_it, cfg_find_cfg(rube_fixture, "vertices.y"));

                    rube_x = cfg_it_next(&x_it);
                    rube_y = cfg_it_next(&y_it);

                    if (rube_x == NULL || rube_y == NULL) {
                        CPE_ERROR(
                            em, "plugin_chipmunk_import_from_rube: fixture %s.%s: chain no first point!",
                            body_data->name, cfg_get_string(rube_fixture, "name", NULL));
                        return -1;
                    }

                    pre_x = cfg_as_float(rube_x, 0.0f) * ptm;
                    pre_y = - cfg_as_float(rube_y, 0.0f) * ptm;

                    do {
                        ui_ed_obj_t fixture_obj;
                        CHIPMUNK_FIXTURE * fixture_data;

                        rube_x = cfg_it_next(&x_it);
                        rube_y = cfg_it_next(&y_it);

                        if (rube_x == NULL) {
                            if (rube_y != NULL) {
                                CPE_ERROR(
                                    em, "plugin_chipmunk_import_from_rube: fixture %s.%s: chain point count mismatch!",
                                    body_data->name, cfg_get_string(rube_fixture, "name", NULL));
                                return -1;
                            }
                            else {
                                break;
                            }
                        }

                        fixture_obj = plugin_chipmunk_import_from_rube_create_fixture(body_obj, rube_fixture, em);
                        if (fixture_obj == NULL) return -1;
                        fixture_data = ui_ed_obj_data(fixture_obj);
                
                        fixture_data->fixture_type = chipmunk_fixture_type_segment;
                        fixture_data->fixture_data.segment.a.x = pre_x;
                        fixture_data->fixture_data.segment.a.y = pre_y;
                        fixture_data->fixture_data.segment.b.x = cfg_as_float(rube_x, 0.0f) * ptm;
                        fixture_data->fixture_data.segment.a.y = - cfg_as_float(rube_y, 0.0f) * ptm;

                        pre_x = fixture_data->fixture_data.segment.b.x;
                        pre_y = fixture_data->fixture_data.segment.b.y;
                    } while(1);
                }
                else {
                    CPE_ERROR(
                        em, "plugin_chipmunk_import_from_rube: fixture %s.%s: unknown shape type %s!",
                        body_data->name, cfg_get_string(rube_fixture, "name", ""), shape_type);
                    return -1;
                }
            }
        }
    }

    return 0;
}

static ui_ed_obj_t plugin_chipmunk_import_from_rube_create_fixture(ui_ed_obj_t body_obj, cfg_t rube_fixture, error_monitor_t em) {
    ui_ed_obj_t fixture_obj;
    CHIPMUNK_FIXTURE * fixture_data;
            
    fixture_obj = ui_ed_obj_new(body_obj, ui_ed_obj_type_chipmunk_fixture);
    if (fixture_obj == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_rube: create fixture obj faile!");
        return NULL;
    }
    fixture_data = ui_ed_obj_data(fixture_obj);

    cpe_str_dup(fixture_data->name, sizeof(fixture_data->name), cfg_get_string(rube_fixture, "name", ""));

    fixture_data->mass = 0.0f;
    fixture_data->elasticity = cfg_get_float(rube_fixture, "restitution", 0.0f);
    fixture_data->density = cfg_get_float(rube_fixture, "density", 0.0f);
    fixture_data->friction = cfg_get_float(rube_fixture, "friction", 0.0f);
    //fixture_data->surface_velocity = 0.0f;
    fixture_data->collision_mask = 0;
    fixture_data->collision_group = 0;
    fixture_data->collision_category = 0;
    fixture_data->is_sensor = 0;

    return fixture_obj;
}
