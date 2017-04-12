#ifndef DROW_PLUGIN_SPINE_MANIP_IMPORT_H
#define DROW_PLUGIN_SPINE_MANIP_IMPORT_H
#include "plugin_spine_manip.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_spine_manip_import(
    ui_ed_mgr_t ed_mgr, const char * module_path,
    const char * atlas_file, const char * pic, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
