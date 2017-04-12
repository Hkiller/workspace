#ifndef UI_SPRITE_CHIPMUNK_WITH_GROUP_I_H
#define UI_SPRITE_CHIPMUNK_WITH_GROUP_I_H
#include "cpe/pal/pal_queue.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "ui/sprite_chipmunk/ui_sprite_chipmunk_with_group.h"
#include "ui_sprite_chipmunk_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_sprite_chipmunk_with_group {
    ui_sprite_chipmunk_module_t m_module;
    const char * m_group;
    ui_sprite_chipmunk_obj_runtime_group_list_t m_runtime_groups;
};

int ui_sprite_chipmunk_with_group_regist(ui_sprite_chipmunk_module_t module);
void ui_sprite_chipmunk_with_group_unregist(ui_sprite_chipmunk_module_t module);
    
#ifdef __cplusplus
}
#endif

#endif
