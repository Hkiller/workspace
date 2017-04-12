#include <cassert>
#include "plugin/ui/plugin_ui_control_action.h"
#include "plugin/ui/plugin_ui_aspect.h"
#include "RGUIControlActionBuilder.hpp"

namespace Drow {

void * ControlActionBuilder::_create(plugin_ui_event_fun_t fun) {
    assert(m_event != plugin_ui_event_max);
    plugin_ui_control_action_t action = plugin_ui_control_action_create(m_control, m_event, m_scope, fun, NULL);
    assert(action);
    if (m_name_prefix) plugin_ui_control_action_set_name_prefix(action, m_name_prefix);
    if (m_asspect) plugin_ui_aspect_control_action_add(m_asspect, action, 1);
    return plugin_ui_control_action_data(action);
}

}
