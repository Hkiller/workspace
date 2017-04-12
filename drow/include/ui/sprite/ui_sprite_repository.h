#ifndef UI_SPRITE_REPOSITORY_H
#define UI_SPRITE_REPOSITORY_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "cpe/utils/hash_string.h"
#include "cpe/xcalc/xcalc_types.h"
#include "gd/app/app_types.h"
#include "ui_sprite_types.h"

#ifdef __cplusplus
extern "C" {
#endif

ui_sprite_repository_t
ui_sprite_repository_create(gd_app_context_t app, mem_allocrator_t alloc, const char * name, error_monitor_t em);
void ui_sprite_repository_free(ui_sprite_repository_t repo);

ui_sprite_repository_t ui_sprite_repository_find(gd_app_context_t app, cpe_hash_string_t name);
ui_sprite_repository_t ui_sprite_repository_find_nc(gd_app_context_t app, const char * name);

gd_app_context_t ui_sprite_repository_app(ui_sprite_repository_t repo);
const char * ui_sprite_repository_name(ui_sprite_repository_t repo);

xcomputer_t ui_sprite_repository_computer(ui_sprite_repository_t repo);

int ui_sprite_repository_register_event(ui_sprite_repository_t repo, LPDRMETA meta);
void ui_sprite_repository_unregister_event(ui_sprite_repository_t repo, const char * name);
LPDRMETA ui_sprite_repository_find_event(ui_sprite_repository_t repo, const char * name);
    
int ui_sprite_repository_register_events_by_prefix(ui_sprite_repository_t repo, LPDRMETALIB metalib, const char * prefix);
void ui_sprite_repository_unregister_events_by_prefix(ui_sprite_repository_t repo, LPDRMETALIB metalib, const char * prefix);

#ifdef __cplusplus
}
#endif

#endif
