#ifndef UI_SPRITE_BASIC_TYPES_H
#define UI_SPRITE_BASIC_TYPES_H
#include "ui/sprite/ui_sprite_types.h"
#include "ui/sprite_fsm/ui_sprite_fsm_types.h"
#include "ui/sprite_cfg/ui_sprite_cfg_types.h"
#include "protocol/ui/sprite_basic/ui_sprite_basic_data.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_basic_module * ui_sprite_basic_module_t;

/*actions*/
typedef struct ui_sprite_basic_send_event * ui_sprite_basic_send_event_t;
typedef struct ui_sprite_basic_gen_entities * ui_sprite_basic_gen_entities_t;
typedef struct ui_sprite_basic_noop * ui_sprite_basic_noop_t;
typedef struct ui_sprite_basic_debug * ui_sprite_basic_debug_t;
typedef struct ui_sprite_basic_join_group * ui_sprite_basic_join_group_t;
typedef struct ui_sprite_basic_set_attrs * ui_sprite_basic_set_attrs_t;
typedef struct ui_sprite_basic_guard_attrs * ui_sprite_basic_guard_attrs_t;

typedef struct ui_sprite_basic_wait_event * ui_sprite_basic_wait_event_t;
typedef struct ui_sprite_basic_wait_condition * ui_sprite_basic_wait_condition_t;
    
typedef struct ui_sprite_basic_wait_entity_destory * ui_sprite_basic_wait_entity_destory_t;
typedef struct ui_sprite_basic_wait_entity_in_state * ui_sprite_basic_wait_entity_in_state_t;    
typedef struct ui_sprite_basic_wait_entity_not_in_state * ui_sprite_basic_wait_entity_not_in_state_t;
    
typedef struct ui_sprite_basic_destory_self * ui_sprite_basic_destory_self_t;

/*utils*/
typedef struct ui_sprite_basic_value_generator * ui_sprite_basic_value_generator_t;

#ifdef __cplusplus
}
#endif

#endif


