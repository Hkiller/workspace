#ifndef UIPP_SPRITE_COMPONENT_REGISTER_H
#define UIPP_SPRITE_COMPONENT_REGISTER_H
#include <memory>
#include "ComponentMeta.hpp"
#include "Repository.hpp"

namespace UI { namespace Sprite {

template<typename T>
class ComponentReg {
public:
    typedef int (T::*enter_fun_t)();
    typedef void (T::*enter_fun_2_t)();
    typedef void (T::*exit_fun_t)();
    typedef void (T::*update_fun_t)(float delta);

    ComponentReg(Repository & repo)
        : m_meta(repo.createComponentMeta(T::NAME, sizeof(T)))
    {
        ui_sprite_component_meta_set_init_fun(m_meta, &call_init, NULL);
        ui_sprite_component_meta_set_copy_fun(m_meta, &call_clone, NULL);
        ui_sprite_component_meta_set_free_fun(m_meta, &call_free, NULL);
    }

    ComponentReg & on_enter(enter_fun_t fun) {
        static enter_fun_t s_fun = fun;
        ui_sprite_component_meta_set_enter_fun(m_meta, &call_enter, &s_fun);
        return *this;
    }

    ComponentReg & on_enter(enter_fun_2_t fun) {
        static enter_fun_2_t s_fun = fun;
        ui_sprite_component_meta_set_enter_fun(m_meta, &call_enter_2, &s_fun);
        return *this;
    }

    ComponentReg & on_exit(exit_fun_t fun) {
        static exit_fun_t s_fun = fun;
        ui_sprite_component_meta_set_exit_fun(m_meta, &call_exit, &s_fun);
        return *this;
    }

    ComponentReg & on_update(update_fun_t fun) {
        static update_fun_t s_fun = fun;
        ui_sprite_component_meta_set_update_fun(m_meta, &call_update, &s_fun);
        return *this;
    }

    ComponentReg & with_data(void) {
        ui_sprite_component_meta_set_data_meta(
            m_meta,
            Cpe::Dr::MetaTraits<typename T::ComponentDataType>::META,
            T::data_start(), sizeof(typename T::ComponentDataType));
        return *this;
    }

private:
    static T & cast(ui_sprite_component_t component) { return *(T*)ui_sprite_component_data(component); }

    static  int call_enter(ui_sprite_component_t component, void * ctx) {
        try {
            enter_fun_t * fun = (enter_fun_t *)ctx;
            return (cast(component).**fun)();
        }
        catch(...) {
            return -1;
        }
    }

    static  int call_enter_2(ui_sprite_component_t component, void * ctx) {
        try {
            enter_fun_2_t * fun = (enter_fun_2_t *)ctx;
            (cast(component).**fun)();
            return 0;
        }
        catch(...) {
            return -1;
        }
    }

    static  void call_exit(ui_sprite_component_t component, void * ctx) {
        try {
            exit_fun_t * fun = (exit_fun_t *)ctx;
            return (cast(component).**fun)();
        }
        catch(...) {
        }
    }

    static  void call_update(ui_sprite_component_t component, void * ctx, float delta) {
        try {
            update_fun_t * fun = (update_fun_t *)ctx;
            return (cast(component).**fun)(delta);
        }
        catch(...) {
        }
    }

    static int call_init(ui_sprite_component_t component, void * ctx) {
        try {
            new (ui_sprite_component_data(component)) T(*(Component *)component);
            return 0;
        }
        catch(...) {
            return -1;
        }
    }

    static void call_free(ui_sprite_component_t component, void * ctx) {
        cast(component).~T();
    }

    static int call_clone(ui_sprite_component_t to, ui_sprite_component_t from, void * ctx) {
        try {
            new (ui_sprite_component_data(to)) T(*(Component *)to, cast(from));
            return 0;
        }            
        catch(...) {
            return -1;
        }
    }

    ComponentMeta & m_meta;
};

}}

#endif
