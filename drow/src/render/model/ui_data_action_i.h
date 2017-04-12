#ifndef UI_DATA_ACTION_INTERNAL_H
#define UI_DATA_ACTION_INTERNAL_H
#include "render/model/ui_data_action.h"
#include "ui_data_mgr_i.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_data_actor_list, ui_data_actor) ui_data_actor_list_t;
typedef TAILQ_HEAD(ui_data_actor_layer_list, ui_data_actor_layer) ui_data_actor_layer_list_t;
typedef TAILQ_HEAD(ui_data_actor_frame_list, ui_data_actor_frame) ui_data_actor_frame_list_t;

struct ui_data_action {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    uint32_t m_actor_count;
    ui_data_actor_list_t m_actors;
};

struct ui_data_actor {
    ui_data_action_t m_action;
    struct cpe_hash_entry m_hh_for_mgr;
    TAILQ_ENTRY(ui_data_actor) m_next_for_action;
    uint32_t m_layer_count;
    ui_data_actor_layer_list_t m_layers;
    UI_ACTOR m_data;
};

struct ui_data_actor_layer {
    ui_data_actor_t m_actor;
    TAILQ_ENTRY(ui_data_actor_layer) m_next_for_actor;
    uint32_t m_frame_count;
    ui_data_actor_frame_list_t m_frames;
    UI_ACTOR_LAYER m_data;
};

struct ui_data_actor_frame {
    ui_data_actor_layer_t m_layer;
    TAILQ_ENTRY(ui_data_actor_frame) m_next_for_layer;
    UI_ACTOR_FRAME m_data;
};

int ui_data_action_update_using(ui_data_src_t user);
    
uint32_t ui_data_actor_hash(const ui_data_actor_t actor);
int ui_data_actor_eq(const ui_data_actor_t l, const ui_data_actor_t r);
    
#ifdef __cplusplus
}
#endif

#endif
