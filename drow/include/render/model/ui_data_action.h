#ifndef UI_MODEL_DATA_ACTION_H
#define UI_MODEL_DATA_ACTION_H
#include "protocol/render/model/ui_action.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_actor_it {
    ui_data_actor_t (*next)(struct ui_data_actor_it * it);
    char m_data[64];
};

struct ui_data_actor_layer_it {
    ui_data_actor_layer_t (*next)(struct ui_data_actor_layer_it * it);
    char m_data[64];
};

struct ui_data_actor_frame_it {
    ui_data_actor_frame_t (*next)(struct ui_data_actor_frame_it * it);
    char m_data[64];
};

/*action*/
ui_data_action_t ui_data_action_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_action_free(ui_data_action_t action);
uint32_t ui_data_action_actor_count(ui_data_action_t action);
void ui_data_action_actors(ui_data_actor_it_t it, ui_data_action_t action);

/*actor*/
ui_data_actor_t ui_data_actor_create(ui_data_action_t action);
void ui_data_actor_free(ui_data_actor_t actor);
ui_data_actor_t ui_data_actor_find_by_id(ui_data_action_t action, uint32_t id);
ui_data_actor_t ui_data_actor_find_by_name(ui_data_action_t action, const char * name);
int ui_data_actor_set_id(ui_data_actor_t actor, uint32_t id); 
UI_ACTOR * ui_data_actor_data(ui_data_actor_t actor);
uint32_t ui_data_actor_layer_count(ui_data_actor_t actor);
uint16_t ui_data_actor_frame_total(ui_data_actor_t actor);
int ui_data_actor_bounding_rect(ui_data_actor_t actor, ui_rect_t rect);
LPDRMETA ui_data_actor_meta(ui_data_mgr_t mgr);
void ui_data_actor_layers(ui_data_actor_layer_it_t it, ui_data_actor_t actor);

#define ui_data_actor_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*actor_layer*/
ui_data_actor_layer_t ui_data_actor_layer_create(ui_data_actor_t actor);
void ui_data_actor_layer_free(ui_data_actor_layer_t actor_layer);
ui_data_actor_layer_t ui_data_actor_layer_find_by_name(ui_data_actor_t actor, const char * name);
ui_data_actor_layer_t ui_data_actor_layer_get_at(ui_data_actor_t actor, uint32_t idx);
void ui_data_actor_layer_frames(ui_data_actor_frame_it_t it, ui_data_actor_layer_t actor_layer);
UI_ACTOR_LAYER * ui_data_actor_layer_data(ui_data_actor_layer_t actor_layer);
LPDRMETA ui_data_actor_layer_meta(ui_data_mgr_t data_mgr);
uint32_t ui_data_actor_layer_frame_count(ui_data_actor_layer_t actor_layer);
int ui_data_actor_layer_copy(ui_data_actor_layer_t to_actor_layer, ui_data_actor_layer_t from_actor_layer);
    
#define ui_data_actor_layer_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*actor_frame*/
ui_data_actor_frame_t ui_data_actor_frame_create(ui_data_actor_layer_t actor_layer);
void ui_data_actor_frame_free(ui_data_actor_frame_t actor_frame);
ui_data_actor_frame_t ui_data_actor_frame_find_by_frame(ui_data_actor_layer_t actor_layer, uint32_t frame);
ui_data_actor_frame_t ui_data_actor_frame_get_at(ui_data_actor_layer_t actor_layer, uint32_t idx);
UI_ACTOR_FRAME * ui_data_actor_frame_data(ui_data_actor_frame_t actor_frame);
LPDRMETA ui_data_actor_frame_meta(ui_data_mgr_t data_mgr);
int ui_data_actor_frame_copy(ui_data_actor_frame_t to_actor_frame, ui_data_actor_frame_t from_actor_frame);

#define ui_data_actor_frame_it_next(it) ((it)->next ? (it)->next(it) : NULL)

#ifdef __cplusplus
}
#endif

#endif
