#include <assert.h>
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "ui_ed_src_i.h"
#include "ui_ed_obj_i.h"
#include "ui_ed_rel_i.h"

int ui_ed_src_load_module(ui_ed_src_t ed_src) {
    ui_data_module_t module;
    struct ui_data_img_block_it img_block_it;
    ui_data_img_block_t img_block;

    module = ui_data_src_product(ed_src->m_data_src);
    assert(module);

    ui_data_img_block_in_module(&img_block_it, module);
    while((img_block = ui_data_img_block_it_next(&img_block_it))) {
        ui_ed_obj_t obj =
            ui_ed_obj_create_i(
                ed_src, ed_src->m_root_obj,
                ui_ed_obj_type_img_block,
                img_block, ui_data_img_block_data(img_block), sizeof(*ui_data_img_block_data(img_block)));
        if (obj == NULL) {
            return -1;
        }
    }

    return 0;
}

ui_ed_obj_t ui_ed_obj_create_img_block(ui_ed_obj_t parent) {
    ui_data_module_t module;
    ui_ed_src_t ed_src = parent->m_src;
    ui_data_img_block_t img_block;
    ui_ed_obj_t obj;

    module = ui_data_src_product(ed_src->m_data_src);
    assert(module);

    img_block = ui_data_img_block_create(module);
    if (img_block == NULL) return NULL;

    obj = ui_ed_obj_create_i(
        parent->m_src, parent,
        ui_ed_obj_type_img_block,
        img_block, ui_data_img_block_data(img_block), sizeof(*ui_data_img_block_data(img_block)));
    if (obj == NULL) {
        ui_data_img_block_free(img_block);
        return NULL;
    }

    return obj;
}
