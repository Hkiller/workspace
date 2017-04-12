#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "plugin_chipmunk_data_constraint_i.h"
#include "plugin_chipmunk_data_scene_i.h"
#include "plugin_chipmunk_data_fixture_i.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_chipmunk_data_constraint_t
plugin_chipmunk_data_constraint_create(plugin_chipmunk_data_scene_t data_scene) {
    plugin_chipmunk_module_t module = data_scene->m_module;
    plugin_chipmunk_data_constraint_t data_constraint;

    data_constraint = (plugin_chipmunk_data_constraint_t)mem_alloc(module->m_alloc, sizeof(struct plugin_chipmunk_data_constraint));
    if(data_constraint == NULL) {
        CPE_ERROR(module->m_em, "create constraint proto: alloc fail!");
        return NULL;
    }

    bzero(data_constraint, sizeof(*data_constraint));
    data_constraint->m_scene = data_scene;
        
    data_scene->m_constraint_count++;
    TAILQ_INSERT_TAIL(&data_scene->m_constraint_list, data_constraint, m_next_for_scene);

    return data_constraint;
}

void plugin_chipmunk_data_constraint_free(plugin_chipmunk_data_constraint_t data_constraint) {
    plugin_chipmunk_data_scene_t data_scene = data_constraint->m_scene;
    plugin_chipmunk_module_t module = data_scene->m_module;

    data_scene->m_constraint_count--;
    TAILQ_REMOVE(&data_scene->m_constraint_list, data_constraint, m_next_for_scene);

    mem_free(module->m_alloc, data_constraint);
}
    
CHIPMUNK_CONSTRAINT * plugin_chipmunk_data_constraint_data(plugin_chipmunk_data_constraint_t data_constraint) {
    return &data_constraint->m_data;
}

#ifdef __cplusplus
}
#endif
