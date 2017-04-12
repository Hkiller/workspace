#ifndef UI_MODEL_DATA_SRC_USER_H
#define UI_MODEL_DATA_SRC_USER_H
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_data_src_on_unload_fun_t)(void * ctx, ui_data_src_t src);
    
ui_data_src_user_t ui_data_src_user_create(ui_data_src_t src, void * ctx, ui_data_src_on_unload_fun_t fun);
void ui_data_src_user_free(ui_data_src_user_t src_user);

uint8_t ui_data_src_remove_user(ui_data_src_t src, void * ctx);
    
#ifdef __cplusplus
}
#endif

#endif
