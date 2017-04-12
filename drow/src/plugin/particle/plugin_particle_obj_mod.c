#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_color.h"
#include "plugin_particle_obj_i.h"
#include "plugin_particle_data_i.h"
#include "plugin_particle_obj_emitter_i.h"
#include "plugin_particle_obj_particle_i.h"
#include "plugin_particle_obj_mod_data_i.h"

static float plugin_particle_mod_rand_in_range(float min, float max) {
	return min + (max - min) * ((float)rand() / RAND_MAX);
}

static void plugin_particle_mod_update_accel_attract(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data,
    plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    ui_vector_2 location;
    ui_vector_2 p;
    UI_PARTICLE_MOD_ACCEL_ATTRACT const * mod_data = &mod->data.accel_attract;
    float power_adj;
    float accel_adj;
    float distance;
    
    if (mod_data->is_location_local) {
        if (particle->m_track_location.x == 0.0f && particle->m_track_location.y == 0.0f) {
            particle->m_track_location.x = mod_data->location.value[0];
            particle->m_track_location.y = mod_data->location.value[1];
        }
        location = particle->m_track_location;
    }
    else {
        location.x = mod_data->location.value[0];
        location.y = mod_data->location.value[1];
    }
    
    if (mod_data->kill_zone > 0) {
        if (cpe_math_distance_square(pre_location->x, pre_location->y, location.x, location.y)
            <= mod_data->kill_zone * mod_data->kill_zone)
        {
            particle->m_relative_time = 1.0f;
            return;
        }
    }

    p.x = location.x - pre_location->x;
    p.y = location.y - pre_location->y;

    switch(mod_data->attract_type) {
    case UI_PARTICLE_ATTRACT_MOD_TYPE_BASIC:
        ui_vector_2_inline_normalize(&p);

        accel->x += p.x * mod_data->power * particle->m_spawn_scale.x;
        accel->y += p.y * mod_data->power * particle->m_spawn_scale.y;
        break;
    case UI_PARTICLE_ATTRACT_MOD_TYPE_CURVED:
        power_adj = plugin_particle_obj_emitter_curve_sample(
            particle->m_emitter, mod_data->attract_data.curved.power_adj_curve_id, particle->m_relative_time);

        ui_vector_2_inline_normalize(&p);

        accel_adj = mod_data->power * power_adj;
        accel->x += p.x * accel_adj * particle->m_spawn_scale.x;
        accel->y += p.y * accel_adj * particle->m_spawn_scale.y; 
        break;
    case UI_PARTICLE_ATTRACT_MOD_TYPE_AI_MOVE: {
        ui_vector_2 adj_v;
        ui_vector_2 max_v;
        float max_accel = 0;
        float max_velocity = 0;
        
        ui_vector_2_inline_normalize(&p);

        max_accel = mod_data->attract_data.ai_move.max_accel;
        max_velocity = mod_data->attract_data.ai_move.max_velocity;

        adj_v.x = p.x * max_velocity - particle->m_velocity.x;
        adj_v.y = p.y * max_velocity - particle->m_velocity.y;

        max_v.x = p.x * max_accel;
        max_v.y = p.y * max_accel;

        if (adj_v.x > max_v.x
            && adj_v.y > max_v.y) {
                adj_v.x = max_v.x;
                adj_v.y = max_v.y;
        }

        accel->x += adj_v.x * particle->m_spawn_scale.x;
        accel->y += adj_v.y * particle->m_spawn_scale.y;

        break;
    }
    case UI_PARTICLE_ATTRACT_MOD_TYPE_AI_SPRING:
        distance = cpe_math_distance(0.0f, 0.0f, p.x, p.y);
        distance -= mod_data->attract_data.ai_spring.base_distance; 

        if(distance > mod_data->attract_data.ai_spring.max_distance)
            distance = mod_data->attract_data.ai_spring.max_distance;

        if (fabs(distance) > 0.0f) {
            ui_vector_2_inline_normalize(&p);
            accel_adj = mod_data->attract_data.ai_spring.K * distance;
            
            accel->x += p.x * accel_adj * particle->m_spawn_scale.x;
            accel->y += p.y * accel_adj * particle->m_spawn_scale.y;
        }
        break; 
    case UI_PARTICLE_ATTRACT_MOD_TYPE_AI_GRAVITATION:
        distance = cpe_math_distance(0.0f, 0.0f, p.x, p.y);
        distance = cpe_limit_in_range(distance, mod_data->attract_data.ai_gravitation.min_distance, mod_data->attract_data.ai_gravitation.min_distance);

        if (distance > 0.0f) {
            ui_vector_2_inline_normalize(&p);

            accel_adj = mod_data->attract_data.ai_gravitation.G / (distance * distance);
            
            accel->x += p.x * accel_adj * particle->m_spawn_scale.x;
            accel->y += p.y * accel_adj * particle->m_spawn_scale.y;
        }
        break; 
    }
}

static void plugin_particle_mod_update_accel_damping(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_ACCEL_DAMPING const * mod_data = &mod->data.accel_damping;

    accel->x += -particle->m_velocity.x * mod_data->size;
    accel->y += -particle->m_velocity.y * mod_data->size;
}

static void plugin_particle_mod_init_accel_seed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_ACCEL_SEED const * mod_data = &mod->data.accel_seed;
    float base_accel_x;
    float base_accel_y;

    base_accel_x = plugin_particle_mod_rand_in_range(mod_data->min_base.value[0], mod_data->max_base.value[0]);
    base_accel_y = plugin_particle_mod_rand_in_range(mod_data->min_base.value[1], mod_data->max_base.value[1]);
    
    particle->m_base_accel.x += base_accel_x * particle->m_spawn_scale.x;
    particle->m_base_accel.y += base_accel_y * particle->m_spawn_scale.y;
}

static void plugin_particle_mod_update_accel_sine(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_ACCEL_SINE const * mod_data = &mod->data.accel_sine;
    ui_vector_2 mod_accel;
    float omega;
    float adj;
    
    mod_accel.x = mod_data->base.value[0] * particle->m_spawn_scale.x;
    mod_accel.y = mod_data->base.value[1] * particle->m_spawn_scale.y;

    omega = 360.0f / (mod_data->cycle * particle->m_spawn_scale.x);

    adj = cpe_fast_cos(omega * ( particle->m_relative_time / particle->m_one_over_max_life));
    
    accel->x += mod_accel.x * adj;
    accel->y += mod_accel.y * adj;
}

static void plugin_particle_mod_update_color_curved(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_COLOR_CURVED const * mod_data = &mod->data.color_curved;
    ui_color color;

    color.a = plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->chanel_a_curve_id, particle->m_relative_time);
    color.r = plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->chanel_r_curve_id, particle->m_relative_time);
    color.g = plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->chanel_g_curve_id, particle->m_relative_time);
    color.b = plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->chanel_b_curve_id, particle->m_relative_time);

    if (particle->m_color != 0xFFFFFFFF) {
        ui_color p_color;
        ui_color_set_from_argb(&p_color, particle->m_color);
        ui_color_inline_mul(&color, &p_color);
    }
    
    particle->m_color = ui_color_make_argb(&color);
}

static void plugin_particle_mod_update_color_curved_alpha(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_COLOR_CURVED_ALPHA const * mod_data = &mod->data.color_curved_alpha;
    ui_color color;

    color.r = color.g = color.b = 1.0f;
    color.a = plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->chanel_a_curve_id, particle->m_relative_time);

    if (particle->m_color != 0xFFFFFFFF) {
        ui_color p_color;
        ui_color_set_from_argb(&p_color, particle->m_color);
        ui_color_inline_mul(&color, &p_color);
    }
    
    particle->m_color = ui_color_make_argb(&color);
}

static void plugin_particle_mod_init_color_fixed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_COLOR_FIXED const * mod_data = &mod->data.color_fixed;

    particle->m_base_color =
        ui_color_make_argb_from_value(
            mod_data->base_color.r,
            mod_data->base_color.g,
            mod_data->base_color.b,
            mod_data->base_color.a);
}

static void plugin_particle_mod_update_color_over_life(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_COLOR_OVER_LIFE const * mod_data = &mod->data.color_over_life;
    float percentage = 0.0f;
    ui_color color;

    if (mod_data->fade_begin_time <= particle->m_relative_time) {
        percentage = (particle->m_relative_time - mod_data->fade_begin_time) / (1.0f - mod_data->fade_begin_time);
    }

    color.a = mod_data->min_base_color.a + (mod_data->max_base_color.a - mod_data->min_base_color.a) * percentage;
    color.r = mod_data->min_base_color.r + (mod_data->max_base_color.r - mod_data->min_base_color.r) * percentage;
    color.g = mod_data->min_base_color.g + (mod_data->max_base_color.g - mod_data->min_base_color.g) * percentage;
    color.b = mod_data->min_base_color.b + (mod_data->max_base_color.b - mod_data->min_base_color.b) * percentage;

    if (particle->m_color != 0xFFFFFFFF) {
        ui_color p_color;
        ui_color_set_from_argb(&p_color, particle->m_color);
        ui_color_inline_mul(&color, &p_color);
    }
    
    particle->m_color = ui_color_make_argb(&color);
}

static void plugin_particle_mod_init_color_seed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_COLOR_SEED const * mod_data = &mod->data.color_seed;
    ui_color color;

    color.a = plugin_particle_mod_rand_in_range(mod_data->min_base_color.a, mod_data->max_base_color.a);
    color.r = plugin_particle_mod_rand_in_range(mod_data->min_base_color.r, mod_data->max_base_color.r);
    color.g = plugin_particle_mod_rand_in_range(mod_data->min_base_color.g, mod_data->max_base_color.g);
    color.b = plugin_particle_mod_rand_in_range(mod_data->min_base_color.b, mod_data->max_base_color.b);

    particle->m_base_color = ui_color_make_argb(&color);
}

static void plugin_particle_mod_init_lifetime_seed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_LIFETIME_SEED const * mod_data = &mod->data.lifetime_seed;
    float base_life_time;

    base_life_time = plugin_particle_mod_rand_in_range(mod_data->min_base_time, mod_data->max_base_time);

    if (particle->m_one_over_max_life != 0.0f) {
        base_life_time += 1.0f / particle->m_one_over_max_life;
    }

    particle->m_one_over_max_life = 1.0f / base_life_time;
}

static void plugin_particle_mod_init_location_orbit(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_LOCATION_ORBIT const * mod_data = &mod->data.location_orbit;
    ui_vector_2 v;

    v.x = particle->m_location.x - mod_data->orbit_x;
    v.y = particle->m_location.y - mod_data->orbit_y;

    particle->m_orbit.x = cpe_math_angle(0.0f, 0.0f, v.x, v.y);
    particle->m_orbit.y = cpe_math_distance(0.0f, 0.0f, v.x, v.y);
}

static void plugin_particle_mod_update_location_orbit(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_LOCATION_ORBIT const * mod_data = &mod->data.location_orbit;
    float fcos;
    float fsin;

    particle->m_orbit.x += delta * mod_data->orbit_rate;
    particle->m_orbit.y -= delta * mod_data->orbit_offset;

    fcos = cpe_fast_cos(particle->m_orbit.x);
    fsin = cpe_fast_sin(particle->m_orbit.x);

    particle->m_location.x = particle->m_orbit.y * fcos + mod_data->orbit_x;
    particle->m_location.y = particle->m_orbit.y * fsin + mod_data->orbit_y;
}

static void plugin_particle_mod_init_location_seed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_LOCATION_SEED const * mod_data = &mod->data.location_seed;
    ui_vector_2 base_location;
    
    base_location.x = plugin_particle_mod_rand_in_range(mod_data->min_base_location.value[0], mod_data->max_base_location.value[0]);
    base_location.y = plugin_particle_mod_rand_in_range(mod_data->min_base_location.value[1], mod_data->max_base_location.value[1]);

    particle->m_location.x += base_location.x * particle->m_spawn_scale.x;
    particle->m_location.y += base_location.y * particle->m_spawn_scale.y;
}

static void plugin_particle_mod_init_rotation2d_seed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_ROTATION2D_SEED const * mod_data = &mod->data.rotation2d_seed;
    float spin;
    float rate;

    spin = plugin_particle_mod_rand_in_range(mod_data->min_init_spin, mod_data->max_init_spin);
    rate = plugin_particle_mod_rand_in_range(mod_data->min_spin_rate, mod_data->max_spin_rate);

    particle->m_spin_init += cpe_math_angle_to_radians(spin);
    particle->m_spin_rate += cpe_math_angle_to_radians(rate);
}

static void plugin_particle_mod_update_size_curved_uniform(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_SIZE_CURVED_UNIFORM const * mod_data = &mod->data.size_curved_uniform;
    float scale;

    scale = plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->chanel_size_curve_id, particle->m_relative_time);
    
    particle->m_size.x *= scale;
    particle->m_size.y *= scale;
}

static void plugin_particle_mod_update_size_curved(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_SIZE_CURVED const * mod_data = &mod->data.size_curved;

    particle->m_size.x *= plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->chanel_x_curve_id, particle->m_relative_time);
    particle->m_size.y *= plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->chanel_y_curve_id, particle->m_relative_time);
}

static void plugin_particle_mod_update_size_uniform_over_life(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_SIZE_UNIFORM_OVER_LIFE const * mod_data = &mod->data.size_uniform_over_life;
    float percentage = 0.0f;
    float adj;
    
    if (mod_data->fade_begin_time <= particle->m_relative_time) {
        percentage = (particle->m_relative_time - mod_data->fade_begin_time) / (1.0f - mod_data->fade_begin_time);
    }

    adj = mod_data->min_base_size + (mod_data->max_base_size - mod_data->min_base_size) * percentage;
    particle->m_size.x *= adj;
    particle->m_size.y *= adj;
}

static void plugin_particle_mod_update_size_over_life(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_SIZE_OVER_LIFE const * mod_data = &mod->data.size_over_life;
    float percentage = 0.0f;
    
    if (mod_data->fade_begin_time <= particle->m_relative_time) {
        percentage = (particle->m_relative_time - mod_data->fade_begin_time) / (1.0f - mod_data->fade_begin_time);
    }

    particle->m_size.x *= mod_data->min_base_size.value[0] + (mod_data->max_base_size.value[0] - mod_data->min_base_size.value[0]) * percentage;
    particle->m_size.y *= mod_data->min_base_size.value[1] + (mod_data->max_base_size.value[1] - mod_data->min_base_size.value[1]) * percentage;
}

static void plugin_particle_mod_init_size_seed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_SIZE_SEED const * mod_data = &mod->data.size_seed;
    ui_vector_2 base_size;

    base_size.x = plugin_particle_mod_rand_in_range(mod_data->min_base_size.value[0], mod_data->max_base_size.value[0]);
    base_size.y = plugin_particle_mod_rand_in_range(mod_data->min_base_size.value[1], mod_data->max_base_size.value[1]);

    cpe_assert_float_sane(base_size.x);
    cpe_assert_float_sane(base_size.y);
    
    particle->m_base_size.x *= base_size.x;
    particle->m_base_size.y *= base_size.y;
}

static void plugin_particle_mod_init_size_uniform_seed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_SIZE_UNIFORM_SEED const * mod_data = &mod->data.size_uniform_seed;
    float base_size;

    base_size = plugin_particle_mod_rand_in_range(mod_data->min_base_size, mod_data->max_base_size);
    cpe_assert_float_sane(base_size);
    
    particle->m_base_size.x *= base_size;
    particle->m_base_size.y *= base_size;
}

static void plugin_particle_mod_init_texcoord_flipbook_uv(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    if (particle->m_emitter->m_texture_mode == plugin_particle_obj_emitter_texture_mode_tiled) {
        UI_PARTICLE_MOD_TEXCOORD_FLIPBOOK_UV const * mod_data = &mod->data.texcoord_flipbook_uv;

        particle->m_texture_tile.m_index = mod_data->start_tile_index;
    }
}

static void plugin_particle_mod_update_texcoord_flipbook_uv(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    if (particle->m_emitter->m_texture_mode == plugin_particle_obj_emitter_texture_mode_tiled) {
        UI_PARTICLE_MOD_TEXCOORD_FLIPBOOK_UV const * mod_data = &mod->data.texcoord_flipbook_uv;
        UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(particle->m_emitter);
        float time  = particle->m_relative_time / particle->m_one_over_max_life;
        uint32_t idx   = mod_data->start_tile_index + (uint32_t)(time * mod_data->frame_rate);
        uint32_t tiles = emitter_data->tiling_u * emitter_data->tiling_v;

        if (mod_data->loop) {
            particle->m_texture_tile.m_index = idx % tiles;
        }
        else {
            particle->m_texture_tile.m_index = cpe_min(tiles - 1, idx);
        }
    }
}

static void plugin_particle_mod_update_texcoord_scroll_anim(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    if (particle->m_emitter->m_texture_mode == plugin_particle_obj_emitter_texture_mode_scroll) {
        UI_PARTICLE_MOD_TEXCOORD_SCROLL_ANIM const * mod_data = &mod->data.texcoord_scroll_anim;
        float time;

        time = particle->m_relative_time / particle->m_one_over_max_life;

        particle->m_texture_scroll.m_u = time * mod_data->speed_u;
        particle->m_texture_scroll.m_v = time * mod_data->speed_v;

        if (!mod_data->loop) {
            if (fabs(particle->m_texture_scroll.m_u) >= 1.0f) {
                particle->m_texture_scroll.m_u = 0.0f;
            }
            else {
                if (particle->m_texture_scroll.m_u < 0.0f) particle->m_texture_scroll.m_u += 1.0f;
            }

            if (fabs(particle->m_texture_scroll.m_v) >= 1.0f) {
                particle->m_texture_scroll.m_v = 0.0f;
            }
            else {
                if (particle->m_texture_scroll.m_v < 0.0f) particle->m_texture_scroll.m_v += 1.0f;
            }
        }

        assert(particle->m_texture_scroll.m_u >= 0);
        assert(particle->m_texture_scroll.m_v >= 0);
    }
}

static void plugin_particle_mod_init_texcoord_tile_sub_tex(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    if (particle->m_emitter->m_texture_mode == plugin_particle_obj_emitter_texture_mode_tiled) {
        UI_PARTICLE_MOD_TEXCOORD_TILE_SUB_TEX const * mod_data = &mod->data.texcoord_tile_sub_tex;
        UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_obj_emitter_data_r(particle->m_emitter);
        uint32_t tiles = emitter_data->tiling_u * emitter_data->tiling_v;
        if (tiles == 0) {
            particle->m_texture_tile.m_index = 0;
            return;
        }

        if (mod_data->tile_index >= 0) {
            particle->m_texture_tile.m_index = mod_data->tile_index;
            if (particle->m_texture_tile.m_index >= tiles) {
                particle->m_texture_tile.m_index = (tiles - 1);
            }
        }
        else {
            particle->m_texture_tile.m_index = floor(plugin_particle_mod_rand_in_range(0.0f , tiles));
        }
    }
}

static void plugin_particle_mod_init_uber_circle_init_data(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data) {
    UI_PARTICLE_MOD_UBER_CIRCLE_SPAWN const * mod_data = &mod->data.uber_circle_spawn;
    mod_r_data->m_uber_circle.m_emitter_angle_theta = mod_data->initial_angle;
}

static void plugin_particle_mod_init_uber_circle_spawn(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle)
{
    UI_PARTICLE_MOD_UBER_CIRCLE_SPAWN const * mod_data = &mod->data.uber_circle_spawn;
    float l;
    float theta;
    ui_vector_2 d;
    ui_vector_2 r;
    ui_vector_2 t;
    float adj;

    assert(mod_r_data);
    
    if (mod_data->angle_delta == 0.0f) {
        mod_r_data->m_uber_circle.m_emitter_angle_theta = plugin_particle_mod_rand_in_range(mod_data->min_arc_angle, mod_data->max_arc_angle);
    }
    else {
        mod_r_data->m_uber_circle.m_emitter_angle_theta += mod_data->angle_delta;

        if (mod_r_data->m_uber_circle.m_emitter_angle_theta >= mod_data->max_arc_angle) {
            mod_r_data->m_uber_circle.m_emitter_angle_theta = mod_data->min_arc_angle;
        }
        else if (mod_r_data->m_uber_circle.m_emitter_angle_theta <= mod_data->min_arc_angle ) {
            mod_r_data->m_uber_circle.m_emitter_angle_theta = mod_data->max_arc_angle;
        }
    }
    
    l = mod_data->circle_radius - mod_data->distribute_delta * ((float)rand() / RAND_MAX) ;
	
	if (l < 0.0f) l = 0.0f;
    
	theta = mod_r_data->m_uber_circle.m_emitter_angle_theta + 180.0f;
	d.x = l * cpe_fast_cos(theta);
	d.y = l * cpe_fast_sin(theta);

	ui_vector_2_normalize(&r, &d);

	t.x = -r.y;
	t.y = r.x;

    particle->m_location.x += d.x * particle->m_spawn_scale.x;
    particle->m_location.y += d.y * particle->m_spawn_scale.y;

    adj = plugin_particle_mod_rand_in_range(mod_data->min_radial_velocity, mod_data->max_radial_velocity);
    particle->m_velocity.x += r.x * adj * particle->m_spawn_scale.x;
    particle->m_velocity.y += r.y * adj * particle->m_spawn_scale.y;
	
    adj = plugin_particle_mod_rand_in_range(mod_data->min_radial_accel, mod_data->max_radial_accel);
    particle->m_base_accel.x += r.x * adj * particle->m_spawn_scale.x;
    particle->m_base_accel.y += r.y * adj * particle->m_spawn_scale.y;

    adj = plugin_particle_mod_rand_in_range(mod_data->min_tangent_accel, mod_data->max_tangent_accel);
    particle->m_base_accel.x += t.x * adj * particle->m_spawn_scale.x;
    particle->m_base_accel.y += t.y * adj * particle->m_spawn_scale.y;

	if(plugin_particle_obj_emitter_data_r(particle->m_emitter)->auto_up_dir) {
        particle->m_spin_init = cpe_math_angle_to_radians(theta + 90.0f);
    }
}

static void plugin_particle_mod_init_uber_ellipse_init_data(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data) {
    UI_PARTICLE_MOD_UBER_ELLIPSE_SPAWN const * mod_data = &mod->data.uber_ellipse_spawn;
    mod_r_data->m_uber_ellipse.m_emitter_angle_theta = mod_data->initial_angle;
}

static void plugin_particle_mod_init_uber_ellipse_spawn(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_UBER_ELLIPSE_SPAWN const * mod_data = &mod->data.uber_ellipse_spawn;
    float theta;
    ui_vector_2 d;
    ui_vector_2 r;
    ui_vector_2 t;
    float adj;

    assert(mod_r_data);
    
    if (mod_data->angle_delta == 0.0f) {
        mod_r_data->m_uber_ellipse.m_emitter_angle_theta = plugin_particle_mod_rand_in_range(mod_data->min_arc_angle, mod_data->max_arc_angle);
    }
    else {
        mod_r_data->m_uber_ellipse.m_emitter_angle_theta += mod_data->angle_delta;

        if (mod_r_data->m_uber_ellipse.m_emitter_angle_theta >= mod_data->max_arc_angle) {
            mod_r_data->m_uber_ellipse.m_emitter_angle_theta = mod_data->min_arc_angle;
        }
        else if (mod_r_data->m_uber_ellipse.m_emitter_angle_theta <= mod_data->min_arc_angle ) {
            mod_r_data->m_uber_ellipse.m_emitter_angle_theta = mod_data->max_arc_angle;
        }
    }
    
    theta = mod_r_data->m_uber_ellipse.m_emitter_angle_theta + 180.0f;

    d.x = cpe_fast_sin(theta) * 0.5f * mod_data->ellipse_axis_x;
    d.y = cpe_fast_cos(theta) * 0.5f * mod_data->ellipse_axis_y;

    ui_vector_2_normalize(&r, &d);

    t.x = -r.y;
    t.y = r.x;

    particle->m_location.x += d.x * particle->m_spawn_scale.x;
    particle->m_location.y += d.y * particle->m_spawn_scale.y;

    adj = plugin_particle_mod_rand_in_range(mod_data->min_radial_velocity, mod_data->max_radial_velocity);
    particle->m_velocity.x += r.x * adj * particle->m_spawn_scale.x;
    particle->m_velocity.y += r.y * adj * particle->m_spawn_scale.y;

    adj = plugin_particle_mod_rand_in_range(mod_data->min_radial_accel, mod_data->max_radial_accel);
    particle->m_base_accel.x += r.x * adj * particle->m_spawn_scale.x;
    particle->m_base_accel.y += r.y * adj * particle->m_spawn_scale.y;

    adj = plugin_particle_mod_rand_in_range(mod_data->min_tangent_accel, mod_data->max_tangent_accel);
    particle->m_base_accel.x += t.x * adj * particle->m_spawn_scale.x;
    particle->m_base_accel.y += t.y * adj * particle->m_spawn_scale.y;
}

static void plugin_particle_mod_update_velocity_attract(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_VELOCITY_ATTRACT const * mod_data = &mod->data.velocity_attract;
    float power;
    ui_vector_2 p;
    float adj;
    
    power = plugin_particle_obj_emitter_curve_sample(particle->m_emitter, mod_data->power_chanel_curve_id, particle->m_relative_time);

    p.x = mod_data->attract_location.value[0] - pre_location->x;
    p.y = mod_data->attract_location.value[1] - pre_location->y;

    ui_vector_2_inline_normalize(&p);

    adj = power;
    particle->m_velocity.x = p.x * adj * particle->m_spawn_scale.x;
    particle->m_velocity.y = p.y * adj * particle->m_spawn_scale.y;
}

static void plugin_particle_mod_init_velocity_seed(UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle) {
    UI_PARTICLE_MOD_VELOCITY_SEED const * mod_data = &mod->data.velocity_seed;
    ui_vector_2 base_velocity;
    float multiplier;
    float adj;

    base_velocity.x = plugin_particle_mod_rand_in_range(mod_data->min_base_velocity.value[0], mod_data->max_base_velocity.value[0]);
    base_velocity.y = plugin_particle_mod_rand_in_range(mod_data->min_base_velocity.value[1], mod_data->max_base_velocity.value[1]);
    multiplier = plugin_particle_mod_rand_in_range(mod_data->min_multiplier, mod_data->max_multiplier);

    adj = multiplier;

    particle->m_velocity.x += base_velocity.x * adj * particle->m_spawn_scale.x;
    particle->m_velocity.y += base_velocity.y * adj * particle->m_spawn_scale.y;
}

static void plugin_particle_mod_update_velocity_threshold_stop(
    UI_PARTICLE_MOD const * mod, plugin_particle_obj_mod_data_t mod_r_data, plugin_particle_obj_particle_t particle, ui_vector_2 * accel,
    ui_vector_2_t pre_location, float delta)
{
    UI_PARTICLE_MOD_VELOCITY_THRESHOLD_STOP const * mod_data = &mod->data.velocity_threshold_stop;
    float threshold;

    threshold = plugin_particle_mod_rand_in_range(mod_data->min_stop_threshold, mod_data->max_stop_threshold);
    
    if (threshold <= pre_location->y) {
        // kill speed and acceleration
        accel->x = accel->y = 0.0f;
        particle->m_base_accel.x = particle->m_base_accel.y = 0.0f;
        particle->m_velocity.x = particle->m_velocity.y = 0.0f;
        particle->m_spin_rate = 0.0f;
    }
}

struct plugin_particle_mod_def g_mod_defs[UI_PARTICLE_MOD_TYPE_MAX] = {
    { NULL, NULL, NULL },
    { NULL, NULL, plugin_particle_mod_update_accel_attract },
    { NULL, NULL, plugin_particle_mod_update_accel_damping },
    { NULL, plugin_particle_mod_init_accel_seed, NULL },
	{ NULL, NULL, plugin_particle_mod_update_accel_sine },
	{ NULL, NULL, plugin_particle_mod_update_color_curved },
	{ NULL, NULL, plugin_particle_mod_update_color_curved_alpha },
	{ NULL, plugin_particle_mod_init_color_fixed, NULL },
	{ NULL, NULL, plugin_particle_mod_update_color_over_life},
	{ NULL, plugin_particle_mod_init_color_seed, NULL },
	{ NULL, plugin_particle_mod_init_lifetime_seed, NULL },
	{ NULL, plugin_particle_mod_init_location_orbit, plugin_particle_mod_update_location_orbit },
	{ NULL, plugin_particle_mod_init_location_seed, NULL },
	{ NULL, plugin_particle_mod_init_rotation2d_seed, NULL },
	{ NULL, NULL, plugin_particle_mod_update_size_curved_uniform },
	{ NULL, NULL, plugin_particle_mod_update_size_curved },
	{ NULL, NULL, plugin_particle_mod_update_size_uniform_over_life},
	{ NULL, NULL, plugin_particle_mod_update_size_over_life },
	{ NULL, plugin_particle_mod_init_size_seed, NULL },
    { NULL, plugin_particle_mod_init_size_uniform_seed, NULL },
    { NULL, plugin_particle_mod_init_texcoord_flipbook_uv, plugin_particle_mod_update_texcoord_flipbook_uv},
    { NULL, NULL, plugin_particle_mod_update_texcoord_scroll_anim},
    { NULL, plugin_particle_mod_init_texcoord_tile_sub_tex, NULL },
    { plugin_particle_mod_init_uber_circle_init_data, plugin_particle_mod_init_uber_circle_spawn, NULL },
    { plugin_particle_mod_init_uber_ellipse_init_data, plugin_particle_mod_init_uber_ellipse_spawn, NULL },
    { NULL, NULL, plugin_particle_mod_update_velocity_attract },
    { NULL, plugin_particle_mod_init_velocity_seed, NULL },
    { NULL, NULL, plugin_particle_mod_update_velocity_threshold_stop},
};
