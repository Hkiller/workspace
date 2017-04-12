#ifndef UI_SPRITE_BARRAGE_ENABLE_EMITTER_SEQ_I_H
#define UI_SPRITE_BARRAGE_ENABLE_EMITTER_SEQ_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite_barrage/ui_sprite_barrage_enable_seq.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_barrage_enable_seq_node * ui_sprite_barrage_enable_seq_node_t;
typedef TAILQ_HEAD(ui_sprite_barrage_enable_seq_node_list, ui_sprite_barrage_enable_seq_node) ui_sprite_barrage_enable_seq_node_list_t;

struct ui_sprite_barrage_enable_seq {
    ui_sprite_barrage_module_t m_module;
    char * m_cfg_collision_event;
    char * m_cfg_loop_count;
    ui_sprite_barrage_enable_seq_node_list_t m_cfg_nodes;
    
    ui_sprite_event_t m_evt;
    uint16_t m_loop_count;
    uint16_t m_runing_loop_count;
    
    ui_sprite_barrage_enable_seq_node_t m_curent_node;
    char * m_curent_group_name;
    uint8_t m_curent_is_enable;
};

int ui_sprite_barrage_enable_seq_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_enable_seq_unregist(ui_sprite_barrage_module_t module);

struct ui_sprite_barrage_enable_seq_node {
    TAILQ_ENTRY(ui_sprite_barrage_enable_seq_node) m_next;
    char * m_cfg_loop_count;
    char * m_cfg_group_name;
};

ui_sprite_barrage_enable_seq_node_t ui_sprite_barrage_enable_seq_node_create(ui_sprite_barrage_enable_seq_t seq);
void ui_sprite_barrage_enable_seq_node_free(ui_sprite_barrage_enable_seq_t seq, ui_sprite_barrage_enable_seq_node_t node);
    
#ifdef __cplusplus
}
#endif

#endif
