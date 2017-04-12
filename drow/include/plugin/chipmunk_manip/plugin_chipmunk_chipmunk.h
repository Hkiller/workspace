#ifndef DROW_PLUGIN_CHIPMUNK_MANIP_CHIPMUNK_H
#define DROW_PLUGIN_CHIPMUNK_MANIP_CHIPMUNK_H
#include "render/model_ed/ui_ed_types.h"
#include "plugin/chipmunk/plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_chipmunk_import_from_chipmunk(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr,
    const char * chipmunk_plist_path, const char * to_chipmunk_path,
    const char * adj_picture, float ptm, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
