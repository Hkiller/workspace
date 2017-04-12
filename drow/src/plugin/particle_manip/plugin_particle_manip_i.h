#ifndef PLUGIN_PARTICLE_MANIP_I_H
#define PLUGIN_PARTICLE_MANIP_I_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "gd/app/app_context.h"
#include "plugin/particle_manip/plugin_particle_manip.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_manip {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    ui_ed_mgr_t m_ed_mgr;
    plugin_particle_module_t m_particle_module;
    uint8_t m_debug;
};

int plugin_particle_ed_src_load(ui_ed_src_t src);

ui_ed_obj_t ui_ed_obj_create_particle_emitter(ui_ed_obj_t parent);
ui_ed_obj_t ui_ed_obj_create_particle_mod(ui_ed_obj_t parent);

#ifdef __cplusplus
}
#endif

#endif 
