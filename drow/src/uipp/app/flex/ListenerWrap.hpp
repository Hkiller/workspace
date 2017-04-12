#ifndef UIPP_APP_ENV_FLEX_LISTENER_WRAP_H
#define UIPP_APP_ENV_FLEX_LISTENER_WRAP_H
#include <string>
#include <vector>
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/TypeUtils.hpp"
#include "cpe/utils/error.h"
#include "gd/app/app_basic.h"
#include "System.hpp"

template<class T>
class ListenerWrap : public Cpe::Utils::Noncopyable {
public:
    typedef ListenerWrap<T> ListenerWrapBase;
    
    ListenerWrap(::std::string const & tag)
        : m_tag(tag)
    {
    }
    
    ~ListenerWrap() {
        clearEventListener();
    }

protected:
    typedef void (T::*ProcessFunT)(var as3Args);

    template<typename R, typename A1>
    R guardCall(R (T::*fun)(A1), A1 arg) {
        try {
            T * self = Cpe::Utils::calc_cast<T>(this);
            return (self->*fun)(arg);
        }
        catch(var e) {
            char *err = internal::utf8_toString(e);
            CPE_ERROR(gd_app_em(gd_app_ins()), "%s: guardCall: %s", m_tag.c_str(), err);
            free(err);
            return R();
        }
    }

    template<typename A1>
    void guardCall(void (T::*fun)(A1), A1 arg) {
        try {
            T * self = Cpe::Utils::calc_cast<T>(this);
            (self->*fun)(arg);
        }
        catch(var e) {
            char *err = internal::utf8_toString(e);
            CPE_ERROR(gd_app_em(gd_app_ins()), "%s: guardCall: %s", m_tag.c_str(), err);
            free(err);
        }
    }

    void guardCall(void (T::*fun)()) {
        try {
            T * self = Cpe::Utils::calc_cast<T>(this);
            (self->*fun)();
        }
        catch(var e) {
            char *err = internal::utf8_toString(e);
            CPE_ERROR(gd_app_em(gd_app_ins()), "%s: guardCall: %s", m_tag.c_str(), err);
            free(err);
        }
    }
    
    void clearEventListener(flash::events::EventDispatcher target) {
        for(typename ListenerContainer::iterator it = m_listeners.begin();
            it != m_listeners.end();
            )
        {
            if ((*it)->m_target != target) {
                ++it;
                continue;
            }
            
            try {
                (*it)->m_target->removeEventListener((*it)->m_event, (*it)->m_listener);
            }
            catch(var e) {
                if (gd_app_context_t app = gd_app_ins()) {
                    char *err = internal::utf8_toString(e);
                    CPE_ERROR(gd_app_em(app), "%s: removeEventListener: %s", m_tag.c_str(), err);
                    free(err);
                }
            }
            delete *it;

            it = m_listeners.erase(it);
        }
    }
    
    void clearEventListener(void) {
        for(typename ListenerContainer::iterator it = m_listeners.begin();
            it != m_listeners.end();
            ++it)
        {
            try {
                (*it)->m_target->removeEventListener((*it)->m_event, (*it)->m_listener);
            }
            catch(var e) {
                if (gd_app_context_t app = gd_app_ins()) {
                    char *err = internal::utf8_toString(e);
                    CPE_ERROR(gd_app_em(app), "%s: removeEventListener: %s", m_tag.c_str(), err);
                    free(err);
                }
            }
            delete *it;
        }
        m_listeners.clear();
    }
    
    void addEventListener(flash::events::EventDispatcher target, String event, ProcessFunT fun) {
        Listener * listener = new Listener();
        listener->m_target = target;
        listener->m_obj = Cpe::Utils::calc_cast<T>(this);
        listener->m_fun = fun;
        listener->m_listener = Function::_new(&Listener::process, listener);
        listener->m_event = event;
        m_listeners.push_back(listener);

        try {
            target->addEventListener(event, listener->m_listener, false, 0, true);
        }
        catch(var e) {
            if (gd_app_context_t app = gd_app_ins()) {
                char *err = internal::utf8_toString(e);
                CPE_ERROR(gd_app_em(app), "%s: %s", m_tag.c_str(), err);
                free(err);
            }
        }
    }
    
private:
    struct Listener {
        flash::events::EventDispatcher m_target;
        String m_event;
        var m_listener;
        T * m_obj;
        ProcessFunT m_fun;

        static var process(void * p, var as3Args) {
            Listener * self = (Listener *)p;
            try {
                (self->m_obj->*self->m_fun)(as3Args);
            }
            catch(var e) {
                if (gd_app_context_t app = gd_app_ins()) {
                    char *err = internal::utf8_toString(e);
                    char *arg = internal::utf8_toString(var(as3Args[0]));
                    CPE_ERROR(gd_app_em(app), "%s: on event: %s: %s", self->m_obj->m_tag.c_str(), arg, err);
                    free(err);
                    free(arg);
                }
            }
            catch(::std::runtime_error const & e) {
                if (gd_app_context_t app = gd_app_ins()) {
                    char *arg = internal::utf8_toString(var(as3Args[0]));
                    CPE_ERROR(gd_app_em(app), "%s: on event: %s: %s", self->m_obj->m_tag.c_str(), arg, e.what());
                    free(arg);
                }
            }
            catch(...) {
                if (gd_app_context_t app = gd_app_ins()) {
                    char *arg = internal::utf8_toString(var(as3Args[0]));
                    CPE_ERROR(gd_app_em(app), "%s: on event: %s: unknown error", self->m_obj->m_tag.c_str(), arg);
                    free(arg);
                }
            }
            
            return internal::_undefined;
        }
    };

    typedef ::std::vector<Listener *> ListenerContainer;
    
    ::std::string m_tag;
    ListenerContainer m_listeners;
};
    
#endif
