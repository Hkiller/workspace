#ifndef UI_RUNTIME_RENDER_PROGRAM_STATE_BINDING_H
#define UI_RUNTIME_RENDER_PROGRAM_STATE_BINDING_H
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"
#include "render/utils/ui_vector_4.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_transform.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_runtime_render_program_state_unif_it {
    ui_runtime_render_program_state_unif_t (*next)(struct ui_runtime_render_program_state_unif_it * it);
    char m_data[64];
};

struct ui_runtime_render_program_state_unif_data {
    ui_runtime_render_program_unif_type_t m_type;
    union {
        float m_f;
        int32_t m_i;
        ui_vector_2 m_v2;
        ui_vector_3 m_v3;
        ui_vector_4 m_v4;
        ui_transform m_m16;
        struct {
            ui_cache_res_t m_res;
            ui_runtime_render_texture_filter_t m_min_filter;
            ui_runtime_render_texture_filter_t m_mag_filter;
            ui_runtime_render_texture_wrapping_t m_wrap_s;
            ui_runtime_render_texture_wrapping_t m_wrap_t;
            uint8_t m_texture_idx;
        } m_tex;
    } m_data;
};

ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_create(
    ui_runtime_render_program_state_t state,
    ui_runtime_render_program_unif_t unif,
    ui_runtime_render_program_state_unif_data_t unif_data);

void ui_runtime_render_program_state_unif_free(ui_runtime_render_program_state_unif_t unif);

ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_find_by_buildin(
    ui_runtime_render_program_state_t state, ui_runtime_render_program_unif_buildin_t buidin);

ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_find_by_name(
    ui_runtime_render_program_state_t state, const char * name);

ui_runtime_render_program_state_unif_t
ui_runtime_render_program_state_unif_find_by_unif(
    ui_runtime_render_program_state_t state, ui_runtime_render_program_unif_t unif);

    
ui_runtime_render_program_unif_t ui_runtime_render_program_state_unif_unif(ui_runtime_render_program_state_unif_t unif);
ui_runtime_render_program_state_unif_data_t ui_runtime_render_program_state_unif_data(ui_runtime_render_program_state_unif_t unif);

int ui_runtime_render_program_state_unif_data_cmp(ui_runtime_render_program_state_unif_data_t l, ui_runtime_render_program_state_unif_data_t r);

#define ui_runtime_render_program_state_unif_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
