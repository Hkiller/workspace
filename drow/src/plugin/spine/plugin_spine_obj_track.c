#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "plugin_spine_obj_track_i.h"
#include "plugin_spine_obj_anim_i.h"

plugin_spine_obj_track_t
plugin_spine_obj_track_create(plugin_spine_obj_t obj, const char * track_name) {
    plugin_spine_module_t module = obj->m_module;
    plugin_spine_obj_track_t track;
    uint16_t new_track_idx = 0;
    
    TAILQ_FOREACH(track, &obj->m_tracks, m_next) {
        if (strcmp(track->m_name, track_name) == 0) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_track_create: track %s already exist!", track_name);
            return NULL;
        }

        if (track->m_track_index >= new_track_idx) new_track_idx = track->m_track_index + 1;
    }

    track = TAILQ_FIRST(&module->m_free_tracks);
    if (track) {
        TAILQ_REMOVE(&module->m_free_tracks, track, m_next);
    }
    else {
        track = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_obj_track));
        if (track == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_track_create: alloc fail!");
            return NULL;
        }
    }

    track->m_obj = obj;
    cpe_str_dup(track->m_name, sizeof(track->m_name), track_name);
    TAILQ_INIT(&track->m_anims);
    TAILQ_INIT(&track->m_done_anims);
    track->m_track_index = new_track_idx;
    track->m_time_scale = 1.0f;
    
    TAILQ_INSERT_TAIL(&obj->m_tracks, track, m_next);

    return track;
}

void plugin_spine_obj_track_free(plugin_spine_obj_track_t track) {
    plugin_spine_obj_t obj = track->m_obj;
    plugin_spine_module_t module = obj->m_module;

    while(!TAILQ_EMPTY(&track->m_anims)) {
        plugin_spine_obj_anim_free(TAILQ_FIRST(&track->m_anims));
    }

    spAnimationState_clearTrack(obj->m_anim_state, track->m_track_index);

    while(!TAILQ_EMPTY(&track->m_done_anims)) {
        plugin_spine_obj_anim_free(TAILQ_FIRST(&track->m_done_anims));
    }

    TAILQ_REMOVE(&obj->m_tracks, track, m_next);

    TAILQ_INSERT_TAIL(&module->m_free_tracks, track, m_next);
}

plugin_spine_obj_track_t plugin_spine_obj_track_find(plugin_spine_obj_t obj, const char * track_name) {
    plugin_spine_obj_track_t track;

    TAILQ_FOREACH(track, &obj->m_tracks, m_next) {
        if (strcmp(track->m_name, track_name) == 0) return track;
    }

    return NULL;
}

float plugin_spine_obj_track_time_scale(plugin_spine_obj_track_t track) {
    return track->m_time_scale;
}

void plugin_spine_obj_track_set_time_scale(plugin_spine_obj_track_t track, float time_scale) {
    plugin_spine_obj_anim_t anim;
    
    track->m_time_scale = time_scale;

    TAILQ_FOREACH(anim, &track->m_anims, m_next) {
        assert(anim->m_track_entry);
        anim->m_track_entry->timeScale = time_scale;
    }
}

void plugin_spine_obj_track_apply_all_animations(plugin_spine_obj_track_t track) {
    plugin_spine_obj_t obj = track->m_obj;
    plugin_spine_module_t module = obj->m_module;
	spTrackEntry * entry;
    int events_buf_count = 0;

    if (track->m_track_index >= obj->m_anim_state->tracksCount) return;
    
    entry = obj->m_anim_state->tracks[track->m_track_index];
    if (entry == NULL) return;

    spAnimation_apply(
        entry->animation, obj->m_skeleton, entry->lastTime, entry->endTime, 0,
        module->m_events_buf, &events_buf_count);

    while((entry = entry->next)) {
        spAnimation_apply(
            entry->animation, obj->m_skeleton, -1.0f, entry->endTime, 0,
            module->m_events_buf, &events_buf_count);
    }
}

void plugin_spine_obj_track_real_free_all(plugin_spine_module_t module) {
    while(!TAILQ_EMPTY(&module->m_free_tracks)) {
        plugin_spine_obj_track_t track = TAILQ_FIRST(&module->m_free_tracks);
        TAILQ_REMOVE(&module->m_free_tracks, track, m_next);
        mem_free(module->m_alloc, track);
    }
}
