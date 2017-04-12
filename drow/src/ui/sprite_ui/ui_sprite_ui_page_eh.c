#include <assert.h>
#include "ui/sprite/ui_sprite_entity.h"
#include "ui/sprite/ui_sprite_event.h"
#include "ui_sprite_ui_page_eh_i.h"

int ui_sprite_ui_page_eh_init(void * ctx, plugin_ui_page_eh_t b_eh) {
    struct ui_sprite_ui_page_eh * eh = plugin_ui_page_eh_data(b_eh);
    eh->m_handler = NULL;
    return 0;
}

void ui_sprite_ui_page_eh_fini(void * ctx, plugin_ui_page_eh_t b_eh) {
    struct ui_sprite_ui_page_eh * eh = plugin_ui_page_eh_data(b_eh);
    assert(eh->m_handler == NULL);
}

static void ui_sprite_ui_page_eh_call(void * ctx, ui_sprite_event_t evt) {
    plugin_ui_page_eh_t b_eh = ctx;
    //struct ui_sprite_ui_page_eh * eh = plugin_ui_page_eh_data(b_eh);

    plugin_ui_page_eh_call(b_eh, evt->meta, evt->data, evt->size);
}

int ui_sprite_ui_page_eh_active(void * ctx, plugin_ui_page_eh_t b_eh) {
    ui_sprite_ui_env_t env = ctx;
    struct ui_sprite_ui_page_eh * eh = plugin_ui_page_eh_data(b_eh);
        
    assert(eh->m_handler == NULL);

    eh->m_handler = ui_sprite_entity_add_event_handler(
        env->m_entity, ui_sprite_event_scope_self,
        plugin_ui_page_eh_event(b_eh), ui_sprite_ui_page_eh_call, b_eh);
    if (eh->m_handler == NULL) {
        CPE_ERROR(
            env->m_module->m_em, "ui_sprite_ui_page_eh_active: create handler of %s fail!",
            plugin_ui_page_eh_event(b_eh));
        return -1;
    }

    return 0;
}

void ui_sprite_ui_page_eh_deactive(void * ctx, plugin_ui_page_eh_t b_eh) {
    ui_sprite_ui_env_t env = ctx;
    struct ui_sprite_ui_page_eh * eh = plugin_ui_page_eh_data(b_eh);

    assert(eh->m_handler);
    ui_sprite_event_handler_free(ui_sprite_entity_world(env->m_entity), eh->m_handler);
    eh->m_handler = NULL;
}    

