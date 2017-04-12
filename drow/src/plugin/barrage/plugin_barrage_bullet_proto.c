#include "render/model/ui_data_src.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "plugin_barrage_bullet_proto_i.h"

plugin_barrage_bullet_proto_t
plugin_barrage_bullet_proto_create(plugin_barrage_group_t group, const char * name) {
    plugin_barrage_module_t module = group->m_env->m_module;
    plugin_barrage_bullet_proto_t proto;
    size_t name_len = strlen(name) + 1;
    ui_data_src_t particle_src;
    plugin_particle_data_t particle;
    plugin_particle_data_emitter_t data_emitter;
    UI_PARTICLE_EMITTER * data_emitter_data;
    const char * dead_anim;
    plugin_particle_obj_emitter_t dead_emitter = NULL;
    char * res;
    char * sep;
    
    proto = (plugin_barrage_bullet_proto_t)mem_alloc(module->m_alloc, sizeof(struct plugin_barrage_bullet_proto) + name_len);
    if (proto == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: alloc fail!");
        return NULL;
    }

    memcpy(proto + 1, name, name_len);
    res = (char*)(proto + 1);

    sep = strrchr(res, '#');
    if (sep == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: proto path %s format error!", name);
        mem_free(module->m_alloc, proto);
        return NULL;
    }

    *sep = 0;

    particle_src = ui_data_src_find_by_path(module->m_data_mgr, res, ui_data_src_type_particle);
    if (particle_src == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: particle %s not exist!", res);
        mem_free(module->m_alloc, proto);
        return NULL;
    }

    particle = (plugin_particle_data_t)ui_data_src_product(particle_src);
    if (particle == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: particle %s not loaded!", res);
        mem_free(module->m_alloc, proto);
        return NULL;
    }

    data_emitter = plugin_particle_data_emitter_find(particle, sep + 1);
    if (data_emitter == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: particle %s no emitter %s!", res, sep + 1);
        mem_free(module->m_alloc, proto);
        return NULL;
    }

    *sep = '#';
    
    proto->m_group = group;
    proto->m_name = (char*)(proto + 1);
    proto->m_emitter = plugin_particle_obj_emitter_create(group->m_particle_obj, data_emitter);
    if (proto->m_emitter == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: create emitter fail!");
        mem_free(module->m_alloc, proto);
        return NULL;
    }

    data_emitter_data = plugin_particle_obj_emitter_data_w(proto->m_emitter);
    if (data_emitter_data == NULL) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: get emitter data fail!");
        plugin_particle_obj_emitter_free(proto->m_emitter);
        mem_free(module->m_alloc, proto);
        return NULL;
    }
    plugin_particle_obj_emitter_set_use_state(proto->m_emitter, plugin_particle_obj_emitter_use_state_passive);
    data_emitter_data->particle_repeat_times = 0;

    /*创建消弹发射器 */
    dead_anim = plugin_particle_obj_emitter_dead_anim(proto->m_emitter);
    if (dead_anim[0]) {
        if (plugin_particle_obj_emitter_find(group->m_particle_obj, dead_anim) == NULL) {
            plugin_particle_data_emitter_t data_dead_emitter;
            
            data_dead_emitter = plugin_particle_data_emitter_find(particle, dead_anim);
            if (data_dead_emitter == NULL) {
                CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: particle %s no dead emitter %s!", res, dead_anim);
                plugin_particle_obj_emitter_free(proto->m_emitter);
                mem_free(module->m_alloc, proto);
                return NULL;
            }

            dead_emitter = plugin_particle_obj_emitter_create(group->m_particle_obj, data_dead_emitter);
            if (dead_emitter == NULL) {
                CPE_ERROR(
                    module->m_em, "plugin_barrage_bullet_proto_create: particle %s create dead emitter %s fail!",
                    res, dead_anim);
                plugin_particle_obj_emitter_free(proto->m_emitter);
                mem_free(module->m_alloc, proto);
                return NULL;
            }

            plugin_particle_obj_emitter_set_use_state(dead_emitter, plugin_particle_obj_emitter_use_state_passive);
        }

    }
    
    cpe_hash_entry_init(&proto->m_hh);
    if (cpe_hash_table_insert_unique(&group->m_bullet_protos, proto) != 0) {
        CPE_ERROR(module->m_em, "plugin_barrage_bullet_proto_create: name %s duplicate!", name);
        if (dead_emitter) plugin_particle_obj_emitter_free(dead_emitter);
        plugin_particle_obj_emitter_free(proto->m_emitter);
        mem_free(module->m_alloc, proto);
        return NULL;
    }
    
    return proto;
}

void plugin_barrage_bullet_proto_free(plugin_barrage_bullet_proto_t proto) {
    plugin_barrage_group_t group = proto->m_group;

    plugin_particle_obj_emitter_free(proto->m_emitter);

    cpe_hash_table_remove_by_ins(&group->m_bullet_protos, proto);

    mem_free(group->m_env->m_module->m_alloc, proto);
}

plugin_barrage_bullet_proto_t
plugin_barrage_bullet_proto_find(plugin_barrage_group_t group, const char * name) {
    struct plugin_barrage_bullet_proto key;

    key.m_name = name;

    return (plugin_barrage_bullet_proto_t)cpe_hash_table_find(&group->m_bullet_protos, &key);
}

void plugin_barrage_bullet_proto_free_all(plugin_barrage_group_t group) {
    struct cpe_hash_it proto_it;
    plugin_barrage_bullet_proto_t proto;

    cpe_hash_it_init(&proto_it, &group->m_bullet_protos);

    proto = (plugin_barrage_bullet_proto_t)cpe_hash_it_next(&proto_it);
    while (proto) {
        plugin_barrage_bullet_proto_t next = (plugin_barrage_bullet_proto_t)cpe_hash_it_next(&proto_it);
        plugin_barrage_bullet_proto_free(proto);
        proto = next;
    }
}

uint32_t plugin_barrage_bullet_proto_hash(const plugin_barrage_bullet_proto_t proto) {
    return cpe_hash_str(proto->m_name, strlen(proto->m_name));
}

int plugin_barrage_bullet_proto_eq(const plugin_barrage_bullet_proto_t l, const plugin_barrage_bullet_proto_t r) {
    return strcmp(l->m_name, r->m_name) == 0;
}

