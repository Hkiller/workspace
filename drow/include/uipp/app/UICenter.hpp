#ifndef UIPP_APP_PAGECENTER_H
#define UIPP_APP_PAGECENTER_H
#include <memory>
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_page.h"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "System.hpp"

namespace UI { namespace App {

class UICenter {
public:
    plugin_ui_page_t findPage(const char * name) const { return plugin_ui_page_find(uiEnv(), name); }
    plugin_ui_page_t page(const char * name) const;

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(plugin_ui_env_app(uiEnv())); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(plugin_ui_env_app(uiEnv())); }
    
    virtual plugin_ui_env_t uiEnv(void) const = 0;

    virtual void phaseSwitch(const char * phase_name, const char * load_phase_name, dr_data_t data = NULL) = 0;
    virtual void phaseReset(void) = 0;    
    virtual void phaseCall(const char * phase_name, const char * load_phase_name, const char * back_phase_name, dr_data_t data = NULL) = 0;
    virtual void phaseBack(void) = 0;

    virtual void sendEvent(const char * def, dr_data_source_t data_source = NULL) = 0;
    virtual void sendEvent(LPDRMETA meta, void const * data, size_t data_size) = 0;

    virtual const char * visibleMsg(uint32_t msg_id) const = 0;
    virtual const char * visibleMsg(uint32_t msg_id, char * args) const = 0;
    virtual const char * visiableTime(uint32_t msg_id, uint32_t t) const = 0;
    virtual const char * visiableTime(uint32_t msg_id, uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t sec, uint8_t min) const = 0;
    virtual const char * visiableTimeDuration(uint32_t msg_id, int time_diff) const = 0;
    virtual const char * visiableTimeDuration(uint32_t msg_id, uint32_t base, uint32_t v) const = 0;

    template<typename T>
    void sendEvent(T const & data = T()) {
        sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    /*UI代理对象 */
    virtual Sprite::Entity & entity(void) = 0;
    virtual Sprite::Entity const & entity(void) const = 0;

    virtual ~UICenter();
};

}}

#endif
