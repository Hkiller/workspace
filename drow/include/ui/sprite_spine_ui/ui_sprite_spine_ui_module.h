#ifndef UI_SPRITE_SPINE_UI_MODULE_H
#define UI_SPRITE_SPINE_UI_MODULE_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "gd/app/app_types.h"
#include "plugin/ui/plugin_ui_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_types.h"
#include "ui_sprite_spine_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_spine_ui_module_t
ui_sprite_spine_ui_module_create(
    gd_app_context_t app, ui_sprite_repository_t repo, ui_sprite_fsm_module_t fsm_module, ui_sprite_cfg_loader_t loader,
    ui_sprite_render_module_t sprite_render, ui_sprite_ui_module_t ui_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em);

void ui_sprite_spine_ui_module_free(ui_sprite_spine_ui_module_t module);

ui_sprite_spine_ui_module_t ui_sprite_spine_ui_module_find(gd_app_context_t app, cpe_hash_string_t name);
ui_sprite_spine_ui_module_t ui_sprite_spine_ui_module_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_sprite_spine_ui_module_app(ui_sprite_spine_ui_module_t repo);
const char * ui_sprite_spine_ui_module_name(ui_sprite_spine_ui_module_t repo);

#ifdef __cplusplus
}
#endif

#endif
