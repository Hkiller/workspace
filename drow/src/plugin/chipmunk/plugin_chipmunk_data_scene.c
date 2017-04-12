#include <assert.h>
#include "plugin_chipmunk_data_scene_i.h"
#include "plugin_chipmunk_data_body_i.h"
#include "plugin_chipmunk_data_constraint_i.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_data_scene_t
plugin_chipmunk_data_scene_create(plugin_chipmunk_module_t module, ui_data_src_t src) {
    plugin_chipmunk_data_scene_t scene;

    if (ui_data_src_type(src) != ui_data_src_type_chipmunk_scene) {
        CPE_ERROR(
            module->m_em, "create scene at %s: src not scene!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    if (ui_data_src_product(src)) {
        CPE_ERROR(
            module->m_em, "create scene at %s: product already loaded!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    scene = (plugin_chipmunk_data_scene_t)mem_alloc(module->m_alloc, sizeof(struct plugin_chipmunk_data_scene));
    if (scene == NULL) {
        CPE_ERROR(
            module->m_em, "create scene at %s: alloc scene fail!",
            ui_data_src_path_dump(&module->m_dump_buffer, src));
        return NULL;
    }

    scene->m_module = module;
    scene->m_src = src;
    scene->m_body_count = 0;
    TAILQ_INIT(&scene->m_body_list);
    scene->m_constraint_count = 0;
    TAILQ_INIT(&scene->m_constraint_list);

    ui_data_src_set_product(src, scene);

    return scene;
}

void plugin_chipmunk_data_scene_free(plugin_chipmunk_data_scene_t scene) {
    plugin_chipmunk_module_t module = scene->m_module;

    while(!TAILQ_EMPTY(&scene->m_constraint_list)) {
        plugin_chipmunk_data_constraint_free(TAILQ_FIRST(&scene->m_constraint_list));
    }

    while(!TAILQ_EMPTY(&scene->m_body_list)) {
        plugin_chipmunk_data_body_free(TAILQ_FIRST(&scene->m_body_list));
    }

    mem_free(module->m_alloc, scene);
}

CHIPMUNK_SCENE * plugin_chipmunk_data_scene_data(plugin_chipmunk_data_scene_t scene) {
    return &scene->m_data;
}

uint32_t plugin_chipmunk_data_scene_body_count(plugin_chipmunk_data_scene_t scene) {
    return scene->m_body_count;
}

static plugin_chipmunk_data_body_t plugin_chipmunk_data_scene_body_next(struct plugin_chipmunk_data_body_it * it) {
    plugin_chipmunk_data_body_t * data = (plugin_chipmunk_data_body_t *)(it->m_data);
    plugin_chipmunk_data_body_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_scene);

    return r;
}

void plugin_chipmunk_data_scene_bodies(plugin_chipmunk_data_body_it_t body_it, plugin_chipmunk_data_scene_t scene) {
    *(plugin_chipmunk_data_body_t *)(body_it->m_data) = TAILQ_FIRST(&scene->m_body_list);
    body_it->next = plugin_chipmunk_data_scene_body_next;
}

uint32_t plugin_chipmunk_data_scene_constraint_count(plugin_chipmunk_data_scene_t scene) {
    return scene->m_constraint_count;
}

static plugin_chipmunk_data_constraint_t plugin_chipmunk_data_scene_constraint_next(struct plugin_chipmunk_data_constraint_it * it) {
    plugin_chipmunk_data_constraint_t * data = (plugin_chipmunk_data_constraint_t *)(it->m_data);
    plugin_chipmunk_data_constraint_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_scene);

    return r;
}

void plugin_chipmunk_data_scene_constraints(plugin_chipmunk_data_constraint_it_t constraint_it, plugin_chipmunk_data_scene_t scene) {
    *(plugin_chipmunk_data_constraint_t *)(constraint_it->m_data) = TAILQ_FIRST(&scene->m_constraint_list);
    constraint_it->next = plugin_chipmunk_data_scene_constraint_next;
}
    
#ifdef __cplusplus
}
#endif
