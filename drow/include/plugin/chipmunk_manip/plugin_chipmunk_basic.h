#ifndef DROW_PLUGIN_CHIPMUNK_MANIP_BASIC_H
#define DROW_PLUGIN_CHIPMUNK_MANIP_BASIC_H
#include "render/model_ed/ui_ed_types.h"
#include "plugin/chipmunk/plugin_chipmunk_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_chipmunk_import_from_sprite(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr,
    const char * sprite_path, const char * to_chipmunk_path, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
