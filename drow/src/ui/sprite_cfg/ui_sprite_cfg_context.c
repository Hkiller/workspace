#include "ui_sprite_cfg_context_i.h"

ui_sprite_cfg_context_t ui_sprite_cfg_context_create(ui_sprite_cfg_loader_t loader, cfg_t cfg) {
    ui_sprite_cfg_context_t context = mem_alloc(loader->m_alloc, sizeof(struct ui_sprite_cfg_context));
    if (context == NULL) {
        CPE_ERROR(loader->m_em, "%s: create context: alloc fail!", ui_sprite_cfg_loader_name(loader));
        return NULL;
    }

    context->m_loader = loader;
    context->m_cfg = cfg;

    TAILQ_INSERT_HEAD(&loader->m_contexts, context, m_next_for_loader);

    return context;
}

void ui_sprite_cfg_context_free(ui_sprite_cfg_context_t context) {
    ui_sprite_cfg_loader_t loader = context->m_loader;
    
    TAILQ_REMOVE(&loader->m_contexts, context, m_next_for_loader);

    mem_free(loader->m_alloc, context);
}

void ui_sprite_cfg_context_free_all(ui_sprite_cfg_loader_t loader) {
    while(!TAILQ_EMPTY(&loader->m_contexts)) {
        ui_sprite_cfg_context_free(TAILQ_FIRST(&loader->m_contexts));
    }
}
