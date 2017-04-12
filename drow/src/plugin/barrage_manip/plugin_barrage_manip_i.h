#ifndef PLUGIN_BARRAGE_MODULE_I_H
#define PLUGIN_BARRAGE_MODULE_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "plugin/barrage_manip/plugin_barrage_manip.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_manip {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_ed_mgr_t m_ed_mgr;
    plugin_barrage_module_t m_barrage_module;
    uint8_t m_debug;
};

int plugin_barrage_bullets_ed_src_load(ui_ed_src_t src);
ui_ed_obj_t plugin_barrage_emitter_emitter_ed_obj_create(ui_ed_obj_t parent);
ui_ed_obj_t plugin_barrage_emitter_emitter_trigger_ed_obj_create(ui_ed_obj_t parent);
ui_ed_obj_t plugin_barrage_bullet_trigger_ed_obj_create(ui_ed_obj_t parent);

int plugin_barrage_emitter_ed_src_load(ui_ed_src_t src);

int plugin_barrage_bullets_proj_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_barrage_bullets_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif 
