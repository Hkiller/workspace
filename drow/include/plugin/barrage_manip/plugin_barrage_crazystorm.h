#ifndef DROW_PLUGIN_BARRAGE_MANIP_CRAZYSTORM_H
#define DROW_PLUGIN_BARRAGE_MANIP_CRAZYSTORM_H
#include "render/model_ed/ui_ed_types.h"
#include "plugin/barrage/plugin_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int plugin_barrage_export_crazystorm_bullets(
    ui_data_mgr_t data_mgr, const char * bullet_path,
    const char * to_crazystorm_file, error_monitor_t em);

int plugin_barrage_import_crazystorm_emitter(
    ui_ed_mgr_t ed_mgr, const char * input_path,
    const char * to_emitter, const char * use_buletts, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
