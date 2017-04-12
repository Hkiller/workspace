#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "ui_sprite_ui_navigation_i.h"

int ui_sprite_ui_env_navigation_init(void * ctx, plugin_ui_navigation_t b_navigation) {
    ui_sprite_ui_navigation_t navigation = plugin_ui_navigation_data(b_navigation);

    navigation->m_env = ctx;
    navigation->m_event = NULL;

    return 0;
}

void ui_sprite_ui_env_navigation_fini(void * ctx, plugin_ui_navigation_t b_navigation) {
    ui_sprite_ui_env_t env = ctx;    
    ui_sprite_ui_navigation_t navigation = plugin_ui_navigation_data(b_navigation);

    if (navigation->m_event) {
        mem_free(env->m_module->m_alloc, navigation->m_event);
        navigation->m_event = NULL;
    }
}

ui_sprite_ui_navigation_t ui_sprite_ui_navigation_cast(plugin_ui_navigation_t b_navigation) {
    return plugin_ui_navigation_data(b_navigation);
}

const char * ui_sprite_ui_navigation_event(ui_sprite_ui_navigation_t navigation) {
    return navigation->m_event;
}

int ui_sprite_ui_navigation_set_event(ui_sprite_ui_navigation_t navigation, const char * event) {
    if (navigation->m_event) {
        mem_free(navigation->m_env->m_module->m_alloc, navigation->m_event);
    }

    if (event) {
        navigation->m_event = cpe_str_mem_dup(navigation->m_env->m_module->m_alloc, event);
        if (navigation->m_event == NULL) {
            CPE_ERROR(navigation->m_env->m_module->m_em, "ui_sprite_ui_navigation_event: alloc fail!");
            return -1;
        }
    }
    else {
        navigation->m_event = NULL;
    }

    return 0;
}
