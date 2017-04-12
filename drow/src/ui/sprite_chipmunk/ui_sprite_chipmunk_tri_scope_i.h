#ifndef UI_SPRITE_CHIPMUNK_TRI_SCOPE_I_H
#define UI_SPRITE_CHIPMUNK_TRI_SCOPE_I_H
#include "chipmunk/chipmunk_private.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_tri_scope.h"
#include "ui_sprite_chipmunk_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif


enum ui_sprite_chipmunk_tri_scope_type {
    ui_sprite_chipmunk_tri_scope_unknown
    , ui_sprite_chipmunk_tri_scope_dynamic
    , ui_sprite_chipmunk_tri_scope_fix
};

struct ui_sprite_chipmunk_tri_scope {
    ui_sprite_chipmunk_env_t m_env;
    TAILQ_ENTRY(ui_sprite_chipmunk_tri_scope) m_next_for_env;
    ui_sprite_chipmunk_obj_t m_obj;
    TAILQ_ENTRY(ui_sprite_chipmunk_tri_scope) m_next_for_obj;
    uint32_t m_group;
    uint32_t m_mask;
    enum ui_sprite_chipmunk_tri_scope_type m_type;
    union {
        struct {
            ui_sprite_chipmunk_tri_scope_update_fun_t m_fun;
            char m_ctx[64];
        } m_dynamic;
        struct {
            ui_sprite_2d_part_t m_part;
        } m_fix;
    };
};

void ui_sprite_chipmunk_tri_scope_real_free(ui_sprite_chipmunk_tri_scope_t scope);
    
#ifdef __cplusplus
}
#endif

#endif
