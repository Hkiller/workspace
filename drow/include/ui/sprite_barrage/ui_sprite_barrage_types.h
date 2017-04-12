#ifndef UI_SPRITE_BARRAGE_TYPES_H
#define UI_SPRITE_BARRAGE_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_2d/ui_sprite_2d_types.h"
#include "plugin/barrage/plugin_barrage_types.h"
#include "ui/sprite_render/ui_sprite_render_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_barrage_module * ui_sprite_barrage_module_t;
typedef struct ui_sprite_barrage_env * ui_sprite_barrage_env_t;
typedef struct ui_sprite_barrage_obj * ui_sprite_barrage_obj_t;
typedef struct ui_sprite_barrage_barrage * ui_sprite_barrage_barrage_t;

typedef struct ui_sprite_barrage_pause_emitter * ui_sprite_barrage_pause_emitter_t;
typedef struct ui_sprite_barrage_mask * ui_sprite_barrage_mask_t;
    
typedef struct ui_sprite_barrage_enable_emitter * ui_sprite_barrage_enable_emitter_t;
typedef struct ui_sprite_barrage_enable_seq * ui_sprite_barrage_enable_seq_t;
typedef struct ui_sprite_barrage_enable_emitter_random * ui_sprite_barrage_enable_emitter_random_t;
typedef struct ui_sprite_barrage_clear_bullets * ui_sprite_barrage_clear_bullets_t;    
typedef struct ui_sprite_barrage_on_bullet_hit * ui_sprite_barrage_on_bullet_hit_t;

#ifdef __cplusplus
}
#endif

#endif
