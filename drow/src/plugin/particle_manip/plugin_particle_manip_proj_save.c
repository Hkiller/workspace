#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_stream.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin_particle_manip_save_utils.h"
#include "plugin_particle_manip_utils.h"
#include "plugin_particle_manip_i.h"

static int plugin_particle_manip_proj_save_emitter_i(write_stream_t s, plugin_particle_data_t particle, plugin_particle_data_emitter_t emitter, error_monitor_t em) {
    struct plugin_particle_data_mod_it particle_mod_it;
    plugin_particle_data_mod_t particle_mod;
    UI_PARTICLE_EMITTER *  emitter_data = plugin_particle_data_emitter_data(emitter);

    stream_printf(s, "<RParticleSpriteSRC>\n");
    plugin_particle_manip_proj_save_float(s, 1, "SpawnRate", emitter_data->spawn_rate);
    plugin_particle_manip_proj_save_int(s, 1, "MaxAmount", (int)emitter_data->max_amount);

    if (emitter_data->name_id) plugin_particle_manip_proj_save_str(s, 1, "Name", plugin_particle_data_emitter_msg(emitter, emitter_data->name_id));
    if (emitter_data->xform_mod != 0) plugin_particle_manip_proj_save_int(s, 1, "XFormMode", emitter_data->xform_mod);
    if (emitter_data->bound_mod != 0) plugin_particle_manip_proj_save_int(s, 1, "BoundMode", emitter_data->bound_mod);
    if (emitter_data->auto_up_dir != 0) plugin_particle_manip_proj_save_bool(s, 1, "AutoUpDir", emitter_data->auto_up_dir);
    if (emitter_data->init_delay_time != 0) plugin_particle_manip_proj_save_float(s, 1, "InitDelayTime", emitter_data->init_delay_time);
    if (emitter_data->loop_delay_time != 0) plugin_particle_manip_proj_save_float(s, 1, "LoopDelayTime", emitter_data->loop_delay_time);
    if (emitter_data->duration != 0) plugin_particle_manip_proj_save_float(s, 1, "Duration", emitter_data->duration);
    if (emitter_data->time_scale != 1.0f) plugin_particle_manip_proj_save_float(s, 1, "TimeScale", emitter_data->time_scale);
    if (emitter_data->repeat_time != 1) plugin_particle_manip_proj_save_int(s, 1, "RepeatTimes", emitter_data->repeat_time);
    if (emitter_data->max_extra_brust != 0) plugin_particle_manip_proj_save_int(s, 1, "MaxExtraBrust", emitter_data->max_extra_brust);
    if (emitter_data->min_extra_brust != 0) plugin_particle_manip_proj_save_int(s, 1, "MinExtraBrust", emitter_data->min_extra_brust);
    if (emitter_data->scale != 1.0f) plugin_particle_manip_proj_save_float(s, 1, "Scale", emitter_data->scale);
    if (emitter_data->child_fx_file_id) plugin_particle_manip_proj_save_str(s, 1, "ChildFxFile", plugin_particle_data_emitter_msg(emitter, emitter_data->child_fx_file_id));
    if (emitter_data->wait_child_fx) plugin_particle_manip_proj_save_bool(s, 1, "WaitChildFX", emitter_data->wait_child_fx);
    if (emitter_data->pass_on_color) plugin_particle_manip_proj_save_bool(s, 1, "PassOnColor", emitter_data->pass_on_color);

    plugin_particle_data_emitter_mods(&particle_mod_it, emitter);
    while((particle_mod = plugin_particle_data_mod_it_next(&particle_mod_it))) {
        UI_PARTICLE_MOD const * particle_mod_data = plugin_particle_data_mod_data(particle_mod);
        
        stream_printf(
            s, "    <MOD Type=\"%s\" Hash=\"%u\">\n",
            plugin_particle_manip_proj_particle_mod_type_name(particle_mod_data->type), 
            plugin_particle_manip_proj_particle_mod_type_hash(particle_mod_data->type));

        switch(particle_mod_data->type) {
        case ui_particle_mod_type_accel_attract:
            break;
        case ui_particle_mod_type_accel_damping:
            break;
        case ui_particle_mod_type_accel_seed:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseAccelX", particle_mod_data->data.accel_seed.min_base.value[0]);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseAccelY", particle_mod_data->data.accel_seed.min_base.value[1]);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseAccelZ", particle_mod_data->data.accel_seed.min_base.value[2]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseAccelX", particle_mod_data->data.accel_seed.max_base.value[0]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseAccelY", particle_mod_data->data.accel_seed.max_base.value[1]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseAccelZ", particle_mod_data->data.accel_seed.max_base.value[2]);
            break;
        case ui_particle_mod_type_accel_sine:
            break;
        case ui_particle_mod_type_color_curved:
            plugin_particle_manip_proj_save_curve_chanel(s, 2, "CurveR", particle, particle_mod_data->data.color_curved.chanel_r_curve_id);
            plugin_particle_manip_proj_save_curve_chanel(s, 2, "CurveG", particle, particle_mod_data->data.color_curved.chanel_g_curve_id);
            plugin_particle_manip_proj_save_curve_chanel(s, 2, "CurveB", particle, particle_mod_data->data.color_curved.chanel_b_curve_id);
            plugin_particle_manip_proj_save_curve_chanel(s, 2, "CurveA", particle, particle_mod_data->data.color_curved.chanel_a_curve_id);
            break;
        case ui_particle_mod_type_color_curved_alpha:
            plugin_particle_manip_proj_save_curve_chanel(s, 2, "CurveA", particle, particle_mod_data->data.color_curved_alpha.chanel_a_curve_id);
            break;
        case ui_particle_mod_type_color_fixed:
            break;
        case ui_particle_mod_type_color_over_life:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseColorA", particle_mod_data->data.color_over_life.min_base_color.a);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseColorR", particle_mod_data->data.color_over_life.min_base_color.r);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseColorG", particle_mod_data->data.color_over_life.min_base_color.g);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseColorB", particle_mod_data->data.color_over_life.min_base_color.b);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseColorA", particle_mod_data->data.color_over_life.max_base_color.a);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseColorR", particle_mod_data->data.color_over_life.max_base_color.r);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseColorG", particle_mod_data->data.color_over_life.max_base_color.g);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseColorB", particle_mod_data->data.color_over_life.max_base_color.b);
            plugin_particle_manip_proj_save_float(s, 2, "FadeBeginTime", particle_mod_data->data.color_over_life.fade_begin_time);
            break;
        case ui_particle_mod_type_color_seed:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseColorA", particle_mod_data->data.color_seed.min_base_color.a);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseColorR", particle_mod_data->data.color_seed.min_base_color.r);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseColorG", particle_mod_data->data.color_seed.min_base_color.g);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseColorB", particle_mod_data->data.color_seed.min_base_color.b);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseColorA", particle_mod_data->data.color_seed.max_base_color.a);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseColorR", particle_mod_data->data.color_seed.max_base_color.r);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseColorG", particle_mod_data->data.color_seed.max_base_color.g);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseColorB", particle_mod_data->data.color_seed.max_base_color.b);
            break;
        case ui_particle_mod_type_lifetime_seed:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseLifetime", particle_mod_data->data.lifetime_seed.min_base_time);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseLifetime", particle_mod_data->data.lifetime_seed.max_base_time);
            break;
        case ui_particle_mod_type_location_orbit:
            break;
        case ui_particle_mod_type_location_seed:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseLocationX", particle_mod_data->data.location_seed.min_base_location.value[0]);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseLocationY", particle_mod_data->data.location_seed.min_base_location.value[1]);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseLocationZ", particle_mod_data->data.location_seed.min_base_location.value[2]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseLocationX", particle_mod_data->data.location_seed.max_base_location.value[0]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseLocationY", particle_mod_data->data.location_seed.max_base_location.value[1]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseLocationZ", particle_mod_data->data.location_seed.max_base_location.value[2]);
            break;
        case ui_particle_mod_type_rotation2d_seed:
            plugin_particle_manip_proj_save_float(s, 2, "MinSpinRate", particle_mod_data->data.rotation2d_seed.min_spin_rate);
            plugin_particle_manip_proj_save_float(s, 2, "MaxSpinRate", particle_mod_data->data.rotation2d_seed.max_spin_rate);
            plugin_particle_manip_proj_save_float(s, 2, "MinInitSpin", particle_mod_data->data.rotation2d_seed.min_init_spin);
            plugin_particle_manip_proj_save_float(s, 2, "MaxInitSpin", particle_mod_data->data.rotation2d_seed.max_init_spin);
            break;
        case ui_particle_mod_type_size_curved_uniform:
            break;
        case ui_particle_mod_type_size_curved:
            break;
        case ui_particle_mod_type_size_uniform_over_life:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseSize", particle_mod_data->data.size_uniform_over_life.min_base_size);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseSize", particle_mod_data->data.size_uniform_over_life.max_base_size);
            plugin_particle_manip_proj_save_float(s, 2, "FadeBeginTime", particle_mod_data->data.size_uniform_over_life.fade_begin_time);
            break;
        case ui_particle_mod_type_size_over_life:
            break;
        case ui_particle_mod_type_size_seed:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseSizeX", particle_mod_data->data.size_seed.min_base_size.value[0]);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseSizeY", particle_mod_data->data.size_seed.min_base_size.value[1]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseSizeX", particle_mod_data->data.size_seed.max_base_size.value[0]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseSizeY", particle_mod_data->data.size_seed.max_base_size.value[1]);
            break;
        case ui_particle_mod_type_size_uniform_seed:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseSize", particle_mod_data->data.size_uniform_seed.min_base_size);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseSize", particle_mod_data->data.size_uniform_seed.max_base_size);
            break;
        case ui_particle_mod_type_texcoord_flipbook_uv:
            plugin_particle_manip_proj_save_float(s, 2, "Framerate", particle_mod_data->data.texcoord_flipbook_uv.frame_rate);
            plugin_particle_manip_proj_save_bool(s, 2, "Loop", particle_mod_data->data.texcoord_flipbook_uv.loop);
            plugin_particle_manip_proj_save_int(s, 2, "StartTile", particle_mod_data->data.texcoord_flipbook_uv.start_tile_index);
            break;
        case ui_particle_mod_type_texcoord_scroll_anim:
            break;
        case ui_particle_mod_type_texcoord_tile_sub_tex:
            break;
        case ui_particle_mod_type_uber_circle_spawn:
            plugin_particle_manip_proj_save_float(s, 2, "AngleDelta", particle_mod_data->data.uber_circle_spawn.angle_delta);
            plugin_particle_manip_proj_save_float(s, 2, "CircleRadius", particle_mod_data->data.uber_circle_spawn.circle_radius);
            plugin_particle_manip_proj_save_float(s, 2, "MinRadialAccel", particle_mod_data->data.uber_circle_spawn.min_radial_accel);
            plugin_particle_manip_proj_save_float(s, 2, "MaxRadialAccel", particle_mod_data->data.uber_circle_spawn.max_radial_accel);
            plugin_particle_manip_proj_save_float(s, 2, "MinTangentAccel", particle_mod_data->data.uber_circle_spawn.min_tangent_accel);
            plugin_particle_manip_proj_save_float(s, 2, "MaxTangentAccel", particle_mod_data->data.uber_circle_spawn.max_tangent_accel);
            plugin_particle_manip_proj_save_float(s, 2, "MinRadialVelocity", particle_mod_data->data.uber_circle_spawn.min_radial_velocity);
            plugin_particle_manip_proj_save_float(s, 2, "MaxRadialVelocity", particle_mod_data->data.uber_circle_spawn.max_radial_velocity);
            plugin_particle_manip_proj_save_float(s, 2, "MinArcAngle", particle_mod_data->data.uber_circle_spawn.min_arc_angle);
            plugin_particle_manip_proj_save_float(s, 2, "MaxArcAngle", particle_mod_data->data.uber_circle_spawn.max_arc_angle);
            plugin_particle_manip_proj_save_float(s, 2, "DistributeDelta", particle_mod_data->data.uber_circle_spawn.distribute_delta);
            plugin_particle_manip_proj_save_float(s, 2, "InitialAngle", particle_mod_data->data.uber_circle_spawn.initial_angle);
            break;
        case ui_particle_mod_type_uber_ellipse_spawn:
            break;
        case ui_particle_mod_type_velocity_attract:
            break;
        case ui_particle_mod_type_velocity_seed:
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseVelocityX", particle_mod_data->data.velocity_seed.min_base_velocity.value[0]);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseVelocityY", particle_mod_data->data.velocity_seed.min_base_velocity.value[1]);
            plugin_particle_manip_proj_save_float(s, 2, "MinBaseVelocityZ", particle_mod_data->data.velocity_seed.min_base_velocity.value[2]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseVelocityX", particle_mod_data->data.velocity_seed.max_base_velocity.value[0]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseVelocityY", particle_mod_data->data.velocity_seed.max_base_velocity.value[1]);
            plugin_particle_manip_proj_save_float(s, 2, "MaxBaseVelocityZ", particle_mod_data->data.velocity_seed.max_base_velocity.value[2]);
            plugin_particle_manip_proj_save_float(s, 2, "MinMultiplier", particle_mod_data->data.velocity_seed.min_multiplier);
            plugin_particle_manip_proj_save_float(s, 2, "MaxMultiplier", particle_mod_data->data.velocity_seed.max_multiplier);
            break;
        case ui_particle_mod_type_velocity_threshold_stop:
            break;
        }

        stream_printf(s, "    </MOD>\n");
    }

    if (emitter_data->tiling_u != 1.0f) plugin_particle_manip_proj_save_float(s, 1, "TilingU", emitter_data->tiling_u);
    if (emitter_data->tiling_v != 1.0f) plugin_particle_manip_proj_save_float(s, 1, "TilingV", emitter_data->tiling_v);
    if (emitter_data->blend_mode != 0) plugin_particle_manip_proj_save_int(s, 1, "BlendMode", emitter_data->blend_mode);
    if (emitter_data->atlas_x != 0.0f) plugin_particle_manip_proj_save_float(s, 1, "TexAtlasBiasX", emitter_data->atlas_x);
    if (emitter_data->atlas_y != 0.0f) plugin_particle_manip_proj_save_float(s, 1, "TexAtlasBiasY", emitter_data->atlas_y);
    if (emitter_data->atlas_w != 0.0f) plugin_particle_manip_proj_save_float(s, 1, "TexAtlasSizeX", emitter_data->atlas_w);
    if (emitter_data->atlas_h != 0.0f) plugin_particle_manip_proj_save_float(s, 1, "TexAtlasSizeY", emitter_data->atlas_h);
    if (emitter_data->collision_atlas_x != 0.0f) plugin_particle_manip_proj_save_float(s, 1, "ColAtlasBiasX", emitter_data->collision_atlas_x);
    if (emitter_data->collision_atlas_y != 0.0f) plugin_particle_manip_proj_save_float(s, 1, "ColAtlasBiasY", emitter_data->collision_atlas_y);
    if (emitter_data->collision_atlas_w != 0.0f) plugin_particle_manip_proj_save_float(s, 1, "ColAtlasSizeX", emitter_data->collision_atlas_w);
    if (emitter_data->collision_atlas_h != 0.0f) plugin_particle_manip_proj_save_float(s, 1, "ColAtlasSizeY", emitter_data->collision_atlas_h);
    if (emitter_data->origin != ui_pos_policy_center) plugin_particle_manip_proj_save_int(s, 1, "Origin", emitter_data->origin);
    if (emitter_data->texture_id) plugin_particle_manip_proj_save_str(s, 1, "BaseTexel", plugin_particle_data_emitter_msg(emitter, emitter_data->texture_id));

    stream_printf(s, "</RParticleSpriteSRC>\n");

    return 0;
}

static int plugin_particle_manip_proj_do_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    //plugin_particle_manip_t manip = ctx;
    struct vfs_write_stream fs;
    write_stream_t s;
    plugin_particle_data_t particle = ui_data_src_product(src);
    struct plugin_particle_data_emitter_it emitter_it;
    plugin_particle_data_emitter_t emitter;
    int rv = 0;

    vfs_write_stream_init(&fs, fp);
    s = (write_stream_t)&fs;
    
    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF8\" ?>\n");

    plugin_particle_data_emitters(&emitter_it, particle);

    while((emitter = plugin_particle_data_emitter_it_next(&emitter_it))) {
        if (plugin_particle_manip_proj_save_emitter_i(s, particle, emitter, em) != 0) {
            rv = -1;
        }
    }

    return rv;
}

int plugin_particle_manip_proj_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "particle", plugin_particle_manip_proj_do_save, ctx,  em);
}

int plugin_particle_manip_proj_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "particle", em);
}

void plugin_particle_manip_install_proj_saver(plugin_particle_manip_t manip) {
    ui_data_mgr_set_saver(
        ui_ed_mgr_data_mgr(manip->m_ed_mgr),
        ui_data_src_type_particle,
        plugin_particle_manip_proj_save,
        plugin_particle_manip_proj_rm,
        manip);
}
