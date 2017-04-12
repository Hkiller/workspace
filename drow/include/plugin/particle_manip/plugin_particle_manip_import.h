#ifndef DROW_PLUGIN_PARTICLE_MANIP_IMPORT_H
#define DROW_PLUGIN_PARTICLE_MANIP_IMPORT_H
#include "plugin_particle_manip.h"

#ifdef __cplusplus
extern "C" {
#endif
    
int plugin_particle_manip_import_cocos_particle(
    ui_ed_mgr_t ed_mgr, const char * particle_path,
    const char * plist, const char * pic, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
