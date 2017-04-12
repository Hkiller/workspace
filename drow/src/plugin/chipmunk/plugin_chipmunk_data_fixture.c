#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "plugin_chipmunk_data_fixture_i.h"
#include "plugin_chipmunk_data_body_i.h"
#include "plugin_chipmunk_data_polygon_node_i.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_data_fixture_t
plugin_chipmunk_data_fixture_create(plugin_chipmunk_data_body_t data_body) {
    plugin_chipmunk_module_t module = data_body->m_scene->m_module;
    plugin_chipmunk_data_fixture_t data_fixture;

    data_fixture = (plugin_chipmunk_data_fixture_t)mem_alloc(module->m_alloc, sizeof(struct plugin_chipmunk_data_fixture));
    if(data_fixture == NULL) {
        CPE_ERROR(module->m_em, "create fixture proto: alloc fail!");
        return NULL;
    }

    bzero(data_fixture, sizeof(*data_fixture));
    data_fixture->m_body = data_body;

    TAILQ_INIT(&data_fixture->m_polygon_node_list);

    data_body->m_fixture_count++;
    TAILQ_INSERT_TAIL(&data_body->m_fixture_list, data_fixture, m_next_for_body);

    return data_fixture;
}

void plugin_chipmunk_data_fixture_free(plugin_chipmunk_data_fixture_t data_fixture) {
    plugin_chipmunk_data_body_t data_body = data_fixture->m_body;
    plugin_chipmunk_module_t module = data_body->m_scene->m_module;

    while(!TAILQ_EMPTY(&data_fixture->m_polygon_node_list)) {
        plugin_chipmunk_data_polygon_node_free(TAILQ_FIRST(&data_fixture->m_polygon_node_list));
    }

    data_body->m_fixture_count--;
    TAILQ_REMOVE(&data_body->m_fixture_list, data_fixture, m_next_for_body);

    mem_free(module->m_alloc, data_fixture);
}

CHIPMUNK_FIXTURE * plugin_chipmunk_data_fixture_data(plugin_chipmunk_data_fixture_t data_fixture) {
    return &data_fixture->m_data;
}

uint32_t plugin_chipmunk_data_fixture_polygon_node_count(plugin_chipmunk_data_fixture_t fixture) {
    return fixture->m_polygon_node_count;
}

#ifdef __cplusplus
}
#endif
