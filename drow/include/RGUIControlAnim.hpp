#ifndef DROW_RGUI_ANIM_CONTROL_ANIM_H_INCLEDED
#define DROW_RGUI_ANIM_CONTROL_ANIM_H_INCLEDED
#include "plugin/ui/plugin_ui_anim_control_anim.h"
#include "render/model/ui_data_layout.h"
#include "RGUIAnimGen.hpp"

namespace Drow {

class ControlAnim
    : public AnimationGen<ControlAnim, plugin_ui_anim_control_anim_t>
{
public:
    static const char * TYPE_NAME;

    ControlAnim & show(void) { return run(ui_control_anim_type_show); }
    ControlAnim & hide(void) { return run(ui_control_anim_type_hide); }
    ControlAnim & dead(void) { return run(ui_control_anim_type_dead); }
    ControlAnim & down(void) { return run(ui_control_anim_type_down); }
    ControlAnim & rise(void) { return run(ui_control_anim_type_rise); }
    ControlAnim & user(void) { return run(ui_control_anim_type_user); }
    
    ControlAnim & run(uint8_t data);
    ControlAnim & run(ui_data_control_anim_t data);

    ControlAnim & setLoopCount(uint32_t loop_count) {
        plugin_ui_anim_control_anim_set_loop_count(*this, loop_count);
        return *this;
    }
};

}

#endif
