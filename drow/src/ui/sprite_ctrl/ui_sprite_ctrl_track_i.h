#ifndef UI_SPRITE_CTRL_TRACK_I_H
#define UI_SPRITE_CTRL_TRACK_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_track.h"
#include "ui_sprite_ctrl_track_meta_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_ctrl_track_point * ui_sprite_ctrl_track_point_t;
typedef TAILQ_HEAD(ui_sprite_ctrl_track_list, ui_sprite_ctrl_track) ui_sprite_ctrl_track_list_t;
 
struct ui_sprite_ctrl_track_point {
    ui_vector_2 m_point;
    ui_sprite_ctrl_track_point_meta_t m_point_meta;
    ui_sprite_render_anim_t m_anim;
};

struct ui_sprite_ctrl_track {
    ui_sprite_ctrl_track_mgr_t m_track_mgr;
    const char * m_name;
    ui_sprite_ctrl_track_meta_t m_meta;
    uint8_t m_is_show;

    /*运行跟踪数据 */
    ui_sprite_ctrl_track_point_meta_t m_next_point_meta;
    ui_vector_2 m_last_input_point;
    float m_last_interval;

    /*已经生成的点信息 */
    uint16_t m_point_capacity;
    uint16_t m_point_count;
    struct ui_sprite_ctrl_track_point * m_points;
    TAILQ_ENTRY(ui_sprite_ctrl_track) m_next;
};

int ui_sprite_ctrl_track_point_show(
    ui_sprite_ctrl_track_point_t point, ui_sprite_ctrl_track_t track, ui_sprite_render_env_t backend);

void ui_sprite_ctrl_track_point_hide(
    ui_sprite_ctrl_track_point_t point, ui_sprite_ctrl_track_t track);

#ifdef __cplusplus
}
#endif

#endif
