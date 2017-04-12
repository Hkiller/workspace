#ifndef UI_MODEL_TYPES_H
#define UI_MODEL_TYPES_H
#include "cpe/utils/error.h"
#include "cpe/dr/dr_types.h"
#include "protocol/render/model/ui_common.h"
#include "render/utils/ui_utils_types.h"
#include "render/cache/ui_cache_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ui_data_src_type_all = 0 /*dummy for search*/
    , ui_data_src_type_dir = 1
    , ui_data_src_type_module = 2
    , ui_data_src_type_sprite = 3
    , ui_data_src_type_action = 4
    , ui_data_src_type_layout = 5
    , ui_data_src_type_particle = 6
    , ui_data_src_type_spine_skeleton = 7
    , ui_data_src_type_spine_state_def = 8
    , ui_data_src_type_removed /*bullets*/ = 9
    , ui_data_src_type_barrage = 10
    , ui_data_src_type_moving_plan = 11
    , ui_data_src_type_chipmunk_scene = 12
    , ui_data_src_type_tiledmap_scene = 13
    , ui_data_src_type_scrollmap_scene = 14
    , ui_data_src_type_swf = 15
    , ui_data_src_type_mask = 16
} ui_data_src_type_t;
#define UI_DATA_SRC_TYPE_MIN (1)
#define UI_DATA_SRC_TYPE_MAX (17)

typedef enum {
    ui_data_src_state_loaded = 1
    , ui_data_src_state_notload = 2
} ui_data_src_load_state_t;

typedef enum {
    ui_data_src_strings_none = 0
    , ui_data_src_strings_r = 1
    , ui_data_src_strings_w = 2
} ui_data_src_strings_state_t;
    
/*for mgr*/
typedef struct ui_data_meta * ui_data_meta_t;
typedef struct ui_data_mgr * ui_data_mgr_t;
typedef struct ui_data_language * ui_data_language_t;
typedef struct ui_data_language_it * ui_data_language_it_t;

typedef struct ui_data_evt_collector * ui_data_evt_collector_t;
    
/*for src*/
typedef struct ui_data_src * ui_data_src_t;
typedef struct ui_data_src_user * ui_data_src_user_t;
typedef struct ui_data_src_ref * ui_data_src_ref_t;
typedef struct ui_data_src_it * ui_data_src_it_t;
typedef struct ui_data_src_ref_it * ui_data_src_ref_it_t;

typedef void (*ui_data_product_free_fun_t)(void *);
typedef void * (*ui_data_product_create_fun_t)(ui_data_mgr_t mgr, ui_data_src_t src);
typedef int (*product_load_fun_t)(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
typedef int (*product_save_fun_t)(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
typedef int (*product_remove_fun_t)(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
typedef int (*product_using_src_update_using_fun_t)(ui_data_src_t src);
    
/*ui_data_src_group*/
typedef struct ui_data_src_group * ui_data_src_group_t;
typedef struct ui_data_src_group_item * ui_data_src_group_item_t;

/*ui_data_src_res*/    
typedef struct ui_data_src_res * ui_data_src_res_t;
typedef struct ui_data_src_res_it * ui_data_src_res_it_t;

/*ui_data_src_src*/    
typedef struct ui_data_src_src * ui_data_src_src_t;
typedef struct ui_data_src_src_it * ui_data_src_src_it_t;
    
/*for module*/
typedef struct ui_data_module * ui_data_module_t;
typedef struct ui_data_img_block * ui_data_img_block_t;
typedef struct ui_data_img_block_it * ui_data_img_block_it_t;

/*for sprite*/
typedef struct ui_data_sprite * ui_data_sprite_t;
typedef struct ui_data_frame * ui_data_frame_t;
typedef struct ui_data_frame_it * ui_data_frame_it_t;
typedef struct ui_data_frame_img * ui_data_frame_img_t;
typedef struct ui_data_frame_img_it * ui_data_frame_img_it_t;
typedef struct ui_data_frame_collision * ui_data_frame_collision_t;
typedef struct ui_data_frame_collision_it * ui_data_frame_collision_it_t;
    
/*for action*/
typedef struct ui_data_action * ui_data_action_t;
typedef struct ui_data_actor * ui_data_actor_t;
typedef struct ui_data_actor_it * ui_data_actor_it_t;
typedef struct ui_data_actor_layer * ui_data_actor_layer_t;
typedef struct ui_data_actor_layer_it * ui_data_actor_layer_it_t;
typedef struct ui_data_actor_frame * ui_data_actor_frame_t;
typedef struct ui_data_actor_frame_it * ui_data_actor_frame_it_t;

/*for layout*/
typedef struct ui_data_layout * ui_data_layout_t;
typedef struct ui_data_control * ui_data_control_t;
typedef struct ui_data_control_it * ui_data_control_it_t;

typedef struct ui_data_control_anim * ui_data_control_anim_t;
typedef struct ui_data_control_anim_it * ui_data_control_anim_it_t;

typedef struct ui_data_control_anim_frame * ui_data_control_anim_frame_t;
typedef struct ui_data_control_anim_frame_it * ui_data_control_anim_frame_it_t;

typedef struct ui_data_control_addition * ui_data_control_addition_t;
typedef struct ui_data_control_addition_it * ui_data_control_addition_it_t;

#ifdef __cplusplus
}
#endif

#endif
