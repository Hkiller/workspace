#ifndef UI_SPRITE_UI_NAVIGATION_I_H
#define UI_SPRITE_UI_NAVIGATION_I_H
#include "plugin/ui/plugin_ui_navigation.h"
#include "ui/sprite_ui/ui_sprite_ui_navigation.h"
#include "ui_sprite_ui_state_i.h"

struct ui_sprite_ui_navigation {
    ui_sprite_ui_env_t m_env;
    char * m_event;
};

int ui_sprite_ui_env_navigation_init(void * ctx, plugin_ui_navigation_t b_navigation);
void ui_sprite_ui_env_navigation_fini(void * ctx, plugin_ui_navigation_t b_navigation);

#endif
