#ifndef PLUGIN_BARRAGE_ENV_I_H
#define PLUGIN_BARRAGE_ENV_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/dr/dr_types.h"
#include "chipmunk/chipmunk_private.h"
#include "plugin/barrage/plugin_barrage_env.h"
#include "plugin/chipmunk/plugin_chipmunk_env.h"
#include "plugin_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PLUGIN_BARRAGE_OP_QUEUE_COUNT 64

struct plugin_barrage_env {
    plugin_barrage_module_t m_module;
    plugin_chipmunk_env_t m_chipmunk_env;
    int m_debug;
    uint32_t m_collision_type;
    uint32_t m_data_count;

    float m_fps;
    float m_frame_spane;
    float m_left_span;
    uint32_t m_cur_frame;

    struct plugin_barrage_group_list m_groups;

    uint32_t m_op_count;
    uint8_t m_near_op_pos;
    plugin_barrage_op_list_t m_near_ops[PLUGIN_BARRAGE_OP_QUEUE_COUNT];
    plugin_barrage_op_list_t m_far_ops;

    uint32_t m_bullet_count;
    struct plugin_barrage_bullet_list m_active_bullets;
    struct plugin_barrage_bullet_list m_collided_bullets;

    uint32_t m_emitter_count;

    uint32_t m_barrage_count;
    plugin_barrage_barrage_list_t m_idle_barrages;
    plugin_barrage_barrage_list_t m_active_barrages;
    
    plugin_barrage_barrage_list_t m_free_barrages;
    plugin_barrage_emitter_list_t m_free_emitters;
    struct plugin_barrage_bullet_list m_free_bullets;
    struct plugin_barrage_trigger_op_list m_free_trigger_ops;
};

#ifdef __cplusplus
}
#endif

#endif
