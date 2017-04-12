#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin/particle/plugin_particle_obj.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "plugin/particle/plugin_particle_obj_plugin.h"
#include "plugin/particle/plugin_particle_obj_plugin_data.h"
#include "plugin_scrollmap_team_i.h"
#include "plugin_scrollmap_obj_i.h"
#include "plugin_scrollmap_env_i.h"
#include "plugin_scrollmap_obj_type_map_i.h"

struct plugin_scrollmap_team_particle_slot {
    plugin_scrollmap_team_t m_team;
    plugin_scrollmap_obj_t m_obj;
};
typedef struct plugin_scrollmap_team_particle_slot * plugin_scrollmap_team_particle_slot_t;
static int plugin_scrollmap_team_particle_slot_init(void * ctx, plugin_particle_obj_plugin_data_t data);
static void plugin_scrollmap_team_particle_slot_fini(void * ctx, plugin_particle_obj_plugin_data_t data);
static void plugin_scrollmap_team_particle_slot_update(void * ctx, plugin_particle_obj_plugin_data_t data);

int plugin_scrollmap_team_init_from_particle(plugin_scrollmap_team_t team, ui_data_src_t src, plugin_particle_data_t particle_data) {
    plugin_scrollmap_module_t module = team->m_env->m_module;
    ui_runtime_render_obj_t render_obj;
    plugin_particle_obj_t particle_obj;
    plugin_particle_obj_emitter_t particle_emitter;
    struct plugin_particle_obj_emitter_it emitter_it;

    render_obj = ui_runtime_render_obj_create_by_type(module->m_runtime, NULL, "particle");
    if (render_obj == NULL) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: create particle obj fail!");
        return -1;
    }
    ui_runtime_render_obj_set_src(render_obj, src);

    particle_obj = ui_runtime_render_obj_data(render_obj);
    if (plugin_particle_obj_set_data(particle_obj, particle_data) != 0) {
        CPE_ERROR(module->m_em, "plugin_scrollmap_team_create: set particle obj fail!");
        ui_runtime_render_obj_free(render_obj);
        return -1;
    }
    
    team->m_type = plugin_scrollmap_team_type_particle;
    team->m_particle.m_obj = particle_obj;

    plugin_particle_obj_emitters(&emitter_it, particle_obj);
    while((particle_emitter = plugin_particle_obj_emitter_it_next(&emitter_it))) {
        plugin_particle_obj_plugin_t plugin;

        if (plugin_particle_obj_emitter_use_state(particle_emitter) != plugin_particle_obj_emitter_use_state_active) continue;

        plugin =
            plugin_particle_obj_plugin_create(
                particle_emitter,
                team,
                sizeof(struct plugin_scrollmap_team_particle_slot),
                plugin_scrollmap_team_particle_slot_init,
                plugin_scrollmap_team_particle_slot_fini,
                plugin_scrollmap_team_particle_slot_update);
        
        if (plugin == NULL) {
            ui_runtime_render_obj_free(render_obj);
            return -1;
        }
    }
    
    return 0;
}

void plugin_scrollmap_team_update_particle(plugin_scrollmap_team_t team, float delta) {
    ui_runtime_render_obj_t render_obj = ui_runtime_render_obj_from_data(team->m_particle.m_obj);
    
    ui_runtime_render_obj_update(render_obj, delta);
    if (!ui_runtime_render_obj_is_playing(render_obj)) {
        plugin_scrollmap_team_free(team);
    }
}

void plugin_scrollmap_team_fini_particle(plugin_scrollmap_team_t team) {
    assert(team->m_particle.m_obj);
    ui_runtime_render_obj_free(ui_runtime_render_obj_from_data(team->m_particle.m_obj));
    team->m_particle.m_obj = NULL;
}

int plugin_scrollmap_team_particle_slot_init(void * ctx, plugin_particle_obj_plugin_data_t data) {
    plugin_scrollmap_team_t team = ctx;
    plugin_scrollmap_team_particle_slot_t slot = (plugin_scrollmap_team_particle_slot_t)plugin_particle_obj_plugin_data_data(data);
    plugin_particle_obj_emitter_t emitter = plugin_particle_obj_plugin_emitter(plugin_particle_obj_plugin_data_plugin(data));
    const char * emitter_name = plugin_particle_obj_emitter_name(emitter);
    const char * proto_name;
    char * args;
    char buf[128];
    plugin_particle_obj_particle_t particle;
    plugin_scrollmap_obj_type_map_t type_map;

    proto_name = plugin_scrollmap_obj_analize_name(emitter_name, buf, sizeof(buf), &args);
    if (proto_name == NULL) {
        CPE_ERROR(team->m_env->m_module->m_em, "plugin_scrollmap_team: particle: analize name %s fail!", emitter_name);
        return -1;
    }
    
    particle = plugin_particle_obj_plugin_data_particle(data);
    
    slot->m_team = team;

    /*开始创建对象 */
    slot->m_obj = plugin_scrollmap_obj_create(team->m_env, team->m_layer, NULL);
    if (slot->m_obj == NULL) {
        CPE_ERROR(team->m_env->m_module->m_em, "plugin_scrollmap_team: particle: create obj fail!");
        return -1;
    }

    if (plugin_scrollmap_obj_set_move_by_particle_team(slot->m_obj, team, particle) != 0) {
        CPE_ERROR(team->m_env->m_module->m_em, "plugin_scrollmap_team: particle: set move by particle fail!");
        plugin_scrollmap_obj_free(slot->m_obj);
        slot->m_obj = NULL;
        return -1;
    }

    type_map = plugin_scrollmap_obj_type_map_find_on_team(team, proto_name);
    if (plugin_scrollmap_obj_do_create(slot->m_obj, type_map ? type_map->m_to_type : proto_name, args) != 0) {
        CPE_ERROR(
            team->m_env->m_module->m_em, "plugin_scrollmap_team: particle: create from proto %s fail!",
            proto_name);
        plugin_scrollmap_obj_free(slot->m_obj);
        return -1;
    }
    
    return 0;
}

void plugin_scrollmap_team_particle_slot_fini(void * ctx, plugin_particle_obj_plugin_data_t data) {
    plugin_scrollmap_team_particle_slot_t slot = (plugin_scrollmap_team_particle_slot_t)plugin_particle_obj_plugin_data_data(data);

    if (slot->m_obj) {
        assert(slot->m_obj->m_move_state == plugin_scrollmap_obj_move_by_team);
        assert(slot->m_obj->m_move_by_team.m_team->m_type == plugin_scrollmap_team_type_particle);
        slot->m_obj->m_move_by_team.m_particle = NULL;
        plugin_scrollmap_obj_free(slot->m_obj);
        slot->m_obj = NULL;
    }
}

static void plugin_scrollmap_team_update_particle_member(plugin_scrollmap_team_t team, plugin_scrollmap_obj_t member) {
    plugin_particle_obj_particle_t particle;
    struct ui_transform particle_transform;
    
    particle = member->m_move_by_team.m_particle;
    plugin_particle_obj_particle_calc_transform(particle, &particle_transform);

    if (!member->m_accept_scale) {
        ui_transform_set_scale(&particle_transform, &UI_VECTOR_3_IDENTITY);
    }

    if (!member->m_accept_angle) {
        ui_transform_set_quation(&particle_transform, &UI_QUATERNION_IDENTITY);
    }

    if (ui_transform_cmp(&team->m_transform, &UI_TRANSFORM_IDENTITY) != 0) {
        ui_transform_adj_by_parent(&particle_transform, &team->m_transform);
    }
    
    member->m_transform = particle_transform;
    member->m_env->m_obj_on_update(member->m_env->m_obj_factory_ctx, member);
}

void plugin_scrollmap_team_particle_slot_update(void * ctx, plugin_particle_obj_plugin_data_t data) {
    plugin_scrollmap_team_particle_slot_t slot = plugin_particle_obj_plugin_data_data(data);

    if (slot->m_obj == NULL) return;

    assert(slot->m_obj->m_move_state == plugin_scrollmap_obj_move_by_team);
    assert(slot->m_obj->m_is_created);

    plugin_scrollmap_team_update_particle_member(slot->m_team, slot->m_obj);
}

void plugin_scrollmap_team_particle_slot_free(plugin_scrollmap_team_particle_slot_t slot) {
    plugin_particle_obj_plugin_data_t plugin_data = plugin_particle_obj_plugin_data_from_data((void*)slot);
    plugin_particle_obj_particle_t particle = plugin_particle_obj_plugin_data_particle(plugin_data);

    plugin_particle_obj_particle_free(particle);
}

int plugin_scrollmap_obj_set_move_by_particle_team(
    plugin_scrollmap_obj_t obj, plugin_scrollmap_team_t team, plugin_particle_obj_particle_t particle)
{
    assert(team->m_type == plugin_scrollmap_team_type_particle);

    if (obj->m_move_state != plugin_scrollmap_obj_move_free) {
        plugin_scrollmap_obj_set_move_free(obj);
    }

    obj->m_move_state = plugin_scrollmap_obj_move_by_team;
    obj->m_move_by_team.m_team = team;
    obj->m_move_by_team.m_particle = particle;
    TAILQ_INSERT_TAIL(&team->m_members, obj, m_move_by_team.m_next);

    plugin_scrollmap_team_update_particle_member(team, obj);
    
    return 0;
}

void plugin_scrollmap_obj_unset_move_by_particle_team(plugin_scrollmap_obj_t obj) {
    plugin_particle_obj_plugin_data_t plugin_data;
    plugin_scrollmap_team_particle_slot_t slot;
            
    if (obj->m_move_by_team.m_particle) {
        plugin_data = plugin_particle_obj_plugin_data_find_by_ctx(obj->m_move_by_team.m_particle, obj->m_move_by_team.m_team);
        assert(plugin_data);
        slot = (plugin_scrollmap_team_particle_slot_t)plugin_particle_obj_plugin_data_data(plugin_data);
        slot->m_obj = NULL;

        plugin_particle_obj_particle_free(obj->m_move_by_team.m_particle);
        obj->m_move_by_team.m_particle = NULL;
    }
}
