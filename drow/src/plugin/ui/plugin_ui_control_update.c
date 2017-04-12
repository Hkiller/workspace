#include "render/runtime/ui_runtime_render_obj.h"
#include "render/runtime/ui_runtime_render_obj_ref.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_control_timer_i.h"
#include "plugin_ui_control_frame_i.h"

int plugin_ui_control_do_update(plugin_ui_control_t control, void * ctx) {
    float delta = *(float*)ctx;
    plugin_ui_control_frame_t frame, next_frame;
    
    if (!TAILQ_EMPTY(&control->m_timers)) {
        plugin_ui_control_timer_process(control, delta * 1000);
    }
        
    if (control->m_cache_flag) {
        plugin_ui_control_update_cache(control, 0);
    }

    for(frame = TAILQ_FIRST(&control->m_frames); frame; frame = next_frame) {
        next_frame = TAILQ_NEXT(frame, m_next);

        ui_runtime_render_obj_ref_update(frame->m_render_obj_ref, delta);

        if (frame->m_auto_remove && !ui_runtime_render_obj_is_playing(ui_runtime_render_obj_ref_obj(frame->m_render_obj_ref))) {
            plugin_ui_control_frame_free(frame);
        }            
    }

    if (control->m_meta->m_update) control->m_meta->m_update(control, delta);

    cpe_ba_set(&control->m_flag, plugin_ui_control_flag_frame_updated, cpe_ba_false);

    return 1;
}
