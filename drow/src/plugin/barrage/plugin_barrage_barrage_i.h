#ifndef PLUGIN_BARRAGE_BARRAGE_I_H
#define PLUGIN_BARRAGE_BARRAGE_I_H
#include "render/utils/ui_vector_2.h"
#include "plugin/barrage/plugin_barrage_barrage.h"
#include "plugin_barrage_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_barrage {
    plugin_barrage_group_t m_group;
    TAILQ_ENTRY(plugin_barrage_barrage) m_next_for_group;
    TAILQ_ENTRY(plugin_barrage_barrage) m_next_for_env;
    float m_speed_adj;
    float m_emitter_adj;
    plugin_barrage_data_t m_carray_data;
    plugin_barrage_target_fun_t m_target_fun;
    void * m_target_fun_ctx;
    uint8_t m_is_enable;
    uint16_t m_frame_loop;

    uint32_t m_collision_category;
    uint32_t m_collision_mask;
    uint32_t m_collision_group;
    uint32_t m_show_dead_anim_mask;
    uint32_t m_loop_count;

    ui_vector_2 m_pos;
    float m_angle;
    plugin_barrage_emitter_list_t m_emitters;

    BARRAGE_BARRAGE m_data;

    uint8_t m_cache_target_valid;
    ui_vector_2 m_cache_target;
};

void plugin_barrage_barrage_real_free(plugin_barrage_barrage_t barrage);
void plugin_barrage_barrage_reset(plugin_barrage_barrage_t barrage);

#ifdef __cplusplus
}
#endif

#endif
