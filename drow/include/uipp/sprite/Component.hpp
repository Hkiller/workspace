#ifndef UIPP_SPRITE_COMPONENT_H
#define UIPP_SPRITE_COMPONENT_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Utils.hpp"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "System.hpp"

namespace UI { namespace Sprite {

class Component : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_component_t () const { return (ui_sprite_component_t)this; }

    Entity & entity(void) { return *(Entity *)ui_sprite_component_entity(*this); }
    Entity const & entity(void) const { return *(Entity const *)ui_sprite_component_entity(*this); }

    World & world(void) { return *(World *)ui_sprite_entity_world(ui_sprite_component_entity(*this)); }
    World const & world(void) const { return *(World *)ui_sprite_entity_world(ui_sprite_component_entity(*this)); }

    const char * name(void) const { return ui_sprite_component_name(*this); }
    ComponentMeta const & meta(void) const { return *(ComponentMeta const *)ui_sprite_component_meta(*this); }

    bool isActive(void) const { return ui_sprite_component_is_active(*this) ? true : false; }
    bool isUpdate(void) const { return ui_sprite_component_is_update(*this) ? true : false; }
    void startUpdate(void);
    void stopUpdate(void) { ui_sprite_component_stop_update(*this); }
    void syncUpdate(bool is_update) { ui_sprite_component_sync_update(*this, is_update ? 1 : 0); }

    void enter(void);
    void exit(void) { ui_sprite_component_exit(*this); }

    void * data(void) { return ui_sprite_component_data(*this); }
    void const * data(void) const { return ui_sprite_component_data(*this); }
    size_t dataSize(void) const { return ui_sprite_component_data_size(*this); }

    void setAttrData(dr_data_t data);

    template<typename T>
    void setAttrData(T const & data) {
        dr_data d;
        d.m_meta = Cpe::Dr::MetaTraits<T>::META;
        d.m_data = (void*)&data;
        d.m_size = Cpe::Dr::MetaTraits<T>::data_size(data);
        setAttrData(&d);
    }

    template<typename T>
    T & as(void) { return *(T*)data(); }

    template<typename T>
    T const & as(void) const { return *(T const *)data(); }
    
};

}}

#endif
