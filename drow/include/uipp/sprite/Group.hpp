#ifndef UIPP_SPRITE_GROUP_H
#define UIPP_SPRITE_GROUP_H
#include <memory>
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Utils.hpp"
#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/Data.hpp"
#include "ui/sprite/ui_sprite_group.h"
#include "Entity.hpp"
#include "EntityIterator.hpp"

namespace UI { namespace Sprite {

class Group : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_group_t () const { return (ui_sprite_group_t)this; }
    static Group & cast(ui_sprite_group_t g) { return *(Group*)g; }

    World & world(void) { return *(World *)ui_sprite_group_world(*this); }
    World const & world(void) const { return *(World const *)ui_sprite_group_world(*this); }

    uint32_t id(void) const { return ui_sprite_group_id(*this); }
    const char * name(void) const { return ui_sprite_group_name(*this); }

    void addElement(Group & element);
    void removeElement(Group & element) { ui_sprite_group_remove_group(*this, element); }

    void addElement(Entity & element);
    void removeElement(Entity & element) { ui_sprite_group_remove_entity(*this, element); }

    bool isEmpty(void) const { return ui_sprite_group_is_empty(*this) ? true : false; }
    uint32_t count(void) const { return ui_sprite_group_count(*this); }

    Entity & firstEntity(void);
    Entity const & firstEntity(void) const;

    Entity * findFirstEntity(void) { return (Entity*)ui_sprite_group_first_entity(*this); }
    Entity const * findFirstEntity(void) const { return (Entity const*)ui_sprite_group_first_entity(*this); }

    ::std::auto_ptr<EntityIterator> entities(mem_allocrator_t alloc = NULL);

    /*reduce */
    template<typename T>
    T reduce(void (*fun)(Entity & entity, T & ctx), T const & init = T()) {
        T ctx(init);
        visit(fun, ctx);
        return ctx;
    }

    /*visit */
    void visit(ui_sprite_group_visit_fun_t visitor, void * ctx) { ui_sprite_group_visit(*this, visitor, ctx); }

    template<typename CtxT>
    void visit(void (*fun)(Entity & entity, CtxT &), CtxT & data) {
        VisitorCtx1<CtxT> ctx(data, fun);
        visit(&do_visit_1<CtxT>, &ctx);
    }

    template<typename CtxT>
    void visit(void (*fun)(Group & group, Entity & entity, CtxT &), CtxT & data) {
        VisitorCtx2<CtxT> ctx(data, fun);
        visit(&do_visit_2<CtxT>, &ctx);
    }

    template<typename VisitorT>
    void visit(VisitorT & visitor, void (VisitorT::*fun)(Group & group, Entity & entity)) {
        VisitorObjCtx2<VisitorT> ctx(visitor, fun);
        visit(&do_visit_obj_2<VisitorT>, &ctx);
    }
    
    template<typename VisitorT>
    void visit(VisitorT & visitor, void (VisitorT::*fun)(Entity & entity)) {
        VisitorObjCtx1<VisitorT> ctx(visitor, fun);
        visit(&do_visit_obj_1<VisitorT>, &ctx);
    }
    
    /*event operations*/
    void sendEvent(LPDRMETA meta, void const * data, size_t data_size) { ui_sprite_group_send_event(*this, meta, data, data_size); }

    template<typename T>
    void sendEvent(T const & data) {
        sendEvent(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

private:
    template<typename VisitorT>
    struct VisitorObjCtx2 {
        VisitorObjCtx2(VisitorT * obj, void (VisitorT::*fun)(Group & group, Entity & entity))
            : m_obj(obj), m_fun(fun)
        {
        }

        VisitorT & m_obj;
        void (VisitorT::*m_fun)(Group & group, Entity & entity);
    };

    template<typename VisitorT>
    static void do_visit_obj_2(ui_sprite_group_t g, ui_sprite_entity_t entity, void * visit_ctx) {
        VisitorObjCtx2<VisitorT> * ctx = (VisitorObjCtx2<VisitorT>*)visit_ctx;
        try {
            (ctx->m_obj.*ctx->m_fun)(*(Group*)g, *(Entity*)entity);
        }
        catch(...) {
        }
    }

    template<typename VisitorT>
    struct VisitorObjCtx1 {
        VisitorObjCtx1(VisitorT * obj, void (VisitorT::*fun)(Entity & entity))
            : m_obj(obj), m_fun(fun)
        {
        }

        VisitorT & m_obj;
        void (VisitorT::*m_fun)(Entity & entity);
    };

    template<typename VisitorT>
    static void do_visit_obj_1(ui_sprite_group_t g, ui_sprite_entity_t entity, void * visit_ctx) {
        VisitorObjCtx1<VisitorT> * ctx = (VisitorObjCtx1<VisitorT>*)visit_ctx;
        try {
            (ctx->m_obj.*ctx->m_fun)(*(Entity*)entity);
        }
        catch(...) {
        }
    }

    template<typename CtxT>
    struct VisitorCtx2 {
        VisitorCtx2(CtxT & init, void (*fun)(Group & group, Entity & entity, CtxT & ctx))
            : m_ctx(init), m_fun(fun)
        {
        }

        CtxT & m_ctx;
        void (*m_fun)(Group & group, Entity & entity, CtxT & ctx);
    };

    template<typename VisitorT>
    static void do_visit_2(ui_sprite_group_t g, ui_sprite_entity_t entity, void * visit_ctx) {
        VisitorCtx2<VisitorT> * ctx = (VisitorCtx2<VisitorT>*)visit_ctx;
        try {
            (*ctx->m_fun)(*(Group*)g, *(Entity*)entity, ctx->m_ctx);
        }
        catch(...) {
        }
    }

    template<typename CtxT>
    struct VisitorCtx1 {
        VisitorCtx1(CtxT & init, void (*fun)(Entity & entity, CtxT & ctx))
            : m_ctx(init), m_fun(fun)
        {
        }

        CtxT & m_ctx;
        void (*m_fun)(Entity & entity, CtxT & ctx);
    };

    template<typename VisitorT>
    static void do_visit_1(ui_sprite_group_t g, ui_sprite_entity_t entity, void * visit_ctx) {
        VisitorCtx1<VisitorT> * ctx = (VisitorCtx1<VisitorT>*)visit_ctx;
        try {
            (*ctx->m_fun)(*(Entity*)entity, ctx->m_ctx);
        }
        catch(...) {
        }
    }
};

}}

#endif
