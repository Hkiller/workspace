#ifndef UI_SPRITE_UI_PAGE_EH_I_H
#define UI_SPRITE_UI_PAGE_EH_I_H
#include "plugin/ui/plugin_ui_page_eh.h"
#include "ui_sprite_ui_env_i.h"

struct ui_sprite_ui_page_eh {
    ui_sprite_event_handler_t m_handler;
};

int ui_sprite_ui_page_eh_init(void * ctx, plugin_ui_page_eh_t eh);
void ui_sprite_ui_page_eh_fini(void * ctx, plugin_ui_page_eh_t eh);
int ui_sprite_ui_page_eh_active(void * ctx, plugin_ui_page_eh_t eh);
void ui_sprite_ui_page_eh_deactive(void * ctx, plugin_ui_page_eh_t eh);

#endif
