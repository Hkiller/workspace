#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_moving_plan_point_i.h"

/*moving_group */
plugin_moving_plan_point_t
plugin_moving_plan_point_create(plugin_moving_plan_track_t track) {
    plugin_moving_module_t module = track->m_plan->m_module;
    plugin_moving_plan_point_t point;

    point = TAILQ_FIRST(&module->m_free_plan_points);
    if (point) {
        TAILQ_REMOVE(&module->m_free_plan_points, point, m_next_for_track);
    }
    else {
        point = mem_alloc(module->m_alloc, sizeof(struct plugin_moving_plan_point));
        if (point == NULL) {
            CPE_ERROR(module->m_em, "create plan point: alloc point fail!");
            return NULL;
        }
    }

    point->m_track = track;
    bzero(&point->m_data, sizeof(point->m_data));

    track->m_point_count++;
    TAILQ_INSERT_TAIL(&track->m_points, point, m_next_for_track);
    
    return point;;
}

void plugin_moving_plan_point_free(plugin_moving_plan_point_t point) {
    plugin_moving_plan_track_t track = point->m_track;
    plugin_moving_module_t module = track->m_plan->m_module;

    track->m_point_count--;
    TAILQ_REMOVE(&track->m_points, point, m_next_for_track);
    
    point->m_track = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_plan_points, point, m_next_for_track);
}

void plugin_moving_plan_point_real_free(plugin_moving_plan_point_t point) {
    plugin_moving_module_t module = (void*)point->m_track;

    TAILQ_REMOVE(&module->m_free_plan_points, point, m_next_for_track);
    mem_free(module->m_alloc, point);
}

MOVING_PLAN_POINT *
plugin_moving_plan_point_data(plugin_moving_plan_point_t point) {
    return &point->m_data;
}
