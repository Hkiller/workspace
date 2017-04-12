#include "cpe/cfg/cfg_read.h"
#include "cpepp/cfg/Tree.hpp"
#include "cpepp/nm/Manager.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "gdpp/app/Application.hpp"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "render/utils/ui_color.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_language.h"
#include "render/model_bin/ui_bin_loader.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_color.h"        
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render.h"
#include "plugin/package/plugin_package_module.h"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Repository.hpp"
#include "uipp/sprite_cfg/CfgLoader.hpp"
#include "uipp/sprite_cfg/CfgContext.hpp"
#include "plugin/ui/plugin_ui_env.h"
#include "ui/sprite_ui/ui_sprite_ui_env.h"
#include "EnvExt.hpp"
#include "UICenterExt.hpp"

namespace UI { namespace App {

EnvExt::EnvExt(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg)
    : m_app(app)
    , m_runing(::std::auto_ptr<RuningExt>(new RuningExt(*this)))
    , m_debug(moduleCfg["debug"].dft((uint8_t)0))
    , m_world(NULL)
    , m_runtime_module(ui_runtime_module_find_nc(app, NULL))
{
    assert(m_runtime_module);
    
    Cpe::Cfg::Tree ui_data;
    if (cfg_bin_read_file(ui_data.root(), gd_app_vfs_mgr(app), "runtime.bin", app.em()) != 0) {
        APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "%s: read runing bin fail!", name());
    }
    Cpe::Cfg::Node const & root_cfg = ui_data.root();

    // Cpe::Utils::MemBuffer buffer(NULL);
    // printf("xxxxx: dump cfg\n\n%s\n", cfg_dump(root_cfg["res-cfg"], buffer, 1, 1));
 
    Cpe::Cfg::Node const & cfg = root_cfg["ui"];

    const char * language = NULL;//app.cfg()["env.language"].asString(NULL);
    if (language) {
        m_language = language;
        APP_CTX_INFO(app, "%s: package language %s!", name(), m_language.c_str());
    }
    
    m_appName = cfg["env.app-name"].asString();

    UI::Sprite::Cfg::CfgContext cfg_ctx(UI::Sprite::Cfg::CfgLoader::instance(app), root_cfg);
    
    try {
        loadLanguages(root_cfg["languages"]);
        loadColors(cfg["colors"]);
        loadPackages();
        
        Sprite::Repository & repo = Sprite::Repository::instance(m_app);

        m_world = Sprite::World::create(repo, "UI");
        registerEvents(repo, cfg["env.events"]);

        m_uiCenter.reset(UICenterExt::create(*this, cfg));
    }
    catch(...) {
        doFini();
        throw;
    }
}

EnvExt::~EnvExt() {
    doFini();
}

uint8_t EnvExt::debug(void) const {
    return m_debug;
}

const char * EnvExt::language(void) const {
    if (m_language.empty()) {
        m_language = detectLanguage();
    }

    return m_language.c_str();
}

const char * EnvExt::appName(void) const {
    return m_appName.c_str();
}

Gd::App::Application & EnvExt::app(void) {
    return m_app;
}

Gd::App::Application const & EnvExt::app(void) const {
    return m_app;
}

UICenterExt & EnvExt::uiCenter(void) {
    return m_uiCenter;
}

UICenterExt const & EnvExt::uiCenter(void) const {
    return m_uiCenter;
}

RuningExt & EnvExt::runing(void) {
    return m_runing;
}

RuningExt const & EnvExt::runing(void) const {
    return m_runing;
}

ui_runtime_module_t EnvExt::runtime(void) {
    return m_runtime_module;
}

ui_runtime_render_t EnvExt::context(void) {
    return ui_runtime_module_render(m_runtime_module);
}

ui_cache_manager_t EnvExt::cacheMgr(void) {
    return ui_runtime_module_cache_mgr(m_runtime_module);
}

ui_data_mgr_t EnvExt::dataMgr(void) {
    return ui_runtime_module_data_mgr(m_runtime_module);
}

Sprite::World & EnvExt::world(void) {
    return *(Sprite::World*)m_world;
}

Sprite::World const & EnvExt::world(void) const {
    return *(Sprite::World const *)m_world;
}

void EnvExt::registerEvents(Sprite::Repository & repo, Cpe::Cfg::Node const & config) {
    dr_store_manage_t store_mgr = dr_store_manage_default(app());
    if (store_mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(), ::std::runtime_error,
            "%s: registerEvents: store_mgr not exist", name());
    }

    Cpe::Cfg::NodeConstIterator childs = config.childs();
    while(Cpe::Cfg::Node const * child = childs.next()) {
        const char * metalib = (*child)["metalib"].asString(NULL);

        if (metalib == NULL) {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "%s: registerEvents: metalib not configured", name());
        }

        dr_store_t metalib_store = dr_store_find(store_mgr, metalib);
        if (metalib_store == NULL) {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "%s: registerEvents: metalib %s not exist", name(), metalib);
        }

        if (const char * prefix = (*child)["prefix"].asString(NULL)) {
            repo.registerEventsByPrefix(dr_store_lib(metalib_store), prefix);
        }

        Cpe::Cfg::NodeConstIterator events = (*child)["events"].childs();
        if (Cpe::Cfg::Node const * event = events.next()) {
            const char * event_name = event->asString(NULL);
            if (event_name == NULL) {
                APP_CTX_THROW_EXCEPTION(
                    app(), ::std::runtime_error,
                    "%s: registerEvents: events format error", name());
            }

            LPDRMETA event_meta = dr_lib_find_meta_by_name(dr_store_lib(metalib_store), event_name);
            if (event_meta == NULL) {
                APP_CTX_THROW_EXCEPTION(
                    app(), ::std::runtime_error,
                    "%s: registerEvents: event %s not exist in metalib %s", name(), event_name, metalib);
            }

            repo.registerEvent(event_meta);
        }
    }
}

void EnvExt::doFini(void) {
    m_uiCenter.clear();

    if (m_world) {
		ui_sprite_world_free(m_world);
		m_world = NULL;
	}
}

void EnvExt::loadColors(Cpe::Cfg::Node const & color_cfg) {
    ui_cache_manager_t cache_mgr = cacheMgr();
    
    Cpe::Cfg::NodeConstIterator childs = color_cfg.childs();
    while (Cpe::Cfg::Node const * child_cfg = childs.next()) {
        if (child_cfg->childCount() != 1) {
            APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "load colors: format error!");
        }

        child_cfg = &child_cfg->onlyChild();
        
        const char * color_name = child_cfg->name();
        const char * color_str = child_cfg->asString(NULL);
        if (color_str == NULL) {
            APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "load colors: format error(read value str)!");
        }

        ui_color c;
        if (ui_cache_find_color(cache_mgr, color_str, &c) != 0) {
            APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "load colors: color %s unknown!", color_str);
        }
        
        if (ui_cache_set_color(cache_mgr, color_name, &c) != 0) {
            APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "load colors: set color %s to %s fail!", color_name, color_str);
        }
    }
}

void EnvExt::loadPackages(void) {
    ui_data_mgr_t data_mgr = ui_data_mgr_find_nc(m_app, NULL);
    if (data_mgr == NULL) {
        APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "ui_data_mgr exist!");
    }

    ui_bin_loader_t bin_loader = ui_bin_loader_find_nc(m_app, NULL);
    if (bin_loader == NULL) {
        APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "ui_bin_loader not exist!");
    }
    
    if (ui_data_bin_loader_set_load_to_data_mgr(bin_loader, data_mgr) != 0) {
        APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "ui_bin_loader set to data mgr fail!");
    }
    
    if (plugin_package_module_install_summary(plugin_package_module_find_nc(m_app, NULL)) != 0
        || plugin_package_module_install_packages(plugin_package_module_find_nc(m_app, NULL)) != 0)
    {
        APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "install package install fail!");
    }
}

void EnvExt::loadLanguages(Cpe::Cfg::Node const & cfg) {
    ui_data_mgr_t data_mgr = ui_data_mgr_find_nc(m_app, NULL);
    struct cfg_it language_it;
    cfg_t language_cfg;

    cfg_it_init(&language_it, cfg);
    while((language_cfg = cfg_it_next(&language_it))) {
        ui_data_language_t language;
        const char * str_language;

        str_language = cfg_as_string(language_cfg, NULL);
        if (str_language == NULL) {
            APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "ui_sprite_ui_env_load_languages: format error!");
        }

        language = ui_data_language_create(data_mgr, str_language);
        if (language == NULL) {
            APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "ui_sprite_ui_env_load_languages: format error!");
        }
    }
}

Env::~Env() {
}

Env &
Env::instance(Gd::App::Application & app) {
    Env * r =
        dynamic_cast<Env *>(
            &app.nmManager().object(Env::NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "Env cast fail!");
    }

    return *r;
}

EnvExt &
EnvExt::instance(Gd::App::Application & app) {
    EnvExt * r =
        dynamic_cast<EnvExt *>(
            &Gd::App::Application::instance().nmManager().object(EnvExt::NAME));
    if (r == NULL) {
        APP_THROW_EXCEPTION(::std::runtime_error, "Env cast fail!");
    }

    return *r;
}

static cpe_hash_string_buf s_Env_Name = CPE_HS_BUF_MAKE("UIAppEnv");
cpe_hash_string_t Env::NAME = (cpe_hash_string_t)(void*)&s_Env_Name;

#if defined FLEX
extern "C" int main(int argc, char * argv[]);
#endif

extern "C"
EXPORT_DIRECTIVE
int UIAppEnv_app_init(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg) {                                                                   \

#if defined FLEX
    do { char buf[64]; snprintf(buf, sizeof(buf), "%p", main); } while(0);
#endif
    
    Env * product = NULL;
    try {
        product = new (app.nmManager(), module.name()) EnvExt(app, module, moduleCfg);
        return 0;
    }
    APP_CTX_CATCH_EXCEPTION(app, "UIAppEnv init:");
    if (product) app.nmManager().removeObject(module.name());
    return -1;
}

extern "C"
EXPORT_DIRECTIVE
void UIAppEnv_app_fini(Gd::App::Application & app, Gd::App::Module & module) {
    app.nmManager().removeObject(module.name());
}

}}

