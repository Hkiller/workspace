#ifndef DROW_RGUI_ANIM_CONTROL_ALPHAIN_H_INCLEDED
#define DROW_RGUI_ANIM_CONTROL_ALPHAIN_H_INCLEDED
#include "plugin/ui/plugin_ui_anim_control_alpha_in.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlAlphaIn : public AnimationGen<ControlAlphaIn, plugin_ui_anim_control_alpha_in_t>
{
public:
    static const char * TYPE_NAME;

    ControlAlphaIn & setDecorator(const char * decorator) {
        plugin_ui_anim_control_alpha_in_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlAlphaIn & setTakeTime(float take_time) {
        plugin_ui_anim_control_alpha_in_set_take_time(*this, take_time);
        return *this;
    }

    ControlAlphaIn & setTakeTimeFrame(uint32_t frame) {
        plugin_ui_anim_control_alpha_in_set_take_time_frame(*this, frame);
        return *this;
    }
};

}

#endif
