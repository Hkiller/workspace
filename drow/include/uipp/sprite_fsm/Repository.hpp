#ifndef UIPP_SPRITE_FSM_REPOSITORY_H
#define UIPP_SPRITE_FSM_REPOSITORY_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_module.h"
#include "ui/sprite_fsm/ui_sprite_fsm_action_meta.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Fsm {

class Repository : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_fsm_module_t () const { return (ui_sprite_fsm_module_t)this; }

    const char * name(void) const { return ui_sprite_fsm_module_name(*this); }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)ui_sprite_fsm_module_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)ui_sprite_fsm_module_app(*this); }

    ActionMeta * findActionMeta(const char * name) { return (ActionMeta*)ui_sprite_fsm_action_meta_find(*this, name); }
    ActionMeta const * findActionMeta(const char * name) const { return (ActionMeta const *)ui_sprite_fsm_action_meta_find(*this, name); }

    ActionMeta & actionMeta(const char * name);
    ActionMeta const & actionMeta(const char * name) const;

    ActionMeta & createActionMeta(const char * name, size_t size);
    void removeActionMeta(const char * name);

    template<typename T>
    void removeActionMeta(void) { removeActionMeta(T::NAME); }

    static Repository & instance(gd_app_context_t app, const char * name = NULL);
};

}}}

#endif
