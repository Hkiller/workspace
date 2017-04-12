#ifndef UIPP_APP_ENV_EXT_H
#define UIPP_APP_ENV_EXT_H
#include "cpepp/utils/ObjHolder.hpp"
#include "cpepp/cfg/Node.hpp"
#include "uipp/app/Env.hpp"
#include "System.hpp"
#include "UICenterExt.hpp"
#include "RuningExt.hpp"

namespace UI { namespace App {

class EnvExt : public Env {
public:
    EnvExt(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg);
    ~EnvExt();

    virtual uint8_t debug(void) const;

    virtual const char * language(void) const;
    virtual const char * appName(void) const;

    virtual Gd::App::Application & app(void);
    virtual Gd::App::Application const & app(void) const;

    virtual RuningExt & runing(void);
    virtual RuningExt const & runing(void) const;

    virtual UICenterExt & uiCenter(void);
    virtual UICenterExt const & uiCenter(void) const;

    virtual Sprite::World & world(void);
    virtual Sprite::World const & world(void) const;

    virtual ui_runtime_module_t runtime(void);
    virtual ui_runtime_render_t context(void);
    virtual ui_cache_manager_t cacheMgr(void);
    virtual ui_data_mgr_t dataMgr(void);
    
    virtual const char * documentPath(void) const;

    static EnvExt & instance(Gd::App::Application & app);

private:
    void doFini(void);
    void registerEvents(Sprite::Repository & repo, Cpe::Cfg::Node const & config);
    void loadModel(Cpe::Cfg::Node const & config);
    void loadLanguages(Cpe::Cfg::Node const & cfg);
    void loadColors(Cpe::Cfg::Node const & srcs_cfg);
    void loadPackages(void);

    const char * detectLanguage(void) const;

    Gd::App::Application & m_app;
    Cpe::Utils::ObjHolder<UICenterExt> m_uiCenter;
    Cpe::Utils::ObjHolder<RuningExt> m_runing;

    mutable ::std::string m_language;
    mutable ::std::string m_documentPath;

    ::std::string m_appName;
    uint8_t m_debug;

    ui_sprite_world_t m_world;
    ui_runtime_module_t m_runtime_module;
};

}}

#endif



