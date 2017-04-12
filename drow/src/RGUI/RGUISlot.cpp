#include <assert.h>
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_page_slot.h"
#include "gdpp/app/Log.hpp"
#include "RGUISlot.hpp"

namespace Drow {

Slot & Slot::operator=(const char * value) {
    if (plugin_ui_page_slot_set_by_str(*this, value) != 0) {
        APP_CTX_THROW_EXCEPTION(
            plugin_ui_env_app(plugin_ui_page_env(plugin_ui_page_slot_page(*this))), ::std::runtime_error,
            "Slot: set str %s fail!", value);
    }
    
    return *this;
}

}
