#ifndef DROW_RGUI_ANIM_CONTROL_FRAME_MOVE_H_INCLEDED
#define DROW_RGUI_ANIM_CONTROL_FRAME_MOVE_H_INCLEDED
#include "render/utils/ui_vector_2.h"
#include "plugin/ui/plugin_ui_anim_control_frame_move.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlFrameMove
    : public AnimationGen<ControlFrameMove, plugin_ui_anim_control_frame_move_t>
{
public:
    static const char * TYPE_NAME;

    ControlFrameMove & setDecorator(const char * decorator) {
        plugin_ui_anim_control_frame_move_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlFrameMove & setTakeTime(float take_time) {
        plugin_ui_anim_control_frame_move_set_take_time(*this, take_time);
        return *this;
    }

    ControlFrameMove & setTargetPos(float pos) {
        ui_vector_2 pt = UI_VECTOR_2_INITLIZER(pos, pos);
        plugin_ui_anim_control_frame_move_set_target_pos(*this, &pt);
        return *this;
    }

    ControlFrameMove & setTargetPos(ui_vector_2 const & pos) {
        plugin_ui_anim_control_frame_move_set_target_pos(*this, (ui_vector_2_t)(&pos));
        return *this;
    }
};

}

#endif
