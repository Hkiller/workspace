#include "cpe/utils/time_utils.h"
#include "cpe/pal/pal_stdio.h"
#include "cpepp/tl/Manager.hpp"
#include "cpepp/dr/Utils.hpp"
#include "uipp/sprite_2d/System.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/ui/plugin_ui_env.h"
#include "render/utils/ui_rect.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "protocol/plugin/app_env/app_env_pro.h"
#include "RuningExt.hpp"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"

namespace UI { namespace App {

RuningExt::RuningExt(EnvExt & env)
    : m_env(env)
    , m_lastUpdateTime(0)
{
}

RuningExt::~RuningExt() {
}

void RuningExt::setSize(int32_t w, int32_t h) {
    ui_vector_2 runtime_sz = UI_VECTOR_2_INITLIZER((float)w, (float)h);
    plugin_ui_env_set_runtime_sz(m_env.uiCenter().uiEnv(), &runtime_sz);

    ui_runtime_render_set_view_size(m_env.context(), &runtime_sz);

    APP_CTX_INFO(m_env.app(), "Runing: window size to (%d-%d)", w, h);
}

void RuningExt::init(void) {
	m_lastUpdateTime = cur_time_ms();
    m_fpsCalc.init();
}

void RuningExt::update(void) {
	int64_t curTime = cur_time_ms();
    int64_t diffTime = curTime - m_lastUpdateTime;
    if (diffTime < 0) {
        m_lastUpdateTime = curTime;
        m_fpsCalc.updateRenderTick(0.0f);
        doUpdate(0.0f);
    }
    else {
        int64_t delta_time = curTime - m_lastUpdateTime;
        uint32_t max_frame_time = 1000;
        if (delta_time > max_frame_time) {
            delta_time = max_frame_time;
        }

        float updateDelta = (float)delta_time / 1000.f;
        
        m_fpsCalc.updateRenderTick(updateDelta);
        m_lastUpdateTime = curTime;
        doUpdate(updateDelta);
    }
}

void RuningExt::doUpdate(float deltaTime) {
    if (state() <= ui_runtime_pause) return;
    m_env.app().tick(deltaTime);
}

void RuningExt::processInput(TouchAction _action, uint32_t _id, int16_t _x, int16_t _y) {
    ui_vector_2 pt = UI_VECTOR_2_INITLIZER((float)_x, (float)_y);
    plugin_ui_env_t ui_env = this->m_env.uiCenter().uiEnv();

    if (_action == TouchBegin) {
        plugin_ui_env_process_touch_down(ui_env, _id, &pt);
    }
    else if (_action == TouchMove) {
        plugin_ui_env_process_touch_move(ui_env, _id, &pt);
    }
    else if (_action == TouchEnd) {
        plugin_ui_env_process_touch_rise(ui_env, _id, &pt);
    }
}

const char * RuningExt::state_name(ui_runtime_runing_level_t state) {
    switch(state) {
    case ui_runtime_stop:
        return "Stop";
    case ui_runtime_pause:
        return "Pause";
    case ui_runtime_runing:
        return "Runing";
    default:
        return "Unknown";
    }
}

ui_runtime_runing_level_t RuningExt::state(void) const {
    return ui_runtime_module_runing_level(m_env.runtime());
}

void RuningExt::setState(ui_runtime_runing_level_t state) {
    if (this->state() == state) return;

    if (this->state() == ui_runtime_runing) {
        APP_CTX_INFO(m_env.app(), "Runing: notify suspend");

        APP_ENV_SUSPEND evt;
        evt.is_suspend = 1;
        plugin_app_env_send_notification(
            plugin_app_env_module_find_nc(m_env.app(), NULL),  Cpe::Dr::metaOf(evt), &evt, sizeof(evt));
    }
    
    APP_CTX_INFO(m_env.app(), "Runing: state %s ==> %s", state_name(this->state()), state_name(state));
    ui_runtime_module_set_runing_level(m_env.runtime(), state);

    if (this->state() == ui_runtime_runing) {
        APP_CTX_INFO(m_env.app(), "Runing: notify resume");
        tl_manage_update(m_env.app().tlManager());
        m_lastUpdateTime = cur_time_ms();

        APP_ENV_SUSPEND evt;
        evt.is_suspend = 0;
        plugin_app_env_send_notification(
            plugin_app_env_module_find_nc(m_env.app(), NULL),  Cpe::Dr::metaOf(evt), &evt, sizeof(evt));
    }
}

void RuningExt::stop(void) {
    plugin_app_env_module_t app_env = plugin_app_env_module_find_nc(m_env.app(), NULL);
    APP_ENV_STOP evt;
    plugin_app_env_send_notification(app_env,  Cpe::Dr::metaOf(evt), &evt, sizeof(evt));
}

Runing::~Runing() {
}

}}
