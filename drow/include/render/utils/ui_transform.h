#ifndef UI_UTILS_TRANSFORM_H
#define UI_UTILS_TRANSFORM_H
#include "cpe/utils/utils_types.h"
#include "ui_utils_types.h"
#include "ui_matrix_4x4.h"
#include "ui_vector_3.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_transform {
    ui_vector_3 m_s;
    uint8_t m_r_p;
    uint8_t m_init_p;
    ui_matrix_4x4 m_m4;
};

int ui_transform_cmp(ui_transform_t l, ui_transform_t r);

void ui_transform_set_pos_3(ui_transform_t transform, ui_vector_3_t pos);
void ui_transform_get_pos_3(ui_transform_t transform, ui_vector_3_t pos);
void ui_transform_adj_by_pos_3(ui_transform_t transform, ui_vector_3_t pos);

void ui_transform_set_pos_2(ui_transform_t transform, ui_vector_2_t pos);
void ui_transform_get_pos_2(ui_transform_t transform, ui_vector_2_t pos);
void ui_transform_adj_by_pos_2(ui_transform_t transform, ui_vector_2_t pos);

void ui_transform_set_quation(ui_transform_t transform, ui_quaternion_t q);
    
void ui_transform_set_scale(ui_transform_t transform, ui_vector_3_t s);
void ui_transform_normalize_scale(ui_transform_t transform);

uint8_t ui_transform_scale_zero(ui_transform_t transform);
    
void ui_transform_set_quation_scale(ui_transform_t transform, ui_quaternion_t q, ui_vector_3_t s);

void ui_transform_adj_by_parent(ui_transform_t transform, ui_transform_t parent);
void ui_transform_adj_by_flip(ui_transform_t transform, ui_vector_3_t flip);

void ui_transform_cross_product(ui_transform_t to, ui_transform_t l, ui_transform_t r);

void ui_transform_reverse(ui_transform_t to, ui_transform_t origin);
void ui_transform_inline_reverse(ui_transform_t t);
    
ui_matrix_4x4_t ui_transform_calc_matrix_4x4(ui_transform_t transform);

void ui_transform_adj_vector_2(ui_transform_t transform, ui_vector_2_t to, ui_vector_2_t i);
void ui_transform_inline_adj_vector_2(ui_transform_t transform, ui_vector_2_t v);

void ui_transform_adj_vector_2_no_t(ui_transform_t transform, ui_vector_2_t to, ui_vector_2_t i);
void ui_transform_inline_adj_vector_2_no_t(ui_transform_t transform, ui_vector_2_t v);

void ui_transform_reverse_adj_vector_2(ui_transform_t transform, ui_vector_2_t to, ui_vector_2_t i);
void ui_transform_inline_reverse_adj_vector_2(ui_transform_t transform, ui_vector_2_t v);
    
void ui_transform_adj_vector_3(ui_transform_t transform, ui_vector_3_t to, ui_vector_3_t i);
void ui_transform_inline_adj_vector_3(ui_transform_t transform, ui_vector_3_t v);

void ui_transform_adj_vector_3_no_t(ui_transform_t transform, ui_vector_3_t to, ui_vector_3_t i);
void ui_transform_inline_adj_vector_3_no_t(ui_transform_t transform, ui_vector_3_t v);

float ui_transform_calc_angle_z_rad(ui_transform_t transform);

void ui_transform_print(write_stream_t s, ui_transform_t t, const char * prefix);
const char * ui_transform_dump(mem_buffer_t buffer, ui_transform_t t);
    
void ui_transform_print_2d(write_stream_t s, ui_transform_t t);
const char * ui_transform_dump_2d(mem_buffer_t buffer, ui_transform_t t);    
    
extern ui_transform UI_TRANSFORM_IDENTITY;
extern ui_transform UI_TRANSFORM_ZERO;    

#if ! DEBUG
#define ui_transform_assert_sane(__t)
#else
#define ui_transform_assert_sane(__t) do {          \
        cpe_assert_float_sane((__t)->m_s.x);        \
        cpe_assert_float_sane((__t)->m_s.y);        \
        cpe_assert_float_sane((__t)->m_s.z);        \
        cpe_assert_float_sane((__t)->m_m4.m14);     \
        cpe_assert_float_sane((__t)->m_m4.m24);     \
        cpe_assert_float_sane((__t)->m_m4.m34);     \
        if((__t)->m_init_p) {                       \
            cpe_assert_float_sane((__t)->m_m4.m11); \
            cpe_assert_float_sane((__t)->m_m4.m22); \
            cpe_assert_float_sane((__t)->m_m4.m33); \
        }                                           \
        assert(cpe_float_cmp((__t)->m_s.x, 0.0f, UI_FLOAT_PRECISION) != 0); \
        assert(cpe_float_cmp((__t)->m_s.y, 0.0f, UI_FLOAT_PRECISION) != 0); \
        assert(cpe_float_cmp((__t)->m_s.z, 0.0f, UI_FLOAT_PRECISION) != 0); \
    } while(0)
#endif
    
#ifdef __cplusplus
}
#endif

#endif
