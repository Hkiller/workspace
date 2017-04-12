#ifndef DROW_RGUI_ANIM_CONTROL_MOVEIN_H_INCLEDED
#define DROW_RGUI_ANIM_CONTROL_MOVEIN_H_INCLEDED
#include "plugin/ui/plugin_ui_anim_control_move_in.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlMoveIn : public AnimationGen<ControlMoveIn, plugin_ui_anim_control_move_in_t>
{
public:
    static const char * TYPE_NAME;

    ControlMoveIn & setDecorator(const char * decorator) {
        plugin_ui_anim_control_move_in_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlMoveIn & setTakeTime(float take_time) {
        plugin_ui_anim_control_move_in_set_take_time(*this, take_time);
        return *this;
    }

    ControlMoveIn & setTakeTimeFrame(uint32_t frame) {
        plugin_ui_anim_control_move_in_set_take_time_frame(*this, frame);
        return *this;
    }

    ControlMoveIn & setOriginPos(plugin_ui_control_move_pos_t origin_pos) {
        plugin_ui_anim_control_move_in_set_origin_pos(*this, origin_pos);
        return *this;
    }

    ControlMoveIn & setBeginAt(float begin_at) {
        plugin_ui_anim_control_move_in_set_begin_at(*this, begin_at);
        return *this;
    }

    ControlMoveIn & setDelay(float delay) {
        //plugin_ui_animation_set_delay(*this, delay);
        //plugin_ui_anim_control_move_in_set_begin_at(*this, begin_at);
        return *this;
    }

};

}

#endif
