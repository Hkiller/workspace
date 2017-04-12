#ifndef UIPP_SPRITE_FSM_ACTION_REGISTER_H
#define UIPP_SPRITE_FSM_ACTION_REGISTER_H
#include <memory>
#include "ActionMeta.hpp"
#include "Action.hpp"
#include "Repository.hpp"

namespace UI { namespace Sprite { namespace Fsm {

template<typename T>
class ActionReg {
public:
    typedef int (T::*enter_fun_t)();
    typedef void (T::*enter_fun_2_t)();
    typedef void (T::*exit_fun_t)();
    typedef void (T::*update_fun_t)(float delta);

    ActionReg(Repository & repo)
        : m_meta(repo.createActionMeta(T::NAME, sizeof(T)))
    {
        ui_sprite_fsm_action_meta_set_init_fun(m_meta, &call_init, NULL);
        ui_sprite_fsm_action_meta_set_copy_fun(m_meta, &call_clone, NULL);
        ui_sprite_fsm_action_meta_set_free_fun(m_meta, &call_free, NULL);
    }

    ActionReg & on_enter(enter_fun_t fun) {
        static enter_fun_t s_fun = fun;
        ui_sprite_fsm_action_meta_set_enter_fun(m_meta, &call_enter, &s_fun);
        return *this;
    }

    ActionReg & on_enter(enter_fun_2_t fun) {
        static enter_fun_2_t s_fun = fun;
        ui_sprite_fsm_action_meta_set_enter_fun(m_meta, &call_enter_2, &s_fun);
        return *this;
    }

    ActionReg & on_exit(exit_fun_t fun) {
        static exit_fun_t s_fun = fun;
        ui_sprite_fsm_action_meta_set_exit_fun(m_meta, &call_exit, &s_fun);
        return *this;
    }

    ActionReg & on_update(update_fun_t fun) {
        static update_fun_t s_fun = fun;
        ui_sprite_fsm_action_meta_set_update_fun(m_meta, &call_update, &s_fun);
        return *this;
    }

private:
    static T & cast(ui_sprite_fsm_action_t fsm_action) { return *(T*)ui_sprite_fsm_action_data(fsm_action); }

    static  int call_enter(ui_sprite_fsm_action_t fsm_action, void * ctx) {
        try {
            enter_fun_t * fun = (enter_fun_t *)ctx;
            return (cast(fsm_action).**fun)();
        }
        catch(...) {
            return -1;
        }
    }

    static  int call_enter_2(ui_sprite_fsm_action_t fsm_action, void * ctx) {
        try {
            enter_fun_2_t * fun = (enter_fun_2_t *)ctx;
            (cast(fsm_action).**fun)();
            return 0;
        }
        catch(...) {
            return -1;
        }
    }

    static  void call_exit(ui_sprite_fsm_action_t fsm_action, void * ctx) {
        try {
            exit_fun_t * fun = (exit_fun_t *)ctx;
            return (cast(fsm_action).**fun)();
        }
        catch(...) {
        }
    }

    static  void call_update(ui_sprite_fsm_action_t fsm_action, void * ctx, float delta) {
        try {
            update_fun_t * fun = (update_fun_t *)ctx;
            return (cast(fsm_action).**fun)(delta);
        }
        catch(...) {
        }
    }

    static int call_init(ui_sprite_fsm_action_t fsm_action, void * ctx) {
        try {
            new (ui_sprite_fsm_action_data(fsm_action)) T(*(UI::Sprite::Fsm::Action *)fsm_action);
            return 0;
        }
        catch(...) {
            return -1;
        }
    }

    static void call_free(ui_sprite_fsm_action_t fsm_action, void * ctx) {
        cast(fsm_action).~T();
    }

    static int call_clone(ui_sprite_fsm_action_t to, ui_sprite_fsm_action_t from, void * ctx) {
        try {
            new (ui_sprite_fsm_action_data(to)) T(*(UI::Sprite::Fsm::Action *)to, cast(from));
            return 0;
        }            
        catch(...) {
            return -1;
        }
    }

    ActionMeta & m_meta;
};

}}}

#endif
