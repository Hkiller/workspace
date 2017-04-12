#include "plugin_ui_env_i.h"
#include "plugin_ui_touch_track_i.h"
#include "plugin_ui_control_i.h"

uint8_t plugin_ui_env_accept_input(plugin_ui_env_t env) {
    return env->m_accept_input;
}

void plugin_ui_env_set_accept_input(plugin_ui_env_t env, uint8_t accept) {
    env->m_accept_input = accept;

    if (!env->m_accept_input) {
        while(!TAILQ_EMPTY(&env->m_touch_tracks)) {
            plugin_ui_touch_track_t track = TAILQ_FIRST(&env->m_touch_tracks);
            if (track->m_catch_control) {
                plugin_ui_control_dispatch_event(
                    track->m_catch_control, track->m_catch_control, plugin_ui_event_mouse_up, plugin_ui_event_dispatch_to_self_and_parent);
            }
            plugin_ui_touch_track_free(track);
        }
    }
}

float plugin_ui_env_long_push_span(plugin_ui_env_t env) {
    return env->m_long_push_span;
}

void plugin_ui_env_set_long_push_span(plugin_ui_env_t env, float span) {
    env->m_long_push_span = span;
}

uint8_t plugin_ui_env_process_touch_down(plugin_ui_env_t env, int32_t track_id, ui_vector_2_t pt) {
    plugin_ui_touch_track_t track;

    //CPE_ERROR(env->m_module->m_em, "xxxxx: plugin_ui_env_process_touch_down pt=(%f,%f)\n", pt->x, pt->y);
    
	if (!env->m_accept_input) return 0;
    if (!TAILQ_EMPTY(&env->m_touch_tracks) && !env->m_accept_multi_touch) {
        plugin_ui_env_dispatch_event(env, NULL, plugin_ui_event_mouse_down);
        return 0;
    }

    track = plugin_ui_touch_track_create(env, track_id);
    if (track == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_ui_env_process_touch_down: create track fail!");
        plugin_ui_env_dispatch_event(env, NULL, plugin_ui_event_mouse_down);
        return 0;
    }

    plugin_ui_touch_track_notify_down(track, pt);

    return track->m_catch_control ? 1 : 0;
}

uint8_t plugin_ui_env_process_touch_move(plugin_ui_env_t env, int32_t track_id, ui_vector_2_t pt) {
    plugin_ui_touch_track_t track;

	if (!env->m_accept_input) return 0;

    track = plugin_ui_touch_track_find(env, track_id);
    if (track == NULL) {
        if (!TAILQ_EMPTY(&env->m_touch_tracks) && !env->m_accept_multi_touch) {
            plugin_ui_env_dispatch_event(env, NULL, plugin_ui_event_mouse_down);
            return 0;
        }
        
        track = plugin_ui_touch_track_create(env, track_id);
        if (track == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_process_touch_move: create track fail!");
            plugin_ui_env_dispatch_event(env, NULL, plugin_ui_event_mouse_down);
            return 0;
        }

        plugin_ui_touch_track_notify_down(track, pt);
        return track->m_catch_control ? 1 : 0;
    }

    if (track->m_catch_control) {
        plugin_ui_touch_track_notify_move(track, pt);
        return 1;
    }
    else {
        plugin_ui_env_dispatch_event(env, NULL, plugin_ui_event_mouse_move);
        return 0;
    }
}

uint8_t plugin_ui_env_process_touch_rise(plugin_ui_env_t env, int32_t track_id, ui_vector_2_t pt) {
    plugin_ui_touch_track_t track;
    
	if (!env->m_accept_input) return 0;

    track = plugin_ui_touch_track_find(env, track_id);
    if (track == NULL) {
        if (!TAILQ_EMPTY(&env->m_touch_tracks) && !env->m_accept_multi_touch) {
            plugin_ui_env_dispatch_event(env, NULL, plugin_ui_event_mouse_up);
            return 0;
        }
        
        track = plugin_ui_touch_track_create(env, track_id);
        if (track == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_env_process_touch_move: create track fail!");
            plugin_ui_env_dispatch_event(env, NULL, plugin_ui_event_mouse_up);
            return 0;
        }

        plugin_ui_touch_track_notify_down(track, pt);
    }

    if (track->m_catch_control) {
        plugin_ui_touch_track_notify_rise(track, pt);
        plugin_ui_touch_track_free(track);
        return 1;
    }
    else {
        plugin_ui_env_dispatch_event(env, NULL, plugin_ui_event_mouse_up);
        plugin_ui_touch_track_free(track);
        return 0;
    }
}

