#include <vector>
#include <map>
#include <set>
#include "RGUILib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_strings.h"
#include "cpepp/cfg/Node.hpp"
#include "render/utils/ui_string_table.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_texture.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_group.h"
#include "plugin/ui/plugin_ui_module.h"
#include "plugin/ui/plugin_ui_env.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Entity.hpp"
#include "uipp/sprite/Repository.hpp"
#include "ui/sprite_ui/ui_sprite_ui_module.h"
#include "ui/sprite_ui/ui_sprite_ui_env.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/evt/EventCenter.hpp"
#include "gdpp/evt/EventResponserBase.hpp"
#include "EnvExt.hpp"

namespace UI { namespace App {

class UICenterImpl
    : public UICenterExt
    , public Gd::Evt::EventResponserBase
{
public:
    UICenterImpl(EnvExt & env, Cpe::Cfg::Node const & cfg)
        : Gd::Evt::EventResponserBase(Gd::Evt::EventCenter::instance(env.app()))
        , m_env(env)
        , m_ui_env(ui_sprite_ui_env_create(ui_sprite_ui_module_find_nc(env.app(), NULL), env.world()))
        , m_entity(*(UI::Sprite::Entity *)ui_sprite_ui_env_entity(m_ui_env))
    {
        ui_vector_2 base_size;
        base_size.x = cfg["env.base-size.w"];
        base_size.y = cfg["env.base-size.h"];

        plugin_ui_env_set_origin_sz(ui_sprite_ui_env_env(m_ui_env), &base_size);

        RGUILib::Init(ui_sprite_ui_env_env(m_ui_env));

        try {
            if (ui_sprite_ui_env_load(m_ui_env, cfg, env.language()) != 0) {
                APP_CTX_THROW_EXCEPTION(m_env.app(), ::std::runtime_error, "ui_sprite_ui_env_load fail!");
            }
        }
        catch(...) {
            doFini();
            throw;
        }
    }

    ~UICenterImpl() {
        doFini();
    }

    virtual Env & env(void) {
        return m_env;
    }

    virtual Env const & env(void) const {
        return m_env;
    }

    virtual Sprite::Entity & entity(void) {
        return m_entity;
    }

    virtual Sprite::Entity const & entity(void) const {
        return m_entity;
    }
    
    virtual void phaseSwitch(const char * phase_name, const char * load_phase_name, dr_data_t data = NULL) {
        plugin_ui_env_phase_switch_by_name(ui_sprite_ui_env_env(m_ui_env),  phase_name, load_phase_name, data);
    }

    virtual void phaseReset(void) {
        plugin_ui_env_phase_reset(ui_sprite_ui_env_env(m_ui_env));
    }

    virtual void phaseCall(const char * phase_name, const char * load_phase_name, const char * back_phase_name, dr_data_t data = NULL) {
        plugin_ui_env_phase_call_by_name(ui_sprite_ui_env_env(m_ui_env),  phase_name, load_phase_name, back_phase_name, data);
    }

    virtual void phaseBack(void) {
        plugin_ui_env_phase_back(ui_sprite_ui_env_env(m_ui_env));
    }

    virtual const char * visibleMsg(uint32_t msg_id) const {
        return ui_string_table_message(plugin_ui_env_string_table(uiEnv()), msg_id);
    }

    virtual const char * visibleMsg(uint32_t msg_id, char * args) const {
        return ui_string_table_message_format(plugin_ui_env_string_table(uiEnv()), msg_id, args);
    }

    virtual const char * visiableTime(uint32_t msg_id, uint32_t t) const {
        return ui_string_table_message_format_time_local(plugin_ui_env_string_table(uiEnv()), msg_id, t);
    }
    
    virtual const char * visiableTime(uint32_t msg_id, uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t sec, uint8_t min) const {
        return ui_string_table_message_format_time(plugin_ui_env_string_table(uiEnv()), msg_id, year, mon, day, hour, sec, min);
    }

    const char * visiableTimeDuration(uint32_t msg_id, int32_t time_diff) const {
        return ui_string_table_message_format_time_duration(plugin_ui_env_string_table(uiEnv()), msg_id, time_diff);
    }

    const char * visiableTimeDuration(uint32_t msg_id, uint32_t base, uint32_t v) const {
        return ui_string_table_message_format_time_duration_by_base(plugin_ui_env_string_table(uiEnv()), msg_id, base, v);
    }

    virtual plugin_ui_env_t uiEnv(void) const {
        return ui_sprite_ui_env_env(m_ui_env);
    }

    virtual void sendEvent(LPDRMETA meta, void const * data, size_t data_size) {
        plugin_ui_env_send_event(ui_sprite_ui_env_env(m_ui_env), meta, (void*)data, data_size);
    }

    virtual void sendEvent(const char * def, dr_data_source_t data_source) {
        plugin_ui_env_build_and_send_event(ui_sprite_ui_env_env(m_ui_env), def, data_source);

        ui_string_table_message(
            plugin_ui_env_string_table(ui_sprite_ui_env_env(m_ui_env)), 1);
    }

private:
    void doFini(void) {
        RGUILib::ShutDown(ui_sprite_ui_env_env(m_ui_env));
        ui_sprite_ui_env_free(m_env.world());
        m_ui_env = NULL;
    }

    EnvExt & m_env;
    ui_sprite_ui_env_t m_ui_env;
    Sprite::Entity & m_entity;
};

UICenter::~UICenter() {
}

::std::auto_ptr<UICenterExt>
UICenterExt::create(EnvExt & env, Cpe::Cfg::Node const & config) {
    return ::std::auto_ptr<UICenterExt>(new UICenterImpl(env, config));
}

plugin_ui_page_t
UICenter::page(const char * name) const {
    plugin_ui_page_t page = findPage(name);
    if (page == NULL) {
        APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "page %s not exist!", name);
    }

	return page;
}

}}

