#ifndef UIPP_SPRITE_REPOSITORY_H
#define UIPP_SPRITE_REPOSITORY_H
#include "cpepp/utils/ClassCategory.hpp"
#include "ui/sprite/ui_sprite_repository.h"
#include "ui/sprite/ui_sprite_component_meta.h"
#include "System.hpp"

namespace UI { namespace Sprite {

class Repository : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_repository_t () const { return (ui_sprite_repository_t)this; }

    const char * name(void) const { return ui_sprite_repository_name(*this); }

    Gd::App::Application & app(void) { return *(Gd::App::Application*)ui_sprite_repository_app(*this); }
    Gd::App::Application const & app(void) const { return *(Gd::App::Application*)ui_sprite_repository_app(*this); }

    /*components*/
    ComponentMeta * findComponentMeta(const char * name) { return (ComponentMeta*)ui_sprite_component_meta_find(*this, name); }
    ComponentMeta const * findComponentMeta(const char * name) const { return (ComponentMeta const *)ui_sprite_component_meta_find(*this, name); }

    ComponentMeta & componentMeta(const char * name);
    ComponentMeta const & componentMeta(const char * name) const;

    ComponentMeta & createComponentMeta(const char * name, size_t size);
    void removeComponentMeta(const char * name);

    template<typename T>
    void removeComponentMeta(void) { removeComponentMeta(T::NAME); }

    /*events*/
    void registerEvent(LPDRMETA meta);
    void unregisterEvent(const char * name) { ui_sprite_repository_unregister_event(*this, name); }

    void registerEventsByPrefix(LPDRMETALIB metalib, const char * prefix);
    void unregisterEventsByPrefix(LPDRMETALIB metalib, const char * prefix) { ui_sprite_repository_unregister_events_by_prefix(*this, metalib, prefix); }

    /**/
    static Repository & instance(gd_app_context_t app, const char * name = NULL);
};

}}

#endif
