#ifndef DROW_PLUGIN_BARRAGE_MANIP_H
#define DROW_PLUGIN_BARRAGE_MANIP_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "render/model_ed/ui_ed_types.h"
#include "plugin/barrage/plugin_barrage_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct plugin_barrage_manip * plugin_barrage_manip_t;

plugin_barrage_manip_t
plugin_barrage_manip_create(
    gd_app_context_t app, ui_ed_mgr_t ed_mgr, plugin_barrage_module_t barrage_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void plugin_barrage_manip_free(plugin_barrage_manip_t manip);

plugin_barrage_manip_t plugin_barrage_manip_find(gd_app_context_t app, cpe_hash_string_t name);
plugin_barrage_manip_t plugin_barrage_manip_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t plugin_barrage_manip_app(plugin_barrage_manip_t manip);
const char * plugin_barrage_manip_name(plugin_barrage_manip_t manip);

void plugin_barrage_manip_install_proj_loader(plugin_barrage_manip_t manip);
void plugin_barrage_manip_install_proj_saver(plugin_barrage_manip_t manip);

#ifdef __cplusplus
}
#endif

#endif