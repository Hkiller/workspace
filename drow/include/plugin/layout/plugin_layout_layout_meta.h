#ifndef DROW_LAYOUT_LAYOUT_META_H
#define DROW_LAYOUT_LAYOUT_META_H
#include "plugin_layout_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_layout_layout_init_fun_t)(plugin_layout_layout_t layout);
typedef void (*plugin_layout_layout_fini_fun_t)(plugin_layout_layout_t layout);
typedef int (*plugin_layout_layout_setup_fun_t)(plugin_layout_layout_t layout, char * arg_buf_will_change);
typedef int (*plugin_layout_layout_analize_fun_t)(plugin_layout_layout_t layout);
typedef int (*plugin_layout_layout_layout_fun_t)(plugin_layout_layout_t layout);    
typedef int (*plugin_layout_layout_update_fun_t)(plugin_layout_layout_t layout, int begin_pos, int end_pos, uint32_t const * text, size_t text_len);

plugin_layout_layout_meta_t
plugin_layout_layout_meta_create(
    plugin_layout_module_t module,
    const char * layout_name,
    uint32_t data_capacity,
    plugin_layout_layout_init_fun_t init_fun,
    plugin_layout_layout_fini_fun_t fini_fun,
    plugin_layout_layout_setup_fun_t setup_fun,
    plugin_layout_layout_analize_fun_t analize_fun,
    plugin_layout_layout_layout_fun_t layout_fun,
    plugin_layout_layout_update_fun_t update_fun);

void plugin_layout_layout_meta_free(plugin_layout_layout_meta_t);

plugin_layout_layout_meta_t
plugin_layout_layout_meta_find(plugin_layout_module_t module, const char * type);
    
#ifdef __cplusplus
}
#endif

#endif

