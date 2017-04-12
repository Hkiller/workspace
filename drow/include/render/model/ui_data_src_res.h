#ifndef UI_MODEL_DATA_SRC_RES_H
#define UI_MODEL_DATA_SRC_RES_H
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src_res_it {
    ui_cache_res_t (*next)(struct ui_data_src_res_it * it);
    char m_data[64];
};

/*src create and free*/
ui_data_src_res_t ui_data_src_res_create(ui_data_src_t src, ui_cache_res_t res);
void ui_data_src_res_free(ui_data_src_res_t src_res);

ui_data_src_res_t ui_data_src_res_create_by_path(ui_data_src_t src, const char * path);
    
#define ui_data_src_res_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
