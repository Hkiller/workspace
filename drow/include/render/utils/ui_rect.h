#ifndef UI_UTILS_RECT_H
#define UI_UTILS_RECT_H
#include "ui_vector_2.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_rect {
	struct ui_vector_2 lt;
	struct ui_vector_2 rb;
};

#define UI_RECT_INITLIZER( __lt, __tp, __rt, __bm ) { UI_VECTOR_2_INITLIZER( __lt, __tp), UI_VECTOR_2_INITLIZER( __rt, __bm) }
#define UI_INIT_ZERO_RECT { UI_INIT_ZERO_VECTOR_2, UI_INIT_ZERO_VECTOR_2 }

int ui_rect_cmp(ui_rect_t l, ui_rect_t r);
    
uint8_t ui_rect_is_valid(ui_rect_t r);
uint8_t ui_rect_is_contain_pt(ui_rect_t r, ui_vector_2_t pt);

void ui_rect_adj_by_pt(ui_rect_t r, ui_vector_2_t pt);
float ui_rect_width(ui_rect_t r);
void ui_rect_set_width(ui_rect_t r, float width);    
float ui_rect_height(ui_rect_t r);
void ui_rect_set_height(ui_rect_t r, float height);

void ui_rect_intersection(ui_rect_t to, ui_rect_t a, ui_rect_t b);
uint8_t ui_rect_is_intersection_valid(ui_rect_t r, ui_rect_t c);
void ui_rect_inline_intersection(ui_rect_t to, ui_rect_t other);

void ui_rect_union(ui_rect_t to, ui_rect_t a, ui_rect_t b);
void ui_rect_inline_union(ui_rect_t to, ui_rect_t other);

void ui_rect_union_pt(ui_rect_t to, ui_rect_t a, ui_vector_2_t pt);
void ui_rect_inline_union_pt(ui_rect_t to, ui_vector_2_t pt);
    
extern ui_rect UI_RECT_ZERO;

#define ui_assert_rect_sane( __v ) ui_assert_vector_2_sane( &(__v)->lt ); ui_assert_vector_2_sane( &(__v)->rb );
    
#ifdef __cplusplus
}
#endif

#endif
