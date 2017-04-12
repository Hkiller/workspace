#ifndef UI_DATA_INTERNAL_MODULE_H
#define UI_DATA_INTERNAL_MODULE_H
#include "render/model/ui_data_module.h"
#include "ui_data_mgr_i.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_data_img_block_list, ui_data_img_block) ui_data_img_block_list_t;

struct ui_data_module {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    ui_data_img_block_list_t m_img_blocks;
};

struct ui_data_img_block {
    ui_data_module_t m_module;
    struct cpe_hash_entry m_hh_for_mgr;
    TAILQ_ENTRY(ui_data_img_block) m_next_for_module;
    UI_IMG_BLOCK m_data;
    ui_cache_res_t m_using_res;
};

uint32_t ui_data_img_block_hash(const ui_data_img_block_t src);
int ui_data_img_block_eq(const ui_data_img_block_t l, const ui_data_img_block_t r);

int ui_data_module_update_using(ui_data_src_t user);
    
#ifdef __cplusplus
}
#endif

#endif
