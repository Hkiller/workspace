#ifndef UI_CACHE_RES_PLUGIN_H
#define UI_CACHE_RES_PLUGIN_H
#include "render/utils/ui_utils_types.h"
#include "ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*ui_cache_res_plugin_on_loaded_fun_t)(void * ctx, ui_cache_res_t res);
typedef void (*ui_cache_res_plugin_on_unloaded_fun_t)(void * ctx, ui_cache_res_t res, uint8_t is_external_unload);

struct ui_cache_res_plugin_addition_fun {
    union {
        struct {
            int (*m_upload_part)(void * ctx, ui_cache_res_t res, ui_rect_t rect, void const * data);
        } m_texture;
    };
};
        
ui_cache_res_plugin_t
ui_cache_res_plugin_create(
    ui_cache_manager_t mgr, ui_cache_res_type_t res_type,
    const char * name, uint16_t capacitiy, void * ctx,
    ui_cache_res_plugin_on_loaded_fun_t on_load,
    ui_cache_res_plugin_on_unloaded_fun_t on_unload,
    struct ui_cache_res_plugin_addition_fun * addition_funcs);

void ui_cache_res_plugin_free(ui_cache_res_plugin_t plugin);

ui_cache_res_plugin_t ui_cache_res_plugin_find_by_type(ui_cache_manager_t mgr, ui_cache_res_type_t res_type);
ui_cache_res_plugin_t ui_cache_res_plugin_find_by_ctx(ui_cache_manager_t mgr, void * ctx);

void * ui_cache_res_plugin_data(ui_cache_res_t res);
    
#ifdef __cplusplus
}
#endif

#endif

