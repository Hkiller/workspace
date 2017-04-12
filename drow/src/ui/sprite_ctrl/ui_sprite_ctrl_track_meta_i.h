#ifndef UI_SPRITE_CTRL_TRACK_META_I_H
#define UI_SPRITE_CTRL_TRACK_META_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite_ctrl/ui_sprite_ctrl_track_meta.h"
#include "ui_sprite_ctrl_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_ctrl_track_meta_list, ui_sprite_ctrl_track_meta) ui_sprite_ctrl_track_meta_list_t;

typedef struct ui_sprite_ctrl_track_point_meta * ui_sprite_ctrl_track_point_meta_t;
typedef TAILQ_HEAD(ui_sprite_ctrl_track_point_meta_list, ui_sprite_ctrl_track_point_meta) ui_sprite_ctrl_track_point_meta_list_t;

struct ui_sprite_ctrl_track_point_meta {
    float m_interval;
    const char * m_res;
    TAILQ_ENTRY(ui_sprite_ctrl_track_point_meta) m_next;
};
 
struct ui_sprite_ctrl_track_meta {
    ui_sprite_ctrl_track_mgr_t m_track_mgr;
    const char * m_type_name;
    const char * m_anim_layer;
    ui_sprite_ctrl_track_point_meta_list_t m_point_metas;
    TAILQ_ENTRY(ui_sprite_ctrl_track_meta) m_next;
};

void ui_sprite_ctrl_track_meta_remove_point(ui_sprite_ctrl_track_meta_t track_meta, ui_sprite_ctrl_track_point_meta_t point_meta);

#ifdef __cplusplus
}
#endif

#endif
