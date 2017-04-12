#ifndef UI_MODEL_DATA_MODULE_H
#define UI_MODEL_DATA_MODULE_H
#include "protocol/render/model/ui_module.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_img_block_it {
    ui_data_img_block_t (*next)(struct ui_data_img_block_it * it);
    char m_data[64];
};

ui_data_module_t ui_data_module_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_module_free(ui_data_module_t module);

/*img block*/
ui_data_img_block_t ui_data_img_block_create(ui_data_module_t module);
ui_data_img_block_t ui_data_img_block_find_by_id(ui_data_module_t module, uint32_t id);
ui_data_img_block_t ui_data_img_block_find_by_name(ui_data_module_t module, const char * name);
ui_data_module_t ui_data_img_block_module(ui_data_img_block_t block);
int ui_data_img_block_set_id(ui_data_img_block_t block, uint32_t id);

ui_cache_res_t ui_data_img_block_using_texture(ui_data_img_block_t block);
const char * ui_data_img_block_using_texture_path(ui_data_img_block_t block);
const char * ui_data_img_block_name(ui_data_img_block_t block);

uint8_t ui_data_img_block_is_scale_style(ui_data_img_block_t block);
uint8_t ui_data_img_block_is_alpha_blend(ui_data_img_block_t block);

void ui_data_img_block_free(ui_data_img_block_t img_block);
void ui_data_img_block_free_in_module(ui_data_module_t module);

void ui_data_img_block_in_module(ui_data_img_block_it_t it, ui_data_module_t module);

void ui_data_img_block_bounding_rect(ui_data_img_block_t img_block, ui_rect_t rect);
    
UI_IMG_BLOCK * ui_data_img_block_data(ui_data_img_block_t img_block);
LPDRMETA ui_data_img_block_meta(ui_data_mgr_t data_mgr);
ui_data_src_t ui_data_img_block_src(ui_data_img_block_t img_block);
        
#define ui_data_img_block_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
