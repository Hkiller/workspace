#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "plugin_layout_animation_i.h"
#include "plugin_layout_animation_meta_i.h"
#include "plugin_layout_render_i.h"

plugin_layout_animation_t
plugin_layout_animation_create(plugin_layout_render_t render, plugin_layout_animation_meta_t meta) {
    plugin_layout_module_t module = render->m_module;
    plugin_layout_animation_t animation;

    animation = TAILQ_FIRST(&module->m_free_animations);
    if (animation) {
        TAILQ_REMOVE(&module->m_free_animations, animation, m_next_for_render);
    }
    else {
        animation = mem_alloc(module->m_alloc, sizeof(struct plugin_layout_animation) + module->m_animation_max_capacity);
        if (animation == NULL) {
            CPE_ERROR(module->m_em, "plugin_layout_animation: create: alloc fail!");
            return NULL;
        }
    }

    animation->m_render = render;
    animation->m_meta = meta;
    animation->m_id = module->m_max_animation_id + 1;
    bzero(animation + 1, module->m_animation_max_capacity);
    
    if (meta->m_init_fun && meta->m_init_fun(animation, meta->m_ctx) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_animation: type %s init fail!", meta->m_name);
        animation->m_render = (plugin_layout_render_t)module;
        TAILQ_INSERT_TAIL(&module->m_free_animations, animation, m_next_for_render);
        return NULL;
    }

    cpe_hash_entry_init(&animation->m_hh);
    if (cpe_hash_table_insert(&module->m_animations, animation) != 0) {
        CPE_ERROR(module->m_em, "plugin_layout_animation: id %d duplicate!", animation->m_id);
        TAILQ_INSERT_TAIL(&module->m_free_animations, animation, m_next_for_render);
        return NULL;
    }
    TAILQ_INSERT_TAIL(&meta->m_animations, animation, m_next_for_meta);
    TAILQ_INSERT_TAIL(&render->m_animations, animation, m_next_for_render);
    
    module->m_max_animation_id++;

    if (!render->m_need_update && meta->m_layout_fun) {
        meta->m_layout_fun(animation, meta->m_ctx);
    }
    
    return animation;
}

plugin_layout_animation_t
plugin_layout_animation_create_by_type_name(plugin_layout_render_t render, const char * type_name) {
    plugin_layout_animation_meta_t meta;

    meta = plugin_layout_animation_meta_find(render->m_module, type_name);
    if (meta == NULL) {
        CPE_ERROR(render->m_module->m_em, "plugin_layout_animation: create: meta %s not exist!", type_name);
        return NULL;
    }

    return plugin_layout_animation_create(render, meta);
}

plugin_layout_animation_t
plugin_layout_animation_find_first_by_type(plugin_layout_render_t render, const char * type_name) {
    plugin_layout_animation_t animation;

    TAILQ_FOREACH(animation, &render->m_animations, m_next_for_render) {
        if (strcmp(animation->m_meta->m_name, type_name) == 0) return animation;
    }

    return NULL;
}

void plugin_layout_animation_free(plugin_layout_animation_t animation) {
    plugin_layout_module_t module = animation->m_render->m_module;
    
    if (animation->m_meta->m_fini_fun) {
        animation->m_meta->m_fini_fun(animation, animation->m_meta->m_ctx);
    }

    TAILQ_REMOVE(&animation->m_render->m_animations, animation, m_next_for_render);
    TAILQ_REMOVE(&animation->m_meta->m_animations, animation, m_next_for_meta);
    cpe_hash_table_remove_by_ins(&module->m_animations, animation);

    animation->m_render = (plugin_layout_render_t)module;
    TAILQ_INSERT_TAIL(&module->m_free_animations, animation, m_next_for_render);
}
    
void plugin_layout_animation_real_free(plugin_layout_animation_t animation) {
    plugin_layout_module_t module = (plugin_layout_module_t)animation->m_render;
    
    TAILQ_REMOVE(&module->m_free_animations, animation, m_next_for_render);
    mem_free(module->m_alloc, animation);
}

plugin_layout_render_t plugin_layout_animation_render(plugin_layout_animation_t animation) {
    return animation->m_render;
}

uint32_t plugin_layout_animation_id(plugin_layout_animation_t animation) {
    return animation->m_id;
}

void * plugin_layout_animation_data(plugin_layout_animation_t animation) {
    return animation + 1;
}

plugin_layout_animation_t plugin_layout_animation_from_data(void * data) {
    return ((plugin_layout_animation_t)data) - 1;
}

plugin_layout_animation_t
plugin_layout_animation_find(plugin_layout_module_t module, uint32_t animation_id) {
    struct plugin_layout_animation key;
    key.m_id = animation_id;
    return cpe_hash_table_find(&module->m_animations, &key);
}

uint32_t plugin_layout_animation_hash(const plugin_layout_animation_t meta) {
    return meta->m_id;
}

int plugin_layout_animation_eq(const plugin_layout_animation_t l, const plugin_layout_animation_t r) {
    return l->m_id == r->m_id ? 1 : 0;
}
