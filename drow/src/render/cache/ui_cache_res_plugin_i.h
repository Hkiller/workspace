#ifndef UI_CACHE_RES_PLUGIN_I_H
#define UI_CACHE_RES_PLUGIN_I_H
#include "render/cache/ui_cache_res_plugin.h"
#include "ui_cache_manager_i.h"
#include "ui_cache_res_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_res_plugin {
    ui_cache_manager_t m_mgr;
    ui_cache_res_type_t m_res_type;
    char m_name[32];
    uint16_t m_capacity;
    void * m_ctx;
    ui_cache_res_plugin_on_loaded_fun_t m_on_load;
    ui_cache_res_plugin_on_unloaded_fun_t m_on_unload;
    struct ui_cache_res_plugin_addition_fun m_addition_funcs;
};

#ifdef __cplusplus
}
#endif

#endif
