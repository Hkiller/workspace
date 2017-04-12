#ifndef UI_SPRITE_TYPES_H
#define UI_SPRITE_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/pal/pal_types.h"
#include "cpe/dr/dr_types.h"
#include "gd/app/app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_event * ui_sprite_event_t;
typedef struct ui_sprite_event_handler * ui_sprite_event_handler_t;
typedef struct ui_sprite_attr_monitor * ui_sprite_attr_monitor_t;

typedef struct ui_sprite_repository * ui_sprite_repository_t;
typedef struct ui_sprite_world * ui_sprite_world_t;
typedef struct ui_sprite_world_res * ui_sprite_world_res_t;
typedef struct ui_sprite_entity * ui_sprite_entity_t;
typedef struct ui_sprite_group * ui_sprite_group_t;
typedef struct ui_sprite_entity_it * ui_sprite_entity_it_t;
typedef struct ui_sprite_component * ui_sprite_component_t;
typedef struct ui_sprite_component_meta * ui_sprite_component_meta_t;

typedef enum ui_sprite_event_scope {
    ui_sprite_event_scope_self = 1,
    ui_sprite_event_scope_world
} ui_sprite_event_scope_t;

typedef void (*ui_sprite_event_process_fun_t)(void * ctx, ui_sprite_event_t evt);
typedef void (*ui_sprite_attr_monitor_fun_t)(void * ctx);

struct ui_sprite_event {
    uint32_t from_entity_id;
    LPDRMETA meta;
    void const * data;
    size_t size;
};

#ifdef __cplusplus
}
#endif

#endif
