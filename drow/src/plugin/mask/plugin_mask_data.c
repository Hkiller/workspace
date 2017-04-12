#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/bitarry.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "plugin_mask_data_i.h"
#include "plugin_mask_data_block_i.h"

plugin_mask_data_t
plugin_mask_data_create(plugin_mask_module_t module, ui_data_src_t src) {
    plugin_mask_data_t data;

    if (ui_data_src_type(src) != ui_data_src_type_mask) {
        CPE_ERROR(
            module->m_em, "create data at %s: src not data!",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create data at %s: product already loaded!",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return NULL;
    }

    data = (plugin_mask_data_t)mem_alloc(module->m_alloc, sizeof(struct plugin_mask_data));
    if (data == NULL) {
        CPE_ERROR(
            module->m_em, "create data at %s: alloc data fail!",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return NULL;
    }

    data->m_module = module;
    data->m_src = src;
    data->m_block_count = 0;
    data->m_format = plugin_mask_data_format_bit;
    TAILQ_INIT(&data->m_blocks);
    
    ui_data_src_set_product(src, data);
    return data;
}

void plugin_mask_data_free(plugin_mask_data_t data) {
    plugin_mask_module_t module = data->m_module;
    
    while(!TAILQ_EMPTY(&data->m_blocks)) {
        plugin_mask_data_block_free(TAILQ_FIRST(&data->m_blocks));
    }
    assert(data->m_block_count == 0);

    mem_free(module->m_alloc, data);
}

uint32_t plugin_mask_data_blocks_count(plugin_mask_data_t data) {
    return data->m_block_count;
}

plugin_mask_data_format_t plugin_mask_data_format(plugin_mask_data_t data) {
    return data->m_format;
}

int plugin_mask_data_set_format(plugin_mask_data_t data, plugin_mask_data_format_t format) {
    if (data->m_format == format) return 0;

    if (!TAILQ_EMPTY(&data->m_blocks)) {
        CPE_ERROR(data->m_module->m_em, "mask: can`t set format with blocks!");
        return -1;
    }

    data->m_format = format;
    return 0;
}

int plugin_mask_data_format_from_str(const char * str, plugin_mask_data_format_t * format) {
    if (strcmp(str, "bit") == 0) {
        *format = plugin_mask_data_format_bit;
    }
    else if (strcmp(str, "byte") == 0) {
        *format = plugin_mask_data_format_1;
    }
    else if (strcmp(str, "byte2") == 0) {
        *format = plugin_mask_data_format_2;
    }
    else if (strcmp(str, "byte4") == 0) {
        *format = plugin_mask_data_format_4;
    }
    else {
        return -1;
    }

    return 0;
}

const char * plugin_mask_data_format_to_str(plugin_mask_data_format_t foramt) {
    switch(foramt) {
    case plugin_mask_data_format_bit:
        return "bit";
    case plugin_mask_data_format_1:
        return "byte";
    case plugin_mask_data_format_2:
        return "byte2";
    case plugin_mask_data_format_4:
        return "byte4";
    default:
        return "unknown";
    }
}

int plugin_mask_data_regist(plugin_mask_module_t module) {
    if (ui_data_mgr_register_type(
            module->m_data_mgr, ui_data_src_type_mask,
            (ui_data_product_create_fun_t)plugin_mask_data_create, module,
            (ui_data_product_free_fun_t)plugin_mask_data_free, module,
            NULL)
        != 0)
    {
        CPE_ERROR(module->m_em, "plugin_mask_data_regist: create: register type mask fail!");
        return -1;
    }
    
    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_mask, plugin_mask_data_bin_load, module);

    ui_data_mgr_set_saver(
        module->m_data_mgr, ui_data_src_type_mask, plugin_mask_data_bin_save, plugin_mask_data_bin_rm, module);

    return 0;
}

void plugin_mask_data_unregist(plugin_mask_module_t module) {
    ui_data_mgr_unregister_type(module->m_data_mgr, ui_data_src_type_mask);
    ui_data_mgr_set_loader(module->m_data_mgr, ui_data_src_type_mask, NULL, NULL);
    ui_data_mgr_set_saver(module->m_data_mgr, ui_data_src_type_mask, NULL, NULL, NULL);
}
