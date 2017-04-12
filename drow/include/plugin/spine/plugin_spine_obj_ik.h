#ifndef PLUGIN_SPINE_OBJ_IK_H
#define PLUGIN_SPINE_OBJ_IK_H
#include "render/model/ui_model_types.h"
#include "plugin_spine_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_spine_obj_ik_it {
    plugin_spine_obj_ik_t (*next)(struct plugin_spine_obj_ik_it * it);
    char m_data[64];
};
    
plugin_spine_obj_ik_t plugin_spine_obj_ik_create(plugin_spine_obj_t obj, struct spIkConstraint * ik);
void plugin_spine_obj_ik_free(plugin_spine_obj_ik_t ik);

plugin_spine_obj_ik_t plugin_spine_obj_ik_find(plugin_spine_obj_t obj, const char * name);

const char * plugin_spine_obj_ik_name(plugin_spine_obj_ik_t ik);

void plugin_spine_obj_ik_set_target_pos(plugin_spine_obj_ik_t ik, ui_vector_2_t pos);
void plugin_spine_obj_ik_restore(plugin_spine_obj_ik_t ik);
    
struct spIkConstraint * plugin_spine_obj_ik_ik(plugin_spine_obj_ik_t ik);
uint8_t plugin_spine_obj_ik_is_runing(plugin_spine_obj_ik_t ik);
    
void plugin_spine_obj_iks(plugin_spine_obj_t obj, plugin_spine_obj_ik_it_t it);

#define plugin_spine_obj_ik_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
