#ifndef DROW_RGUI_ANIM_CONTROL_FRAME_SCALE_H_INCLEDED
#define DROW_RGUI_ANIM_CONTROL_FRAME_SCALE_H_INCLEDED
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_anim_control_frame_scale.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlFrameScale
    : public AnimationGen<ControlFrameScale, plugin_ui_anim_control_frame_scale_t>
{
public:
    static const char * TYPE_NAME;

    ControlFrameScale & setDecorator(const char * decorator) {
        plugin_ui_anim_control_frame_scale_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlFrameScale & setTakeTime(float take_time) {
        plugin_ui_anim_control_frame_scale_set_take_time(*this, take_time);
        return *this;
    }

    ControlFrameScale & setTargetScale(float frame_scale) {
        ui_vector_2 s = UI_VECTOR_2_INITLIZER(frame_scale, frame_scale);
        plugin_ui_anim_control_frame_scale_set_target_scale(*this, &s);
        return *this;
    }

    ControlFrameScale & setTargetScale(ui_vector_2 const & frame_scale) {
        plugin_ui_anim_control_frame_scale_set_target_scale(*this, (ui_vector_2_t)(&frame_scale));
        return *this;
    }
};

}

#endif
