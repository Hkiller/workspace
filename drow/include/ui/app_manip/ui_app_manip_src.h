#ifndef UI_APP_MANIP_SRC_H
#define UI_APP_MANIP_SRC_H
#include "ui_app_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_app_manip_collect_src_from_scene(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t scene_cfg, error_monitor_t em);
int ui_app_manip_collect_src_from_entity(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t entity_cfg, error_monitor_t em);    
int ui_app_manip_collect_src_from_components(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t components_cfg, error_monitor_t em);
int ui_app_manip_collect_src_from_component(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t component_cfg, error_monitor_t em);
int ui_app_manip_collect_src_from_fsm_state(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t state_cfg, error_monitor_t em);
int ui_app_manip_collect_src_from_fsm(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t fsm_cfg, error_monitor_t em);
int ui_app_manip_collect_src_from_fsm_action(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t action_cfg, error_monitor_t em);

int ui_app_manip_collect_src_from_additions(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t addition_cfg, error_monitor_t em);
int ui_app_manip_collect_src_from_addition(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t addition_cfg, error_monitor_t em);

int ui_app_manip_collect_src_by_res(ui_data_src_group_t src_group, ui_cache_group_t cache_group, const char * res, error_monitor_t em);

int ui_app_manip_collect_src_from_page(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t page_cfg, error_monitor_t em);
int ui_app_manip_collect_src_from_popup(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t page_cfg, error_monitor_t em);    
int ui_app_manip_collect_src_from_control_attr(ui_data_src_group_t src_group, ui_cache_group_t cache_group, const char * name, const char * value, error_monitor_t em);
int ui_app_manip_collect_src_from_control_binding(ui_data_src_group_t src_group, ui_cache_group_t cache_group, const char * name, cfg_t control_attr_cfg, error_monitor_t em);    
    
#ifdef __cplusplus
}
#endif

#endif
