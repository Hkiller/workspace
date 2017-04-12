#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "plugin_ui_touch_track_i.h"
#include "plugin_ui_phase_node_i.h"
#include "plugin_ui_state_node_i.h"
#include "plugin_ui_state_node_page_i.h"
#include "plugin_ui_page_i.h"
#include "plugin_ui_control_frame_i.h"

plugin_ui_touch_track_t plugin_ui_touch_track_create(plugin_ui_env_t env, int32_t track_id) {
    plugin_ui_touch_track_t track;

    /*保护windows环境下，鼠标Up事件丢失的情况（调试） */
    track = plugin_ui_touch_track_find(env, track_id);
    if (track) {
        plugin_ui_touch_track_free(track);
    }

    track = TAILQ_FIRST(&env->m_free_touch_tracks);
    if (track) {
        TAILQ_REMOVE(&env->m_free_touch_tracks, track, m_next_for_env);
    }
    else {
        track = mem_alloc(env->m_module->m_alloc, sizeof(struct plugin_ui_touch_track));
        if (track == NULL) {
            CPE_ERROR(env->m_module->m_em, "plugin_ui_touch_track_create: alloc fail!");
            return NULL;
        }
    }

    track->m_env = env;
    track->m_track_id = track_id;
    track->m_cache_duration = 0.0f;
    track->m_long_push_sended = 0;
    track->m_catch_control = NULL;
    track->m_process_control = NULL;
    track->m_control_down_pt = UI_VECTOR_2_ZERO;
    
    TAILQ_INSERT_TAIL(&env->m_touch_tracks, track, m_next_for_env);

    return track;
}

void plugin_ui_touch_track_free(plugin_ui_touch_track_t track) {
    TAILQ_REMOVE(&track->m_env->m_touch_tracks, track, m_next_for_env);
    TAILQ_INSERT_TAIL(&track->m_env->m_free_touch_tracks, track, m_next_for_env);
}

void plugin_ui_touch_track_real_free(plugin_ui_touch_track_t track) {
    TAILQ_REMOVE(&track->m_env->m_free_touch_tracks, track, m_next_for_env);
    mem_free(track->m_env->m_module->m_alloc, track);
}

plugin_ui_touch_track_t plugin_ui_touch_track_find(plugin_ui_env_t env, int32_t track_id) {
    plugin_ui_touch_track_t track;

    TAILQ_FOREACH(track, &env->m_touch_tracks, m_next_for_env) {
        if (track->m_track_id == track_id) return track;
    }

    return NULL;
}

plugin_ui_control_t plugin_ui_touch_track_process_control(plugin_ui_touch_track_t track) {
    return track->m_process_control;
}

void plugin_ui_touch_track_set_process_control(plugin_ui_touch_track_t track, plugin_ui_control_t process_control) {
    track->m_process_control = process_control;
}

plugin_ui_control_t plugin_ui_touch_track_catch_control(plugin_ui_touch_track_t track) {
    return track->m_catch_control;
}

ui_vector_2_t plugin_ui_touch_track_down_pt(plugin_ui_touch_track_t track) {
    return track->m_catch_control ? &track->m_down_pt : NULL;
}

ui_vector_2_t plugin_ui_touch_track_last_pt(plugin_ui_touch_track_t track) {
    return track->m_catch_control ? &track->m_last_pt : NULL;
}

ui_vector_2_t plugin_ui_touch_track_cur_pt(plugin_ui_touch_track_t track) {
    return track->m_catch_control ? &track->m_cur_pt : NULL;
}

ui_vector_2_t plugin_ui_touch_track_control_down_pt(plugin_ui_touch_track_t track) {
    return (track->m_catch_control
            && cpe_ba_get(&track->m_catch_control->m_flag, plugin_ui_control_flag_accept_move))
        ? &track->m_control_down_pt
        : NULL;
}

uint8_t plugin_ui_touch_track_is_horz_move(plugin_ui_touch_track_t track) {
    if (track->m_catch_control == NULL) return 0;
    return fabs(track->m_cur_pt.x - track->m_down_pt.x) > 8.0f;
}

uint8_t plugin_ui_touch_track_is_vert_move(plugin_ui_touch_track_t track) {
    if (track->m_catch_control == NULL) return 0;
    return fabs(track->m_cur_pt.y - track->m_down_pt.y) > 8.0f;
}

void plugin_ui_touch_track_notify_down(plugin_ui_touch_track_t track, ui_vector_2_t pt) {
    plugin_ui_phase_node_t cur_phase;

    if (track->m_catch_control) {
        plugin_ui_control_dispatch_event(
            track->m_catch_control, track->m_catch_control, plugin_ui_event_mouse_up, plugin_ui_event_dispatch_to_self_and_parent);
        track->m_catch_control = NULL;
        track->m_cache_duration = 0.0f;
    }

    track->m_catch_control = NULL;
    if ((cur_phase = plugin_ui_phase_node_current(track->m_env))) {
        plugin_ui_page_t page;
            
        TAILQ_FOREACH_REVERSE(page, &track->m_env->m_visible_pages, plugin_ui_page_list, m_next_for_visible_queue) {
            track->m_catch_control = plugin_ui_control_find_click(&page->m_control, pt);
            if (track->m_catch_control) {
                goto CONTROL_FOUND;
            }
        }
    }

CONTROL_FOUND:
    if (track->m_catch_control) {
        plugin_ui_control_t control = track->m_catch_control;
        plugin_ui_page_t page = control->m_page;
        uint8_t page_tag_local = 0;
        uint8_t control_tag_local = 0;
        plugin_ui_control_frame_t touchable_frame;
        
        assert(!page->m_control.m_is_free);
        assert(!control->m_is_free);

        if (!page->m_control.m_is_processing) {
            page->m_control.m_is_processing = 1;
            page_tag_local = 1;
        }
    
        if (!control->m_is_processing) {
            control->m_is_processing = 1;
            control_tag_local = 1;
        }

        touchable_frame = plugin_ui_control_frame_find_touchable(control);
        
        track->m_down_pt = *pt;
        track->m_last_pt = *pt;
        track->m_cur_pt = *pt;

        if (touchable_frame) {
            plugin_ui_control_frame_touch_dispatch(touchable_frame, track->m_track_id, ui_runtime_touch_begin, pt);
        }
        else {
            plugin_ui_control_dispatch_event(
                track->m_catch_control, track->m_catch_control,
                plugin_ui_event_mouse_down, plugin_ui_event_dispatch_to_self_and_parent);
        }
        
        if (page->m_control.m_is_free || control->m_is_free) goto COMPLETE;
        
        plugin_ui_env_set_focus_control(track->m_env, track->m_catch_control);

        if (track->m_env->m_debug) {
            CPE_INFO(
                track->m_env->m_module->m_em,
                "plugin_ui_touch_track_notify_down: catch control %s",
                plugin_ui_control_path_dump(&track->m_env->m_module->m_dump_buffer, track->m_catch_control));
        }

        if (cpe_ba_get(&track->m_catch_control->m_flag, plugin_ui_control_flag_accept_move)) {
            track->m_control_down_pt = track->m_catch_control->m_render_pt_abs;

            plugin_ui_control_dispatch_event(
                track->m_catch_control, track->m_catch_control,
                plugin_ui_event_move_begin, plugin_ui_event_dispatch_to_self_and_parent);
            
            if (page->m_control.m_is_free || control->m_is_free) goto COMPLETE;
        }

    COMPLETE:
        if (control_tag_local) {
            control->m_is_processing = 0;
            if (control->m_is_free) plugin_ui_control_free(control);
        }

        if (page_tag_local) {
            page->m_control.m_is_processing = 0;
            if (page->m_control.m_is_free) plugin_ui_page_free(page);
        }
    }
    else {
        plugin_ui_env_dispatch_event(track->m_env, NULL, plugin_ui_event_mouse_down);
    }        
}

void plugin_ui_touch_track_notify_move(plugin_ui_touch_track_t track, ui_vector_2_t pt) {
    plugin_ui_control_t control = track->m_catch_control;
    plugin_ui_page_t page = control->m_page;
    uint8_t page_tag_local = 0;
    uint8_t control_tag_local = 0;
    plugin_ui_control_frame_t touchable_frame;
    
    assert(track->m_catch_control);

    assert(!page->m_control.m_is_free);
    assert(!control->m_is_free);

    touchable_frame = plugin_ui_control_frame_find_touchable(control);
    
    if (!page->m_control.m_is_processing) {
        page->m_control.m_is_processing = 1;
        page_tag_local = 1;
    }
    
    if (!control->m_is_processing) {
        control->m_is_processing = 1;
        control_tag_local = 1;
    }

    assert(track->m_catch_control);

    track->m_last_pt = track->m_cur_pt;
    track->m_cur_pt = *pt;

    if (touchable_frame) {
        plugin_ui_control_frame_touch_dispatch(touchable_frame, track->m_track_id, ui_runtime_touch_move, pt);
    }
    else {
        plugin_ui_control_dispatch_event(
            track->m_catch_control, track->m_catch_control,
            plugin_ui_event_mouse_move, plugin_ui_event_dispatch_to_self_and_parent);
    }
    
    if (page->m_control.m_is_free || control->m_is_free) goto COMPLETE;

    if (cpe_ba_get(&track->m_catch_control->m_flag, plugin_ui_control_flag_accept_move)) {
        ui_vector_2 cur_control_pt;

        if((fabs(track->m_cur_pt.x - track->m_down_pt.x)  > 6.0f) || (fabs(track->m_cur_pt.y - track->m_down_pt.y) > 6.0f)) {
            cur_control_pt.x = track->m_control_down_pt.x + (track->m_cur_pt.x - track->m_down_pt.x);
            cur_control_pt.y = track->m_control_down_pt.y + (track->m_cur_pt.y - track->m_down_pt.y);
            plugin_ui_control_set_render_pt_abs(track->m_catch_control, &cur_control_pt);
            plugin_ui_control_update_cache(track->m_catch_control, 0);

            plugin_ui_control_dispatch_event(
                track->m_catch_control, track->m_catch_control,
                plugin_ui_event_move_moving, plugin_ui_event_dispatch_to_self_and_parent);
            if (page->m_control.m_is_free || control->m_is_free) goto COMPLETE;
        }
    }

COMPLETE:
    if (control_tag_local) {
        control->m_is_processing = 0;
        if (control->m_is_free) plugin_ui_control_free(control);
    }

    if (page_tag_local) {
        page->m_control.m_is_processing = 0;
        if (page->m_control.m_is_free) plugin_ui_page_free(page);
    }
}

void plugin_ui_touch_track_notify_rise(plugin_ui_touch_track_t track, ui_vector_2_t pt) {
    plugin_ui_control_t control = track->m_catch_control;
    plugin_ui_page_t page = control->m_page;
    uint8_t page_tag_local = 0;
    uint8_t control_tag_local = 0;
    plugin_ui_control_frame_t touchable_frame;

    assert(track->m_catch_control);

    assert(!page->m_control.m_is_free);
    assert(!control->m_is_free);

    touchable_frame = plugin_ui_control_frame_find_touchable(control);
    
    if (!page->m_control.m_is_processing) {
        page->m_control.m_is_processing = 1;
        page_tag_local = 1;
    }
    
    if (!control->m_is_processing) {
        control->m_is_processing = 1;
        control_tag_local = 1;
    }

    track->m_last_pt = track->m_cur_pt;
    track->m_cur_pt = *pt;

    if (touchable_frame) {
        plugin_ui_control_frame_touch_dispatch(touchable_frame, track->m_track_id, ui_runtime_touch_end, pt);
    }
    else {
        plugin_ui_control_dispatch_event(
            track->m_catch_control, track->m_catch_control,
            plugin_ui_event_mouse_up, plugin_ui_event_dispatch_to_self_and_parent);
    }
    if (page->m_control.m_is_free || control->m_is_free) goto COMPLETE;
    
    if (track->m_process_control == NULL
        && (track->m_catch_control && plugin_ui_control_contain_test(track->m_catch_control, pt)))
    {
        if (cpe_ba_get(&control->m_flag, plugin_ui_control_flag_accept_double_click)) {
            if (page->m_env->m_double_click_control == control) {
                plugin_ui_control_dispatch_event(
                    track->m_catch_control, track->m_catch_control,
                    plugin_ui_event_mouse_double_click, plugin_ui_event_dispatch_to_self_and_parent);
                page->m_env->m_double_click_control = NULL;
                page->m_env->m_double_click_duration = 0.0f;
            }
            else {
                plugin_ui_env_set_double_click_control(page->m_env, control);
            }
        }
        else {
            plugin_ui_control_dispatch_event(
                track->m_catch_control, track->m_catch_control,
                plugin_ui_event_mouse_click, plugin_ui_event_dispatch_to_self_and_parent);
        }
        
        if (page->m_control.m_is_free || control->m_is_free) goto COMPLETE;
    }

    if (track->m_catch_control && cpe_ba_get(&track->m_catch_control->m_flag, plugin_ui_control_flag_accept_move)) {
        ui_vector_2 cur_control_pt;

        cur_control_pt.x = track->m_control_down_pt.x + (track->m_cur_pt.x - track->m_down_pt.x);
        cur_control_pt.y = track->m_control_down_pt.y + (track->m_cur_pt.y - track->m_down_pt.y);
        plugin_ui_control_set_render_pt_abs(track->m_catch_control, &cur_control_pt);
        plugin_ui_control_update_cache(track->m_catch_control, 0);
        
        plugin_ui_control_dispatch_event(
            track->m_catch_control, track->m_catch_control,
            plugin_ui_event_move_done, plugin_ui_event_dispatch_to_self_and_parent);
        if (page->m_control.m_is_free || control->m_is_free) goto COMPLETE;
    }

COMPLETE:
    if (control_tag_local) {
        control->m_is_processing = 0;
        if (control->m_is_free) plugin_ui_control_free(control);
    }

    if (page_tag_local) {
        page->m_control.m_is_processing = 0;
        if (page->m_control.m_is_free) plugin_ui_page_free(page);
    }
    
    track->m_catch_control = NULL;
    track->m_cache_duration = 0.0f;
}
