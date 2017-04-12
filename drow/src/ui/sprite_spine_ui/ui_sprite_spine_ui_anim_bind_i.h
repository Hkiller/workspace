#ifndef UI_SPRITE_SPINE_UI_ANIM_BIND_I_H
#define UI_SPRITE_SPINE_UI_ANIM_BIND_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite_spine_ui/ui_sprite_spine_ui_anim_bind.h"
#include "ui_sprite_spine_ui_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_sprite_spine_ui_anim_bind_root * ui_sprite_spine_ui_anim_bind_root_t;
typedef struct ui_sprite_spine_ui_anim_bind_control * ui_sprite_spine_ui_anim_bind_control_t;
typedef TAILQ_HEAD(ui_sprite_spine_ui_anim_bind_root_list, ui_sprite_spine_ui_anim_bind_root) ui_sprite_spine_ui_anim_bind_root_list_t;

struct ui_sprite_spine_ui_anim_bind {
    ui_sprite_spine_ui_module_t m_module;
    plugin_ui_aspect_t m_aspect;
    uint8_t m_debug;
    char * m_prefix;
    ui_sprite_spine_ui_anim_bind_root_list_t m_roots;
};

struct ui_sprite_spine_ui_anim_bind_root {
    ui_sprite_spine_ui_anim_bind_t m_bind;
    TAILQ_ENTRY(ui_sprite_spine_ui_anim_bind_root) m_next;
    char * m_root;
    plugin_ui_aspect_t m_aspect;
};

typedef enum ui_sprite_spine_ui_anim_bind_way {
    ui_sprite_spine_ui_anim_bind_area,
    ui_sprite_spine_ui_anim_bind_area_scale,
    ui_sprite_spine_ui_anim_bind_bone_scale,
} ui_sprite_spine_ui_anim_bind_way_t;
    
struct ui_sprite_spine_ui_anim_bind_control {
    struct spSlot * m_slot;
    ui_sprite_spine_ui_anim_bind_way_t m_way;
};
    
int ui_sprite_spine_ui_anim_bind_regist(ui_sprite_spine_ui_module_t module);
void ui_sprite_spine_ui_anim_bind_unregist(ui_sprite_spine_ui_module_t module);

#ifdef __cplusplus
}
#endif

#endif
