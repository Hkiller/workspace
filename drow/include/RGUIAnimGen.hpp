#ifndef DROW_RGUI_ANIM_GEN_H_INCLEDED
#define DROW_RGUI_ANIM_GEN_H_INCLEDED
#include "cpepp/utils/TypeUtils.hpp"
#include "plugin/ui/plugin_ui_animation.h"
#include "RGUIAnimation.hpp"

namespace Drow {

template<class OuterT,  typename wrap_t>
class AnimationGen : public Cpe::Utils::SimulateObject {
public:
    operator wrap_t () const { return (wrap_t)this; }

    Animation & toAnim(void) { return *(Animation*)plugin_ui_animation_from_data(this); }
    Animation const & toAnim(void) const { return *(Animation*)plugin_ui_animation_from_data(this); }

    OuterT & setup(const char * args) { toAnim().setup(args); return Cpe::Utils::calc_cast<OuterT, AnimationGen>(*this); }
    OuterT & setup(char * args) { toAnim().setup(args); return Cpe::Utils::calc_cast<OuterT, AnimationGen>(*this); }
    
    bool start(void) { return toAnim().start(); }

    static OuterT * cast(plugin_ui_animation_t anim) {
        if (strcmp(OuterT::TYPE_NAME, plugin_ui_animation_type_name(anim)) == 0) {
            return (OuterT*)plugin_ui_animation_data(anim);
        }
        else {
            return NULL;
        }
    }

    template<typename ObjT>
    OuterT & onComplete(ObjT & obj, void(ObjT::*fun)(void)) {
        CallCtx<ObjT> * ctx = new CallCtx<ObjT>(obj, fun);
        
        plugin_ui_animation_set_on_complete(plugin_ui_animation_from_data(this), (void*)this, &call_on_complete<ObjT>,(void*)ctx, CallCtx<ObjT>::free);
        
        return Cpe::Utils::calc_cast<OuterT, AnimationGen>(*this);
    }

    OuterT & setDelayFrame(uint32_t frame) { toAnim().setDelayFrame(frame); return Cpe::Utils::calc_cast<OuterT, AnimationGen>(*this); }
    OuterT & setDelay(float delay) { toAnim().setDelay(delay); return Cpe::Utils::calc_cast<OuterT, AnimationGen>(*this); }
    OuterT & setLoop(uint32_t loop_count, float loop_delay) { toAnim().setLoop(loop_count, loop_delay); return Cpe::Utils::calc_cast<OuterT, AnimationGen>(*this); }

private:
    template<typename ObjT>
    class CallCtx {
    public:
        CallCtx(ObjT & obj, void(ObjT::*fun)(void))
            : m_obj(obj)
            , m_fun(fun)
        {
        }
        
        ObjT & m_obj;
        void(ObjT::*m_fun)(void);

        static void free(void * p) { delete (CallCtx<ObjT>*)p ; }
    };
    
    template<typename ObjT>
    static void call_on_complete(void * i_ctx, plugin_ui_animation_t animation, void * args) {
        CallCtx<ObjT> * ctx = (CallCtx<ObjT>*)ctx;
        (ctx->m_obj.*ctx->m_fun)();
    }
};

}

#endif
