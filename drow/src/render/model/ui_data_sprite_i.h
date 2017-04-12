#ifndef UI_DATA_SPRITE_INTERNAL_H
#define UI_DATA_SPRITE_INTERNAL_H
#include "render/model/ui_data_sprite.h"
#include "ui_data_mgr_i.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_data_frame_list, ui_data_frame) ui_data_frame_list_t;
typedef TAILQ_HEAD(ui_data_frame_img_list, ui_data_frame_img) ui_data_frame_img_list_t;
typedef TAILQ_HEAD(ui_data_frame_collision_list, ui_data_frame_collision) ui_data_frame_collision_list_t;
    
struct ui_data_sprite {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    uint32_t m_frame_count;
    ui_data_frame_list_t m_frames;
};

struct ui_data_frame {
    ui_data_sprite_t m_sprite;
    struct cpe_hash_entry m_hh_for_mgr;
    TAILQ_ENTRY(ui_data_frame) m_next_for_sprite;
    uint32_t m_img_count;
    uint32_t m_collision_count;    
    ui_data_frame_img_list_t m_img_refs;
    ui_data_frame_collision_list_t m_collisions;    
    UI_FRAME m_data;
};

uint32_t ui_data_frame_hash(const ui_data_frame_t src);
int ui_data_frame_eq(const ui_data_frame_t l, const ui_data_frame_t r);

struct ui_data_frame_img {
    ui_data_frame_t m_frame;
    TAILQ_ENTRY(ui_data_frame_img) m_next_for_frame;
    UI_IMG_REF m_data;
};

struct ui_data_frame_collision {
    ui_data_frame_t m_frame;
    TAILQ_ENTRY(ui_data_frame_collision) m_next_for_frame;
    UI_COLLISION m_data;
};

int ui_data_sprite_update_using(ui_data_src_t user);

#ifdef __cplusplus
}
#endif

#endif
