#ifndef DROW_RGUI_ANIM_CONTROL_MOVEOUT_H_OUTCLEDED
#define DROW_RGUI_ANIM_CONTROL_MOVEOUT_H_OUTCLEDED
#include "plugin/ui/plugin_ui_anim_control_move_out.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlMoveOut : public AnimationGen<ControlMoveOut, plugin_ui_anim_control_move_out_t> {
public:
    static const char * TYPE_NAME;

    ControlMoveOut & setDecorator(const char * decorator) {
        plugin_ui_anim_control_move_out_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlMoveOut & setTakeTime(float take_time) {
        plugin_ui_anim_control_move_out_set_take_time(*this, take_time);
        return *this;
    }

    ControlMoveOut & setTakeTimeFrame(uint32_t frame) {
        plugin_ui_anim_control_move_out_set_take_time_frame(*this, frame);
        return *this;
    }

    ControlMoveOut & setTargetPos(plugin_ui_control_move_pos_t target_pos) {
        plugin_ui_anim_control_move_out_set_target_pos(*this, target_pos);
        return *this;
    }
};

}

#endif
