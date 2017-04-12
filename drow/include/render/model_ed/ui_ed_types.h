#ifndef UI_MODEL_ED_TYPES_H
#define UI_MODEL_ED_TYPES_H
#include "cpe/utils/memory.h"
#include "cpe/utils/error.h"
#include "../model/ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ui_ed_obj_type {
    ui_ed_obj_type_src = 1
    , ui_ed_obj_type_img_block = 2
    , ui_ed_obj_type_frame = 3
    , ui_ed_obj_type_frame_img = 4
    , ui_ed_obj_type_frame_collision = 5
    , ui_ed_obj_type_actor = 6
    , ui_ed_obj_type_actor_layer = 7
    , ui_ed_obj_type_actor_frame = 8
    , ui_ed_obj_type_particle_emitter = 9
    , ui_ed_obj_type_particle_mod = 10
    , ui_ed_obj_type_barrage = 11
    , ui_ed_obj_type_barrage_emitter = 12
    , ui_ed_obj_type_barrage_emitter_trigger = 13
    , ui_ed_obj_type_barrage_bullet_trigger = 14
    , ui_ed_obj_type_moving_plan = 15
    , ui_ed_obj_type_moving_track = 16
    , ui_ed_obj_type_moving_point = 17
    , ui_ed_obj_type_moving_node = 18
    , ui_ed_obj_type_moving_segment = 19
    , ui_ed_obj_type_chipmunk_scene = 20
    , ui_ed_obj_type_chipmunk_body = 21
    , ui_ed_obj_type_chipmunk_fixture = 22
    , ui_ed_obj_type_chipmunk_polygon_node = 23
    , ui_ed_obj_type_chipmunk_constraint = 24
    , ui_ed_obj_type_tiledmap_scene = 25
    , ui_ed_obj_type_tiledmap_layer = 26
    , ui_ed_obj_type_tiledmap_tile = 27
    , ui_ed_obj_type_scrollmap_scene = 28
    , ui_ed_obj_type_scrollmap_layer = 29
    , ui_ed_obj_type_scrollmap_tile = 30
    , ui_ed_obj_type_scrollmap_block = 31
    , ui_ed_obj_type_scrollmap_script = 32
} ui_ed_obj_type_t;
#define UI_ED_OBJ_TYPE_MIN 1
#define UI_ED_OBJ_TYPE_MAX 33

typedef enum ui_ed_rel_type {
    ui_ed_rel_type_use_img = 1
    , ui_ed_rel_type_use_actor = 2
} ui_ed_rel_type_t;
#define UI_ED_REL_TYPE_MIN 1
#define UI_ED_REL_TYPE_MAX 3

typedef enum ui_ed_src_state {
    ui_ed_src_state_normal = 0
    , ui_ed_src_state_new = 1
    , ui_ed_src_state_removed = 2
    , ui_ed_src_state_changed = 3
} ui_ed_src_state_t;

typedef struct ui_ed_mgr * ui_ed_mgr_t;

typedef struct ui_ed_search * ui_ed_search_t;

typedef struct ui_ed_src * ui_ed_src_t;

typedef struct ui_ed_obj_meta * ui_ed_obj_meta_t;
typedef struct ui_ed_obj * ui_ed_obj_t;
typedef struct ui_ed_obj_it * ui_ed_obj_it_t;

typedef struct ui_ed_rel_meta * ui_ed_rel_meta_t;
typedef struct ui_ed_rel * ui_ed_rel_t;

typedef struct ui_ed_op * ui_ed_op_t;

typedef struct ui_ed_change * ui_ed_change_t;

typedef int (*ui_ed_src_load_fun_t)(ui_ed_src_t src);

typedef ui_ed_obj_t (*ui_ed_rel_load_fun_t)(ui_ed_obj_t obj);
typedef ui_ed_obj_t (*ui_ed_obj_create_fun_t)(ui_ed_obj_t parent);
typedef int (*ui_ed_obj_set_id_fun_t)(void * product, uint32_t id);
typedef void (*ui_ed_obj_delete_fun_t)(void * product);
typedef void (*ui_ed_obj_init_fun_t)(ui_ed_obj_t obj);

#ifdef __cplusplus
}
#endif

#endif
