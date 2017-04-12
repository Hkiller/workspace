#ifndef DROW_UI_SPRITE_MANIP_H
#define DROW_UI_SPRITE_MANIP_H
#include "gd/app/app_types.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "plugin/package_manip/plugin_package_manip_types.h"
#include "ui/sprite/ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_manip * ui_sprite_manip_t;

ui_sprite_manip_t
ui_sprite_manip_create(
    gd_app_context_t app, plugin_package_manip_t package_manip,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void ui_sprite_manip_free(ui_sprite_manip_t manip);

ui_sprite_manip_t ui_sprite_manip_find(gd_app_context_t app, cpe_hash_string_t name);
ui_sprite_manip_t ui_sprite_manip_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_sprite_manip_app(ui_sprite_manip_t manip);
const char * ui_sprite_manip_name(ui_sprite_manip_t manip);

#ifdef __cplusplus
}
#endif

#endif
