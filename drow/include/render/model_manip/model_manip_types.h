#ifndef UI_MODEL_MANIP_TYPES_H
#define UI_MODEL_MANIP_TYPES_H
#include "../model_ed/ui_ed_types.h"

typedef enum ui_manip_action_import_frame_position {
    ui_manip_action_import_frame_center = 1 
    , ui_manip_action_import_frame_center_left
    , ui_manip_action_import_frame_center_right
    , ui_manip_action_import_frame_bottom_center
    , ui_manip_action_import_frame_bottom_left
    , ui_manip_action_import_frame_bottom_right
    , ui_manip_action_import_frame_top_center
    , ui_manip_action_import_frame_top_left
    , ui_manip_action_import_frame_top_right
} ui_manip_action_import_frame_position_t;

typedef enum ui_manip_action_import_frame_order {
    ui_manip_action_import_frame_order_native = 1 
    , ui_manip_action_import_frame_order_postfix
} ui_manip_action_import_frame_order_t;

#endif
