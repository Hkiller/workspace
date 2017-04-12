#ifndef PLUGIN_CHIPMUNK_RENDER_I_H
#define PLUGIN_CHIPMUNK_RENDER_I_H
#include "plugin/chipmunk/plugin_chipmunk_render.h"
#include "plugin_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_chipmunk_render {
    plugin_chipmunk_env_t m_env;
};

int plugin_chipmunk_render_regist(plugin_chipmunk_module_t module);
void plugin_chipmunk_render_unregist(plugin_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
