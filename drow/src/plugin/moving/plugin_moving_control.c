#include <assert.h>
#include "render/utils/ui_vector_2.h"
#include "plugin_moving_control_i.h"
#include "plugin_moving_node_i.h"
#include "plugin_moving_plan_track_i.h"
#include "plugin_moving_plan_point_i.h"
#include "plugin_moving_plan_node_i.h"
#include "plugin_moving_plan_segment_i.h"

plugin_moving_control_t
plugin_moving_control_create(plugin_moving_env_t env, plugin_moving_plan_t plan) {
    plugin_moving_module_t module = env->m_module;
    plugin_moving_control_t control;

    control = TAILQ_FIRST(&env->m_free_controls);
    if (control) {
        TAILQ_REMOVE(&env->m_free_controls, control, m_next_for_env);
    }
    else {
        control = mem_alloc(module->m_alloc, sizeof(struct plugin_moving_control));
        if (control == NULL) {
            CPE_ERROR(module->m_em, "plugin_moving_control: create: alloc fail!");
            return NULL;
        }
    }
    
    control->m_env = env;
    control->m_plan = plan;
    control->m_origin_pos.x = 0.0f;
    control->m_origin_pos.y = 0.0f;
    TAILQ_INIT(&control->m_nodes);

    TAILQ_INSERT_TAIL(&env->m_controls, control, m_next_for_env);
    
    return control;
}

void plugin_moving_control_free(plugin_moving_control_t control) {
    plugin_moving_env_t env = control->m_env;

    TAILQ_REMOVE(&env->m_controls, control, m_next_for_env);

    while(!TAILQ_EMPTY(&control->m_nodes)) {
        plugin_moving_node_free(TAILQ_FIRST(&control->m_nodes));
    }

    control->m_env = env;
    TAILQ_INSERT_TAIL(&env->m_free_controls, control, m_next_for_env);
}

void plugin_moving_control_real_free(plugin_moving_control_t control) {
    plugin_moving_env_t env = control->m_env;

    TAILQ_REMOVE(&env->m_free_controls, control, m_next_for_env);

    mem_free(env->m_module->m_alloc, control);
}

ui_vector_2_t plugin_moving_control_origin_pos(plugin_moving_control_t control) {
    return &control->m_origin_pos;
}

void plugin_moving_control_set_origin_pos(plugin_moving_control_t control, ui_vector_2_t pos) {
    control->m_origin_pos = *pos;
}

plugin_moving_plan_t plugin_moving_control_plan(plugin_moving_control_t control) {
    return control->m_plan;
}

int plugin_moving_control_update(plugin_moving_control_t control, float delta) {
    plugin_moving_node_t node, next;

    for(node = TAILQ_FIRST(&control->m_nodes); node; node = next) {
        next = TAILQ_NEXT(node, m_next_for_control);
        plugin_moving_node_update(node, delta);
    }
                  
    return 0;
}

void plugin_moving_control_adj_origin_pos_for_point(plugin_moving_control_t control, plugin_moving_plan_point_t point, ui_vector_2_t point_pos) {
    control->m_origin_pos.x = point_pos->x - point->m_data.pos.x;
    control->m_origin_pos.y = point_pos->y - point->m_data.pos.y;
}

int plugin_moving_control_adj_origin_pos_for_track_end_at(plugin_moving_control_t control, plugin_moving_plan_track_t track, ui_vector_2_t pos) {
    plugin_moving_plan_point_t base_point = TAILQ_LAST(&track->m_points, plugin_moving_plan_point_list);
    if (base_point == NULL) return -1;
    
    plugin_moving_control_adj_origin_pos_for_point(control, base_point, pos);
    return 0;
}

int plugin_moving_control_adj_origin_pos_for_track_begin_at(plugin_moving_control_t control, plugin_moving_plan_track_t track, ui_vector_2_t pos) {
    plugin_moving_plan_point_t base_point = TAILQ_FIRST(&track->m_points);
    if (base_point == NULL) return -1;

    plugin_moving_control_adj_origin_pos_for_point(control, base_point, pos);
    return 0;
}

int plugin_moving_control_adj_origin_pos_for_node_end_at(plugin_moving_control_t control, plugin_moving_plan_node_t node, ui_vector_2_t pos) {
    ui_vector_2 end_pos;

    if (plugin_moving_plan_node_end_pos(node, &end_pos) != 0) return -1;

    control->m_origin_pos.x = pos->x - end_pos.x;
    control->m_origin_pos.y = pos->y - end_pos.y;
    
    return 0;
}

int plugin_moving_control_adj_origin_pos_for_node_begin_at(plugin_moving_control_t control, plugin_moving_plan_node_t node, ui_vector_2_t pos) {
    ui_vector_2 begin_pos;

    if (plugin_moving_plan_node_begin_pos(node, &begin_pos) != 0) return -1;

    control->m_origin_pos.x = pos->x - begin_pos.x;
    control->m_origin_pos.y = pos->y - begin_pos.y;
    
    return 0;
}

int plugin_moving_plan_node_begin_pos(plugin_moving_plan_node_t node, ui_vector_2_t begin_pos) {
    plugin_moving_plan_track_t track;
    plugin_moving_plan_point_t point_begin;
        
    track = plugin_moving_plan_track_find_by_id(node->m_plan, node->m_data.track);
    if (track == NULL) return -1;

    point_begin = TAILQ_FIRST(&track->m_points);
    if (point_begin == NULL) return -1;

    begin_pos->x = point_begin->m_data.pos.x;
    begin_pos->y = point_begin->m_data.pos.y;    
    
    return 0;
}

int plugin_moving_plan_node_end_pos(plugin_moving_plan_node_t node, ui_vector_2_t end_pos) {
    plugin_moving_plan_segment_t segment;
    plugin_moving_plan_track_t track;
    plugin_moving_plan_point_t point_begin = NULL;
    plugin_moving_plan_point_t point_end;
    float length_left;
        
    track = plugin_moving_plan_track_find_by_id(node->m_plan, node->m_data.track);
    if (track == NULL) return -1;

    point_begin = point_end = TAILQ_FIRST(&track->m_points);
    if (point_end == NULL || TAILQ_NEXT(point_end, m_next_for_track) == NULL) return -1;

    length_left = 0.0f;
    
    for(segment = TAILQ_FIRST(&node->m_segments); segment; segment = TAILQ_NEXT(segment, m_next_for_node)) {
        float segment_length = segment->m_data.length;

        if (segment_length <= 0.0f) continue;
        
    NEXT_POINT:
        while (length_left <= 0) {
            point_begin = point_end;
            point_end = TAILQ_NEXT(point_begin, m_next_for_track);
            if (point_end == NULL) break;
            
            length_left = point_end->m_data.length;
        }
        
        if (segment_length > length_left) {
            segment_length -= length_left;
            length_left = 0;
            goto NEXT_POINT;
        }
        else if (segment_length == length_left) {
            length_left = 0;
            continue;
        }
        else {
            assert(segment_length < length_left);
            length_left -= segment_length;
            break;
        }
    };

    assert(point_begin);

    if (point_end) {
        if (length_left == 0.0f) {
            end_pos->x = point_end->m_data.pos.x;
            end_pos->y = point_end->m_data.pos.y;
        }
        else {
            assert(point_end->m_data.length > 0.0f);
            assert(point_end->m_data.length >= length_left);
            
            plugin_moving_control_calc_pos(
                end_pos, point_begin, point_end,
                (point_end->m_data.length - length_left) / point_end->m_data.length);
        }
    }
    else {
        end_pos->x = point_begin->m_data.pos.x;
        end_pos->y = point_begin->m_data.pos.y;
    }


    /* printf( */
    /*     "xxxxxxx: base=(%f,%f), start=(%f,%f), end=(%f,%f), result=(%f,%f)\n", */
    /*     pos->x, pos->y, */
    /*     point_start->m_data.pos.x, point_start->m_data.pos.y, */
    /*     end_pos.x, end_pos.y, */
    /*     control->m_origin_pos.x, control->m_origin_pos.y); */
    return 0;
}

void plugin_moving_control_calc_pos(ui_vector_2_t r, plugin_moving_plan_point_t begin_point, plugin_moving_plan_point_t end_point, float t1) {
    float   ax, bx, cx;
    float   ay, by, cy;
    float   tSquared, tCubed;
 
    /*計算多項式係數 */
    cx = 3.0 * (begin_point->m_data.control_next.x - begin_point->m_data.pos.x);
    bx = 3.0 * (end_point->m_data.control_pre.x - begin_point->m_data.control_next.x) - cx;
    ax = (end_point->m_data.pos.x - begin_point->m_data.pos.x) - cx - bx;
 
    cy = 3.0 * (begin_point->m_data.control_next.y - begin_point->m_data.pos.y);
    by = 3.0 * (end_point->m_data.control_pre.y - begin_point->m_data.control_next.y) - cy;
    ay = (end_point->m_data.pos.y - begin_point->m_data.pos.y) - cy - by;
 
    /*計算位於參數值t的曲線點 */
    tSquared = t1 * t1;
    tCubed = tSquared * t1;
 
    r->x = (ax * tCubed) + (bx * tSquared) + (cx * t1) + begin_point->m_data.pos.x;
    r->y = (ay * tCubed) + (by * tSquared) + (cy * t1) + begin_point->m_data.pos.y;

    /* printf( */
    /*     "xxxxx: calc: t=%f, p0=(%f,%f), p1=(%f,%f), p2=(%f,%f), p3=(%f,%f), result=%f,%f\n", */
    /*     t1, */
    /*     begin_point->m_data.pos.x, begin_point->m_data.pos.y, */
    /*     begin_point->m_data.control_next.x, begin_point->m_data.control_next.y, */
    /*     end_point->m_data.control_pre.x, end_point->m_data.control_pre.y,         */
    /*     end_point->m_data.pos.x, end_point->m_data.pos.y, */
    /*     r->x, r->y); */
}
