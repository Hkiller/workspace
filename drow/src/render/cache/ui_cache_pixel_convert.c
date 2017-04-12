#include "ui_cache_pixel_convert_i.h"

static void ui_cache_pixel_r8g8b8a8_from_r8g8b8(void * i_o, void const * i_i, size_t count) {
    uint8_t * o = (uint8_t *)i_o;
    uint8_t const * i = (uint8_t const *)i_i;

    while(count > 0) {
        o[0] = i[0]; o[1] = i[1]; o[2] = i[2]; o[3] = 255;
        o += 4;
        i += 3;
        count--;
    }
}

ui_cache_pixel_format_convert_fun_t
ui_cache_pixel_find_convert(ui_cache_pixel_format_t to, ui_cache_pixel_format_t from) {
    switch(to) {
    case ui_cache_pf_r8g8b8a8:
        switch(from) {
        case ui_cache_pf_r8g8b8:
            return ui_cache_pixel_r8g8b8a8_from_r8g8b8;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return NULL;
}
