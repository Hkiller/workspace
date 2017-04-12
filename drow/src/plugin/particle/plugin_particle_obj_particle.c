#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin_particle_obj_particle_i.h"
#include "plugin_particle_obj_emitter_mod_i.h"
#include "plugin_particle_obj_mod_data_i.h"
#include "plugin_particle_obj_plugin_data_i.h"
#include "plugin_particle_obj_emitter_binding_i.h"

static void plugin_particle_obj_particle_create_emitter_binding(void * ctx, const char * value) {
    plugin_particle_obj_particle_t particle = ctx;
    plugin_particle_obj_t obj = particle->m_emitter->m_obj;
    const char * args_begin;
    const char * args_end = NULL;
    const char * emitter_name;
    char buf[32];
    plugin_particle_obj_emitter_t bind_emitter;
    uint8_t is_tie = 0;
    plugin_particle_obj_emitter_binding_t binding;
    
    if (value[0] == '+') {
        is_tie = 1;
        value += 1;
    }

    if ((args_begin = strchr(value, '['))) {
        emitter_name = cpe_str_dup_range(buf, sizeof(buf), value, args_begin);
        if (emitter_name == NULL) {
            CPE_ERROR(
                particle->m_emitter->m_obj->m_module->m_em,
                "plugin_particle_obj_particle_trigger_particle: emitter %s not exist!", emitter_name);
            return;
        }

        args_end = strchr(args_begin + 1, ']');
        if (args_end == NULL) {
            CPE_ERROR(
                particle->m_emitter->m_obj->m_module->m_em,
                "plugin_particle_obj_particle_create_emitter_binding: emitter def %s format error!", value);
            args_begin = NULL;
        }
        else {
            args_begin++;
        }
    }
    else {
        emitter_name = value;
    }

    
    bind_emitter = plugin_particle_obj_emitter_find(obj, emitter_name);
    if (bind_emitter == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_particle_obj_particle_create: bind emitter %s not exist!", emitter_name);
        return;
    }

    if (bind_emitter->m_use_state == plugin_particle_obj_emitter_use_state_suspend) return;
    
    binding = plugin_particle_obj_emitter_binding_create(bind_emitter, particle);
    if (binding == NULL) {
        CPE_ERROR(obj->m_module->m_em, "plugin_particle_obj_particle_create: bind emitter %s create fail!", value);
        return;
    }

    binding->m_is_tie = is_tie;

    if (args_begin) {
        if (cpe_str_read_arg_range(buf, sizeof(buf), args_begin, args_end, "angle", ',', '=') == 0) {
            binding->m_accept_angle = atoi(buf);
        }

        if (cpe_str_read_arg_range(buf, sizeof(buf), args_begin, args_end, "scale", ',', '=') == 0) {
            binding->m_accept_scale = atoi(buf);
        }
    }
}

plugin_particle_obj_particle_t
plugin_particle_obj_particle_create_at(plugin_particle_obj_emitter_t emitter, ui_transform_t transform) {
    plugin_particle_module_t module = emitter->m_obj->m_module;
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);
    plugin_particle_obj_particle_t particle;

    particle = TAILQ_FIRST(&module->m_free_particles);
    if (particle == NULL) {
        particle = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_obj_particle));
        if (particle == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_obj_particle_create: alloc fail!");
            return NULL;
        }
    }
    else {
        module->m_free_particle_count--;
        TAILQ_REMOVE(&module->m_free_particles, particle, m_next);
    }

    bzero(particle, sizeof(*particle));

    particle->m_time_scale = 1.0f;
    particle->m_location = UI_VECTOR_2_ZERO;
    particle->m_emitter = emitter;
    particle->m_base_color  = 0xFFFFFFFF;
    particle->m_base_size.x = 1.0f;
    particle->m_base_size.y = 1.0f;
    particle->m_repeat_count = emitter_data->particle_repeat_times;
    cpe_assert_float_sane(particle->m_spin_init);
    cpe_assert_float_sane(particle->m_moved_distance);

    if (transform) {
        cpe_assert_float_sane(transform->m_s.x);
        cpe_assert_float_sane(transform->m_s.y);
        
        particle->m_spawn_scale.x = fabs(transform->m_s.x);
        particle->m_spawn_scale.y = fabs(transform->m_s.y);
    }
    else {
        particle->m_spawn_scale = UI_VECTOR_2_IDENTITY;
    }
    
    TAILQ_INIT(&particle->m_plugin_datas);
    TAILQ_INIT(&particle->m_binding_emitters);

    particle->m_follow_to = NULL;
    particle->m_follow_is_tie = 0;
    particle->m_follow_angle = 0;
    particle->m_follow_scale = 0;
    TAILQ_INIT(&particle->m_follow_particles);
    
    if (!TAILQ_EMPTY(&emitter->m_mods)) {
        plugin_particle_obj_emitter_mod_t mod;
        plugin_particle_obj_mod_data_t cur_mod_data = TAILQ_FIRST(&emitter->m_mod_datas);
        
        TAILQ_FOREACH(mod, &emitter->m_mods, m_next) {
            plugin_particle_obj_mod_data_t mod_data = NULL;

            assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);

            if (g_mod_defs[mod->m_data.type].data_init) {
                assert(cur_mod_data);
                mod_data = cur_mod_data;
                cur_mod_data = TAILQ_NEXT(cur_mod_data, m_next);
            }
            
            if (g_mod_defs[mod->m_data.type].particle_init) {
                g_mod_defs[mod->m_data.type].particle_init(&mod->m_data, mod_data, particle);
            }
        }
    }
    else if (emitter->m_static_data) {
        plugin_particle_data_mod_t mod;
        plugin_particle_obj_mod_data_t cur_mod_data = TAILQ_FIRST(&emitter->m_mod_datas);

        TAILQ_FOREACH(mod, &emitter->m_static_data->m_mods, m_next_for_emitter) {
            plugin_particle_obj_mod_data_t mod_data = NULL;
            assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);

            if (g_mod_defs[mod->m_data.type].data_init) {
                assert(cur_mod_data);
                mod_data = cur_mod_data;
                cur_mod_data = TAILQ_NEXT(cur_mod_data, m_next);
            }
            
            if (g_mod_defs[mod->m_data.type].particle_init) {
                g_mod_defs[mod->m_data.type].particle_init(&mod->m_data, mod_data, particle);
            }
        }
    }

    if (transform) {
        ui_transform_inline_adj_vector_2(transform, &particle->m_location);
        ui_transform_inline_adj_vector_2_no_t(transform, &particle->m_velocity);
        ui_transform_inline_adj_vector_2_no_t(transform, &particle->m_base_accel);

        cpe_assert_float_sane(particle->m_spin_init);
        particle->m_spin_init +=
            cpe_math_angle_flip_rad(
                ui_transform_calc_angle_z_rad(transform),
                transform->m_s.x < 0.0f ? 1 : 0, transform->m_s.y < 0.0f ? 1 : 0);
        cpe_assert_float_sane(particle->m_spin_init);
    }
        
    if (emitter_data->auto_up_dir) {
        plugin_particle_obj_particle_calc_auto_dir_up(particle);
    }

    emitter->m_obj->m_particle_count++;
    emitter->m_particle_count++;
    module->m_active_particle_count++;
    TAILQ_INSERT_TAIL(&emitter->m_particles, particle, m_next);

    if (!TAILQ_EMPTY(&emitter->m_plugins)) {
        plugin_particle_obj_plugin_t plugin;
        TAILQ_FOREACH(plugin, &emitter->m_plugins, m_next) {
            plugin_particle_obj_plugin_data_create(plugin, particle);
        }
    }

    if (emitter_data->bind_emitters_id) {
        cpe_str_list_for_each(
            plugin_particle_obj_emitter_msg(emitter, emitter_data->bind_emitters_id),
            ':', plugin_particle_obj_particle_create_emitter_binding, particle);
    }
    
    if (emitter_data->on_particle_begin_id) {
        ui_runtime_render_obj_send_event(
            ui_runtime_render_obj_from_data(emitter->m_obj),
            plugin_particle_obj_emitter_msg(emitter, emitter_data->on_particle_begin_id));
    }

    return particle;
}

plugin_particle_obj_particle_t
plugin_particle_obj_particle_create(plugin_particle_obj_emitter_t emitter) {
    ui_transform_t world_t;
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);    

    cpe_assert_float_sane(emitter->m_transform.m_s.x);
    cpe_assert_float_sane(emitter->m_transform.m_s.y);

    if (emitter_data->xform_mod == UI_PARTICLE_XFORM_WORLD) {
        world_t = ui_runtime_render_obj_transform(ui_runtime_render_obj_from_data(emitter->m_obj));

        cpe_assert_float_sane(world_t->m_s.x);
        cpe_assert_float_sane(world_t->m_s.y);
    }
    else {
        world_t = NULL;
    }
    
    if (emitter->m_have_transform) {
        ui_transform transform = emitter->m_transform;
        assert(ui_transform_cmp(&emitter->m_transform, &UI_TRANSFORM_IDENTITY) != 0);
        if (world_t) ui_transform_adj_by_parent(&transform, world_t);
        return plugin_particle_obj_particle_create_at(emitter, &transform);
    }
    else {
        assert(ui_transform_cmp(&emitter->m_transform, &UI_TRANSFORM_IDENTITY) == 0);
        return plugin_particle_obj_particle_create_at(emitter, world_t);
    }
}

void plugin_particle_obj_particle_free(plugin_particle_obj_particle_t particle) {
    plugin_particle_obj_emitter_t emitter = particle->m_emitter;
    plugin_particle_module_t module = emitter->m_obj->m_module;
    UI_PARTICLE_EMITTER const * emitter_data;

    if (particle->m_follow_to) {
        TAILQ_REMOVE(&particle->m_follow_to->m_follow_particles, particle, m_next_for_follow);
        particle->m_follow_to = NULL;
    }
    
    while(!TAILQ_EMPTY(&particle->m_follow_particles)) {
        plugin_particle_obj_particle_free(TAILQ_FIRST(&particle->m_follow_particles));
    }

    while(!TAILQ_EMPTY(&particle->m_plugin_datas)) {
        plugin_particle_obj_plugin_data_free(TAILQ_FIRST(&particle->m_plugin_datas));
    }

    while(!TAILQ_EMPTY(&particle->m_binding_emitters)) {
        plugin_particle_obj_emitter_binding_free(TAILQ_FIRST(&particle->m_binding_emitters));
    }

    assert(emitter->m_obj->m_particle_count > 0);
    emitter->m_obj->m_particle_count--;
    assert(emitter->m_particle_count > 0);
    emitter->m_particle_count--;
    module->m_active_particle_count--;
    module->m_free_particle_count++;
    TAILQ_REMOVE(&emitter->m_particles, particle, m_next);
    TAILQ_INSERT_TAIL(&module->m_free_particles, particle, m_next);

    emitter_data = plugin_particle_obj_emitter_data_r(particle->m_emitter);
    if (emitter_data->on_particle_end_id) {
        ui_runtime_render_obj_send_event(
            ui_runtime_render_obj_from_data(emitter->m_obj),
            plugin_particle_obj_emitter_msg(emitter, emitter_data->on_particle_end_id));
    }
}

void plugin_particle_obj_particle_real_free(plugin_particle_module_t module, plugin_particle_obj_particle_t particle) {
    module->m_free_particle_count--;
    TAILQ_REMOVE(&module->m_free_particles, particle, m_next);
    mem_free(module->m_alloc, particle);
}

plugin_particle_obj_emitter_t plugin_particle_obj_particle_emitter(plugin_particle_obj_particle_t particle) {
    return particle->m_emitter;
}

plugin_particle_obj_particle_t plugin_particle_obj_particle_follow_to(plugin_particle_obj_particle_t particle) {
    return particle->m_follow_to;
}

uint8_t plugin_particle_obj_particle_follow_is_tie(plugin_particle_obj_particle_t particle) {
    return particle->m_follow_is_tie;
}

void plugin_particle_obj_particle_set_follow_to(
    plugin_particle_obj_particle_t particle, plugin_particle_obj_particle_t follow_to, uint8_t is_tie, uint8_t angle, uint8_t scale)
{
    if (particle->m_follow_to) {
        TAILQ_REMOVE(&particle->m_follow_to->m_follow_particles, particle, m_next_for_follow);
    }

    particle->m_follow_to = follow_to;
    particle->m_follow_is_tie = is_tie;
    particle->m_follow_angle = angle;
    particle->m_follow_scale = scale;

    if (particle->m_follow_to) {
        TAILQ_INSERT_TAIL(&particle->m_follow_to->m_follow_particles, particle, m_next_for_follow);
    }
}

static plugin_particle_obj_particle_t plugin_particle_obj_particle_follow_next(struct plugin_particle_obj_particle_it * it) {
    plugin_particle_obj_particle_t * data = (plugin_particle_obj_particle_t *)(it->m_data);
    plugin_particle_obj_particle_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next_for_follow);

    return r;
}

void plugin_particle_obj_particle_follow_particles(plugin_particle_obj_particle_it_t particle_it, plugin_particle_obj_particle_t particle) {
    *(plugin_particle_obj_particle_t *)(particle_it->m_data) = TAILQ_FIRST(&particle->m_follow_particles);
    particle_it->next = plugin_particle_obj_particle_follow_next;
}

ui_vector_2_t plugin_particle_obj_particle_track_location(plugin_particle_obj_particle_t particle) {
    return &particle->m_track_location;
}

void plugin_particle_obj_particle_set_track_location(plugin_particle_obj_particle_t particle, ui_vector_2_t pos) {
    plugin_particle_obj_emitter_t emitter = particle->m_emitter;
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);

    particle->m_track_location = *pos;
    
    if (emitter_data->xform_mod == UI_PARTICLE_XFORM_LOCAL) {
        ui_transform_t world_t = ui_runtime_render_obj_transform(ui_runtime_render_obj_from_data(emitter->m_obj));
        if (world_t) {
            ui_transform_inline_reverse_adj_vector_2(world_t, &particle->m_track_location);
        }
    }
}

void plugin_particle_obj_particle_calc_transform(plugin_particle_obj_particle_t particle, ui_transform_t transform) {
    float time = particle->m_relative_time / particle->m_one_over_max_life;
    ui_vector_3 s;
    ui_quaternion r;

    bzero(transform, sizeof(*transform));
    
    ui_transform_set_pos_2(transform, &particle->m_location);

    if (particle->m_emitter->m_texture) {
        s.x = (particle->m_size.x / particle->m_emitter->m_tile_size.x) * particle->m_spawn_scale.x;
        s.y = (particle->m_size.y / particle->m_emitter->m_tile_size.y) * particle->m_spawn_scale.y;
    }
    else {
        s.x = particle->m_spawn_scale.x;
        s.y = particle->m_spawn_scale.y;
    }
    
    s.z = 1.0f;

    ui_quaternion_set_z_radians(&r, particle->m_spin_rate * time + particle->m_spin_init);

    ui_transform_set_quation_scale(transform, &r, &s);
}

void plugin_particle_obj_particle_calc_base_transform(plugin_particle_obj_particle_t particle, ui_transform_t transform) {
    float time = particle->m_relative_time / particle->m_one_over_max_life;
    ui_vector_3 s;
    ui_quaternion r;

    ui_transform_set_pos_2(transform, &particle->m_location);

    s.x = particle->m_spawn_scale.x;
    s.y = particle->m_spawn_scale.y;
    s.z = 1.0f;

    ui_quaternion_set_z_radians(&r, particle->m_spin_rate * time + particle->m_spin_init);

    ui_transform_set_quation_scale(transform, &r, &s);
}

void plugin_particle_obj_particle_calc_auto_dir_up(plugin_particle_obj_particle_t particle) {
    float len = ui_vector_2_length(&particle->m_velocity);
    if (len > 0) {
        float sign = particle->m_velocity.x >= 0.0f ? 1.0f : -1.0f;
        particle->m_spin_init = sign * acosf(ui_vector_2_dot_product(&particle->m_velocity, &UI_VECTOR_2_NEGATIVE_UNIT_Y) / len);
        cpe_assert_float_sane(particle->m_spin_init);
    }
}

ui_vector_2_t plugin_particle_obj_particle_pos(plugin_particle_obj_particle_t particle) {
    return &particle->m_location;
}

void plugin_particle_obj_particle_set_pos(plugin_particle_obj_particle_t particle, ui_vector_2_t pos) {
    particle->m_location = *pos;
}

ui_vector_2_t plugin_particle_obj_particle_velocity(plugin_particle_obj_particle_t particle) {
    return &particle->m_velocity;
}

void plugin_particle_obj_particle_set_velocity(plugin_particle_obj_particle_t particle, ui_vector_2_t velocity) {
    particle->m_velocity = *velocity;
}

float plugin_particle_obj_particle_moved_distance(plugin_particle_obj_particle_t particle) {
    return particle->m_moved_distance;
}

ui_vector_2_t plugin_particle_obj_particle_base_size(plugin_particle_obj_particle_t particle) {
    return &particle->m_base_size;
}

void plugin_particle_obj_particle_set_base_size(plugin_particle_obj_particle_t particle, ui_vector_2_t size) {
    cpe_assert_float_sane(size->x);
    cpe_assert_float_sane(size->y);

    particle->m_base_size = *size; 
}

float plugin_particle_obj_particle_time_scale(plugin_particle_obj_particle_t particle) {
    return particle->m_time_scale;
}

void plugin_particle_obj_particle_set_time_scale(plugin_particle_obj_particle_t particle, float time_scale) {
    particle->m_time_scale = time_scale;
}

float plugin_particle_obj_particle_spin_init(plugin_particle_obj_particle_t particle) {
    return particle->m_spin_init;
}

void plugin_particle_obj_particle_set_spin_init(plugin_particle_obj_particle_t particle, float spin_init) {
    cpe_assert_float_sane(spin_init);
    particle->m_spin_init = spin_init;
}

float plugin_particle_obj_particle_spin(plugin_particle_obj_particle_t particle) {
    float spin;
    
    if (particle->m_spin_rate != 0.0f) {
        float time = particle->m_relative_time / particle->m_one_over_max_life;
        spin = particle->m_spin_rate * time + particle->m_spin_init;
    }
    else {
        spin = particle->m_spin_init;
    }

    return spin;
}

ui_vector_2 plugin_particle_obj_particle_world_pos(plugin_particle_obj_particle_t particle) {
    ui_vector_2 r = particle->m_location;
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(particle->m_emitter);

    if (emitter_data->xform_mod == UI_PARTICLE_XFORM_LOCAL) {
        ui_transform_t world_t = ui_runtime_render_obj_transform(ui_runtime_render_obj_from_data(particle->m_emitter->m_obj));
        ui_transform_inline_adj_vector_2(world_t, &r);
    }

    return r;
}

void plugin_particle_obj_particle_adj_pos(plugin_particle_obj_particle_t particle, ui_vector_2_t pos) {
    particle->m_location.x += pos->x;
    particle->m_location.y += pos->y;
}

int plugin_particle_obj_particle_mod_disable(plugin_particle_obj_particle_t particle, UI_PARTICLE_MOD const * mod) {
    int8_t idx = plugin_particle_obj_emitter_mod_index(particle->m_emitter, mod);
    if (idx < 0) return -1;
    cpe_ba_set(particle->m_disable_mods, idx, cpe_ba_true);
    return 0;
}

uint8_t plugin_particle_obj_particle_mod_is_disable(plugin_particle_obj_particle_t particle, UI_PARTICLE_MOD const * mod) {
    int8_t idx = plugin_particle_obj_emitter_mod_index(particle->m_emitter, mod);
    if (idx < 0) return 1;
    
    return cpe_ba_get(particle->m_disable_mods, idx) ? 1 : 0;
}

ui_vector_2 plugin_particle_obj_particle_base_scale(plugin_particle_obj_particle_t particle) {
    ui_vector_2 s;

    assert(particle);
    
    if (particle->m_emitter->m_texture) {
        cpe_assert_float_sane(particle->m_base_size.x);
        cpe_assert_float_sane(particle->m_base_size.y);
        cpe_assert_float_sane(particle->m_emitter->m_tile_size.x);
        cpe_assert_float_sane(particle->m_emitter->m_tile_size.y);
        cpe_assert_float_sane(particle->m_spawn_scale.x);
        cpe_assert_float_sane(particle->m_spawn_scale.y);
        
        s.x = (particle->m_base_size.x / particle->m_emitter->m_tile_size.x) * particle->m_spawn_scale.x;
        s.y = (particle->m_base_size.y / particle->m_emitter->m_tile_size.y) * particle->m_spawn_scale.y;
    }
    else {
        s.x = particle->m_spawn_scale.x;
        s.y = particle->m_spawn_scale.y;
    }
    
    return s;
}

ui_vector_2 plugin_particle_obj_particle_scale(plugin_particle_obj_particle_t particle) {
    ui_vector_2 s;

    if (particle->m_emitter->m_texture) {
        s.x = (particle->m_size.x / particle->m_emitter->m_tile_size.x) * particle->m_spawn_scale.x;
        s.y = (particle->m_size.y / particle->m_emitter->m_tile_size.y) * particle->m_spawn_scale.y;
    }
    else {
        s.x = particle->m_spawn_scale.x;
        s.y = particle->m_spawn_scale.y;
    }

    return s;
}

void plugin_particle_obj_emitter_clear_particles(plugin_particle_obj_emitter_t emitter, uint8_t show_dead_anim) {
    if (show_dead_anim) {
        UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(emitter);
        if (emitter_data->dead_anim_id) {
            const char * dead_anim = plugin_particle_obj_emitter_msg(emitter, emitter_data->dead_anim_id);
            
            while(!TAILQ_EMPTY(&emitter->m_particles)) {
                plugin_particle_obj_particle_t particle = TAILQ_FIRST(&emitter->m_particles);
                plugin_particle_obj_particle_trigger_particle(particle, dead_anim);
                plugin_particle_obj_particle_free(particle);
            }
            return;
        }
    }

    while(!TAILQ_EMPTY(&emitter->m_particles)) {
        plugin_particle_obj_particle_free(TAILQ_FIRST(&emitter->m_particles));
    }
}

void plugin_particle_obj_particle_free_with_dead_anim_r(plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(particle->m_emitter);

    while(!TAILQ_EMPTY(&particle->m_follow_particles)) {
        plugin_particle_obj_particle_free_with_dead_anim_r(TAILQ_FIRST(&particle->m_follow_particles));
    }
    
    if (emitter_data->dead_anim_id) {
        const char * dead_anim = plugin_particle_obj_emitter_msg(particle->m_emitter, emitter_data->dead_anim_id);
        plugin_particle_obj_particle_trigger_particle(particle, dead_anim);
    }
            
    plugin_particle_obj_particle_free(particle);
}

static plugin_particle_obj_particle_t plugin_particle_obj_particle_next(struct plugin_particle_obj_particle_it * it) {
    plugin_particle_obj_particle_t * data = (plugin_particle_obj_particle_t *)(it->m_data);
    plugin_particle_obj_particle_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_particle_obj_emitter_particles(plugin_particle_obj_particle_it_t particle_it, plugin_particle_obj_emitter_t emitter) {
    *(plugin_particle_obj_particle_t *)(particle_it->m_data) = TAILQ_FIRST(&emitter->m_particles);
    particle_it->next = plugin_particle_obj_particle_next;
}

int plugin_particle_obj_particle_trigger_particle(plugin_particle_obj_particle_t particle, const char * emitter_def) {
    UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(particle->m_emitter);
    plugin_particle_obj_emitter_t sub_emitter;
    ui_transform trans;
    const char * args_begin;
    const char * args_end = NULL;
    const char * emitter_name;
    char buf[32];

    if ((args_begin = strchr(emitter_def, '['))) {
        args_end = strchr(args_begin + 1, ']');
        if (args_end == NULL) {
            CPE_ERROR(
                particle->m_emitter->m_obj->m_module->m_em,
                "plugin_particle_obj_particle_trigger_particle: emitter def %s format error!", emitter_def);
            return -1;
        }

        emitter_name = cpe_str_dup_range(buf, sizeof(buf), emitter_def, args_begin);
        if (emitter_name == NULL) {
            CPE_ERROR(
                particle->m_emitter->m_obj->m_module->m_em,
                "plugin_particle_obj_particle_trigger_particle: emitter %s not exist!", emitter_name);
            return -1;
        }
        
        args_begin++;
    }
    else {
        emitter_name = emitter_def;
    }

    sub_emitter = plugin_particle_obj_emitter_find(particle->m_emitter->m_obj, emitter_name);
    if (sub_emitter == NULL) {
        CPE_ERROR(
                  particle->m_emitter->m_obj->m_module->m_em, "plugin_particle_obj_particle_trigger_particle: emitter %s not exist!", emitter_name);
        return -1;
    }

    plugin_particle_obj_particle_calc_base_transform(particle, &trans);
    if (args_begin) {
        if (cpe_str_read_arg_range(buf, sizeof(buf), args_begin, args_end, "angle", ',', '=') == 0 && atoi(buf) == 0) {
            ui_transform_set_quation(&trans, &UI_QUATERNION_IDENTITY);
        }

        if (cpe_str_read_arg_range(buf, sizeof(buf), args_begin, args_end, "scale", ',', '=') == 0 && atoi(buf) == 0) {
            ui_transform_set_scale(&trans, &UI_VECTOR_3_ZERO);
        }
    }
    
    if (emitter_data->xform_mod == UI_PARTICLE_XFORM_LOCAL) {
        plugin_particle_obj_emitter_spawn_at_local(sub_emitter, &trans, 0);
    }
    else {
        plugin_particle_obj_emitter_spawn_at_world(sub_emitter, &trans, 0);
    }

    return 0;
}

