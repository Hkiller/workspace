#ifndef UI_SPRITE_2D_SEARCH_I_H
#define UI_SPRITE_2D_SEARCH_I_H
#include "ui/sprite_2d/ui_sprite_2d_search.h"
#include "ui_sprite_2d_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_2d_search {
    ui_sprite_2d_module_t m_module;
    char * m_on_found;
};

int ui_sprite_2d_search_regist(ui_sprite_2d_module_t module);
void ui_sprite_2d_search_unregist(ui_sprite_2d_module_t module);

#ifdef __cplusplus
}
#endif

#endif
