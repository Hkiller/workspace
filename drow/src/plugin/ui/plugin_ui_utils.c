#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "plugin_ui_utils_i.h"

float plugin_ui_unit_to_real(UI_UNIT const * unit, float base) {
	float temp = unit->k * base + unit->b;
    cpe_assert_float_sane(temp);
	return /*ui_pixel_aligned*/(temp);
}

void plugin_ui_real_to_unit(UI_UNIT * result, float real, float base) {
    cpe_assert_float_sane(real);
    cpe_assert_float_sane(base);
    
    result->k = real / base;
    result->b = 0.0f;
}

void plugin_ui_calc_child(
    ui_vector_2_t result,
    UI_UNIT_VECTOR_2 const * rule, uint8_t szls, ui_vector_2_t curr, ui_vector_2_t orig)
{
    ui_vector_2 base;

    ui_assert_unit_vector_2_sane(rule);
    ui_assert_vector_2_sane(curr);

    plugin_ui_calc_base(&base, szls, curr, orig);
    ui_assert_vector_2_sane(&base);

    result->x = plugin_ui_unit_to_real(&rule->x, base.w);
    result->y = plugin_ui_unit_to_real(&rule->y, base.h);

    ui_assert_vector_2_sane(result);
}

void plugin_ui_calc_base(ui_vector_2_t r_base, uint8_t adj, ui_vector_2_t curr, ui_vector_2_t orig) {
    *r_base = *curr;

    if (!adj) {
		float wr;
		float hr;

        assert(orig->w > 0.0f);
        assert(orig->h > 0.0f);

        wr = curr->w / orig->w;
        hr = curr->h / orig->h;

        if (wr < hr) r_base->h = orig->h * wr;
        if (wr > hr) r_base->w = orig->w * hr;

        r_base->h = r_base->w = cpe_min(r_base->w, r_base->h);
    }
}

void plugin_ui_calc_child_unit(
    UI_UNIT_VECTOR_2 * result,
    uint8_t adj, ui_vector_2_t curr, ui_vector_2_t orig, ui_vector_2_t real)
{
    ui_vector_2 base;
    plugin_ui_calc_base(&base, adj, curr, orig);

    ui_assert_vector_2_sane(real);
    ui_assert_vector_2_sane(&base);
    
    result->x.k = cpe_float_cmp(base.w, 0.0f, UI_FLOAT_PRECISION) == 0 ? 0.0f : real->w / base.w;
    result->x.b = 0.0f;

    result->y.k = cpe_float_cmp(base.h, 0.0f, UI_FLOAT_PRECISION) == 0 ? 0.0f : real->h / base.h;
    result->y.b = 0.0f;

    ui_assert_unit_vector_2_sane(result);

    /* result->x.k = 0.0f; */
    /* result->x.b = real->x; */

    /* result->y.k = 0.0f; */
    /* result->y.b = real->y; */
}

ui_vector_2 plugin_ui_calc_adj_sz_by_pos_policy(ui_vector_2_t sz, ui_vector_2_t pivot, uint8_t policy) {
    ui_vector_2 trans = UI_VECTOR_2_ZERO;

    switch(policy) {
    case ui_pos_policy_top_left:
        break;
    case ui_pos_policy_top_center:
		trans.x = sz->x / 2.0;
        break;
    case ui_pos_policy_top_right:
        trans.x = sz->x;
        break;
    case ui_pos_policy_center_left:
		trans.y = sz->y / 2.0;
        break;
    case ui_pos_policy_center:
		trans.x = sz->x / 2.0;
		trans.y = sz->y / 2.0;
        break;
    case ui_pos_policy_center_right:
		trans.x = sz->x;
		trans.y = sz->y / 2.0;
        break;
    case ui_pos_policy_bottom_left:
		trans.y = sz->y;
        break;
    case ui_pos_policy_bottom_center:
		trans.x = sz->x / 2.0;
		trans.y = sz->y;
        break;
    case ui_pos_policy_bottom_right:
		trans.x = sz->x;
		trans.y = sz->y;
        break;
    case ui_pos_policy_pivot:
		trans.x = sz->x * pivot->x;
		trans.y = sz->y * pivot->y;
        break;
	}

	return trans;
}
