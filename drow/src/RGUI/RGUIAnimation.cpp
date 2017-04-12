#include <assert.h>
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_control.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "RGUIControlAnim.hpp"
#include "RGUIControlScale.hpp"
#include "RGUIControlMoveIn.hpp"
#include "RGUIControlMoveOut.hpp"
#include "RGUIControlAlphaIn.hpp"
#include "RGUIControlAlphaOut.hpp"
#include "RGUIControlMove.hpp"
#include "RGUIControlScroll.hpp"
#include "RGUIControlFrameScale.hpp"
#include "RGUIControlFrameMove.hpp"
#include "RGUILabelAnimTimeDuration.hpp"

namespace Drow {

Gd::App::Application & Animation::app(void) {
    return *(Gd::App::Application*)plugin_ui_env_app( plugin_ui_animation_env(*this));
}

Gd::App::Application const & Animation::app(void) const {
    return *(Gd::App::Application*)plugin_ui_env_app( plugin_ui_animation_env(*this));
}

bool Animation::start(void) {
    return plugin_ui_animation_start(*this) == 0 ? true : false;
}

ControlAnim & ControlAnim::run(uint8_t anim_type) {
    ui_data_control_anim_t anim_data =
        plugin_ui_control_find_anim_data(
            plugin_ui_animation_find_first_tie_control(toAnim()), anim_type);
    if (anim_data == NULL) {
        APP_CTX_THROW_EXCEPTION(
            toAnim().app(), ::std::runtime_error,
            "ControlAnim::run: anim type %d unknown!", anim_type);
    }

    return run(anim_data);
}

ControlAnim & ControlAnim::run(ui_data_control_anim_t data) {
    if (plugin_ui_anim_control_anim_set_data(*this, data) != 0) {
        APP_CTX_THROW_EXCEPTION(
            toAnim().app(), ::std::runtime_error,
            "ControlAnim::run: setData fail!");
    }
    return *this;
}

const char * ControlAnim::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_ANIM;
const char * ControlScale::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_SCALE;
const char * ControlScroll::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_SCROLL;
const char * ControlMoveIn::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_MOVE_IN;
const char * ControlMoveOut::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_MOVE_OUT;
const char * ControlAlphaIn::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_ALPHA_IN;
const char * ControlAlphaOut::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_ALPHA_OUT;
const char * ControlMove::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_MOVE;
const char * ControlFrameScale::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_FRAME_SCALE;
const char * ControlFrameMove::TYPE_NAME = PLUGIN_UI_ANIM_CONTROL_FRAME_MOVE;
const char * LabelTimeDuration::TYPE_NAME = PLUGIN_UI_ANIM_LABEL_TIME_DURATION;

}

