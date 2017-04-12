#ifndef DROW_RGUI_ANIMATION_H_INCLEDED
#define DROW_RGUI_ANIMATION_H_INCLEDED
#include "cpepp/utils/ClassCategory.hpp"
#include "plugin/ui/plugin_ui_types.h"
#include "plugin/ui/plugin_ui_animation.h"
#include "RGUI.h"

namespace Drow {

class Animation : public Cpe::Utils::SimulateObject {
public:
    operator plugin_ui_animation_t () const { return (plugin_ui_animation_t)this; }

    Gd::App::Application & app(void);
    Gd::App::Application const & app(void) const;
    
    template<typename AnimT>
    AnimT & as(void) { return *(AnimT*)plugin_ui_animation_data(*this); }

    const char * name(void) const { return plugin_ui_animation_name(*this); }
    const char * typeName(void) const { return plugin_ui_animation_type_name(*this); }

    plugin_ui_animation_state_t state(void) const { return plugin_ui_animation_state(*this); }
    const char * stateStr(void) const { return plugin_ui_animation_state_str(*this); }    

    bool start(void);

    float delay(void) const { return plugin_ui_animation_delay(*this); }
    void setDelay(float delay) { plugin_ui_animation_set_delay(*this, delay); }
    void setDelayFrame(uint32_t frame) { plugin_ui_animation_set_delay_frame(*this, frame); }

    float loopDelay(void) const { return plugin_ui_animation_loop_delay(*this); }
    uint32_t loopCount(void) const { return plugin_ui_animation_loop_count(*this); }
    void setLoop(uint32_t loop_count, float loop_delay) { plugin_ui_animation_set_loop(*this, loop_count, loop_delay); }
};

}

#endif
