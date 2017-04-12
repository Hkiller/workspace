#ifndef UI_UTILS_COLOR_H
#define UI_UTILS_COLOR_H
#include "ui_utils_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_color {
	float r;
	float g;
	float b;
	float a;
};

uint32_t ui_color_make_abgr(ui_color_t color);
uint32_t ui_color_make_abgr_from_value(float r, float g, float b, float a);

void ui_color_set_from_argb(ui_color_t color, uint32_t argb);
uint32_t ui_color_make_argb(ui_color_t color);
uint32_t ui_color_make_argb_from_value(float r, float g, float b, float a);

void ui_color_mul(ui_color_t to, ui_color_t a, ui_color_t b);
void ui_color_inline_mul(ui_color_t to_color, ui_color_t color);

void ui_color_add(ui_color_t to, ui_color_t a, ui_color_t b);
void ui_color_inline_add(ui_color_t to_color, ui_color_t color);
    
int ui_color_cmp(ui_color_t l, ui_color_t r);

int ui_color_parse_from_str(ui_color_t r, const char * i);

extern ui_color UI_COLOR_WHITE;
extern ui_color UI_COLOR_BLACK;    
    
#ifdef __cplusplus
}
#endif

#endif
