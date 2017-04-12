#ifndef UI_DATA_LAYOUT_INTERNAL_H
#define UI_DATA_LAYOUT_INTERNAL_H
#include "cpe/utils/hash.h"
#include "render/model/ui_data_layout.h"
#include "ui_data_mgr_i.h"
#include "ui_data_src_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_data_control_list, ui_data_control) ui_data_control_list_t;
typedef TAILQ_HEAD(ui_data_control_anim_list, ui_data_control_anim) ui_data_control_anim_list_t;    
typedef TAILQ_HEAD(ui_data_control_anim_frame_list, ui_data_control_anim_frame) ui_data_control_anim_frame_list_t;
typedef TAILQ_HEAD(ui_data_control_addition_list, ui_data_control_addition) ui_data_control_addition_list_t;
    
struct ui_data_layout {
    ui_data_mgr_t m_mgr;
    ui_data_src_t m_src;
    uint16_t m_control_count;
    ui_data_control_t m_root;
    ui_string_table_t m_string_table;
};

/*control*/    
struct ui_data_control {
    ui_data_layout_t m_layout;
    ui_data_control_t m_parent;
    uint16_t m_child_count;
    ui_data_control_list_t m_childs;
    TAILQ_ENTRY(ui_data_control) m_next_for_parent;
    struct cpe_hash_entry m_hh_for_mgr;
    uint16_t m_anim_count;
    ui_data_control_anim_list_t m_anims;
    uint16_t m_addition_count;
    ui_data_control_addition_list_t m_additions;
    UI_CONTROL m_data;
};

uint32_t ui_data_control_hash(const ui_data_control_t control);
int ui_data_control_eq(const ui_data_control_t l, const ui_data_control_t r);

struct ui_data_control_anim {
    ui_data_control_t m_control;
    uint16_t m_anim_frame_count;
    ui_data_control_anim_frame_list_t m_frames;
    TAILQ_ENTRY(ui_data_control_anim) m_next_for_control;
    UI_CONTROL_ANIM m_data;
};

struct ui_data_control_anim_frame {
    ui_data_control_anim_t m_anim;
    TAILQ_ENTRY(ui_data_control_anim_frame) m_next_for_anim;
    UI_CONTROL_ANIM_FRAME m_data;
};

struct ui_data_control_addition {
    ui_data_control_t m_control;
    TAILQ_ENTRY(ui_data_control_addition) m_next_for_control;
    UI_CONTROL_ADDITION m_data;
};
    
int ui_data_layout_update_using(ui_data_src_t user);
    
#ifdef __cplusplus
}
#endif

#endif
