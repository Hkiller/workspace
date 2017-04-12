#ifndef PLUGIN_MASK_DATA_I_H
#define PLUGIN_MASK_DATA_I_H
#include "plugin/mask/plugin_mask_data.h"
#include "plugin_mask_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_mask_data {
    plugin_mask_module_t m_module;
    ui_data_src_t m_src;
    plugin_mask_data_format_t m_format;
    uint32_t m_block_count;
    plugin_mask_data_block_list_t m_blocks;
};

int plugin_mask_data_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_mask_data_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_mask_data_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

int plugin_mask_data_regist(plugin_mask_module_t module);
void plugin_mask_data_unregist(plugin_mask_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
