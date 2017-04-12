#ifndef UIPP_SPRITE_COMPONENT_GEN_H
#define UIPP_SPRITE_COMPONENT_GEN_H
#include "cpepp/utils/TypeUtils.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "Component.hpp"
#include "Entity.hpp"
#include "World.hpp"
#include "Repository.hpp"

namespace UI { namespace Sprite {

template<typename OuterT, typename T>
struct ComponentHandlerTraits {
    static void (OuterT::*s_fun)(T const & evt);
};

template<typename OuterT, int idx>
struct ComponentAttrMonitorTraits {
    static void (OuterT::*s_fun)(void);
};

template<typename BaseT, typename OuterT> 
class ComponentGen : public BaseT {
public:
    typedef ComponentGen ComponentBase;

	ComponentGen(UI::Sprite::Component & component) : m_component(component) {
	}

	ComponentGen(UI::Sprite::Component & component, ComponentGen const & o) : m_component(component) {
	}

    Gd::App::Application & app(void) { return world().app(); }
    Entity & entity(void) { return m_component.entity(); }
    Entity const & entity(void) const { return m_component.entity(); }
    World & world(void) { return entity().world(); }
    World const & world(void) const { return entity().world(); }

    bool isActive(void) const { return m_component.isActive(); }
    bool isUpdate(void) const { return m_component.isUpdate(); }
    void startUpdate(void) { m_component.startUpdate(); }
    void stopUpdate(void) { m_component.stopUpdate(); }
    void syncUpdate(bool is_update) { m_component.syncUpdate(is_update); }

    /*attr*/
    template<typename T>
    void setAttr(const char * attrName, T v) {
        entity().setAttr(attrName, v);
    }
 
    /*event operations*/
    void sendEvent(LPDRMETA meta, void const * data, size_t data_size) { ui_sprite_component_send_event(m_component, meta, data, data_size); }

    template<typename T>
    void sendEvent(T const & data = T()) {
        sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    void sendEventTo(const char * target, LPDRMETA meta, void const * data, size_t data_size) {
        ui_sprite_component_send_event_to(m_component, target, meta, data, data_size);
    }

    template<typename T>
    void sendEventTo(const char * target, T const & data = T()) {
        sendEventTo(target, Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    /* build and send event operations*/
    void buildAndSendEvent(const char * event, dr_data_source_t data_source = NULL) {
        ui_sprite_component_build_and_send_event(m_component, event, data_source);
    }

    template<typename T1>
    void buildAndSendEvent(const char * event, T1 const & arg1) {
        dr_data_source data_source[1];

        data_source[0].m_data.m_meta = Cpe::Dr::MetaTraits<T1>::META;
        data_source[0].m_data.m_data = (void *)&arg1;
        data_source[0].m_data.m_size = Cpe::Dr::MetaTraits<T1>::data_size(arg1);
        data_source[0].m_next = NULL;

        buildAndSendEvent(event, data_source);
    }

    template<typename T1, typename T2>
    void buildAndSendEvent(const char * event, T1 const & arg1, T2 const & arg2) {
        dr_data_source data_source[2];

        data_source[0].m_data.m_meta = Cpe::Dr::MetaTraits<T1>::META;
        data_source[0].m_data.m_data = (void *)&arg1;
        data_source[0].m_data.m_size = Cpe::Dr::MetaTraits<T1>::data_size(arg1);
        data_source[0].m_next = &data_source[1];

        data_source[1].m_data.m_meta = Cpe::Dr::MetaTraits<T2>::META;
        data_source[1].m_data.m_data = (void *)&arg2;
        data_source[1].m_data.m_size = Cpe::Dr::MetaTraits<T2>::data_size(arg2);
        data_source[1].m_next = NULL;

        buildAndSendEvent(event, data_source);
    }

    /*event handler*/
    template<typename T>
    void addEventHandler(void (OuterT::*fun)(T const & evt), ui_sprite_event_scope_t scope = ui_sprite_event_scope_self) {
        ComponentHandlerTraits<OuterT, T>::s_fun = fun;

        if (ui_sprite_component_add_event_handler(
                m_component, scope, Cpe::Dr::MetaTraits<T>::NAME, process_event<T>, Cpe::Utils::calc_cast<OuterT>(this))
            == NULL)
        {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "add event handler of %s fail!", Cpe::Dr::MetaTraits<T>::NAME);
        }
    }

    void clearEventHandlers(void) {
        ui_sprite_component_clear_event_handlers(m_component);
    }

    /*attr monitor*/
    template<int id>
    void addAttrMonitor(const char * attrs, void (OuterT::*fun)(void)) {
        ComponentAttrMonitorTraits<OuterT, id>::s_fun = fun;

        if (ui_sprite_component_add_attr_monitor(
                m_component, attrs, process_attr_update<id>, Cpe::Utils::calc_cast<OuterT>(this))
            == NULL)
        {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "add attr monitor of %s fail!", attrs);
        }
    }

    template<int id>
    void addAttrMonitorByDef(const char * def, void (OuterT::*fun)(void)) {
        ComponentAttrMonitorTraits<OuterT, id>::s_fun = fun;

        if (ui_sprite_component_add_attr_monitor_by_def(
                m_component, def, process_attr_update<id>, Cpe::Utils::calc_cast<OuterT>(this))
            == NULL)
        {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "add attr monitor by def %s fail!", def);
        }
    }

    template<int id>
    void addAttrMonitorByDefs(const char ** defs, uint16_t def_count, void (OuterT::*fun)(void)) {
        ComponentAttrMonitorTraits<OuterT, id>::s_fun = fun;

        if (ui_sprite_component_add_attr_monitor_by_defs(
                m_component, defs, def_count, process_attr_update<id>, Cpe::Utils::calc_cast<OuterT>(this))
            == NULL)
        {
            APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "add attr monitor by defs fail!");
        }
    }
    
    void clearAttrMonitors(void) {
        ui_sprite_component_clear_attr_monitors(m_component);
    }

private:
    template<typename T>
    static void process_event(void * ctx, ui_sprite_event_t evt) {
        try {
            (((OuterT *)ctx)->*ComponentHandlerTraits<OuterT, T>::s_fun)(*(T const *)evt->data);
        }
        catch(...) {
        }
    }

    template<int idx>
    static void process_attr_update(void * ctx) {
        try {
            (((OuterT *)ctx)->*ComponentAttrMonitorTraits<OuterT, idx>::s_fun)();
        }
        catch(...) {
        }
    }

protected:
	UI::Sprite::Component & m_component;
};

template<typename OuterT, typename T>
void (OuterT::* ComponentHandlerTraits<OuterT, T>::s_fun)(T const & evt);

template<typename OuterT, int idx>
void (OuterT::* ComponentAttrMonitorTraits<OuterT, idx>::s_fun)();

}}

#endif
