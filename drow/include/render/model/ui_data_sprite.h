#ifndef UI_MODEL_DATA_SPRITE_H
#define UI_MODEL_DATA_SPRITE_H
#include "protocol/render/model/ui_sprite.h"
#include "render/utils/ui_utils_types.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_frame_it {
    ui_data_frame_t (*next)(struct ui_data_frame_it * it);
    char m_data[64];
};

struct ui_data_frame_img_it {
    ui_data_frame_img_t (*next)(struct ui_data_frame_img_it * it);
    char m_data[64];
};

struct ui_data_frame_collision_it {
    ui_data_frame_collision_t (*next)(struct ui_data_frame_collision_it * it);
    char m_data[64];
};

/*sprite */
ui_data_sprite_t ui_data_sprite_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_sprite_free(ui_data_sprite_t sprite);
void ui_data_sprite_frames(ui_data_frame_it_t it, ui_data_sprite_t sprite);
uint32_t ui_data_sprite_frame_count(ui_data_sprite_t sprite);
    
/*frame*/
ui_data_frame_t ui_data_frame_create(ui_data_sprite_t sprite);
ui_data_frame_t ui_data_frame_find_by_id(ui_data_sprite_t sprite, uint32_t id);
ui_data_frame_t ui_data_frame_find_by_name(ui_data_sprite_t sprite, const char * name);

void ui_data_frame_free(ui_data_frame_t frame);
ui_data_sprite_t ui_data_frame_sprite(ui_data_frame_t frame);
ui_data_src_t ui_data_frame_src(ui_data_frame_t frame);    
uint32_t ui_data_frame_img_count(ui_data_frame_t frame);
uint32_t ui_data_frame_collision_count(ui_data_frame_t frame);
void ui_data_frame_imgs(ui_data_frame_img_it_t it, ui_data_frame_t frame);
void ui_data_frame_collisions(ui_data_frame_collision_it_t it, ui_data_frame_t frame);
const char * ui_data_frame_name(ui_data_frame_t frame);
UI_FRAME * ui_data_frame_data(ui_data_frame_t frame);
LPDRMETA ui_data_frame_meta(ui_data_mgr_t mgr);
int ui_data_frame_set_id(ui_data_frame_t frame, uint32_t id);
int ui_data_frame_bounding_rect(ui_data_frame_t frame, ui_rect_t rect);

#define ui_data_frame_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*img_ref*/
ui_data_frame_img_t ui_data_frame_img_create(ui_data_frame_t frame);
void ui_data_frame_img_free(ui_data_frame_img_t img_ref);
ui_data_frame_img_t ui_data_frame_img_get_at(ui_data_frame_t frame, uint32_t index);
ui_data_frame_t ui_data_frame_img_frame(ui_data_frame_img_t img_ref);
ui_data_src_t ui_data_frame_img_src(ui_data_frame_img_t img_ref);
UI_IMG_REF * ui_data_frame_img_data(ui_data_frame_img_t img_ref);
LPDRMETA ui_data_frame_img_meta(ui_data_mgr_t mgr);
ui_data_img_block_t ui_data_frame_img_using_img_block(ui_data_frame_img_t img_ref);

#define ui_data_frame_img_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*collision*/
ui_data_frame_collision_t ui_data_frame_collision_create(ui_data_frame_t frame);
void ui_data_frame_collision_free(ui_data_frame_collision_t collision);
ui_data_frame_collision_t ui_data_frame_collision_get_at(ui_data_frame_t frame, uint32_t index);
ui_data_frame_t ui_data_frame_collision_frame(ui_data_frame_collision_t collision);
ui_data_src_t ui_data_frame_collision_src(ui_data_frame_collision_t collision);
UI_COLLISION * ui_data_frame_collision_data(ui_data_frame_collision_t collision);
LPDRMETA ui_data_frame_collision_meta(ui_data_mgr_t mgr);

#define ui_data_frame_collision_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
