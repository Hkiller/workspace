#ifndef DROW_UI_SPRITE_MANIP_I_H
#define DROW_UI_SPRITE_MANIP_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "ui/sprite_manip/ui_sprite_manip.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_manip {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    plugin_package_manip_t m_package_manip;
};

int ui_sprite_manip_res_collector_entity_regist(ui_sprite_manip_t module);
void  ui_sprite_manip_res_collector_entity_unregist(ui_sprite_manip_t module);

int ui_sprite_manip_res_collector_fsm_regist(ui_sprite_manip_t module);
void  ui_sprite_manip_res_collector_fsm_unregist(ui_sprite_manip_t module);

int ui_sprite_manip_res_collector_scene_regist(ui_sprite_manip_t module);
void  ui_sprite_manip_res_collector_scene_unregist(ui_sprite_manip_t module);

int ui_sprite_manip_res_collector_page_regist(ui_sprite_manip_t module);
void  ui_sprite_manip_res_collector_page_unregist(ui_sprite_manip_t module);

int ui_sprite_manip_res_collector_popup_regist(ui_sprite_manip_t module);
void  ui_sprite_manip_res_collector_popup_unregist(ui_sprite_manip_t module);
    
#ifdef __cplusplus
}
#endif

#endif 
