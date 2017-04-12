#ifndef PLUGIN_BARRAGE_BULLET_PROTO_I_H
#define PLUGIN_BARRAGE_BULLET_PROTO_I_H
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "plugin_barrage_group_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_barrage_bullet_proto {
    plugin_barrage_group_t m_group;
    const char * m_name;
    struct cpe_hash_entry m_hh;
    plugin_particle_obj_emitter_t m_emitter;
};

plugin_barrage_bullet_proto_t plugin_barrage_bullet_proto_create(plugin_barrage_group_t env, const char * name);
plugin_barrage_bullet_proto_t plugin_barrage_bullet_proto_find(plugin_barrage_group_t env, const char * name);
void plugin_barrage_bullet_proto_free(plugin_barrage_bullet_proto_t proto);

void plugin_barrage_bullet_proto_free_all(plugin_barrage_group_t env);

uint32_t plugin_barrage_bullet_proto_hash(const plugin_barrage_bullet_proto_t proto);
int plugin_barrage_bullet_proto_eq(const plugin_barrage_bullet_proto_t l, const plugin_barrage_bullet_proto_t r);
    
#ifdef __cplusplus
}
#endif

#endif
