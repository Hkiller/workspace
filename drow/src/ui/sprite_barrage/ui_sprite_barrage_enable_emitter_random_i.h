#ifndef UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_I_H
#define UI_SPRITE_BARRAGE_ENABLE_EMITTER_RANDOM_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite_barrage/ui_sprite_barrage_enable_emitter_random.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_barrage_enable_emitter_random_node * ui_sprite_barrage_enable_emitter_random_node_t;
typedef TAILQ_HEAD(ui_sprite_barrage_enable_emitter_random_node_list, ui_sprite_barrage_enable_emitter_random_node) ui_sprite_barrage_enable_emitter_random_node_list_t;

struct ui_sprite_barrage_enable_emitter_random {
    ui_sprite_barrage_module_t m_module;
    uint32_t m_repeat_count;
    ui_sprite_barrage_enable_emitter_random_node_list_t m_nodes;
    ui_sprite_barrage_enable_emitter_random_node_t m_curent_node;
    uint8_t m_curent_is_enable;
    float m_runing_time;
    float m_runing_repeat_count;
};

int ui_sprite_barrage_enable_emitter_random_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_enable_emitter_random_unregist(ui_sprite_barrage_module_t module);

struct ui_sprite_barrage_enable_emitter_random_node {
    TAILQ_ENTRY(ui_sprite_barrage_enable_emitter_random_node) m_next;
    float m_delay;
    float m_duration;
    uint32_t m_weight;
    char m_group_name[64];
    char m_collision_event[128];
    uint32_t m_loop_count;
    uint8_t m_destory_bullets;
};

ui_sprite_barrage_enable_emitter_random_node_t ui_sprite_barrage_enable_emitter_random_node_create(ui_sprite_barrage_enable_emitter_random_t random);
void ui_sprite_barrage_enable_emitter_random_node_free(ui_sprite_barrage_enable_emitter_random_t random, ui_sprite_barrage_enable_emitter_random_node_t node);
    
#ifdef __cplusplus
}
#endif

#endif
