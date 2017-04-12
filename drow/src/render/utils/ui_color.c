#include "cpe/pal/pal_string.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_color.h"

uint32_t ui_color_make_abgr(ui_color_t color) {
	return ((uint32_t)(color->a*255.0f)<<24 | (uint32_t)(color->b*255.0f)<<16 | (uint32_t)(color->g*255.0f)<<8 | (uint32_t)(color->r*255.0f));
}

uint32_t ui_color_make_abgr_from_value(float r, float g, float b, float a) {
    return (((uint32_t)(a*255.0f))<<24 | ((uint32_t)(b*255.0f))<<16 | ((uint32_t)(g*255.0f))<<8 | ((uint32_t)(r*255.0f)));
}

void ui_color_set_from_argb(ui_color_t color, uint32_t argb) {
	color->a = (float)((argb & 0xFF000000)>>24) * (1.0f / 255.0f);
	color->r = (float)((argb & 0x00FF0000)>>16) * (1.0f / 255.0f);
	color->g = (float)((argb & 0x0000FF00)>>8)  * (1.0f / 255.0f);
	color->b = (float)((argb & 0x000000FF))     * (1.0f / 255.0f);
}

uint32_t ui_color_make_argb(ui_color_t color) {
	return ((uint32_t)(color->a*255.0f)<<24 | (uint32_t)(color->r*255.0f)<<16 | (uint32_t)(color->g*255.0f)<<8 | (uint32_t)(color->b*255.0f));
}

uint32_t ui_color_make_argb_from_value(float r, float g, float b, float a) {
	return ((uint32_t)(a*255.0f)<<24 | (uint32_t)(r*255.0f)<<16 | (uint32_t)(g*255.0f)<<8 | (uint32_t)(b*255.0f));
}

void ui_color_mul(ui_color_t to, ui_color_t a, ui_color_t b) {
    to->a = a->a * b->a;
    to->r = a->r * b->r;
    to->g = a->g * b->g;
    to->b = a->b * b->b;
}

void ui_color_inline_mul(ui_color_t to_color, ui_color_t color) {
    to_color->a *= color->a;
    to_color->r *= color->r;
    to_color->g *= color->g;
    to_color->b *= color->b;
}

void ui_color_add(ui_color_t to, ui_color_t a, ui_color_t b) {
    to->a = a->a + b->a; if (to->a > 1.0f) to->a = 1.0f;
    to->r = a->r + b->r; if (to->r > 1.0f) to->r = 1.0f;
    to->g = a->g + b->g; if (to->g > 1.0f) to->g = 1.0f;
    to->b = a->b + b->b; if (to->b > 1.0f) to->b = 1.0f;
}

void ui_color_inline_add(ui_color_t to_color, ui_color_t color) {
    to_color->a += color->a; if (to_color->a > 1.0f) to_color->a = 1.0f;
    to_color->r += color->r; if (to_color->r > 1.0f) to_color->r = 1.0f;
    to_color->g += color->g; if (to_color->g > 1.0f) to_color->g = 1.0f;
    to_color->b += color->b; if (to_color->b > 1.0f) to_color->b = 1.0f;
}

static int ui_color_parse_num(const char * v, float * r) {
    if (strchr(v, '.')) {
        float t = atof(v);
        if (t < 0.0f || t > 1.0f) return -1;
        *r = t;
    }
    else {
        int c = atoi(v);
        if (c > 255 || c < 0) return -1;
        *r = ((float)c) / 255.0f;
    }
    return 0; 
}

static int ui_color_parse_make_one(unsigned char c1, unsigned char c2, float * r) {

    if (c1 >= '0' && c1 <= '9') { c1 = c1 - '0'; }
    else if (c1 >= 'a' && c1 <= 'f') { c1 = c1 - 'a' + 10; }
    else if (c1 >= 'A' && c1 <= 'F') { c1 = c1 - 'A' + 10; }
    else return -1;

    if (c2 >= '0' && c2 <= '9') { c2 = c2 - '0'; }
    else if (c2 >= 'a' && c2 <= 'f') { c2 = c2 - 'a' + 10; }
    else if (c2 >= 'A' && c2 <= 'F') { c2 = c2 - 'A' + 10; }
    else return -1;

    *r = (float)(((c1) << 4) + (c2)) / 255.0f;
    return 0;
}

int ui_color_parse_from_str(ui_color_t r, const char * i) {
    if (i[0] == '#') {
        size_t len = strlen(i + 1);
        i += 1;

        if (len == 6) { 
            if (ui_color_parse_make_one(i[0], i[1], &r->r) != 0
                || ui_color_parse_make_one(i[2], i[3], &r->g) != 0
                || ui_color_parse_make_one(i[4], i[5], &r->b) != 0)
            {
                return -1;
            }
            r->a = 1.0f;
        }
        else if (len == 8) {
            if (ui_color_parse_make_one(i[0], i[1], &r->a) != 0
                || ui_color_parse_make_one(i[2], i[3], &r->r) != 0
                || ui_color_parse_make_one(i[4], i[5], &r->g) != 0
                || ui_color_parse_make_one(i[6], i[7], &r->b) != 0)
            {
                return -1;
            }
        }
        else {
            return -1;
        }

        return 0;
    }
    else if (strchr(i, '=')) {
        char buf[64];

        if (cpe_str_read_arg(buf, sizeof(buf), i, "a", ',', '=') == 0) {
            if (ui_color_parse_num(buf, &r->a) != 0) return -1;
        }
        else {
            r->a = 1.0f;
        }

        if (cpe_str_read_arg(buf, sizeof(buf), i, "r", ',', '=') == 0) {
            if (ui_color_parse_num(buf, &r->r) != 0) return -1;
        }
        else {
            r->r = 0.0f;
        }

        if (cpe_str_read_arg(buf, sizeof(buf), i, "g", ',', '=') == 0) {
            if (ui_color_parse_num(buf, &r->g) != 0) return -1;
        }
        else {
            r->g = 0.0f;
        }

        if (cpe_str_read_arg(buf, sizeof(buf), i, "b", ',', '=') == 0) {
            if (ui_color_parse_num(buf, &r->b) != 0) return -1;
        }
        else {
            r->b = 0.0f;
        }
        
        return 0;
    }
    else {
        return -1;
    }
}

int ui_color_cmp(ui_color_t l, ui_color_t r) {
    int rv;

    if ((rv = cpe_float_cmp(l->a, r->a, UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->r, r->r, UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->g, r->g, UI_FLOAT_PRECISION))) return rv;
    return cpe_float_cmp(l->b, r->b, UI_FLOAT_PRECISION);
}

ui_color UI_COLOR_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
ui_color UI_COLOR_BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
