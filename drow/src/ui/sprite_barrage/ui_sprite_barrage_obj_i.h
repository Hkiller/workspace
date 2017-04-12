#ifndef UI_SPRITE_BARRAGE_OBJ_I_H
#define UI_SPRITE_BARRAGE_OBJ_I_H
#include "plugin/barrage/plugin_barrage_emitter.h"
#include "ui/sprite_barrage/ui_sprite_barrage_obj.h"
#include "ui_sprite_barrage_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_sprite_barrage_barrage_list, ui_sprite_barrage_barrage) ui_sprite_barrage_barrage_list_t;

struct ui_sprite_barrage_obj {
    ui_sprite_barrage_module_t m_module;
    plugin_barrage_group_t m_barrage_group;
    ui_sprite_barrage_barrage_list_t m_barrages;
    char m_speed_adj[64];
    char m_emitter_adj[64];
    uint32_t m_collision_category;
    uint32_t m_collision_mask;
    uint32_t m_collision_show_dead_anim_mask;
    uint32_t m_collision_group;
    char * m_dft_collision_event;
    plugin_barrage_target_fun_t m_target_fun;
    void * m_target_fun_ctx;
};

int ui_sprite_barrage_obj_regist(ui_sprite_barrage_module_t module);
void ui_sprite_barrage_obj_unregist(ui_sprite_barrage_module_t module);

#ifdef __cplusplus
}
#endif

#endif
