#ifndef DROW_RGUI_CONTROL_ACTION_BUILDER_H
#define DROW_RGUI_CONTROL_ACTION_BUILDER_H
#include <cassert>
#include "cpepp/utils/TypeUtils.hpp"
#include "plugin/ui/plugin_ui_control.h"
#include "RGUI.h"

namespace Drow {

class ControlActionBuilder {
public:
    ControlActionBuilder(plugin_ui_control_t control)
        : m_name_prefix(NULL)
        , m_event(plugin_ui_event_max)
        , m_asspect(NULL)
        , m_scope(plugin_ui_event_scope_self)
        , m_control(control)
    {
    }

    ControlActionBuilder & prefix(const char * name_prefix) { m_name_prefix = name_prefix; return *this; }    
    ControlActionBuilder & on(plugin_ui_event_t evt) { m_event = evt; return *this; }
    ControlActionBuilder & on(plugin_ui_event_t evt, plugin_ui_event_scope_t scope) { m_event = evt; m_scope = scope; return *this; }
    ControlActionBuilder & join(plugin_ui_aspect_t asspect) { m_asspect = asspect; return *this; }

    template<typename ObjT>
    ControlActionBuilder & call(ObjT & o, void (ObjT::*fun)()) {
        typedef CtxFun0<ObjT> Ctx;

        Ctx & c = _create<Ctx>();
        c.m_outer = &o;
        c.m_fun = fun;
        
        return *this;
    }

    template<typename ObjT, typename ArgT, typename _ArgT>
    ControlActionBuilder & call(ObjT & o, void (ObjT::*fun)(ArgT), _ArgT arg) {
        typedef CtxFun0Arg1<ObjT, ArgT> Ctx;

        Ctx & c = _create<Ctx>();
        c.m_outer = &o;
        c.m_fun = fun;
        c.m_arg = (ArgT)arg;

        return *this;
    }

    template<typename ObjT, typename ArgT>
    ControlActionBuilder & call(ObjT & o, void (ObjT::*fun)(const ArgT &), const ArgT & arg) {
        typedef CtxFun0Arg1Ref<ObjT, ArgT> Ctx;

        Ctx & c = _create<Ctx>();
        c.m_outer = &o;
        c.m_fun = fun;
        c.m_arg = arg;

        return *this;
    }
    
    template<typename ObjT>
    ControlActionBuilder & call(ObjT & o, void (ObjT::*fun)(RGUIControl & from)) {
        typedef CtxFunC<ObjT> Ctx;

        Ctx & c = _create<Ctx>();
        c.m_outer = &o;
        c.m_fun = fun;
        
        return *this;
    }

    template<typename ObjT, typename ItemT>
    ControlActionBuilder & call(ObjT & o, void (ObjT::*fun)(ItemT * item, uint32_t index)) {
        typedef CtxListItem<ObjT, ItemT> Ctx;

        Ctx & c = _create<Ctx>();
        c.m_outer = &o;
        c.m_fun = fun;
        
        return *this;
    }

    template<typename ObjT, typename ItemT>
    ControlActionBuilder & call(ObjT & o, void (ObjT::*fun)(ItemT * item, uint32_t index, RGUIControl & from)) {
        typedef CtxListItemWithFrom<ObjT, ItemT> Ctx;

        Ctx & c = _create<Ctx>();
        c.m_outer = &o;
        c.m_fun = fun;
        
        return *this;
    }

    template<typename ObjT, typename ArgT, typename _ArgT>
    ControlActionBuilder & call(ObjT & o, void (ObjT::*fun)(RGUIControl & c, ArgT), _ArgT arg) {
        typedef CtxFunCArg1<ObjT, ArgT> Ctx;
            
        Ctx & c = _create<Ctx>();
        c.m_outer = &o;
        c.m_fun = fun;
        c.m_arg = (ArgT)arg;

        return *this;
    }

    template<typename ObjT, typename ArgT>
    ControlActionBuilder & call(ObjT & o, void (ObjT::*fun)(RGUIControl & c, ArgT const &), ArgT const & arg) {
        typedef CtxFunCArg1Ref<ObjT, ArgT> Ctx;
            
        Ctx & c = _create<Ctx>();
        c.m_outer = &o;
        c.m_fun = fun;
        c.m_arg = (ArgT)arg;

        return *this;
    }
    
private:
    void * _create(plugin_ui_event_fun_t fun);

    template<typename CtxT>
    CtxT & _create(void) {
        assert(sizeof(CtxT) < PLUGIN_UI_CONTROL_ACTION_DATA_CAPACITY);
        return *(CtxT *)_create(&CtxT::call);
    }
    
    /*无参数调用 */
    template<typename ObjT>
    struct CtxFun0 {
        ObjT * m_outer;
        void (ObjT::*m_fun)();

        static void call(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
            CtxFun0 * c = (CtxFun0*)ctx;
            try { (c->m_outer->*c->m_fun)(); } catch(...) {}
        }
    };

    /*带参数调用1数值 */
    template<typename ObjT, typename ArgT>
    struct CtxFun0Arg1 {
        ObjT * m_outer;
        void (ObjT::*m_fun)(ArgT);
        ArgT m_arg;

        static void call(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
            CtxFun0Arg1 * c = (CtxFun0Arg1*)ctx;
            try { (c->m_outer->*c->m_fun)(c->m_arg); } catch(...) {}
        }
    };

    /*带参数调用Control+1数值 */
    template<typename ObjT, typename ArgT>
    struct CtxFunCArg1 {
        ObjT * m_outer;
        void (ObjT::*m_fun)(RGUIControl & c, ArgT);
        ArgT m_arg;

        static void call(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
            CtxFunCArg1 * c = (CtxFunCArg1*)ctx;
            try { (c->m_outer->*c->m_fun)(*(RGUIControl*)plugin_ui_control_product(from_control), c->m_arg); } catch(...) {}
        }
    };

    /*带参数调用Control+1数值引用 */
    template<typename ObjT, typename ArgT>
    struct CtxFunCArg1Ref {
        ObjT * m_outer;
        void (ObjT::*m_fun)(RGUIControl & c, ArgT const &);
        ArgT m_arg;

        static void call(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
            CtxFunCArg1Ref * c = (CtxFunCArg1Ref*)ctx;
            try { (c->m_outer->*c->m_fun)(*(RGUIControl*)plugin_ui_control_product(from_control), c->m_arg); } catch(...) {}
        }
    };

    /*带参数调用1引用 */
    template<typename ObjT, typename ArgT>
    struct CtxFun0Arg1Ref {
        ObjT * m_outer;
        void (ObjT::*m_fun)(const ArgT &);
        ArgT m_arg;

        static void call(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
            CtxFun0Arg1Ref * c = (CtxFun0Arg1Ref*)ctx;
            try { (c->m_outer->*c->m_fun)(c->m_arg); } catch(...) {}
        }
    };

    /*带control调用 */
    template<typename ObjT>
    struct CtxFunC {
        ObjT * m_outer;
        void (ObjT::*m_fun)(RGUIControl &);

        static void call(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
            CtxFunC * c = (CtxFunC*)ctx;
            try { (c->m_outer->*c->m_fun)(*(RGUIControl*)plugin_ui_control_product(from_control)); } catch(...) {}
        }
    };

    /*列表回调 */
    template<typename ObjT, typename ItemT>
    struct CtxListItem {
        ObjT * m_outer;
        void (ObjT::*m_fun)(ItemT * item, uint32_t index);

        static void call(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
            CtxListItem * c = (CtxListItem*)ctx;
            ItemT * item = NULL;
            for(plugin_ui_control_t p = from_control; item == NULL && p; p = plugin_ui_control_parent(p)) {
                RGUIControl * c = (RGUIControl*)plugin_ui_control_product(p);
                item = dynamic_cast<ItemT*>(c);
            }

            assert(item);
            try { (c->m_outer->*c->m_fun)(item, item->GetIndex()); } catch(...) {}
        }
    };

    template<typename ObjT, typename ItemT>
    struct CtxListItemWithFrom {
        ObjT * m_outer;
        void (ObjT::*m_fun)(ItemT * item, uint32_t index, RGUIControl & c);

        static void call(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
            CtxListItemWithFrom * c = (CtxListItemWithFrom*)ctx;
            ItemT * item = NULL;
            for(plugin_ui_control_t p = from_control; item == NULL && p; p = plugin_ui_control_parent(p)) {
                RGUIControl * c = (RGUIControl*)plugin_ui_control_product(p);
                item = dynamic_cast<ItemT*>(c);
            }

            assert(item);
            try { (c->m_outer->*c->m_fun)(item, item->GetIndex(), *(RGUIControl*)plugin_ui_control_product(from_control)); } catch(...) {}
        }
    };
        
    const char * m_name_prefix;
    plugin_ui_event_t m_event;
    plugin_ui_aspect_t m_asspect;
    plugin_ui_event_scope_t m_scope;
    plugin_ui_control_t m_control;
};

}

#define RGUICONTROL_DEF_BIND_FUN(__name, __evt)                         \
    ::Drow::ControlActionBuilder bind_ ## __name(                       \
        plugin_ui_event_scope_t scope = plugin_ui_event_scope_self) {   \
        ::Drow::ControlActionBuilder builder(this->control());          \
        builder.on(plugin_ui_event_ ## __evt, scope);                   \
        return builder;                                                 \
    }                                                                   \
    ::Drow::ControlActionBuilder bind_child_ ## __name(                 \
        const char * name) {                                            \
        ::Drow::ControlActionBuilder builder(this->control());          \
        builder                                                         \
            .on(plugin_ui_event_ ## __evt,                              \
                plugin_ui_event_scope_childs)                           \
            .prefix(name);                                              \
        return builder;                                                 \
    }                                                                   \


#endif
