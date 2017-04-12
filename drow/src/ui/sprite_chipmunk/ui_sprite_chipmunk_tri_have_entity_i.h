#ifndef UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY_I_H
#define UI_SPRITE_CHIPMUNK_TRI_HAVE_ENTITY_I_H
#include "pcre2.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_tri_have_entity.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_tri_have_entity {
    ui_sprite_chipmunk_obj_t m_obj;
    ui_sprite_chipmunk_tri_scope_t m_scope;
    char * m_name_pattern;
    pcre2_code * m_name_re;
    pcre2_match_data * m_match_data;
    char * m_require_count;
};

int ui_sprite_chipmunk_tri_have_entity_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_tri_have_entity_unregist(ui_sprite_chipmunk_module_t module);

#ifdef __cplusplus
}
#endif

#endif
