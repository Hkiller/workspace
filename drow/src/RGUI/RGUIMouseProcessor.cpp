#include "plugin/ui/plugin_ui_touch_track.h"
#include "RGUIControl.h"
#include "RGUIMouseProcessor.hpp"

namespace Drow {
    
MouseProcessor::~MouseProcessor() {
}

DragProcessor::DragProcessor(RGUIControl & control, plugin_ui_touch_track_t track, ui_vector_2 const & arg)
    : m_control(control)
    , m_down_pt(plugin_ui_control_pt_world_to_local(control.control(), plugin_ui_touch_track_cur_pt(track)))
    , m_down_arg(arg)
{
}

void DragProcessor::move(plugin_ui_touch_track_t track) {
    ui_vector_2 cur_pt = plugin_ui_control_pt_world_to_local(m_control.control(), plugin_ui_touch_track_cur_pt(track));
    doMove(track, m_down_pt, cur_pt, m_down_arg);
}

void DragProcessor::rise(plugin_ui_touch_track_t track) {
    ui_vector_2 cur_pt = plugin_ui_control_pt_world_to_local(m_control.control(), plugin_ui_touch_track_cur_pt(track));
    ui_vector_2 speed;
    speed.x = 0.0f;
    speed.y = 0.0f;
    doRise(track, m_down_pt, cur_pt, m_down_arg, speed);
}

}
