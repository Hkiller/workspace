#ifndef DROW_RGUI_ANIM_TIME_DURATION_H_INCLEDED
#define DROW_RGUI_ANIM_TIME_DURATION_H_INCLEDED
#include "plugin/ui/plugin_ui_anim_label_time_duration.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class LabelTimeDuration
    : public AnimationGen<LabelTimeDuration, plugin_ui_anim_label_time_duration_t>
{
public:
    static const char * TYPE_NAME;

    LabelTimeDuration & setDuration(float duration) {
        plugin_ui_anim_label_time_duration_set_duration(*this, duration);
        return *this;
    }
};

}

#endif
