#ifndef PLUGIN_MASK_MANIP_I_H
#define PLUGIN_MASK_MANIP_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "plugin/mask/plugin_mask_data.h"
#include "plugin/mask_manip/plugin_mask_manip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_mask_block_builder * plugin_mask_block_builder_t;

struct plugin_mask_manip {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    plugin_package_manip_t m_package_manip;
    plugin_mask_module_t m_mask_module;
    uint8_t m_debug;
};

int plugin_mask_manip_src_convertor_module_regist(plugin_mask_manip_t module);
void plugin_mask_manip_src_convertor_module_unregist(plugin_mask_manip_t module);
    
int plugin_mask_manip_src_convertor_sprite_regist(plugin_mask_manip_t module);
void plugin_mask_manip_src_convertor_sprite_unregist(plugin_mask_manip_t module);

int plugin_mask_manip_src_convertor_tiledmap_regist(plugin_mask_manip_t module);
void plugin_mask_manip_src_convertor_tiledmap_unregist(plugin_mask_manip_t module);
    
#ifdef __cplusplus
}
#endif

#endif 
