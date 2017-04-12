#ifndef DROW_RGUI_POPUPCREATOR_H_INCLEDED
#define DROW_RGUI_POPUPCREATOR_H_INCLEDED
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "plugin/ui/plugin_ui_types.h"
#include "RGUI.h"

namespace Drow {

class Popup : public Cpe::Utils::SimulateObject {
public:
    operator plugin_ui_popup_t () const { return (plugin_ui_popup_t)this; }

    RGUIWindow & page(void);

    bool triggerAction(const char * action_name);

    Popup & set_data(dr_data_t data);
    Popup & set_data(const char * slot_name, const char * slot_value);
    
    template<typename T>
    Popup & set_data(T const & data) {
        dr_data d;
        d.m_meta = Cpe::Dr::MetaTraits<T>::META;
        d.m_data = (void*)&data;
        d.m_size = Cpe::Dr::MetaTraits<T>::data_size(data);
        return set_data(&d);
    }

    template<typename OuterT>
    Popup & add_action(const char * name, OuterT & o, void(OuterT::*fun)(Popup & popup)) {
        typedef CtxWithPopup<OuterT> Ctx;
        Ctx * c = add_action<Ctx>(name);
        c->m_outer = &o;
        c->m_fun = fun;
        return *this;
    }

    template<typename OuterT, typename ArgT>
    Popup & add_action(const char * name, OuterT & o, void(OuterT::*fun)(Popup & popup, ArgT arg), ArgT arg) {
        typedef CtxWithPopupVArg1<OuterT, ArgT> Ctx;
        Ctx * c = add_action<Ctx>(name);
        c->m_outer = &o;
        c->m_fun = fun;
        c->m_arg = arg;
        return *this;
    }

    template<typename OuterT, typename ArgT>
    Popup & add_action(const char * name, OuterT & o, void(OuterT::*fun)(Popup & popup, ArgT const & arg), ArgT const & arg) {
        typedef CtxWithPopupArg1<OuterT, ArgT> Ctx;
        Ctx * c = add_action<Ctx>(name);
        c->m_outer = &o;
        c->m_fun = fun;
        c->m_arg = arg;
        return *this;
    }
    
    template<typename OuterT, typename ArgT>
    Popup & add_action(
        const char * name, OuterT & o, void(OuterT::*fun)(Popup & popup, RGUIControl & from, ArgT const & arg),
        RGUIControl & from, ArgT const & arg)
    {
        typedef CtxWithPopupCArg1<OuterT, ArgT> Ctx;
        Ctx * c = add_action<Ctx>(name);
        c->m_outer = &o;
        c->m_fun = fun;
        c->m_from = &from;
        c->m_arg = arg;
        return *this;
    }
    
    void show(void);
    void hide(void);

    void * data(LPDRMETA check_meta = NULL);
    template<typename T>
    T & data(void) { return *(T*)data(Cpe::Dr::MetaTraits<T>::META); }
    
    static Popup & cast(plugin_ui_popup_t popup);
    static Popup & create(plugin_ui_env_t env, const char * def);

private:
    void * add_action(const char * name, plugin_ui_popup_action_fun_t fun, uint32_t capacity);

    template<typename CtxT>
    CtxT * add_action(const char * name) { return (CtxT*)add_action(name, CtxT::call, sizeof(CtxT)); }

    template<typename OuterT>
    struct CtxWithPopup {
        OuterT * m_outer;
        void (OuterT::*m_fun)(Popup &);

        static void call(void * ctx, plugin_ui_popup_t popup, const char * action_name) {
            CtxWithPopup * c = (CtxWithPopup*)ctx;
            try { (c->m_outer->*c->m_fun)(Popup::cast(popup)); } catch(...) {}
        }
    };


	template<typename OuterT, typename ArgT>
	struct CtxWithPopupVArg1 {
		OuterT * m_outer;
		void (OuterT::*m_fun)(Popup &, ArgT);
		ArgT m_arg;

		static void call(void * ctx, plugin_ui_popup_t popup, const char * action_name) {
			CtxWithPopupVArg1 * c = (CtxWithPopupVArg1*)ctx;
			try { (c->m_outer->*c->m_fun)(Popup::cast(popup), c->m_arg); } catch(...) {}
		}
	};

	template<typename OuterT, typename ArgT>
	struct CtxWithPopupArg1 {
		OuterT * m_outer;
		void (OuterT::*m_fun)(Popup &, ArgT const &);
		ArgT m_arg;

		static void call(void * ctx, plugin_ui_popup_t popup, const char * action_name) {
			CtxWithPopupArg1 * c = (CtxWithPopupArg1*)ctx;
			try { (c->m_outer->*c->m_fun)(Popup::cast(popup), c->m_arg); } catch(...) {}
		}
	};

    template<typename OuterT, typename ArgT>
    struct CtxWithPopupCArg1 {
        OuterT * m_outer;
        void (OuterT::*m_fun)(Popup &, RGUIControl & from, ArgT const &);
        RGUIControl * m_from;
        ArgT m_arg;

        static void call(void * ctx, plugin_ui_popup_t popup, const char * action_name) {
            CtxWithPopupCArg1 * c = (CtxWithPopupCArg1*)ctx;
            try { (c->m_outer->*c->m_fun)(Popup::cast(popup), *c->m_from, c->m_arg); } catch(...) {}
        }
    };
};

}

#endif
