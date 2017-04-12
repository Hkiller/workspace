#ifndef DROW_RGUI_ANIM_CONTROL_SCROLL_H_INCLEDED
#define DROW_RGUI_ANIM_CONTROL_SCROLL_H_INCLEDED
#include "plugin/ui/plugin_ui_anim_control_scroll.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlScroll
    : public AnimationGen<ControlScroll, plugin_ui_anim_control_scroll_t>
{
public:
    static const char * TYPE_NAME;

    ControlScroll & setGuardDone(uint8_t guard_done) {
        plugin_ui_anim_control_scroll_set_guard_done(*this, guard_done);
        return *this;
    }
        
    ControlScroll & setDecorator(const char * decorator) {
        plugin_ui_anim_control_scroll_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlScroll & setTakeTime(float take_time) {
        plugin_ui_anim_control_scroll_set_take_time(*this, take_time);
        return *this;
    }

    ControlScroll & setTargetX(float x) {
        plugin_ui_anim_control_scroll_set_target_x(*this, x);
        return *this;
    }
    uint8_t processX(void) const { return plugin_ui_anim_control_scroll_process_x(*this); }
    
    ControlScroll & setTargetY(float y) {
        plugin_ui_anim_control_scroll_set_target_y(*this, y);
        return *this;
    }
    uint8_t processY(void) const { return plugin_ui_anim_control_scroll_process_y(*this); }
};

}

#endif
