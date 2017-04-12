#ifndef PLUGIN_BARRAGE_GROUP_I_H
#define PLUGIN_BARRAGE_GROUP_I_H
#include "plugin/barrage/plugin_barrage_group.h"
#include "plugin_barrage_env_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_group {
    plugin_barrage_env_t m_env;
    char m_name[64];
    TAILQ_ENTRY(plugin_barrage_group) m_next_for_env;

    plugin_particle_obj_t m_particle_obj;

    struct cpe_hash_table m_bullet_protos;
    plugin_barrage_barrage_list_t m_barrages;
    struct plugin_barrage_bullet_list m_bullets;
};

int plugin_barrage_calc_value(
    float * result, uint8_t calc_type, plugin_barrage_emitter_t emitter, plugin_barrage_bullet_t bullet, error_monitor_t em);
    
#ifdef __cplusplus
}
#endif

#endif
