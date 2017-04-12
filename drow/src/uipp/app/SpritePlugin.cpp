#include "cpepp/utils/OpGuard.hpp"
#include "gdpp/app/ModuleDef.hpp"
#include "gdpp/app/Application.hpp"
#include "uipp/sprite/Repository.hpp"
#include "uipp/sprite_fsm/Repository.hpp"
#include "uipp/sprite_cfg/CfgLoaderExternGen.hpp"
#include "UIAction_UpdateNum.hpp"
#include "UIAction_MaskControl.hpp"
#include "EnvExt.hpp"
#include "UIAction_AudioBGM.hpp"
#include "UIAction_AudioSFX.hpp"
#include "UIAction_TriggerSFX.hpp"
#include "UIAction_UpdateProgress.hpp"
#include "UIAction_ShowControl.hpp"
#include "UIAction_HideControl.hpp"
#include "UIAction_EnableControl.hpp"
#include "UIAction_DisableControl.hpp"
#include "UIAction_SetValue.hpp"
#include "UIAction_BindValue.hpp"
#include "UIAction_BGMInOut.hpp"

namespace UI { namespace App {

#define SPRITEPLUGIN_INSTALL_ACTION(__class) \
addActionLoader(&SpritePlugin::init ## __class);    \
UIAction_ ## __class::install(fsm_repo);\
m_clear_ops.addOp(fsm_repo, &Sprite::Fsm::Repository::removeActionMeta<UIAction_ ## __class>)

class SpritePlugin
    : public Cpe::Nm::Object
    , public Sprite::Cfg::CfgLoaderExternGen<SpritePlugin>
{
public:
    SpritePlugin(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg)
        : m_app(app)
    {
        Sprite::Fsm::Repository & fsm_repo = Sprite::Fsm::Repository::instance(app);

		SPRITEPLUGIN_INSTALL_ACTION(UpdateNum);
		SPRITEPLUGIN_INSTALL_ACTION(UpdateProgress);
        SPRITEPLUGIN_INSTALL_ACTION(AudioBGM);
        SPRITEPLUGIN_INSTALL_ACTION(AudioSFX);
        SPRITEPLUGIN_INSTALL_ACTION(TriggerSFX);
        SPRITEPLUGIN_INSTALL_ACTION(MaskControl);
        SPRITEPLUGIN_INSTALL_ACTION(BGMInOut);
		SPRITEPLUGIN_INSTALL_ACTION(ShowControl);
		SPRITEPLUGIN_INSTALL_ACTION(HideControl);
		SPRITEPLUGIN_INSTALL_ACTION(EnableControl);
		SPRITEPLUGIN_INSTALL_ACTION(DisableControl);
		SPRITEPLUGIN_INSTALL_ACTION(SetValue);
		SPRITEPLUGIN_INSTALL_ACTION(BindValue);
    }

    ~SpritePlugin() {
    }

    Gd::App::Application & app(void) { return m_app; }
    Gd::App::Application const & app(void) const { return m_app; }

private:
	void initUpdateNum(UIAction_UpdateNum & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setControlName(cfg["control"]);
		obj.setBindingValue(cfg["binding-value"].dft(""));
		obj.setInitValue(cfg["init-value"].dft(""));
		obj.setTakeTime(cfg["take-time"].dft(0.0f));
		
		if (const char * def = cfg["decorator"].asString(NULL)) {
			obj.setDecotator(def);
		}	
	}
    
	void initAudioBGM(UIAction_AudioBGM & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setRes(cfg["res"]);
		obj.setLoop(cfg_get_uint8(cfg, "loop", 1));
        obj.setFadeTime(cfg["fade-time"].dft(0.0f));
	}

	void initAudioSFX(UIAction_AudioSFX & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setRes(cfg["res"]);
		obj.setLoop(cfg_get_uint8(cfg, "loop", 0));
		obj.setCut(cfg_get_uint8(cfg, "cut", 0));
	}

	void initTriggerSFX(UIAction_TriggerSFX & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setRes(cfg["res"]);
	}

    void initMaskControl(UIAction_MaskControl & obj, Cpe::Cfg::Node const & cfg) const {
        obj.setControlName(cfg["control"]);
        obj.setTakeTime(cfg["take-time"].dft(0.0f));
        obj.setTargetControl(cfg["target-control"]);
        if (const char * def = cfg["decorator"].asString(NULL)) {
            obj.setDecotator(def);
        }	 
    }

	void initUpdateProgress(UIAction_UpdateProgress & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setControlName(cfg["control"]);
        obj.setBindingValue(cfg["binding-value"]);
		obj.setSpeed(cfg["speed"].dft(1.0f));
		obj.setAlphaMove(cfg_get_uint8(cfg, "alphaMove", 1));
        obj.setTakeTime(cfg["take-time"].dft(0.0f));
	}

	void initShowControl(UIAction_ShowControl & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setControlName(cfg["control"]);
	}

	void initHideControl(UIAction_HideControl & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setControlName(cfg["control"]);
	}

	void initEnableControl(UIAction_EnableControl & obj, Cpe::Cfg::Node const & cfg) const {
        obj.setControlName(cfg["control"]);
	}

	void initDisableControl(UIAction_DisableControl & obj, Cpe::Cfg::Node const & cfg) const {
        obj.setControlName(cfg["control"]);
	}
    
    void initBGMInOut(UIAction_BGMInOut & obj, Cpe::Cfg::Node const & cfg) const {
        obj.setWay(cfg["way"]);
        obj.setTakeTime(cfg["take-time"].dft(0.0f));
        if (const char * def = cfg["decorator"].asString(NULL)) {
            obj.setDecotator(def);
        }	 
    }

	void initSetValue(UIAction_SetValue & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setControlName(cfg["control"]);

        Cpe::Cfg::NodeConstIterator checkIt = cfg["values"].childs();
        while(Cpe::Cfg::Node const * checkNode = checkIt.next()) {
            const char * name = cfg_get_string(*checkNode, "name", NULL);
            const char * value = cfg_get_string(*checkNode, "value", NULL);

            if (name == NULL || value == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initSetValue: values config format error");
            }

            obj.addValue(name, value);
        }
	}

	void initBindValue(UIAction_BindValue & obj, Cpe::Cfg::Node const & cfg) const {
		obj.setControlName(cfg["control"]);

        Cpe::Cfg::NodeConstIterator checkIt = cfg["values"].childs();
        while(Cpe::Cfg::Node const * checkNode = checkIt.next()) {
            const char * name = cfg_get_string(*checkNode, "name", NULL);
            const char * value = cfg_get_string(*checkNode, "value", NULL);

            if (name == NULL || value == NULL) {
                APP_CTX_THROW_EXCEPTION(m_app, ::std::runtime_error, "initBindValue: values config format error");
            }

            obj.addValue(name, value);
        }
	}
    
    Gd::App::Application & m_app;
    Cpe::Utils::OpGuard m_clear_ops;
};

extern "C"
EXPORT_DIRECTIVE
int UISpritePlugin_app_init(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & moduleCfg) {
    SpritePlugin * product = NULL;
    try {
        product = new (app.nmManager(), module.name()) SpritePlugin(app, module, moduleCfg);
        return 0;
    }
    APP_CTX_CATCH_EXCEPTION(app, "UISpritePlugin init:");
    if (product) app.nmManager().removeObject(module.name());
    return -1;
}

extern "C"
EXPORT_DIRECTIVE
void UISpritePlugin_app_fini(Gd::App::Application & app, Gd::App::Module & module) {
    app.nmManager().removeObject(module.name());
}

}}
