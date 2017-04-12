#ifndef UI_CACHE_TEXTURE_I_H
#define UI_CACHE_TEXTURE_I_H
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_texture.h"
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_res {
    ui_cache_manager_t m_mgr;
    ui_cache_res_type_t m_res_type;
    uint16_t m_op_id;
    char m_path[256];
    TAILQ_ENTRY(ui_cache_res) m_next_for_mgr;
    struct cpe_hash_entry m_hh_for_mgr;
    ui_cache_res_ref_list_t m_in_groups;
    ui_cache_res_load_state_t m_load_state;
    ui_cache_res_load_result_t m_load_result;
    uint8_t m_deleting;
    TAILQ_ENTRY(ui_cache_res) m_next_for_deleting;
    uint32_t m_ref_count;

    union {
        struct {
            ui_cache_pixel_format_t m_format;
            uint32_t m_width;
            uint32_t m_height;
            uint8_t m_scale;
            uint8_t m_keep_data_buf;
            uint8_t m_need_part_update;
            uint8_t m_is_dirty;
            ui_cache_pixel_buf_t m_data_buff;
        } m_texture;
        struct {
            ui_cache_sound_format_t m_format;
            ui_cache_sound_buf_t m_data_buff;
        } m_sound;
        struct {
            uint32_t m_data_size;
            void * m_data;
        } m_font;
    };
};

void ui_cache_res_free_all(ui_cache_manager_t mgr);
void ui_cache_res_do_unload(ui_cache_res_t res, uint8_t is_external_unload);

uint32_t ui_cache_res_path_hash(const ui_cache_res_t res);
int ui_cache_res_path_eq(const ui_cache_res_t l, const ui_cache_res_t r);

int ui_cache_texture_do_load(ui_cache_manager_t mgr, ui_cache_res_t res, const char * root);
int ui_cache_texture_load_from_buf(ui_cache_res_t texture, ui_cache_pixel_buf_t buf);

int ui_cache_sound_do_load(ui_cache_manager_t mgr, ui_cache_res_t res, const char * root);

int ui_cache_font_do_load(ui_cache_manager_t mgr, ui_cache_res_t res, const char * root);

#ifdef __cplusplus
}
#endif

#endif
