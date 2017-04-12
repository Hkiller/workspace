#ifndef PLUGIN_BARRAGE_RENDER_I_H
#define PLUGIN_BARRAGE_RENDER_I_H
#include "plugin/barrage/plugin_barrage_render.h"
#include "plugin_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_render {
    plugin_barrage_env_t m_env;
    char m_barrage_group[64];
};

int plugin_barrage_render_regist(plugin_barrage_module_t module);
void plugin_barrage_render_unregist(plugin_barrage_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
