#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "plugin_chipmunk_data_body_i.h"
#include "plugin_chipmunk_data_scene_i.h"
#include "plugin_chipmunk_data_fixture_i.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_data_body_t
plugin_chipmunk_data_body_create(plugin_chipmunk_data_scene_t data_scene) {
    plugin_chipmunk_module_t module = data_scene->m_module;
    plugin_chipmunk_data_body_t data_body;

    data_body = (plugin_chipmunk_data_body_t)mem_alloc(module->m_alloc, sizeof(struct plugin_chipmunk_data_body));
    if(data_body == NULL) {
        CPE_ERROR(module->m_em, "create body proto: alloc fail!");
        return NULL;
    }

    bzero(data_body, sizeof(*data_body));
    data_body->m_scene = data_scene;
    TAILQ_INIT(&data_body->m_fixture_list);
        
    data_scene->m_body_count++;
    TAILQ_INSERT_TAIL(&data_scene->m_body_list, data_body, m_next_for_scene);

    return data_body;
}

void plugin_chipmunk_data_body_free(plugin_chipmunk_data_body_t data_body) {
    plugin_chipmunk_data_scene_t data_scene = data_body->m_scene;
    plugin_chipmunk_module_t module = data_scene->m_module;

    while(!TAILQ_EMPTY(&data_body->m_fixture_list)) {
        plugin_chipmunk_data_fixture_free(TAILQ_FIRST(&data_body->m_fixture_list));
    }

    data_scene->m_body_count--;
    TAILQ_REMOVE(&data_scene->m_body_list, data_body, m_next_for_scene);

    mem_free(module->m_alloc, data_body);
}

plugin_chipmunk_data_body_t
plugin_chipmunk_data_body_find_by_name(plugin_chipmunk_data_scene_t scene, const char * name) {
    plugin_chipmunk_data_body_t r;

    TAILQ_FOREACH(r, &scene->m_body_list, m_next_for_scene) {
        if (strcmp(r->m_data.name, name) == 0) return r;
    }

    return NULL;
}
    
CHIPMUNK_BODY * plugin_chipmunk_data_body_data(plugin_chipmunk_data_body_t data_body) {
    return &data_body->m_data;
}

uint32_t plugin_chipmunk_data_body_fixture_count(plugin_chipmunk_data_body_t body) {
    return body->m_fixture_count;
}

static plugin_chipmunk_data_fixture_t plugin_chipmunk_data_body_fixture_next(struct plugin_chipmunk_data_fixture_it * it) {
    plugin_chipmunk_data_fixture_t * data = (plugin_chipmunk_data_fixture_t *)(it->m_data);
    plugin_chipmunk_data_fixture_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_body);

    return r;
}

void plugin_chipmunk_data_body_fixtures(plugin_chipmunk_data_fixture_it_t fixture_it, plugin_chipmunk_data_body_t body) {
    *(plugin_chipmunk_data_fixture_t *)(fixture_it->m_data) = TAILQ_FIRST(&body->m_fixture_list);
    fixture_it->next = plugin_chipmunk_data_body_fixture_next;
}

#ifdef __cplusplus
}
#endif
