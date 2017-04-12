#ifndef UI_MODEL_DATA_LAYOUT_H
#define UI_MODEL_DATA_LAYOUT_H
#include "protocol/render/model/ui_layout.h"
#include "ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_control_it {
    ui_data_control_t (*next)(struct ui_data_control_it * it);
    char m_data[64];
};

struct ui_data_control_anim_it {
    ui_data_control_anim_t (*next)(struct ui_data_control_anim_it * it);
    char m_data[64];
};

struct ui_data_control_anim_frame_it {
    ui_data_control_anim_frame_t (*next)(struct ui_data_control_anim_frame_it * it);
    char m_data[64];
};

struct ui_data_control_addition_it {
    ui_data_control_addition_t (*next)(struct ui_data_control_addition_it * it);
    char m_data[64];
};
    
ui_data_layout_t ui_data_layout_create(ui_data_mgr_t mgr, ui_data_src_t src);
void ui_data_layout_free(ui_data_layout_t layout);

ui_data_src_t ui_data_layout_src(ui_data_layout_t layout);
ui_data_mgr_t ui_data_layout_mgr(ui_data_layout_t layout);

ui_data_control_t ui_data_layout_root(ui_data_layout_t layout);
uint16_t ui_data_layout_control_count(ui_data_layout_t layout);

ui_string_table_t ui_data_layout_strings(ui_data_layout_t layout);
    
/*control operations*/
ui_data_control_t ui_data_control_create(ui_data_layout_t layout, ui_data_control_t parent);
void ui_data_control_free(ui_data_control_t control);

ui_data_layout_t ui_data_control_layout(ui_data_control_t control);
uint8_t ui_data_control_type(ui_data_control_t control);    
LPDRMETA ui_data_control_meta(ui_data_mgr_t mgr);
UI_CONTROL * ui_data_control_data(ui_data_control_t control);
const char * ui_data_control_msg(ui_data_control_t control, uint32_t msg_id);

ui_data_control_t ui_data_control_parent(ui_data_control_t control);
uint16_t ui_data_control_child_count(ui_data_control_t control);
void ui_data_control_childs(ui_data_control_it_t it, ui_data_control_t control);
ui_data_control_t ui_data_control_child_find_by_name(ui_data_control_t control, const char * name);

int ui_data_control_set_id(ui_data_control_t control, uint32_t id);
uint32_t ui_data_control_id(ui_data_control_t control);
const char * ui_data_control_name(ui_data_control_t control);
    
ui_data_control_t ui_data_control_find_by_id(ui_data_layout_t layout, uint32_t id);

#define ui_data_control_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*control anim*/
ui_data_control_anim_t ui_data_control_anim_create(ui_data_control_t control);
void ui_data_control_anim_free(ui_data_control_anim_t anim);

ui_data_control_anim_t ui_data_control_anim_find(ui_data_control_t control, uint8_t anim_type); 
LPDRMETA ui_data_control_anim_meta(ui_data_mgr_t mgr);

UI_CONTROL_ANIM * ui_data_control_anim_data(ui_data_control_anim_t anim);
uint16_t ui_data_control_anim_count(ui_data_control_t control);
void ui_data_control_anims(ui_data_control_anim_it_t it, ui_data_control_t control);

#define ui_data_control_anim_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*control anim*/
ui_data_control_anim_frame_t ui_data_control_anim_frame_create(ui_data_control_anim_t anim);
void ui_data_control_anim_frame_free(ui_data_control_anim_frame_t anim_frame);

LPDRMETA ui_data_control_anim_frame_meta(ui_data_mgr_t mgr);
UI_CONTROL_ANIM_FRAME * ui_data_control_anim_frame_data(ui_data_control_anim_frame_t anim_frame);

uint16_t ui_data_control_anim_frame_count(ui_data_control_anim_t anim);
void ui_data_control_anim_frames(ui_data_control_anim_frame_it_t it, ui_data_control_anim_t anim);
ui_data_control_anim_frame_t ui_data_control_anim_frame_next(ui_data_control_anim_frame_t anim_frame);
    
#define ui_data_control_anim_frame_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*control addition*/
ui_data_control_addition_t ui_data_control_addition_create(ui_data_control_t control);
void ui_data_control_addition_free(ui_data_control_addition_t addition);

LPDRMETA ui_data_control_addition_meta(ui_data_mgr_t mgr);

UI_CONTROL_ADDITION * ui_data_control_addition_data(ui_data_control_addition_t addition);
uint16_t ui_data_control_addition_count(ui_data_control_t control);
void ui_data_control_additions(ui_data_control_addition_it_t it, ui_data_control_t control);

#define ui_data_control_addition_it_next(it) ((it)->next ? (it)->next(it) : NULL)

const char * ui_data_control_type_name(uint8_t control_type);

/**/
int ui_data_control_ref_to_object_ref(UI_OBJECT_URL * obj_url, UI_CONTROL_OBJECT_URL const * ref);

#ifdef __cplusplus
}
#endif

#endif
