#ifndef UI_SPRITE_SPINE_CHIPMUNK_TYPES_H
#define UI_SPRITE_SPINE_CHIPMUNK_TYPES_H
#include "plugin/spine/plugin_spine_types.h"
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_types.h"
#include "ui/sprite_render/ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_spine_chipmunk_module * ui_sprite_spine_chipmunk_module_t;
typedef struct ui_sprite_spine_chipmunk_with_collision * ui_sprite_spine_chipmunk_with_collision_t;
typedef struct ui_sprite_spine_chipmunk_with_tri * ui_sprite_spine_chipmunk_with_tri_t;

typedef struct ui_sprite_spine_chipmunk_tri_have_entity * ui_sprite_spine_chipmunk_tri_have_entity_t;
    
#ifdef __cplusplus
}
#endif

#endif
