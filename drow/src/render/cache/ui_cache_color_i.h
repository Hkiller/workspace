#ifndef UI_CACHE_COLOR_I_H
#define UI_CACHE_COLOR_I_H
#include "render/utils/ui_color.h"
#include "render/cache/ui_cache_color.h"
#include "ui_cache_manager_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_cache_color {
    ui_cache_manager_t m_mgr;
    struct cpe_hash_entry m_hh;
    const char * m_name;
    ui_color m_color;
};

ui_cache_color_t ui_cache_color_create(ui_cache_manager_t mgr, const char * name, ui_color_t color);
void ui_cache_color_free(ui_cache_color_t color);
void ui_cache_color_free_all(ui_cache_manager_t mgr);

int ui_cache_color_load_defaults(ui_cache_manager_t mgr);
    
ui_cache_color_t ui_cache_color_find(ui_cache_manager_t mgr, const char * name);

uint32_t ui_cache_color_hash(ui_cache_color_t color);
int ui_cache_color_eq(ui_cache_color_t l, ui_cache_color_t r);

#ifdef __cplusplus
}
#endif

#endif
