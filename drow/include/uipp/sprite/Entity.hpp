#ifndef UIPP_SPRITE_ENTITY_H
#define UIPP_SPRITE_ENTITY_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Utils.hpp"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/Data.hpp"
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_component.h"
#include "Component.hpp"

namespace UI { namespace Sprite {

class Entity : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_entity_t () const { return (ui_sprite_entity_t)this; }
    static Entity & cast(ui_sprite_entity_t e) { return *(Entity*)e; }

    World & world(void) { return *(World *)ui_sprite_entity_world(*this); }
    World const & world(void) const { return *(World const *)ui_sprite_entity_world(*this); }

    Gd::App::Application & app(void);
    Gd::App::Application const & app(void) const;

    uint32_t id(void) const { return ui_sprite_entity_id(*this); }
    const char * name(void) const { return ui_sprite_entity_name(*this); }

    void enter(void);
    void exit(void) { ui_sprite_entity_exit(*this); }

    bool isActive(void) const { return ui_sprite_entity_is_active(*this) ? true : false; }

    uint8_t debug(void) const { return ui_sprite_entity_debug(*this); }
    void setDebug(uint8_t dbg) { ui_sprite_entity_set_debug(*this, dbg); }

    /*attributes*/
    Cpe::Dr::DataElement attr(const char * path);
    Cpe::Dr::ConstDataElement attr(const char * path) const;

    void setAttr(const char * attrName, int8_t v);
    void setAttr(const char * attrName, uint8_t v);
    void setAttr(const char * attrName, int16_t v);
    void setAttr(const char * attrName, uint16_t v);
    void setAttr(const char * attrName, int32_t v);
    void setAttr(const char * attrName, uint32_t v);
    void setAttr(const char * attrName, int64_t v);
    void setAttr(const char * attrName, uint64_t v);
    void setAttr(const char * attrName, float v);
    void setAttr(const char * attrName, double v);
    void setAttr(const char * attrName, const char * v);

    void notifyAttrUpdated(const char * attrs);

    /*component*/
    Component * findComponent(const char * name) { return (Component *)ui_sprite_component_find(*this, name); }
    Component const * findComponent(const char * name) const { return (Component const *)ui_sprite_component_find(*this, name); }

    Component & component(const char * name);
    Component const & component(const char * name) const;

    Component & createComponent(const char * name);

    void removeComponent(const char * name);

    template<typename T>
    T * findComponent(void) {
        Component * c = findComponent(T::NAME);
        return c ? (T*)c->data() : NULL;
    }

    template<typename T>
    T const * findComponent(void) const {
        Component const * c = findComponent(T::NAME);
        return c ? (T*)c->data() : NULL;
    }

    template<typename T>
    T & component(void) { return *(T*)component(T::NAME).data(); }

    template<typename T>
    T const & component(void) const { return *(T*)component(T::NAME).data(); }

    template<typename T>
    T & createComponent(void)  { return *(T*)createComponent(T::NAME).data(); }

    template<typename T>
    void removeComponent(void)  { removeComponent(T::NAME); }

    /*event operations*/
    void sendEvent(LPDRMETA meta, void const * data, size_t data_size) { ui_sprite_entity_send_event(*this, meta, data, data_size); }

    void buildAndSendEvent(const char * event_def, dr_data_source_t data_source = NULL) {
        ui_sprite_entity_build_and_send_event(*this,  event_def, data_source);
    }

    template<typename T>
    void sendEvent(T const & data = T()) {
        sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }
};

}}

#endif
