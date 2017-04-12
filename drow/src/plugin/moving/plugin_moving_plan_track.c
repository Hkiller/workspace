#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_moving_plan_track_i.h"
#include "plugin_moving_plan_point_i.h"

/*moving_group */
plugin_moving_plan_track_t
plugin_moving_plan_track_create(plugin_moving_plan_t plan) {
    plugin_moving_module_t module = plan->m_module;
    plugin_moving_plan_track_t track;

    track = TAILQ_FIRST(&module->m_free_plan_tracks);
    if (track) {
        TAILQ_REMOVE(&module->m_free_plan_tracks, track, m_next_for_plan);
    }
    else {
        track = mem_alloc(module->m_alloc, sizeof(struct plugin_moving_plan_track));
        if (track == NULL) {
            CPE_ERROR(module->m_em, "create plan track: alloc track fail!");
            return NULL;
        }
    }

    track->m_plan = plan;
    bzero(&track->m_data, sizeof(track->m_data));
    track->m_point_count = 0;
    
    TAILQ_INIT(&track->m_points);
    
    plan->m_track_count++;
    TAILQ_INSERT_TAIL(&plan->m_tracks, track, m_next_for_plan);
    
    return track;;
}

void plugin_moving_plan_track_free(plugin_moving_plan_track_t track) {
    plugin_moving_plan_t plan = track->m_plan;
    plugin_moving_module_t module = plan->m_module;

    while(!TAILQ_EMPTY(&track->m_points)) {
        plugin_moving_plan_point_free(TAILQ_FIRST(&track->m_points));
    }
    assert(track->m_point_count == 0);
    
    plan->m_track_count--;
    TAILQ_REMOVE(&plan->m_tracks, track, m_next_for_plan);
    
    track->m_plan = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_plan_tracks, track, m_next_for_plan);
}

void plugin_moving_plan_track_real_free(plugin_moving_plan_track_t track) {
    plugin_moving_module_t module = (void*)track->m_plan;

    TAILQ_REMOVE(&module->m_free_plan_tracks, track, m_next_for_plan);
    mem_free(module->m_alloc, track);
}

MOVING_PLAN_TRACK *
plugin_moving_plan_track_data(plugin_moving_plan_track_t track) {
    return &track->m_data;
}

plugin_moving_plan_track_t plugin_moving_plan_track_find_by_id(plugin_moving_plan_t plan, uint16_t track_id) {
    plugin_moving_plan_track_t track;

    TAILQ_FOREACH(track, &plan->m_tracks, m_next_for_plan) {
        if (track->m_data.id == track_id) return track;
    }

    return NULL;
}

static plugin_moving_plan_point_t plugin_moving_plan_points_next(struct plugin_moving_plan_point_it * it) {
    plugin_moving_plan_point_t * data = (plugin_moving_plan_point_t *)(it->m_data);
    plugin_moving_plan_point_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_track);

    return r;
}

void plugin_moving_plan_track_points(plugin_moving_plan_point_it_t it, plugin_moving_plan_track_t track) {
    *(plugin_moving_plan_point_t *)(it->m_data) = TAILQ_FIRST(&track->m_points);
    it->next = plugin_moving_plan_points_next;
}
