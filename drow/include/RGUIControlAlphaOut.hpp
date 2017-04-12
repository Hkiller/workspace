#ifndef DROW_RGUI_ANIM_CONTROL_ALPHAOUT_H_OUTCLEDED
#define DROW_RGUI_ANIM_CONTROL_ALPHAOUT_H_OUTCLEDED
#include "plugin/ui/plugin_ui_anim_control_alpha_out.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlAlphaOut : public AnimationGen<ControlAlphaOut, plugin_ui_anim_control_alpha_out_t> {
public:
    static const char * TYPE_NAME;

    ControlAlphaOut & setDecorator(const char * decorator) {
        plugin_ui_anim_control_alpha_out_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlAlphaOut & setTakeTime(float take_time) {
        plugin_ui_anim_control_alpha_out_set_take_time(*this, take_time);
        return *this;
    }

    ControlAlphaOut & setTakeTimeFrame(uint32_t frame) {
        plugin_ui_anim_control_alpha_out_set_take_time_frame(*this, frame);
        return *this;
    }
};

}

#endif
