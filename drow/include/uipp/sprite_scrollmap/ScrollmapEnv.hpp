#ifndef UIPP_SPRITE_SCROLLMAP_ENV_H
#define UIPP_SPRITE_SCROLLMAP_ENV_H
#include "cpepp/utils/ClassCategory.hpp"
#include "plugin/scrollmap/plugin_scrollmap_script_executor.h"
#include "ui/sprite_scrollmap/ui_sprite_scrollmap_env.h"
#include "System.hpp"

namespace UI { namespace Sprite { namespace Scrollmap {

class ScrollmapEnv : public Cpe::Utils::SimulateObject {
public:
    operator ui_sprite_scrollmap_env_t () const { return (ui_sprite_scrollmap_env_t)this; }
    operator plugin_scrollmap_env_t() const { return ui_sprite_scrollmap_env_env(*this); }

    static const char * NAME;

    template<typename ObjT>
    void registerScriptExecutor(const char * type, ObjT & obj, void (ObjT::*fun)(plugin_scrollmap_script_t script)) {
        typedef ScriptExecutorCtx<ObjT> CtxT;

        plugin_scrollmap_script_executor_t executor =
            plugin_scrollmap_script_executor_create(*this, type, (uint32_t)sizeof(CtxT), execute<ObjT>);
        if (executor == NULL) return;

        CtxT * ctx = (CtxT *)plugin_scrollmap_script_executor_data(executor);
        ctx->m_obj = &obj;
        ctx->m_fun = fun;
    }

    void unregisterScriptExecutor(const char * type) {
        if (plugin_scrollmap_script_executor_t executor = plugin_scrollmap_script_executor_find(*this, type)) {
            plugin_scrollmap_script_executor_free(executor);
        }
    }
    
private:
    template<typename ObjT>
    static void execute(plugin_scrollmap_script_executor_t executor, plugin_scrollmap_script_t script) {
        try {
            typedef ScriptExecutorCtx<ObjT> CtxT;
            CtxT * ctx = (CtxT *)plugin_scrollmap_script_executor_data(executor);
            (ctx->m_obj->*ctx->m_fun)(script);
        }
        catch(...) {
        }
    }

    template<typename ObjT>
    struct ScriptExecutorCtx {
        ObjT * m_obj;
        void (ObjT::*m_fun)(plugin_scrollmap_script_t script);
    };
};

}}}

#endif
