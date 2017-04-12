#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "render/model/ui_data_mgr.h"
#include "ui_data_mgr_i.h"
#include "ui_data_action_i.h"
#include "ui_data_layout_i.h"
#include "ui_data_sprite_i.h"
#include "ui_data_module_i.h"

void ui_data_mgr_product_init(ui_data_mgr_t mgr) {
    struct ui_product_type * product_type;
    uint8_t i;

    bzero(mgr->m_product_types, sizeof(mgr->m_product_types));
    for(i = 0; i < CPE_ARRAY_SIZE(mgr->m_product_types); ++i) {
        TAILQ_INIT(&mgr->m_product_types[i].srcs);
    }

    /*module*/
    product_type = mgr->m_product_types + (ui_data_src_type_module - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create_ctx = mgr;
    product_type->product_create = (ui_data_product_create_fun_t)ui_data_module_create;
    product_type->product_free_ctx = mgr;
    product_type->product_free = (ui_data_product_free_fun_t)ui_data_module_free;
    product_type->product_update_usings = ui_data_module_update_using;

    /*action*/
    product_type = mgr->m_product_types + (ui_data_src_type_action - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create_ctx = mgr;
    product_type->product_create = (ui_data_product_create_fun_t)ui_data_action_create;
    product_type->product_free_ctx = mgr;
    product_type->product_free = (ui_data_product_free_fun_t)ui_data_action_free;
    product_type->product_update_usings = ui_data_action_update_using;

    /*sprite*/
    product_type = mgr->m_product_types + (ui_data_src_type_sprite - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create_ctx = mgr;
    product_type->product_create = (ui_data_product_create_fun_t)ui_data_sprite_create;
    product_type->product_free_ctx = mgr;
    product_type->product_free = (ui_data_product_free_fun_t)ui_data_sprite_free;
    product_type->product_update_usings = ui_data_sprite_update_using;

    /*layout*/
    product_type = mgr->m_product_types + (ui_data_src_type_layout - UI_DATA_SRC_TYPE_MIN);
    product_type->product_create_ctx = mgr;
    product_type->product_create = (ui_data_product_create_fun_t)ui_data_layout_create;
    product_type->product_free_ctx = mgr;
    product_type->product_free = (ui_data_product_free_fun_t)ui_data_layout_free;
    product_type->product_update_usings = ui_data_layout_update_using;
}

void ui_data_mgr_set_loader(ui_data_mgr_t mgr, ui_data_src_type_t type, product_load_fun_t loader, void * ctx) {
    struct ui_product_type * product_type;
    
    assert(type >= UI_DATA_SRC_TYPE_MIN && (uint8_t)type < UI_DATA_SRC_TYPE_MAX);

    product_type = mgr->m_product_types + (type - UI_DATA_SRC_TYPE_MIN);

    product_type->product_load_ctx = ctx;
    product_type->product_load = loader;
}

void ui_data_mgr_set_saver(ui_data_mgr_t mgr, ui_data_src_type_t type, product_save_fun_t saver, product_remove_fun_t remover, void * ctx) {
    struct ui_product_type * product_type;
    
    assert(type >= UI_DATA_SRC_TYPE_MIN && (uint8_t)type < UI_DATA_SRC_TYPE_MAX);

    product_type = mgr->m_product_types + (type - UI_DATA_SRC_TYPE_MIN);

    product_type->product_save_ctx = ctx;
    product_type->product_save = saver;
    product_type->product_remove = remover;
}
