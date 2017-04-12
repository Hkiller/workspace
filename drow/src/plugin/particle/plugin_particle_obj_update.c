#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "cpe/pal/pal_stdlib.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/runtime/ui_runtime_render_obj.h"
#include "plugin_particle_obj_i.h"
#include "plugin_particle_data_i.h"
#include "plugin_particle_obj_emitter_i.h"
#include "plugin_particle_obj_emitter_mod_i.h"
#include "plugin_particle_obj_emitter_binding_i.h"
#include "plugin_particle_obj_mod_data_i.h"
#include "plugin_particle_obj_particle_i.h"
#include "plugin_particle_obj_plugin_i.h"
#include "plugin_particle_obj_plugin_data_i.h"

static void plugin_particle_obj_emitter_spwan_particles(
    plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_runtime_t runtime,
    plugin_particle_obj_emitter_binding_t binding, UI_PARTICLE_EMITTER const * emitter_data, float delta);

static uint8_t plugin_particle_obj_emitter_mod_need_process(
    plugin_particle_obj_particle_t particle, UI_PARTICLE_MOD const * mod_data, uint8_t * need_calc_distance)
{
    int8_t time_check = -1;
    int8_t distance_check = -1;
    
    if (mod_data->begin_time > 0.0f || mod_data->end_time > 0.0f) {
        float t = particle->m_relative_time / particle->m_one_over_max_life;

        time_check = 1;
        if (mod_data->begin_time > 0.0f && t < mod_data->begin_time) {
            time_check = 0;
        }
        if (mod_data->end_time > 0.0f && t > mod_data->end_time) {
            time_check = 0;
        }
    }

    if (mod_data->begin_distance > 0.0f || mod_data->end_distance > 0.0f) {
        *need_calc_distance = 1;
        distance_check = 1;
        if (mod_data->begin_distance > 0.0f && particle->m_moved_distance < mod_data->begin_distance) {
            distance_check = 0;
        }
        if (mod_data->end_distance > 0.0f && particle->m_moved_distance > mod_data->end_distance) {
            distance_check = 0;
        }
    }

    if (time_check == 0) {
        if (distance_check != 1) {
            return 0;
        }
    }

    if (distance_check == 0) {
        if (time_check != 1) {
            return 0;
        }
    }

    return 1;
}

static void plugin_particle_obj_emitter_update_particles(plugin_particle_obj_emitter_t emitter, UI_PARTICLE_EMITTER const * emitter_data, float delta) {
    plugin_particle_obj_particle_t particle;
    plugin_particle_obj_particle_t particle_next;
    
    for(particle = TAILQ_FIRST(&emitter->m_particles); particle != TAILQ_END(&emitter->m_particles); particle = particle_next) {
        ui_vector_2 accel;
        ui_vector_2 prev_location;
        float pre_spin = 0.0f;
        uint8_t need_calc_distance = 0;
        
        particle_next = TAILQ_NEXT(particle, m_next);

        if (!TAILQ_EMPTY(&particle->m_follow_particles)) {
            pre_spin = plugin_particle_obj_particle_spin(particle);
        }
        
        particle->m_relative_time += particle->m_one_over_max_life * delta * particle->m_time_scale;

        if (particle->m_relative_time >= 1.0f) {
            if (particle->m_repeat_count == 1) {
                plugin_particle_obj_particle_free_with_dead_anim_r(particle);
                continue;
            }
            else {
                if (particle->m_repeat_count > 0) particle->m_repeat_count--;
                particle->m_moved_distance = 0.0f;
                while(particle->m_relative_time >= 1.0f) {
                    particle->m_relative_time -= 1.0f;
                }
            }
        }
        
		assert(particle->m_relative_time < 1.0f);
        cpe_assert_float_sane(particle->m_base_size.x);
        cpe_assert_float_sane(particle->m_base_size.y);

		accel = particle->m_base_accel;
		prev_location = particle->m_location;
		particle->m_color = particle->m_base_color;
		particle->m_size = particle->m_base_size;
        
        if (!TAILQ_EMPTY(&particle->m_emitter->m_mods)) {
            plugin_particle_obj_emitter_mod_t mod;
            plugin_particle_obj_mod_data_t cur_mod_data = TAILQ_FIRST(&emitter->m_mod_datas);
            int16_t idx = -1;
            
            TAILQ_FOREACH(mod, &particle->m_emitter->m_mods, m_next) {
                plugin_particle_obj_mod_data_t mod_data = NULL;

                assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);

                idx++;
                if (cpe_ba_get(particle->m_disable_mods, idx)) continue;
                
                if (g_mod_defs[mod->m_data.type].data_init) {
                    assert(cur_mod_data);
                    mod_data = cur_mod_data;
                    cur_mod_data = TAILQ_NEXT(cur_mod_data, m_next);
                }

                if (g_mod_defs[mod->m_data.type].particle_update) {
                    if (plugin_particle_obj_emitter_mod_need_process(particle, &mod->m_data, &need_calc_distance)) {
                        g_mod_defs[mod->m_data.type].particle_update(&mod->m_data, mod_data, particle, &accel, &prev_location, delta);
                    }
                }
            }
        }
        else if (particle->m_emitter->m_static_data) {
            plugin_particle_data_mod_t mod;
            plugin_particle_obj_mod_data_t cur_mod_data = TAILQ_FIRST(&emitter->m_mod_datas);
            int16_t idx = -1;
            
            TAILQ_FOREACH(mod, &particle->m_emitter->m_static_data->m_mods, m_next_for_emitter) {
                plugin_particle_obj_mod_data_t mod_data = NULL;

                assert(mod->m_data.type >= UI_PARTICLE_MOD_TYPE_MIN && mod->m_data.type < UI_PARTICLE_MOD_TYPE_MAX);

                idx++;
                if (cpe_ba_get(particle->m_disable_mods, idx)) continue;
                
                if (g_mod_defs[mod->m_data.type].data_init) {
                    assert(cur_mod_data);
                    mod_data = cur_mod_data;
                    cur_mod_data = TAILQ_NEXT(cur_mod_data, m_next);
                }

                if (g_mod_defs[mod->m_data.type].particle_update) {
                    if (plugin_particle_obj_emitter_mod_need_process(particle, &mod->m_data, &need_calc_distance)) {
                        g_mod_defs[mod->m_data.type].particle_update(&mod->m_data, mod_data, particle, &accel, &prev_location, delta);
                    }
                }
            }
        }

		/* accumulate position and velocity ( approximate ) */
		particle->m_velocity.x += accel.x * delta;
		particle->m_velocity.y += accel.y * delta;
        
		particle->m_location.x += particle->m_velocity.x * delta;
		particle->m_location.y += particle->m_velocity.y * delta;        

        //todo
        need_calc_distance = 1;
        if (need_calc_distance) {
            particle->m_moved_distance += cpe_math_distance(0.0f, 0.0f, particle->m_velocity.x * delta, particle->m_velocity.y * delta);
        }
        
        if(!TAILQ_EMPTY(&particle->m_plugin_datas)) {
            plugin_particle_obj_plugin_data_t plugin_data;
            TAILQ_FOREACH(plugin_data, &particle->m_plugin_datas, m_next_for_particle) {
                if (plugin_data->m_plugin->m_update_fun) {
                    plugin_data->m_plugin->m_update_fun(plugin_data->m_plugin->m_ctx, plugin_data);
                }
            }
        }

		if(emitter_data->auto_up_dir && particle->m_spin_rate == 0.0f) {
            plugin_particle_obj_particle_calc_auto_dir_up(particle);
		}

        /*更新follow particle的相对位置 */
        if (!TAILQ_EMPTY(&particle->m_follow_particles)) {
            plugin_particle_obj_particle_t follow_particle;
            ui_vector_2 pos_diff;
            float spin_diff = plugin_particle_obj_particle_spin(particle) - pre_spin;

            if (particle->m_spawn_scale.x * particle->m_spawn_scale.y * particle->m_size.x * particle->m_size.y < 0.0f) {
                spin_diff = -spin_diff;
            }
            cpe_assert_float_sane(spin_diff);
            
            pos_diff.x = particle->m_location.x - prev_location.x;
            pos_diff.y = particle->m_location.y - prev_location.y;
            
            TAILQ_FOREACH(follow_particle, &particle->m_follow_particles, m_next_for_follow) {
                if (follow_particle->m_follow_is_tie == 0) continue;

                follow_particle->m_location.x += pos_diff.x;
                follow_particle->m_location.y += pos_diff.y;

                if (follow_particle->m_follow_angle) {
                    follow_particle->m_spin_init += spin_diff;
                    cpe_assert_float_sane(follow_particle->m_spin_init);
                }

                if (follow_particle->m_follow_scale) {
                }
            }
        }

        /*检查跟随发射器的发射,生成新跟随粒子 */
        if (!TAILQ_EMPTY(&particle->m_binding_emitters)) {
            plugin_particle_obj_emitter_binding_t binding;
            
            TAILQ_FOREACH(binding, &particle->m_binding_emitters, m_next_for_particle) {
                if (binding->m_emitter->m_use_state == plugin_particle_obj_emitter_use_state_suspend) continue;

                plugin_particle_obj_emitter_spwan_particles(
                    binding->m_emitter, &binding->m_runtime, binding,
                    plugin_particle_obj_emitter_data_r(binding->m_emitter), delta);
            }
        }
    }
}

static void plugin_particle_obj_emitter_spwan_particles(
    plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_runtime_t runtime,
    plugin_particle_obj_emitter_binding_t binding, UI_PARTICLE_EMITTER const * emitter_data, float delta)
{
    float rate = 0.0f;
    int32_t extra = 0;
                
    runtime->m_emit_counter += delta;
    if (emitter_data->spawn_rate == 0) {
        runtime->m_emit_counter = 0;
    }
    else {
        rate = 1.0f / emitter_data->spawn_rate;
    }

    if (runtime->m_is_first_shoot) {
        if (emitter_data->on_emitter_begin_id) {
            ui_runtime_render_obj_send_event(
                ui_runtime_render_obj_from_data(emitter->m_obj),
                plugin_particle_obj_emitter_msg(emitter, emitter_data->on_emitter_begin_id));
        }
        
        extra = (int32_t)(
            (float)emitter_data->min_extra_brust
            +  ((float)emitter_data->max_extra_brust - (float)emitter_data->min_extra_brust) * ((float)rand() / RAND_MAX));

        runtime->m_is_first_shoot = 0;
    }

    if (binding) {
        UI_PARTICLE_EMITTER const * bind_emitter_data = plugin_particle_obj_emitter_data_r(binding->m_particle->m_emitter);
        ui_transform o;
        ui_vector_3 o_s;
        
        plugin_particle_obj_particle_calc_transform(binding->m_particle, &o);

        o_s = o.m_s;
        
        if (!binding->m_accept_scale) {
            o_s.x = o_s.x < 0.0f ? -1.0f : 1.0f;
            o_s.y = o_s.y < 0.0f ? -1.0f : 1.0f;
            o_s.z = o_s.z < 0.0f ? -1.0f : 1.0f;
        }

        if (binding->m_particle->m_size.x < 0.0f) o_s.x *= -1.0f;
        if (binding->m_particle->m_size.y < 0.0f) o_s.y *= -1.0f;

        ui_transform_set_scale(&o, &o_s);

        if (!binding->m_accept_angle) {
            ui_transform_set_quation(&o, &UI_QUATERNION_IDENTITY);
        }
        
        if (emitter_data->xform_mod == UI_PARTICLE_XFORM_WORLD) {
            if (bind_emitter_data->xform_mod == UI_PARTICLE_XFORM_LOCAL) {
                ui_transform_adj_by_parent(&o, ui_runtime_render_obj_transform(ui_runtime_render_obj_from_data(emitter->m_obj)));
            }
        }
        else {
            if (bind_emitter_data->xform_mod == UI_PARTICLE_XFORM_WORLD) {
                ui_transform_t p = ui_runtime_render_obj_transform(ui_runtime_render_obj_from_data(emitter->m_obj));

                if(ui_transform_scale_zero(p)) {
                    o = UI_TRANSFORM_ZERO;
                }
                else {
                    ui_transform p_r;
                    ui_transform_reverse(&p_r, p);
                    ui_transform_adj_by_parent(&p_r, &o);
                    o = p_r;
                }
            }
        }

        if (!ui_transform_scale_zero(&o)) {
            while(emitter->m_particle_count < emitter_data->max_amount && (runtime->m_emit_counter > rate || extra-- > 0)) {
                plugin_particle_obj_particle_t follow_particle;

                follow_particle = plugin_particle_obj_particle_create_at(emitter, &o);
                if (follow_particle == NULL) {
                    CPE_ERROR(emitter->m_obj->m_module->m_em, "plugin_particle_obj_emitter_spwan_particles: create particle fail!");
                }
                else {
                    plugin_particle_obj_particle_set_follow_to(
                        follow_particle, binding->m_particle, binding->m_is_tie, binding->m_accept_angle, binding->m_accept_scale);
                }
            
                if (runtime->m_emit_counter > rate) {
                    runtime->m_emit_counter -= rate;
                }
            }
        }
    }
    else {
        while(emitter->m_particle_count < emitter_data->max_amount && (runtime->m_emit_counter > rate || extra-- > 0)) {
            plugin_particle_obj_particle_t particle;

            particle = plugin_particle_obj_particle_create(emitter);
            if (particle == NULL) {
                CPE_ERROR(emitter->m_obj->m_module->m_em, "plugin_particle_obj_emitter_spwan_particles: create particle fail!");
            }

            if (runtime->m_emit_counter > rate) {
                runtime->m_emit_counter -= rate;
            }
        }
    }
}

void plugin_particle_emitter_update_active(
    plugin_particle_obj_emitter_t emitter, plugin_particle_obj_emitter_runtime_t runtime,
    UI_PARTICLE_EMITTER const * emitter_data, float emitter_delta_time, float delay_time)
{
    assert(emitter->m_use_state == plugin_particle_obj_emitter_use_state_active);
    
    /*处理一次循环完成 */
    if (emitter_data->duration > 0 && runtime->m_elapsed_time >= emitter_data->duration + delay_time) {
        if(emitter->m_particle_count > 0) {
            cpe_ba_set(emitter->m_flags, plugin_particle_obj_emitter_state_waitstop, cpe_ba_true);
            return;
        }

        plugin_particle_obj_emitter_do_deactive(emitter);
        emitter->m_use_state = plugin_particle_obj_emitter_use_state_suspend;
        goto NEXT_REPEAT;
    }

    if (emitter->m_obj->m_enable
        && !cpe_ba_get(emitter->m_flags, plugin_particle_obj_emitter_state_pendkill)
        && !cpe_ba_get(emitter->m_flags, plugin_particle_obj_emitter_state_waitstop))
    {
        /*处理新发射的粒子 */
        plugin_particle_obj_emitter_spwan_particles(emitter, &emitter->m_runtime, NULL, emitter_data, emitter_delta_time);
    }
    else if (emitter->m_particle_count == 0) {
        assert(emitter->m_use_state == plugin_particle_obj_emitter_use_state_active);
        plugin_particle_obj_emitter_do_deactive(emitter);
        emitter->m_use_state = plugin_particle_obj_emitter_use_state_suspend;
        goto NEXT_REPEAT;
    }

    return;

NEXT_REPEAT:
    if (emitter->m_obj->m_enable
        && !cpe_ba_get(emitter->m_flags, plugin_particle_obj_emitter_state_pendkill))
    {
        if(emitter_data->repeat_time == 0
           || emitter->m_play_counter < emitter_data->repeat_time)
        {
            assert(emitter->m_use_state == plugin_particle_obj_emitter_use_state_suspend);
            emitter->m_use_state = plugin_particle_obj_emitter_use_state_active;
            plugin_particle_obj_emitter_do_active(emitter);
        }
        else {
            switch(emitter->m_lifecircle) {
            case plugin_particle_obj_emitter_lifecircle_basic:
                break;
            case plugin_particle_obj_emitter_lifecircle_remove_on_complete:
                assert(emitter->m_use_state == plugin_particle_obj_emitter_use_state_suspend);
                plugin_particle_obj_emitter_free(emitter);
                break;
            default:
                assert(0);
                break;
            }
        }
    }
}

void plugin_particle_obj_update(void * ctx, ui_runtime_render_obj_t obj, float delta) {
    plugin_particle_obj_t particle_obj = ui_runtime_render_obj_data(obj);
    plugin_particle_obj_emitter_t emitter, next;

    /*遍历所有的发射器 */
    for(emitter = TAILQ_FIRST(&particle_obj->m_emitters); emitter; emitter = next) {
        UI_PARTICLE_EMITTER const * emitter_data;
        float emitter_delta_time;

        next = TAILQ_NEXT(emitter, m_next);
        
        if (emitter->m_use_state == plugin_particle_obj_emitter_use_state_suspend) continue;

        emitter_data = plugin_particle_obj_emitter_data_r(emitter);

        if (emitter->m_runtime.m_is_first_frame) {
            emitter->m_runtime.m_is_first_frame = 0;
            emitter_delta_time = 0.0f;
        }
        else {
            emitter_delta_time = delta * emitter->m_time_scale * emitter_data->time_scale;
        }
        
        if(emitter->m_use_state == plugin_particle_obj_emitter_use_state_active) {
            float delay_time = emitter->m_play_counter > 1 ? emitter_data->loop_delay_time : emitter_data->init_delay_time;

            emitter->m_runtime.m_elapsed_time += emitter_delta_time;
            if(emitter->m_runtime.m_elapsed_time < delay_time) continue;

            /*首先更新所有的粒子 */
            plugin_particle_obj_emitter_update_particles(emitter, emitter_data, emitter_delta_time);

            plugin_particle_emitter_update_active(emitter, &emitter->m_runtime, emitter_data, emitter_delta_time, delay_time);
        }
        else if(emitter->m_use_state == plugin_particle_obj_emitter_use_state_passive) {
            plugin_particle_obj_emitter_update_particles(emitter, emitter_data, emitter_delta_time);
        }
    }
}
