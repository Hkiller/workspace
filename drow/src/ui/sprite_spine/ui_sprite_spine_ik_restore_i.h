#ifndef UI_SPRITE_SPINE_IK_RESTORE_I_H
#define UI_SPRITE_SPINE_IK_RESTORE_I_H
#include "plugin/spine/plugin_spine_obj.h"
#include "ui/sprite_render/ui_sprite_render_types.h"
#include "ui/sprite_spine/ui_sprite_spine_ik_restore.h"
#include "ui_sprite_spine_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_spine_ik_restore {
    ui_sprite_spine_module_t m_module;
    char * m_cfg_obj_name;
    char * m_cfg_prefix;

    plugin_spine_obj_t m_spine_obj;
};

int ui_sprite_spine_ik_restore_regist(ui_sprite_spine_module_t module);
void ui_sprite_spine_ik_restore_unregist(ui_sprite_spine_module_t module);

#ifdef __cplusplus
}
#endif

#endif
