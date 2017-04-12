#ifndef UIPP_SPRITE_CFG_LOADEREXTERNGEN_H
#define UIPP_SPRITE_CFG_LOADEREXTERNGEN_H
#include "cpe/pal/pal_queue.h"
#include "cpepp/utils/TypeUtils.hpp"
#include "cpepp/cfg/Node.hpp"
#include "gdpp/app/Application.hpp"
#include "uipp/sprite/World.hpp"
#include "uipp/sprite_fsm/State.hpp"
#include "System.hpp"
#include "CfgLoader.hpp"

namespace UI { namespace Sprite { namespace Cfg {

template<typename OuterT> 
class CfgLoaderExternGen {
protected:
    CfgLoaderExternGen()
        : m_loaders(NULL)
    {
    }

    ~CfgLoaderExternGen() {
        removeAllLoaders();
    }

    template<typename T>
    void addResourceLoader(void (OuterT::*fun)(T & obj, Cpe::Cfg::Node const & cfg) const, CfgLoader * loader = NULL) {
        OuterT * outer = Cpe::Utils::calc_cast<OuterT>(this);
        if (loader == NULL) {
            loader = &CfgLoader::instance(outer->app());
        }

        ::std::auto_ptr< ResLoader<T> > new_loader(new ResLoader<T>()); 

        new_loader->m_mgr = outer;
        new_loader->m_loader = loader;
        new_loader->m_fun = fun;
        new_loader->m_clear = ResLoader<T>::clear;

        loader->addResourceLoader(T::NAME, &load_resource<T>, new_loader.get());

        new_loader->m_next = m_loaders;
        m_loaders = new_loader.get();
        new_loader.release();
    }

    template<typename T>
    void addComponentLoader(void (OuterT::*fun)(T & obj, Cpe::Cfg::Node const & cfg) const, CfgLoader * loader = NULL) {
        OuterT * outer = Cpe::Utils::calc_cast<OuterT>(this);
        if (loader == NULL) {
            loader = &CfgLoader::instance(outer->app());
        }

        ::std::auto_ptr< ComponentLoader<T> > new_loader(new ComponentLoader<T>()); 

        new_loader->m_mgr = outer;
        new_loader->m_loader = loader;
        new_loader->m_fun = fun;
        new_loader->m_clear = ComponentLoader<T>::clear;

        loader->addComponentLoader(T::NAME, &load_component<T>, new_loader.get());

        new_loader->m_next = m_loaders;
        m_loaders = new_loader.get();
        new_loader.release();
    }

    template<typename T>
    void addActionLoader(void (OuterT::*fun)(T & obj, Cpe::Cfg::Node const & cfg) const, CfgLoader * loader = NULL) {
        OuterT * outer = Cpe::Utils::calc_cast<OuterT>(this);
        if (loader == NULL) {
            loader = &CfgLoader::instance(outer->app());
        }

        ::std::auto_ptr< ActionLoader<T> > new_loader(new ActionLoader<T>()); 

        new_loader->m_mgr = outer;
        new_loader->m_loader = loader;
        new_loader->m_fun = fun;
        new_loader->m_clear = ActionLoader<T>::clear;

        loader->addActionLoader(T::NAME, &load_action<T>, new_loader.get());

        new_loader->m_next = m_loaders;
        m_loaders = new_loader.get();
        new_loader.release();
    }

    void removeAllLoaders(void) {
        while(m_loaders) {
            Loader * remove_loaer = m_loaders;
            m_loaders = remove_loaer->m_next;

            remove_loaer->m_clear(remove_loaer);
            delete remove_loaer;
        }
    }

private:
    class Loader {
    public:
        OuterT * m_mgr;
        CfgLoader * m_loader;
        Loader * m_next;
        void (*m_clear)(Loader * loader);
    };

    /*resource*/
    template<typename T>
    class ResLoader : public Loader {
    public:
        void (OuterT::*m_fun)(T & obj, Cpe::Cfg::Node const & cfg) const;

        static void clear(Loader * loader) {
            loader->m_loader->removeResourceLoader(T::NAME);
        }
    };

    template<typename T>
    static ui_sprite_world_res_t load_resource(void * ctx, ui_sprite_world_t input_world, cfg_t cfg) {
        UI::Sprite::World & world = *(UI::Sprite::World*)input_world;
        ResLoader<T> * load_ctx =  (ResLoader<T>*)ctx;

        try {
            T & res = world.createRes<T>();
            
            (load_ctx->m_mgr->*load_ctx->m_fun)(res, Cpe::Cfg::Node::_cast(cfg));

            return ui_sprite_world_res_from_data(&res);
        }
        catch(...) {
            world.removeRes<T>();
            return NULL;
        }
    }

    /*component */
    template<typename T>
    class ComponentLoader : public Loader {
    public:
        void (OuterT::*m_fun)(T & obj, Cpe::Cfg::Node const & cfg) const;

        static void clear(Loader * loader) {
            loader->m_loader->removeComponentLoader(T::NAME);
        }
    };

    template<typename T>
    static int load_component(void * ctx, ui_sprite_component_t component, cfg_t cfg) {
        ComponentLoader<T> * load_ctx =  (ComponentLoader<T>*)ctx;

        try {
            (load_ctx->m_mgr->*load_ctx->m_fun)(*(T*)(ui_sprite_component_data(component)), Cpe::Cfg::Node::_cast(cfg));

            return 0;
        }
        catch(...) {
            return -1;
        }
    }

    /*action */
    template<typename T>
    class ActionLoader : public Loader {
    public:
        void (OuterT::*m_fun)(T & obj, Cpe::Cfg::Node const & cfg) const;

        static void clear(Loader * loader) {
            loader->m_loader->removeActionLoader(T::NAME);
        }
    };

    template<typename T>
    static ui_sprite_fsm_action_t load_action(void * ctx, ui_sprite_fsm_state_t input_fsm_state, const char * name, cfg_t cfg) {
        UI::Sprite::Fsm::State & state = *(UI::Sprite::Fsm::State*)input_fsm_state;
        ActionLoader<T> * load_ctx =  (ActionLoader<T>*)ctx;

        try {
            T & action = state.createAction<T>(name);

            try {
                (load_ctx->m_mgr->*load_ctx->m_fun)(action, Cpe::Cfg::Node::_cast(cfg));

                return ui_sprite_fsm_action_from_data(&action);
            }
            catch(...) {
                ui_sprite_fsm_action_free(ui_sprite_fsm_action_from_data(&action));
                throw;
            }
        }
        catch(...) {
            return NULL;
        }
    }

private:
    Loader * m_loaders;
};

}}}

#endif
