#include <assert.h>
#include "plugin_moving_node_i.h"
#include "plugin_moving_plan_track_i.h"
#include "plugin_moving_plan_point_i.h"
#include "plugin_moving_plan_node_i.h"
#include "plugin_moving_plan_segment_i.h"

static void plugin_moving_node_set_current_point(plugin_moving_node_t node, plugin_moving_plan_point_t point);
static void plugin_moving_node_set_current_segment(plugin_moving_node_t node, plugin_moving_plan_segment_t segment);
static void plugin_moving_node_calc_durations(plugin_moving_node_t node, float segment_start, float point_start);

plugin_moving_node_t
plugin_moving_node_create(plugin_moving_control_t control, plugin_moving_plan_node_t plan_node, uint32_t loop_count) {
    plugin_moving_env_t env = control->m_env;
    plugin_moving_node_t node;
    plugin_moving_plan_track_t track;

    track = plugin_moving_plan_track_find_by_id(plan_node->m_plan, plan_node->m_data.track);
    if (track == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_moving_node: create: track %d not exist!", plan_node->m_data.track);
        return NULL;
    }
    
    node = TAILQ_FIRST(&env->m_free_nodes);
    if (node) {
        TAILQ_REMOVE(&env->m_free_nodes, node, m_next_for_control);
    }
    else {
        node = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_moving_node));
        if (node == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_moving_node: create: alloc fail!");
            return NULL;
        }
    }
    
    node->m_control = control;
    node->m_plan_node = plan_node;
    node->m_track = track;
    node->m_state = plugin_moving_node_state_init;
    node->m_time_scale = 1.0f;
    node->m_runing_time = 0.0f;
    node->m_loop_count = loop_count;
    node->m_update_fun = NULL;
    node->m_update_ctx = NULL;

    plugin_moving_node_set_current_segment(node, TAILQ_FIRST(&plan_node->m_segments));
    plugin_moving_node_set_current_point(node, TAILQ_FIRST(&track->m_points));
    plugin_moving_node_calc_durations(node, 0.0f, 0.0f);
    
    if (node->m_begin_point) {
        node->m_pos.x = control->m_origin_pos.x + node->m_begin_point->m_data.pos.x;
        node->m_pos.y = control->m_origin_pos.y + node->m_begin_point->m_data.pos.x;
    }
    else {
        node->m_pos.x = control->m_origin_pos.x;
        node->m_pos.y = control->m_origin_pos.y;
    }
    
    TAILQ_INSERT_TAIL(&control->m_nodes, node, m_next_for_control);

    return node;
}

void plugin_moving_node_free(plugin_moving_node_t node) {
    plugin_moving_control_t control = node->m_control;
    plugin_moving_env_t env = control->m_env;

    TAILQ_REMOVE(&control->m_nodes, node, m_next_for_control);

    node->m_control = (void*)env;
    TAILQ_INSERT_TAIL(&env->m_free_nodes, node, m_next_for_control);
}

void plugin_moving_node_real_free(plugin_moving_node_t node) {
    plugin_moving_env_t env = (void*)node->m_control;

    TAILQ_REMOVE(&env->m_free_nodes, node, m_next_for_control);
                                      
    mem_free(env->m_module->m_alloc, node);
}

plugin_moving_control_t plugin_moving_node_control(plugin_moving_node_t node) {
    return node->m_control;
}

const char * plugin_moving_node_name(plugin_moving_node_t node) {
    return node->m_plan_node->m_data.name;
}

plugin_moving_plan_segment_t plugin_moving_node_cur_segment(plugin_moving_node_t node) {
    return node->m_segment;
}

float plugin_moving_node_time_scale(plugin_moving_node_t node) {
    return node->m_time_scale;
}

void plugin_moving_node_set_time_scale(plugin_moving_node_t node, float time_scale) {
    node->m_time_scale = time_scale;
}

static void plugin_moving_node_set_current_point(plugin_moving_node_t node, plugin_moving_plan_point_t point) {
    if (point == NULL) {
        node->m_begin_point = NULL;
        node->m_end_point = NULL;
    }
    else {
        node->m_begin_point = point;
        node->m_end_point = TAILQ_NEXT(point, m_next_for_track);
        if (node->m_end_point == NULL) {
            uint8_t need_loop = node->m_loop_count == 1 ? 0 : 1;
            if (node->m_loop_count > 0) node->m_loop_count--;

            if (need_loop) {
                plugin_moving_plan_track_t track;
                if ((track = plugin_moving_plan_track_find_by_id(node->m_plan_node->m_plan, node->m_plan_node->m_data.track))) {
                    node->m_end_point = TAILQ_FIRST(&track->m_points);
                }
            }
        }
    }
    node->m_point_start_duration = 0.0f;
    node->m_point_duration = 0.0f;
}

static void plugin_moving_node_set_current_segment(plugin_moving_node_t node, plugin_moving_plan_segment_t segment) {
    node->m_segment = segment;
    node->m_segment_is_begin = 0;
    node->m_segment_start_duration = 0.0f;
    node->m_segment_duration = 0.0f;
}

static void plugin_moving_node_calc_durations(plugin_moving_node_t node, float segment_start, float point_start) {
    node->m_point_start_duration = point_start;
    node->m_segment_start_duration = segment_start;

    if (node->m_segment && node->m_end_point && node->m_segment->m_data.speed > 0.0f) {
        node->m_point_duration =  node->m_end_point->m_data.length / node->m_segment->m_data.speed - point_start;
        if (node->m_point_duration < 0.0f) node->m_point_duration = 0.0f;

        node->m_segment_duration = node->m_segment->m_data.length / node->m_segment->m_data.speed - segment_start;
        if (node->m_segment_duration < 0.0f) node->m_segment_duration = 0.0f;
    }
    else {
        node->m_point_duration =  0.0f;
        node->m_segment_duration = 0.0f;
    }
}

void plugin_moving_node_set_update_fun(plugin_moving_node_t node, plugin_moving_pos_update_fun_t update_fun, void * update_ctx) {
    node->m_update_fun = update_fun;
    node->m_update_ctx = update_ctx;
}

plugin_moving_pos_update_fun_t plugin_moving_node_update_fun(plugin_moving_node_t node) {
    return node->m_update_fun;
}

void * plugin_moving_node_update_ctx(plugin_moving_node_t node) {
    return node->m_update_ctx;
}

plugin_moving_plan_node_t plugin_moving_node_plan_node(plugin_moving_node_t node) {
    return node->m_plan_node;
}

plugin_moving_node_state_t plugin_moving_node_state(plugin_moving_node_t node) {
    return node->m_state;
}

ui_vector_2_t plugin_moving_node_pos(plugin_moving_node_t node) {
    return &node->m_pos;
}

static void plugin_moving_node_update_pos(plugin_moving_node_t node, float work_duration) {
    float t;
    if (work_duration >= node->m_point_duration) {
        t = 1.0f;
    }
    else {
        t = (node->m_point_start_duration + work_duration) / (node->m_point_start_duration + node->m_point_duration);
    }

    plugin_moving_control_calc_pos(&node->m_pos, node->m_begin_point, node->m_end_point, t);

    node->m_pos.x += node->m_control->m_origin_pos.x;
    node->m_pos.y += node->m_control->m_origin_pos.y;
}

void plugin_moving_node_update(plugin_moving_node_t node, float delta) {
    uint8_t is_complete = 1;

    if (node->m_state == plugin_moving_node_state_done) {
        plugin_moving_node_free(node);
        return;
    }

    delta *= node->m_time_scale;

    node->m_runing_time += delta;
    while (node->m_segment && node->m_end_point) {
        if (!node->m_segment_is_begin) {
            if (node->m_runing_time < node->m_segment->m_data.delay) {
                return;
            }

            if (node->m_update_fun) node->m_update_fun(node->m_update_ctx, node, plugin_moving_node_event_segment_begin);

            node->m_runing_time -= node->m_segment->m_data.delay;
            node->m_segment_is_begin = 1;
            node->m_point_duration =
                node->m_segment->m_data.speed > 0.0f
                ? node->m_end_point->m_data.length / node->m_segment->m_data.speed
                : 0.0f;

            if (node->m_state == plugin_moving_node_state_init) {
                node->m_state = plugin_moving_node_state_working;
            }
        }

        if (node->m_runing_time >= node->m_segment_duration || node->m_runing_time > node->m_point_duration) {
            float used_duration;

            /*point没有达到下一个，segment已经达到下一个节点 */
            if (node->m_segment_duration < node->m_point_duration) {
                plugin_moving_plan_segment_t next_segment;

                used_duration = node->m_segment_duration;
                plugin_moving_node_update_pos(node, used_duration);

                if (node->m_update_fun) node->m_update_fun(node->m_update_ctx, node, plugin_moving_node_event_segment_end);

                next_segment = TAILQ_NEXT(node->m_segment, m_next_for_node);
                if (next_segment == NULL) {
                    next_segment = TAILQ_FIRST(&node->m_plan_node->m_segments);
                    assert(next_segment);
                }

                plugin_moving_node_set_current_segment(node, next_segment);
                plugin_moving_node_calc_durations(node, 0.0f, node->m_point_start_duration + used_duration);
            }
            else if (node->m_point_duration < node->m_segment_duration) {
                used_duration = node->m_point_duration;
                plugin_moving_node_update_pos(node, used_duration);
                plugin_moving_node_set_current_point(node, node->m_end_point);
                plugin_moving_node_calc_durations(node, node->m_segment_start_duration + used_duration, 0.0f);
            }
            else {
                plugin_moving_plan_segment_t next_segment;

                used_duration = node->m_point_duration;
                plugin_moving_node_update_pos(node, used_duration);

                if (node->m_update_fun) node->m_update_fun(node->m_update_ctx, node, plugin_moving_node_event_segment_end);
                next_segment = TAILQ_NEXT(node->m_segment, m_next_for_node);
                if (next_segment == NULL) {
                    next_segment = TAILQ_FIRST(&node->m_plan_node->m_segments);
                    assert(next_segment);
                }

                plugin_moving_node_set_current_segment(node, next_segment);
                plugin_moving_node_set_current_point(node, node->m_end_point);
                plugin_moving_node_calc_durations(node, 0.0, 0.0f);
            }
            
            node->m_runing_time -= used_duration;
            continue;
        }
        else {
            plugin_moving_node_update_pos(node, node->m_runing_time);
            is_complete = 0;
            break;;
        }
    }

    if (is_complete) {
        node->m_state = plugin_moving_node_state_done;
    }
    
    if (node->m_update_fun) node->m_update_fun(node->m_update_ctx, node, plugin_moving_node_event_state_updated);
    return;
}
