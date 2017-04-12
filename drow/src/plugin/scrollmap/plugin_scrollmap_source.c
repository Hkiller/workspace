#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "plugin_scrollmap_source_i.h"
#include "plugin_scrollmap_data_scene_i.h"
#include "plugin_scrollmap_range_i.h"

plugin_scrollmap_source_t plugin_scrollmap_source_create(plugin_scrollmap_env_t env, ui_data_src_t src) {
    plugin_scrollmap_module_t module = env->m_module;
    plugin_scrollmap_data_scene_t scene;
    plugin_scrollmap_source_t source;

    assert(plugin_scrollmap_source_find(env, src) == NULL);

    if (ui_data_src_type(src) != ui_data_src_type_scrollmap_scene) {
        CPE_ERROR(
            module->m_em, "plugin_scrollmap_source_create: src %s %s is not scrollmap scene",
            ui_data_src_type_name(ui_data_src_type(src)),
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return NULL;
    }
    
    scene = ui_data_src_product(src);
    if (scene == NULL) {
        CPE_ERROR(
            module->m_em, "plugin_scrollmap_source_create: src %s %s not loaded",
            ui_data_src_type_name(ui_data_src_type(src)),
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), src));
        return NULL;
    }
    
    source = mem_alloc(module->m_alloc, sizeof(struct plugin_scrollmap_source));
    if (source == NULL) {
        CPE_ERROR(module->m_em, "create env_source fail");
        return NULL;
    }

    source->m_env = env;
    source->m_data = scene;
    TAILQ_INIT(&source->m_ranges);

    /* struct mem_buffer buffer; */
    /* mem_buffer_init(&buffer, NULL); */
    /* printf("xxxx: %s\n%s", path, plugin_scrollmap_data_dump(&buffer, scene)); */
    /* mem_buffer_clear(&buffer); */

    TAILQ_INSERT_TAIL(&env->m_sources, source, m_next_for_env);

    return source;
}

void plugin_scrollmap_source_free(plugin_scrollmap_source_t source) {
    plugin_scrollmap_env_t env = source->m_env;
    plugin_scrollmap_module_t module = env->m_module;
    
    TAILQ_REMOVE(&env->m_sources, source, m_next_for_env);

    while(!TAILQ_EMPTY(&source->m_ranges)) {
        plugin_scrollmap_range_free(TAILQ_FIRST(&source->m_ranges));
    }

    mem_free(module->m_alloc, source);
}

plugin_scrollmap_source_t plugin_scrollmap_source_find(plugin_scrollmap_env_t env, ui_data_src_t src) {
    plugin_scrollmap_source_t source;

    TAILQ_FOREACH(source, &env->m_sources, m_next_for_env) {
        if (source->m_data->m_src == src) return source;
    }

    return NULL;
}

plugin_scrollmap_source_t plugin_scrollmap_source_find_by_path(plugin_scrollmap_env_t env, const char * path) {
    ui_data_src_t src = ui_data_src_find_by_path(env->m_module->m_data_mgr, path, ui_data_src_type_scrollmap_scene);
    if (src == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_scrollmap_source_find: src %s not exist", path);
        return NULL;
    }
    
    return plugin_scrollmap_source_find(env, src);
}

plugin_scrollmap_source_t plugin_scrollmap_source_create_by_path(plugin_scrollmap_env_t env, const char * path) {
    ui_data_src_t src = ui_data_src_find_by_path(env->m_module->m_data_mgr, path, ui_data_src_type_scrollmap_scene);
    if (src == NULL) {
        CPE_ERROR(env->m_module->m_em, "plugin_scrollmap_source_create: src %s not exist", path);
        return NULL;
    }

    return plugin_scrollmap_source_create(env, src);
}

ui_data_src_t plugin_scrollmap_source_src(plugin_scrollmap_source_t source) {
    return source->m_data->m_src;
}

plugin_scrollmap_data_scene_t plugin_scrollmap_source_data(plugin_scrollmap_source_t source) {
    return source->m_data;
}

static plugin_scrollmap_source_t plugin_scrollmap_source_child_next(struct plugin_scrollmap_source_it * it) {
    plugin_scrollmap_source_t * data = (plugin_scrollmap_source_t *)(it->m_data);
    plugin_scrollmap_source_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_env);

    return r;
}

void plugin_scrollmap_env_sources(plugin_scrollmap_source_it_t it, plugin_scrollmap_env_t env) {
    *(plugin_scrollmap_source_t *)(it->m_data) = TAILQ_FIRST(&env->m_sources);
    it->next = plugin_scrollmap_source_child_next;
}
