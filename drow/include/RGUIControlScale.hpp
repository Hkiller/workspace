#ifndef DROW_RGUI_ANIM_CONTROL_SCALE_H_INCLEDED
#define DROW_RGUI_ANIM_CONTROL_SCALE_H_INCLEDED
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_anim_control_scale.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlScale
    : public AnimationGen<ControlScale, plugin_ui_anim_control_scale_t>
{
public:
    static const char * TYPE_NAME;

    ControlScale & setDecorator(const char * decorator) {
        plugin_ui_anim_control_scale_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlScale & setTakeTime(float take_time) {
        plugin_ui_anim_control_scale_set_take_time(*this, take_time);
        return *this;
    }

    ControlScale & setTargetScale(float scale) {
        ui_vector_2 s = UI_VECTOR_2_INITLIZER(scale, scale);
        plugin_ui_anim_control_scale_set_target_scale(*this, &s);
        return *this;
    }

    ControlScale & setTargetScale(ui_vector_2 const & scale) {
        plugin_ui_anim_control_scale_set_target_scale(*this, (ui_vector_2_t)(&scale));
        return *this;
    }
};

}

#endif
