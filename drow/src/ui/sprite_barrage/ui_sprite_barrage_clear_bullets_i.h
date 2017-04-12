#ifndef UI_SPRITE_BARRAGE_CLEAR_BULLETS_I_H
#define UI_SPRITE_BARRAGE_CLEAR_BULLETS_I_H
#include "cpe/pal/pal_queue.h"
#include "ui/sprite_barrage/ui_sprite_barrage_clear_bullets.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_barrage_clear_bullets_node * ui_sprite_barrage_clear_bullets_node_t;
typedef TAILQ_HEAD(ui_sprite_barrage_clear_bullets_node_list, ui_sprite_barrage_clear_bullets_node) ui_sprite_barrage_clear_bullets_node_list_t;

struct ui_sprite_barrage_clear_bullets {
    ui_sprite_barrage_module_t m_module;
    char * m_group_name;
};

int ui_sprite_barrage_clear_bullets_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_clear_bullets_unregist(ui_sprite_barrage_module_t module);

#ifdef __cplusplus
}
#endif

#endif
