#ifndef DROW_PLUGIN_UI_MOVE_ALGORITHM_META_H
#define DROW_PLUGIN_UI_MOVE_ALGORITHM_META_H
#include "plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_ui_move_alogrithm_init_fun_t)(plugin_ui_move_algorithm_t algorithm, void * ctx);
typedef void (*plugin_ui_move_alogrithm_fini_fun_t)(plugin_ui_move_algorithm_t algorithm, void * ctx);    
typedef int (*plugin_ui_move_alogrithm_calc_duration_fun_t)(plugin_ui_move_algorithm_t algorithm, float * result, void * ctx, ui_vector_2_t from, ui_vector_2_t to);
typedef int (*plugin_ui_move_alogrithm_calc_pos_fun_t)(
    plugin_ui_move_algorithm_t algorithm, void * ctx, ui_vector_2_t result,
    ui_vector_2_t from, ui_vector_2_t to, float percent);
typedef int (*plugin_ui_move_algorithm_setup_fun_t)(plugin_ui_move_algorithm_t algorithm, void * ctx, char * arg_buf_will_change);

plugin_ui_move_algorithm_meta_t
plugin_ui_move_algorithm_meta_create(
    plugin_ui_module_t module,
    const char * type_name,
    void * ctx,
    size_t capacity,
    plugin_ui_move_alogrithm_init_fun_t init,
    plugin_ui_move_alogrithm_fini_fun_t fini,
    plugin_ui_move_alogrithm_calc_duration_fun_t calc_duration,
    plugin_ui_move_alogrithm_calc_pos_fun_t calc_pos,
    plugin_ui_move_algorithm_setup_fun_t setup);

void plugin_ui_move_algorithm_meta_free(plugin_ui_move_algorithm_meta_t meta);

plugin_ui_move_algorithm_meta_t
plugin_ui_move_algorithm_meta_find(plugin_ui_module_t module, const char * type_name);

#ifdef __cplusplus
}
#endif

#endif

