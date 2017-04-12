#ifndef UIPP_SPRITE_CFG_ACTION_MODULE_REG_H
#define UIPP_SPRITE_CFG_ACTION_MODULE_REG_H
#include "gdpp/app/Application.hpp"
#include "uipp/sprite_fsm/ActionReg.hpp"
#include "uipp/sprite_fsm/Repository.hpp"
#include "CfgLoader.hpp"

#define UI_FSM_ACTION_REG(__action, __name)                     \
    const char * __action::NAME = __name;                       \
    static ui_sprite_fsm_action_t __action ## _load(            \
        void * ctx, ui_sprite_fsm_state_t fsm_state,      \
            const char * name, cfg_t cfg)                       \
    {                                                           \
    ui_sprite_fsm_action_t action;                                      \
    action = ui_sprite_fsm_action_create(fsm_state, name, __action::NAME); \
    if (action == NULL) return NULL;                                    \
    ((__action*)ui_sprite_fsm_action_data(action))->load(               \
        *(Cpe::Cfg::Node*)cfg);                                 \
    return action;                                              \
    }                                                           \
    extern "C"                                                  \
    EXPORT_DIRECTIVE                                            \
    int __action ## _app_init(                                  \
        Gd::App::Application & app,                             \
        Gd::App::Module & module,                               \
        Cpe::Cfg::Node & moduleCfg)                             \
    {                                                           \
        try {                                                   \
            ::UI::Sprite::Fsm::ActionReg<__action>(             \
                ::UI::Sprite::Fsm::Repository::instance(app))   \
                .on_enter(&__action::enter)                     \
                .on_exit(&__action::exit)                       \
                .on_update(&__action::update)                   \
                ;                                               \
                                                                \
            ::UI::Sprite::Cfg::CfgLoader::instance(app)         \
                  .addActionLoader(                             \
                      __action::NAME, __action ## _load, NULL); \
            return 0;                                           \
        }                                                       \
        APP_CTX_CATCH_EXCEPTION(app, #__action " init:");       \
        return -1;                                              \
    }                                                           \
                                                                \
    extern "C"                                                  \
    EXPORT_DIRECTIVE                                            \
    void __action ## _app_fini(                                 \
        Gd::App::Application & app,                             \
        Gd::App::Module & module)                               \
    {                                                           \
        ::UI::Sprite::Cfg::CfgLoader::instance(app)             \
            .removeActionLoader(__action::NAME);                \
                                                                \
        ::UI::Sprite::Fsm::Repository::instance(app)            \
            .removeActionMeta<__action>();                      \
    }

#endif
