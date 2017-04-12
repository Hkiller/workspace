#ifndef PLUGIN_UI_UTILS_I_H
#define PLUGIN_UI_UTILS_I_H
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_color.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_vector_3.h"
#include "plugin/ui/plugin_ui_types.h"

#ifdef __cplusplus
extern "C" {
#endif

float plugin_ui_unit_to_real(UI_UNIT const * unit, float base);
void plugin_ui_real_to_unit(UI_UNIT * result, float real, float base);
    
void plugin_ui_calc_child(
    ui_vector_2_t result,
    UI_UNIT_VECTOR_2 const * rule, uint8_t szls, ui_vector_2_t curr, ui_vector_2_t orig);
    
void plugin_ui_calc_base(ui_vector_2_t r_base, uint8_t szls, ui_vector_2_t curr, ui_vector_2_t orig);    
    
void plugin_ui_calc_child_unit(
    UI_UNIT_VECTOR_2 * result,
    uint8_t szls, ui_vector_2_t parent_curr, ui_vector_2_t parent_orig,
    ui_vector_2_t real);

ui_vector_2 plugin_ui_calc_adj_sz_by_pos_policy(ui_vector_2_t sz, ui_vector_2_t pivot, uint8_t policy);

#define ui_assert_unit_sane(__p)                \
    cpe_assert_float_sane((__p)->k);            \
    cpe_assert_float_sane((__p)->b)
    
#define ui_assert_unit_vector_2_sane(__p2)      \
    ui_assert_unit_sane(&(__p2)->x);          \
    ui_assert_unit_sane(&(__p2)->y)
    
#ifdef __cplusplus
}
#endif

#endif
