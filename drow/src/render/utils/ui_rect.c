#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_rect.h"

int ui_rect_cmp(ui_rect_t l, ui_rect_t r) {
    int v = ui_vector_2_cmp(&l->lt, &r->lt);
    return v ? v : ui_vector_2_cmp(&l->rb, &r->rb);
}

uint8_t ui_rect_is_valid(ui_rect_t r) {
    return (r->lt.x < (r->rb.x - UI_FLOAT_PRECISION) && r->lt.y < (r->rb.y - UI_FLOAT_PRECISION)) ? 1 : 0;
}

uint8_t ui_rect_is_contain_pt(ui_rect_t r, ui_vector_2_t pt) {
    return pt->x >= r->lt.x
        && pt->y >= r->lt.y
        && pt->x < r->rb.x
        && pt->y < r->rb.y
        ? 1 : 0;
}

void ui_rect_adj_by_pt(ui_rect_t r, ui_vector_2_t pt) {
    r->lt.x += pt->x;
    r->lt.y += pt->y;    
    r->rb.x += pt->x;
    r->rb.y += pt->y;
}

float ui_rect_width(ui_rect_t r) {
    return r->rb.x - r->lt.x;
}

void ui_rect_set_width(ui_rect_t r, float width) {
    r->rb.x = r->lt.x + width;
}

float ui_rect_height(ui_rect_t r) {
    return r->rb.y - r->lt.y;
}

void ui_rect_set_height(ui_rect_t r, float height) {
    r->rb.y = r->lt.y + height;
}

void ui_rect_intersection(ui_rect_t to, ui_rect_t a, ui_rect_t b) {
	if (a->rb.x < b->lt.x || a->lt.x > b->rb.x || a->rb.y < b->lt.y || a->lt.y > b->rb.y) {
        /*没有交集 */
		* to = UI_RECT_ZERO;
    }
    else {
        to->lt.x = cpe_max(a->lt.x, b->lt.x);
        to->lt.y = cpe_max(a->lt.y, b->lt.y);
    
        to->rb.x = cpe_min(a->rb.x, b->rb.x);
        to->rb.y = cpe_min(a->rb.y, b->rb.y);
    }
}

uint8_t ui_rect_is_intersection_valid(ui_rect_t a, ui_rect_t b) {
    return (a->rb.x < b->lt.x || a->lt.x > b->rb.x || a->rb.y < b->lt.y || a->lt.y > b->rb.y) ? 0 : 1;
}

void ui_rect_inline_intersection(ui_rect_t to, ui_rect_t other) {
	if (to->rb.x < other->lt.x || to->lt.x > other->rb.x || to->rb.y < other->lt.y || to->lt.y > other->rb.y) {
        /*没有交集 */
		* to = UI_RECT_ZERO;
    }
    else {
        to->lt.x = cpe_max(to->lt.x, other->lt.x);
        to->lt.y = cpe_max(to->lt.y, other->lt.y);
    
        to->rb.x = cpe_min(to->rb.x, other->rb.x);
        to->rb.y = cpe_min(to->rb.y, other->rb.y);
    }
}

void ui_rect_union_pt(ui_rect_t to, ui_rect_t a, ui_vector_2_t pt) {
    to->lt.x = cpe_min(a->lt.x, pt->x);
    to->lt.y = cpe_min(a->lt.y, pt->y);
    to->rb.x = cpe_min(a->rb.x, pt->x);
    to->rb.y = cpe_min(a->rb.y, pt->y);
}

void ui_rect_inline_union_pt(ui_rect_t to, ui_vector_2_t pt) {
    if (pt->x < to->lt.x) to->lt.x = pt->x;
    if (pt->y < to->lt.y) to->lt.y = pt->y;
    if (pt->x > to->rb.x) to->rb.x = pt->x;
    if (pt->y > to->rb.y) to->rb.y = pt->y;
}

void ui_rect_union(ui_rect_t to, ui_rect_t a, ui_rect_t b) {
    to->lt.x = cpe_min(a->lt.x, b->lt.x);
    to->lt.y = cpe_min(a->lt.y, b->lt.y);
    to->rb.x = cpe_min(a->rb.x, b->rb.x);
    to->rb.y = cpe_min(a->rb.y, b->rb.y);
}

void ui_rect_inline_union(ui_rect_t to, ui_rect_t other) {
    if (other->lt.x < to->lt.x) to->lt.x = other->lt.x;
    if (other->lt.y < to->lt.y) to->lt.y = other->lt.y;
    if (other->rb.x > to->rb.x) to->rb.x = other->rb.x;
    if (other->rb.y > to->rb.y) to->rb.y = other->rb.y;
}

ui_rect UI_RECT_ZERO = UI_INIT_ZERO_RECT;
