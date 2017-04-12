#ifndef UI_DATA_PARTICLE_INTERNAL_H
#define UI_DATA_PARTICLE_INTERNAL_H
#include "render/model/ui_data_src.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin_particle_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(plugin_particle_data_emitter_list, plugin_particle_data_emitter) plugin_particle_data_emitter_list_t;
typedef TAILQ_HEAD(plugin_particle_data_mod_list, plugin_particle_data_mod) plugin_particle_data_mod_list_t;

struct plugin_particle_data {
    plugin_particle_module_t m_module;
    ui_data_src_t m_src;
    uint16_t m_curve_max_id;
    uint16_t m_emitter_count;
    plugin_particle_data_emitter_list_t m_emitters;
    struct cpe_hash_table m_curves;
};

struct plugin_particle_data_emitter {
    plugin_particle_data_t m_particle;
    TAILQ_ENTRY(plugin_particle_data_emitter) m_next_for_particle;
    uint32_t m_mod_count;
    plugin_particle_data_mod_list_t m_mods;
    UI_PARTICLE_EMITTER m_data;
};

struct plugin_particle_data_mod {
    plugin_particle_data_emitter_t m_emitter;
    TAILQ_ENTRY(plugin_particle_data_mod) m_next_for_emitter;
    UI_PARTICLE_MOD m_data;
};

struct plugin_particle_data_curve {
    plugin_particle_data_t m_particle;
    struct cpe_hash_entry m_hh;
    uint16_t m_id;
    uint16_t m_point_count;
    uint16_t m_point_capacity;
    uint8_t m_points_external;
    UI_CURVE_POINT * m_points;
    UI_CURVE_POINT m_point_buf[4];
};
    
int plugin_particle_data_bin_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);
int plugin_particle_data_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em);
int plugin_particle_data_bin_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em);

int plugin_particle_data_update_using(ui_data_src_t src);

/*curve operations*/
void plugin_particle_data_curve_free_all(plugin_particle_data_t particle);
    
uint32_t plugin_particle_data_curve_hash(plugin_particle_data_curve_t curve);
int plugin_particle_data_curve_eq(plugin_particle_data_curve_t l, plugin_particle_data_curve_t r);
float plugin_particle_data_curve_sample(plugin_particle_data_curve_t chanel, float key);
    
#ifdef __cplusplus
}
#endif

#endif
