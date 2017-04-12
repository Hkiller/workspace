#include <assert.h>
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_popup.h"
#include "plugin/ui/plugin_ui_popup_action.h"
#include "plugin/ui/plugin_ui_popup_def.h"
#include "plugin/ui/plugin_ui_page_slot.h"
#include "plugin/ui/plugin_ui_page.h"
#include "gdpp/app/Log.hpp"
#include "RGUIPopup.hpp"

namespace Drow {

RGUIWindow & Popup::page(void) {
    return *(RGUIWindow*)plugin_ui_page_product(plugin_ui_popup_page(*this));
}

bool Popup::triggerAction(const char * action_name) {
    return plugin_ui_popup_trigger_action(*this, action_name) > 0;
}

Popup & Popup::set_data(const char * slot_name, const char * slot_value) {
    plugin_ui_page_slot_t slot = plugin_ui_page_slot_find(plugin_ui_popup_page(*this), slot_name);
    if (slot == NULL) {
        APP_CTX_THROW_EXCEPTION(
            plugin_ui_env_app(plugin_ui_popup_env(*this)), ::std::runtime_error,
            "Popup: no slot %s!", slot_name);
    }

    if (plugin_ui_page_slot_set_by_str(slot, slot_value) != 0) {
        APP_CTX_THROW_EXCEPTION(
            plugin_ui_env_app(plugin_ui_popup_env(*this)), ::std::runtime_error,
            "Popup: slot %s set value %s fail!", slot_name, slot_value);
    }

    return *this;
}

Popup & Popup::set_data(dr_data_t data) {
    if (plugin_ui_popup_set_data(*this, data) != 0) {
        APP_CTX_THROW_EXCEPTION(
            plugin_ui_env_app(plugin_ui_popup_env(*this)), ::std::runtime_error,
            "Popup: set data fail!");
    }

    return *this;
}

void * Popup::data(LPDRMETA check_meta) {
    plugin_ui_page_t page = plugin_ui_popup_page(*this);
    void * r;

    r = plugin_ui_page_data(page);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            plugin_ui_env_app(plugin_ui_popup_env(*this)), ::std::runtime_error,
            "Popup: no data!");
    }
    
    if (check_meta && plugin_ui_page_data_meta(page) != check_meta) {
        APP_CTX_THROW_EXCEPTION(
            plugin_ui_env_app(plugin_ui_popup_env(*this)), ::std::runtime_error,
            "Popup: data meta mismatch, require %s, but is %s!",
            dr_meta_name(check_meta), dr_meta_name(plugin_ui_page_data_meta(page)));
    }

    return r;
}

void * Popup::add_action(const char * name, plugin_ui_popup_action_fun_t fun, uint32_t capacity) {
    plugin_ui_popup_action_t action = plugin_ui_popup_action_create(*this, name, fun, NULL);
    assert(action);
    assert(capacity <= plugin_ui_popup_action_data_capacity(action));
    return plugin_ui_popup_action_data(action);
}

void Popup::show(void) {
    plugin_ui_popup_set_visible(*this, 1);
}

void Popup::hide(void) {
    plugin_ui_popup_set_visible(*this, 0);
}

Popup & Popup::cast(plugin_ui_popup_t popup) {
    return *(Popup*)popup;
}

Popup &
Popup::create(plugin_ui_env_t env, const char * def_name) {
    plugin_ui_popup_def_t popup_def = plugin_ui_popup_def_find(env, def_name);
    if (popup_def == NULL) {
        APP_CTX_THROW_EXCEPTION(
            plugin_ui_env_app(env), ::std::runtime_error,
            "create popup from %s: popup def not exist!", def_name);
    }

    plugin_ui_popup_t popup = plugin_ui_popup_def_create_popup(popup_def);
    if (popup == NULL) {
        APP_CTX_THROW_EXCEPTION(
            plugin_ui_env_app(env), ::std::runtime_error,
            "create popup from %s: popup create fail!", def_name);
    }

    return cast(popup);
}

}
