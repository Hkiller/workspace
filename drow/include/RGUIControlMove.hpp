#ifndef DROW_RGUI_ANIM_CONTROL_MOVE_H_OUTCLEDED
#define DROW_RGUI_ANIM_CONTROL_MOVE_H_OUTCLEDED
#include "plugin/ui/plugin_ui_anim_control_move.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlMove : public AnimationGen<ControlMove, plugin_ui_anim_control_move_t> {
public:
    static const char * TYPE_NAME;

    ControlMove & setDecorator(const char * decorator) {
        plugin_ui_anim_control_move_set_decorator(*this, decorator);
        return *this;
    }
    
    ControlMove & setTakeTime(float take_time) {
        plugin_ui_anim_control_move_set_take_time(*this, take_time);
        return *this;
    }

    ControlMove & setTakeTimeFrame(uint32_t frame) {
        plugin_ui_anim_control_move_set_take_time_frame(*this, frame);
        return *this;
    }

    ControlMove & setTarget(const char * target) {
        plugin_ui_anim_control_move_set_target(*this, target);
        return *this;
    }

    ControlMove & setOrigin(const char * origin) {
        plugin_ui_anim_control_move_set_target(*this, origin);
        return *this;
    }

    ControlMove & setAlogrithm(plugin_ui_move_algorithm_t algorithm) {
        plugin_ui_anim_control_move_set_algorithm(*this, algorithm);
        return *this;
    }
};

}

#endif
