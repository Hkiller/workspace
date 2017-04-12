#include <assert.h>
#include "cpe/utils/tailq_sort.h"
#include "cpe/utils/string_utils.h"
#include "ui_sprite_render_layer_i.h"
#include "ui_sprite_render_anim_i.h"

ui_sprite_render_layer_t
ui_sprite_render_layer_create(ui_sprite_render_env_t env, ui_sprite_render_layer_t before, const char * name) {
    ui_sprite_render_layer_t layer;

    layer = mem_alloc(env->m_module->m_alloc, sizeof(struct ui_sprite_render_layer));
    if (layer == NULL) {
        CPE_ERROR(env->m_module->m_em, "ui_sprite_render_layer_create_before: alloc fail!");
        return NULL;
    }

    layer->m_env = env;
    cpe_str_dup(layer->m_name, sizeof(layer->m_name), name);
    layer->m_is_dirty = 0;
    layer->m_is_free = 0;
    TAILQ_INIT(&layer->m_anims);

    if (before) {
        TAILQ_INSERT_BEFORE(before, layer, m_next);
    }
    else {
        TAILQ_INSERT_TAIL(&env->m_layers, layer, m_next);
    }

    return layer;
}

void ui_sprite_render_layer_free(ui_sprite_render_layer_t layer) {
    ui_sprite_render_env_t env = layer->m_env;

    while(!TAILQ_EMPTY(&layer->m_anims)) {
        ui_sprite_render_anim_free(TAILQ_FIRST(&layer->m_anims));
    }
    
    if (layer == env->m_default_layer) {
        env->m_default_layer = NULL;
    }

    TAILQ_REMOVE(&env->m_layers, layer, m_next);

    mem_free(env->m_module->m_alloc, layer);
}

ui_sprite_render_layer_t ui_sprite_render_layer_default(ui_sprite_render_env_t env) {
    return env->m_default_layer;
}

ui_sprite_render_layer_t ui_sprite_render_layer_find(ui_sprite_render_env_t env, const char * layer_name) {
    ui_sprite_render_layer_t layer;

    if (layer_name[0] == 0) return env->m_default_layer;
    
    TAILQ_FOREACH(layer, &env->m_layers, m_next) {
        if (strcmp(layer->m_name, layer_name) == 0) return layer;
    }

    return NULL;
}

static ui_sprite_render_layer_t ui_sprite_render_env_layer_next(ui_sprite_render_layer_it_t it) {
    ui_sprite_render_layer_t * data = (ui_sprite_render_layer_t *)(it->m_data);
    ui_sprite_render_layer_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void ui_sprite_render_env_layers(ui_sprite_render_env_t env, ui_sprite_render_layer_it_t layer_it) {
    *(ui_sprite_render_layer_t *)(layer_it->m_data) = TAILQ_FIRST(&env->m_layers);
    layer_it->next = ui_sprite_render_env_layer_next;
}

int ui_sprite_render_layer_sort_anim_cmp(ui_sprite_render_anim_t l, ui_sprite_render_anim_t r, void * p) {
    if (l->m_priority == r->m_priority) return 0;
    return l->m_priority < r->m_priority ? -1 : 1;
}

void ui_sprite_render_layer_sort_anims(ui_sprite_render_layer_t layer) {
    TAILQ_SORT(
        &layer->m_anims, ui_sprite_render_anim, ui_sprite_render_anim_list, m_next_for_layer,
        ui_sprite_render_layer_sort_anim_cmp, NULL);
}

const char * ui_sprite_render_layer_name(ui_sprite_render_layer_t layer) {
    return layer->m_name;
}
