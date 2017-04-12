#ifndef UIPP_SPRITE_FSM_ACTION_GEN_H
#define UIPP_SPRITE_FSM_ACTION_GEN_H
#include "cpepp/utils/TypeUtils.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Log.hpp"
#include "ui/sprite_fsm/ui_sprite_fsm_action_calc.h"
#include "Action.hpp"
#include "Repository.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite/Entity.hpp"

namespace UI { namespace Sprite { namespace Fsm {

template<typename OuterT, typename T>
struct ActionHandlerTraits {
    static void (OuterT::*s_fun)(T const & evt);
};

template<typename OuterT, int idx>
struct ActionMonitorTraits {
    static void (OuterT::*s_fun)(void);
};

template<typename BaseT, typename OuterT> 
class ActionGen : public BaseT {
public:
    typedef ActionGen ActionBase;

	ActionGen(UI::Sprite::Fsm::Action & action) : m_action(action) {
	}

	ActionGen(UI::Sprite::Fsm::Action & action, ActionGen const & o) : m_action(action) {
	}
    
    Gd::App::Application & app(void) { return world().app(); }
    Gd::App::Application const & app(void) const { return world().app(); }

    Entity & entity(void) { return m_action.entity(); }
    Entity const & entity(void) const { return m_action.entity(); }

    World & world(void) { return entity().world(); }
    World const & world(void) const { return entity().world(); }

	State & state(void) { return m_action.state(); }
	State const & state(void) const { return m_action.state(); }

	Action & action(void) { return m_action; }
	Action const & action(void) const { return m_action; }

    const char * name(void) const { return m_action.name(); }
    ui_sprite_fsm_action_life_circle_t lifeCircle(void) const { return m_action.lifeCircle(); }

    bool isActive(void) const { return m_action.isActive(); }
    bool isUpdate(void) const { return m_action.isUpdate(); }
    void startUpdate(void) { m_action.startUpdate(); }
    void stopUpdate(void) { m_action.stopUpdate(); }
    void syncUpdate(bool is_update) { m_action.syncUpdate(is_update); }

    /*event operations*/
    void sendEvent(LPDRMETA meta, void const * data, size_t data_size) { ui_sprite_fsm_action_send_event(m_action, meta, data, data_size); }

    template<typename T>
    void sendEvent(T const & data = T()) {
        sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    void sendEventTo(const char * target, LPDRMETA meta, void const * data, size_t data_size) {
        ui_sprite_fsm_action_send_event_to(m_action, target, meta, data, data_size);
    }

    template<typename T>
    void sendEventTo(const char * target, T const & data = T()) {
        sendEventTo(target, Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    /* build and send event operations*/
    void buildAndSendEvent(const char * event, dr_data_source_t data_source = NULL) {
        ui_sprite_fsm_action_build_and_send_event(m_action, event, data_source);
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

    template<typename T>
    void addEventHandler(void (OuterT::*fun)(T const & evt), ui_sprite_event_scope_t scope = ui_sprite_event_scope_self) {
        ActionHandlerTraits<OuterT, T>::s_fun = fun;

        if (ui_sprite_fsm_action_add_event_handler(
                m_action, scope, Cpe::Dr::MetaTraits<T>::NAME, process_event<T>, Cpe::Utils::calc_cast<OuterT>(this))
            != 0)
        {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "add event handler of %s fail!", Cpe::Dr::MetaTraits<T>::NAME);
        }
    }

    template<int idx>
    void addAttrMonitor(const char * attrs, void (OuterT::*fun)(void)) {
        ActionMonitorTraits<OuterT, idx>::s_fun = fun;

        if (ui_sprite_fsm_action_add_attr_monitor(
                m_action, attrs, on_attr_updated<idx>, Cpe::Utils::calc_cast<OuterT>(this))
            != 0)
        {
            APP_CTX_THROW_EXCEPTION(
                app(), ::std::runtime_error,
                "add attr monitorof %s fail!", attrs);
        }
    }

    template<int idx>
    void addAttrMonitorByDef(const char * def, void (OuterT::*fun)(void)) {
        ActionMonitorTraits<OuterT, idx>::s_fun = fun;

        if (ui_sprite_fsm_action_add_attr_monitor_by_def(
                m_action, def, on_attr_updated<idx>, Cpe::Utils::calc_cast<OuterT>(this))
            != 0)
        {
            APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "add attr monitorof %s fail!", def);
        }
    }

    template<int idx>
    void addAttrMonitorByDefs(const char * * defs, uint16_t def_count, void (OuterT::*fun)(void)) {
        ActionMonitorTraits<OuterT, idx>::s_fun = fun;

        if (ui_sprite_fsm_action_add_attr_monitor_by_defs(
                m_action, defs, def_count, on_attr_updated<idx>, Cpe::Utils::calc_cast<OuterT>(this))
            != 0)
        {
            APP_CTX_THROW_EXCEPTION(app(), ::std::runtime_error, "add attr monitoro by defs fail!");
        }
    }

    template<int idx>
    bool tryAddAttrMonitor(const char * attrs, void (OuterT::*fun)(void)) {
        ActionMonitorTraits<OuterT, idx>::s_fun = fun;

        if (ui_sprite_fsm_action_add_attr_monitor(
            m_action, attrs, on_attr_updated<idx>, Cpe::Utils::calc_cast<OuterT>(this))
            != 0)
        {
            return false;
        }

        return true;
    }

    bool calcBool(const char * def, dr_data_source_t data_source = NULL);
    float calcFloat(const char * def, dr_data_source_t data_source = NULL);
    uint32_t calcUInt32(const char * def, dr_data_source_t data_source = NULL);
    int32_t calcInt32(const char * def, dr_data_source_t data_source = NULL);
    const char * calcString(const char * def, mem_buffer_t buff, dr_data_source_t data_source = NULL);

    bool checkCalcBool(const char * def, dr_data_source_t data_source = NULL);
    float checkCalcFloat(const char * def, dr_data_source_t data_source = NULL);
    uint32_t checkCalcUInt32(const char * def, dr_data_source_t data_source = NULL);
    int32_t checkCalcInt32(const char * def, dr_data_source_t data_source = NULL);
    const char * checkCalcString(const char * def, mem_buffer_t buff, dr_data_source_t data_source = NULL);
    
private:
    template<typename T>
    static void process_event(void * ctx, ui_sprite_event_t evt) {
        try {
            (((OuterT *)ctx)->*ActionHandlerTraits<OuterT, T>::s_fun)(*(T const *)evt->data);
        }
        catch(...) {
        }
    }

    template<int idx>
    static void on_attr_updated(void * ctx) {
        try {
            (((OuterT *)ctx)->*ActionMonitorTraits<OuterT, idx>::s_fun)();
        }
        catch(...) {
        }
    }
    
	UI::Sprite::Fsm::Action & m_action;
};

template<typename BaseT, typename OuterT> 
bool ActionGen<BaseT, OuterT>::calcBool(const char * def, dr_data_source_t data_source) {
    uint8_t result;
    if (ui_sprite_fsm_action_try_calc_bool(&result, def, this->action(), data_source, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: calc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result ? true : false;
}

template<typename BaseT, typename OuterT> 
float ActionGen<BaseT, OuterT>::calcFloat(const char * def, dr_data_source_t data_source) {
    float result;
    if (ui_sprite_fsm_action_try_calc_float(&result, def, this->action(), data_source, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: calc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result;
}

template<typename BaseT, typename OuterT> 
uint32_t ActionGen<BaseT, OuterT>::calcUInt32(const char * def, dr_data_source_t data_source) {
    uint32_t result;
    if (ui_sprite_fsm_action_try_calc_uint32(&result, def, this->action(), data_source, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: calc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result;
}

template<typename BaseT, typename OuterT> 
int32_t ActionGen<BaseT, OuterT>::calcInt32(const char * def, dr_data_source_t data_source) {
    int32_t result;
    if (ui_sprite_fsm_action_try_calc_int32(&result, def, this->action(), data_source, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: calc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result;
}

template<typename BaseT, typename OuterT> 
const char * ActionGen<BaseT, OuterT>::calcString(const char * def, mem_buffer_t buff, dr_data_source_t data_source) {
    const char * result = (char*)ui_sprite_fsm_action_calc_str_with_dft(buff, def, this->action(), data_source, NULL);
    if (result == NULL) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: calc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result;
}

template<typename BaseT, typename OuterT> 
bool ActionGen<BaseT, OuterT>::checkCalcBool(const char * def, dr_data_source_t data_source) {
    uint8_t result;
    if (ui_sprite_fsm_action_check_calc_bool(&result, def, this->action(), data_source, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: checkCalc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result ? true : false;
}

template<typename BaseT, typename OuterT> 
float ActionGen<BaseT, OuterT>::checkCalcFloat(const char * def, dr_data_source_t data_source) {
    float result;
    if (ui_sprite_fsm_action_check_calc_float(&result, def, this->action(), data_source, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: checkCalc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result;
}

template<typename BaseT, typename OuterT> 
uint32_t ActionGen<BaseT, OuterT>::checkCalcUInt32(const char * def, dr_data_source_t data_source) {
    uint32_t result;
    if (ui_sprite_fsm_action_check_calc_uint32(&result, def, this->action(), data_source, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: checkCalc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result;
}

template<typename BaseT, typename OuterT> 
int32_t ActionGen<BaseT, OuterT>::checkCalcInt32(const char * def, dr_data_source_t data_source) {
    int32_t result;
    if (ui_sprite_fsm_action_check_calc_int32(&result, def, this->action(), data_source, NULL) != 0) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: checkCalc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result;
}

template<typename BaseT, typename OuterT> 
const char * ActionGen<BaseT, OuterT>::checkCalcString(const char * def, mem_buffer_t buff, dr_data_source_t data_source) {
    const char * result = (char*)ui_sprite_fsm_action_check_calc_str(buff, def, this->action(), data_source, NULL);
    if (result == NULL) {
        APP_CTX_THROW_EXCEPTION(
            this->app(), ::std::runtime_error,
            "entity %d(%s) %s: checkCalc { %s } fail!", 
            this->entity().id(), this->entity().name(), OuterT::NAME, def);
    }

    return result;
}

template<typename OuterT, typename T>
void (OuterT::* ActionHandlerTraits<OuterT, T>::s_fun)(T const & evt);

template<typename OuterT, int idx>
void (OuterT::* ActionMonitorTraits<OuterT, idx>::s_fun)(void);

}}}

#endif
