#ifndef UI_PLUGIN_PARTICLE_DATA_H
#define UI_PLUGIN_PARTICLE_DATA_H
#include "protocol/render/model/ui_particle.h"
#include "protocol/render/model/ui_particle_mod.h"
#include "plugin_particle_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_particle_data_mod_it {
    plugin_particle_data_mod_t (*next)(struct plugin_particle_data_mod_it * it);
    char m_data[64];
};

struct plugin_particle_data_emitter_it {
    plugin_particle_data_emitter_t (*next)(struct plugin_particle_data_emitter_it * it);
    char m_data[64];
};

/*particle*/
plugin_particle_data_t plugin_particle_data_create(plugin_particle_module_t module, ui_data_src_t src);
void plugin_particle_data_free(plugin_particle_data_t particle);
uint32_t plugin_particle_data_emitter_count(plugin_particle_data_t particle);
void plugin_particle_data_emitters(plugin_particle_data_emitter_it_t it, plugin_particle_data_t particle);

/*particle_emitter*/
plugin_particle_data_emitter_t plugin_particle_data_emitter_create(plugin_particle_data_t particle);
void plugin_particle_data_emitter_free(plugin_particle_data_emitter_t emitter);

plugin_particle_data_emitter_t plugin_particle_data_emitter_find(plugin_particle_data_t particle, const char * emitter_name);

UI_PARTICLE_EMITTER * plugin_particle_data_emitter_data(plugin_particle_data_emitter_t emitter);
LPDRMETA plugin_particle_data_emitter_meta(plugin_particle_module_t module);
uint32_t plugin_particle_data_emitter_mod_count(plugin_particle_data_emitter_t emitter);
void plugin_particle_data_emitter_mods(plugin_particle_data_mod_it_t it, plugin_particle_data_emitter_t emitter);
const char * plugin_particle_data_emitter_msg(plugin_particle_data_emitter_t emitter, uint32_t msg_id);
    
#define plugin_particle_data_emitter_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*particle_mod*/
plugin_particle_data_mod_t plugin_particle_data_mod_create(plugin_particle_data_emitter_t emitter);
void plugin_particle_data_mod_free(plugin_particle_data_mod_t particle_mod);
 
UI_PARTICLE_MOD * plugin_particle_data_mod_data(plugin_particle_data_mod_t particle_mod);
LPDRMETA plugin_particle_data_mod_meta(plugin_particle_module_t module);

#define plugin_particle_data_mod_it_next(it) ((it)->next ? (it)->next(it) : NULL)

/*particle_curve*/
plugin_particle_data_curve_t plugin_particle_data_curve_create(plugin_particle_data_t particle, uint16_t curve_id);
void plugin_particle_data_curve_free(plugin_particle_data_curve_t particle_curve);
plugin_particle_data_curve_t plugin_particle_data_curve_find(plugin_particle_data_t particle, uint16_t curve_id);
    
uint16_t plugin_particle_data_curve_id(plugin_particle_data_curve_t particle_curve);
uint16_t plugin_particle_data_curve_point_count(plugin_particle_data_curve_t particle_curve);
UI_CURVE_POINT * plugin_particle_data_curve_point_at(plugin_particle_data_curve_t particle_curve, uint16_t pos);
UI_CURVE_POINT * plugin_particle_data_curve_point_append(plugin_particle_data_curve_t particle_curve);
    
/*collect*/
int plugin_particle_data_collect_res(ui_cache_group_t group, ui_data_src_t src, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
