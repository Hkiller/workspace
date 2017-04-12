#include <assert.h>
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin_moving_manip_i.h"

ui_ed_obj_t plugin_moving_plan_track_ed_obj_create(ui_ed_obj_t parent) {
    plugin_moving_plan_t plan;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_moving_plan_track_t track;
    ui_ed_obj_t obj;

    plan = ui_ed_obj_product(parent);
    assert(plan);

    track = plugin_moving_plan_track_create(plan);
    if (track == NULL) return NULL;
    
    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_moving_track,
        track, plugin_moving_plan_track_data(track), sizeof(*plugin_moving_plan_track_data(track)));
    if (obj == NULL) {
        plugin_moving_plan_track_free(track);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_moving_plan_point_ed_obj_create(ui_ed_obj_t parent) {
    plugin_moving_plan_track_t track;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_moving_plan_point_t point;
    ui_ed_obj_t obj;

    track = ui_ed_obj_product(parent);
    assert(track);

    point = plugin_moving_plan_point_create(track);
    if (point == NULL) return NULL;
    
    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_moving_point,
        point, plugin_moving_plan_point_data(point), sizeof(*plugin_moving_plan_point_data(point)));
    if (obj == NULL) {
        plugin_moving_plan_point_free(point);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_moving_plan_node_ed_obj_create(ui_ed_obj_t parent) {
    plugin_moving_plan_t plan;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_moving_plan_node_t node;
    ui_ed_obj_t obj;

    plan = ui_ed_obj_product(parent);
    assert(plan);

    node = plugin_moving_plan_node_create(plan);
    if (node == NULL) return NULL;
    
    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_moving_node,
        node, plugin_moving_plan_node_data(node), sizeof(*plugin_moving_plan_node_data(node)));
    if (obj == NULL) {
        plugin_moving_plan_node_free(node);
        return NULL;
    }

    return obj;
}

ui_ed_obj_t plugin_moving_plan_segment_ed_obj_create(ui_ed_obj_t parent) {
    plugin_moving_plan_node_t node;
    ui_ed_src_t ed_src = ui_ed_obj_src(parent);
    plugin_moving_plan_segment_t segment;
    ui_ed_obj_t obj;

    node = ui_ed_obj_product(parent);
    assert(node);

    segment = plugin_moving_plan_segment_create(node);
    if (segment == NULL) return NULL;
    
    obj = ui_ed_obj_create_i(
        ed_src, parent,
        ui_ed_obj_type_moving_segment,
        segment, plugin_moving_plan_segment_data(segment), sizeof(*plugin_moving_plan_segment_data(segment)));
    if (obj == NULL) {
        plugin_moving_plan_segment_free(segment);
        return NULL;
    }

    return obj;
}

int plugin_moving_ed_src_load(ui_ed_src_t src) {
    ui_ed_obj_t src_obj = ui_ed_src_root_obj(src);
    plugin_moving_plan_t plan = ui_ed_src_product(src);
    struct plugin_moving_plan_track_it tracks_it;
    plugin_moving_plan_track_t track;
    struct plugin_moving_plan_node_it nodes_it;
    plugin_moving_plan_node_t node;
    ui_ed_obj_t obj_plan;

    assert(plan);

    obj_plan = ui_ed_obj_create_i(
        src, src_obj,
        ui_ed_obj_type_moving_plan,
        plan, plugin_moving_plan_data(plan), sizeof(*plugin_moving_plan_data(plan)));
    if (obj_plan == NULL) {
        return -1;
    }

    /*加载tracks*/
    plugin_moving_plan_tracks(&tracks_it, plan);
    while((track = plugin_moving_plan_track_it_next(&tracks_it))) {
        ui_ed_obj_t obj_track;
        struct plugin_moving_plan_point_it points_it;
        plugin_moving_plan_point_t point;
        
        obj_track = ui_ed_obj_create_i(
            src, obj_plan,
            ui_ed_obj_type_moving_track,
            track, plugin_moving_plan_track_data(track), sizeof(*plugin_moving_plan_track_data(track)));
        if (obj_track == NULL) {
            return -1;
        }

        plugin_moving_plan_track_points(&points_it, track);
        while((point = plugin_moving_plan_point_it_next(&points_it))) {
            ui_ed_obj_t obj_point;

            obj_point = ui_ed_obj_create_i(
                src, obj_track,
                ui_ed_obj_type_moving_point,
                point, plugin_moving_plan_point_data(point), sizeof(*plugin_moving_plan_point_data(point)));
            if (obj_point == NULL) {
                return -1;
            }
        }
    }

    /*加载nodes*/
    plugin_moving_plan_nodes(&nodes_it, plan);
    while((node = plugin_moving_plan_node_it_next(&nodes_it))) {
        ui_ed_obj_t obj_node;
        struct plugin_moving_plan_segment_it segments_it;
        plugin_moving_plan_segment_t segment;

        obj_node = ui_ed_obj_create_i(
            src, obj_plan,
            ui_ed_obj_type_moving_node,
            node, plugin_moving_plan_node_data(node), sizeof(*plugin_moving_plan_node_data(node)));
        if (obj_node == NULL) {
            return -1;
        }

        plugin_moving_plan_node_segments(&segments_it, node);
        while((segment = plugin_moving_plan_segment_it_next(&segments_it))) {
            ui_ed_obj_t obj_segment;

            obj_segment = ui_ed_obj_create_i(
                src, obj_node,
                ui_ed_obj_type_moving_segment,
                segment, plugin_moving_plan_segment_data(segment), sizeof(*plugin_moving_plan_segment_data(segment)));
            if (obj_segment == NULL) {
                return -1;
            }
        }
    }
    
    return 0;
}
