#include <assert.h>
#include "cpe/utils/stream_file.h"
#include "plugin/barrage/plugin_barrage_module.h"
#include "render/model/ui_data_mgr.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin_barrage_manip_i.h"

int plugin_barrage_barrage_proj_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_barrage_barrage_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_barrage_barrage_proj_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

void plugin_barrage_manip_install_proj_loader(plugin_barrage_manip_t manip) {
    ui_data_mgr_set_loader(
        ui_ed_mgr_data_mgr(manip->m_ed_mgr),
        ui_data_src_type_barrage,
        plugin_barrage_barrage_proj_load,
        manip);
}

void plugin_barrage_manip_install_proj_saver(plugin_barrage_manip_t manip) {
    ui_data_mgr_set_saver(
        ui_ed_mgr_data_mgr(manip->m_ed_mgr),
        ui_data_src_type_barrage,
        plugin_barrage_barrage_proj_save,
        plugin_barrage_barrage_proj_rm,
        manip);
}
