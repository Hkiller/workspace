#include <assert.h>
#include "cpe/utils/math_ex.h"
#include "plugin/particle/plugin_particle_obj_emitter.h"
#include "plugin_barrage_bullet_i.h"
#include "plugin_barrage_bullet_proto_i.h"
#include "plugin_barrage_data_emitter_i.h"
#include "plugin_barrage_trigger_op_i.h"

static uint8_t plugin_barrage_bullet_trigger_check_one(plugin_barrage_bullet_t bullet, BARRAGE_EMITTER_BULLET_CONDITION_INFO const * condition) {
    switch(condition->condition_type) {
    case barrage_emitter_bullet_value_frame:
        return plugin_barrage_trigger_check_value_uint(bullet->m_data.frame, condition->condition_op, condition->condition_value);
    case barrage_emitter_bullet_value_pos_x:
        return plugin_barrage_trigger_check_value_float(
            plugin_particle_obj_particle_pos(bullet->m_particle)->x, condition->condition_op, condition->condition_value);
    case barrage_emitter_bullet_value_pos_y:
        return plugin_barrage_trigger_check_value_float(
            plugin_particle_obj_particle_pos(bullet->m_particle)->y, condition->condition_op, condition->condition_value);
    default:
        CPE_ERROR(
            bullet->m_emitter->m_barrage->m_group->m_env->m_module->m_em,
            "plugin_barrage_bullet_trigger_check_one: unknown conditin type %d", condition->condition_type);
        return 0;
    }
}

uint8_t plugin_barrage_bullet_trigger_check(plugin_barrage_bullet_t bullet, plugin_barrage_data_bullet_trigger_t trigger) {
    uint8_t i;

    if (trigger->m_data.condition_count == 0) return 1;
    if (trigger->m_data.condition_count == 1) return plugin_barrage_bullet_trigger_check_one(bullet, &trigger->m_data.conditions[0]);

    switch(trigger->m_data.condition_compose) {
    case barrage_emitter_condition_compose_or:
        for(i = 0; i < trigger->m_data.condition_count; ++i) {
            if (plugin_barrage_bullet_trigger_check_one(bullet, &trigger->m_data.conditions[i])) return 1;
        }
        return 0;
    case barrage_emitter_condition_compose_and:
        for(i = 0; i < trigger->m_data.condition_count; ++i) {
            if (!plugin_barrage_bullet_trigger_check_one(bullet, &trigger->m_data.conditions[i])) return 0;
        }
        return 1;
    default:
        return 0;
    }
}

static float plugin_barrage_bullet_get_value(plugin_barrage_bullet_t bullet, uint8_t value_type) {
    switch(value_type) {
    case barrage_emitter_bullet_value_pos_x:
        return plugin_particle_obj_particle_pos(bullet->m_particle)->x;
    case barrage_emitter_bullet_value_pos_y:
        return plugin_particle_obj_particle_pos(bullet->m_particle)->y;
    case barrage_emitter_bullet_value_life_circle:
        return bullet->m_data.life_circle;
    case barrage_emitter_bullet_value_scale_x: {
        ui_vector_2_t base_size = plugin_particle_obj_particle_base_size(bullet->m_particle);
        ui_vector_2_t texture_size = plugin_particle_obj_emitter_texture_size(bullet->m_proto->m_emitter);
        return fabs(base_size->x / texture_size->x);
    }
    case barrage_emitter_bullet_value_scale_y: {
        ui_vector_2_t base_size = plugin_particle_obj_particle_base_size(bullet->m_particle);
        ui_vector_2_t texture_size = plugin_particle_obj_emitter_texture_size(bullet->m_proto->m_emitter);
        return fabs(base_size->y / texture_size->y);
    }
    case barrage_emitter_bullet_value_color_r:
        return bullet->m_data.color.r;
    case barrage_emitter_bullet_value_color_g:
        return bullet->m_data.color.g;
    case barrage_emitter_bullet_value_color_b:
        return bullet->m_data.color.b;
    case barrage_emitter_bullet_value_color_a:
        return bullet->m_data.color.a;
    case barrage_emitter_bullet_value_angle:
        return cpe_math_radians_to_angle(plugin_particle_obj_particle_spin_init(bullet->m_particle));
    case barrage_emitter_bullet_value_speed:
        return bullet->m_data.speed;
    case barrage_emitter_bullet_value_speed_angle:
        return cpe_math_radians_to_angle(bullet->m_data.speed_angle_rad);
    case barrage_emitter_bullet_value_acceleration:
        return bullet->m_data.acceleration;
    case barrage_emitter_bullet_value_acceleration_angle:
        return bullet->m_data.acceleration_angle;
    case barrage_emitter_bullet_value_x_rate:
        return bullet->m_data.x_rate;
    case barrage_emitter_bullet_value_y_rate:
        return bullet->m_data.y_rate;
    default:
        return 0.0f;
    }
}

static void plugin_barrage_bullet_set_value(plugin_barrage_bullet_t bullet, uint8_t value_type, float value) {
    switch(value_type) {
    case barrage_emitter_bullet_value_pos_x: {
        ui_vector_2 pos = *plugin_particle_obj_particle_pos(bullet->m_particle);
        pos.x = value;
        plugin_particle_obj_particle_set_pos(bullet->m_particle, &pos);
        break;
    }
    case barrage_emitter_bullet_value_pos_y: {
        ui_vector_2 pos = *plugin_particle_obj_particle_pos(bullet->m_particle);
        pos.y = value;
        plugin_particle_obj_particle_set_pos(bullet->m_particle, &pos);
        break;
    }
    case barrage_emitter_bullet_value_life_circle:
        bullet->m_data.life_circle = value;
        break;
    case barrage_emitter_bullet_value_scale_x: {
        ui_vector_2 base_size = *plugin_particle_obj_particle_base_size(bullet->m_particle);
        ui_vector_2_t texture_size = plugin_particle_obj_emitter_texture_size(bullet->m_proto->m_emitter);
        base_size.x = value * texture_size->x * (base_size.x < 0.0f ? -1.0f : 1.0f);
        plugin_particle_obj_particle_set_base_size(bullet->m_particle, &base_size);
        plugin_barrage_bullet_update_shape(bullet);
        break;
    }
    case barrage_emitter_bullet_value_scale_y: {
        ui_vector_2 base_size = *plugin_particle_obj_particle_base_size(bullet->m_particle);
        ui_vector_2_t texture_size = plugin_particle_obj_emitter_texture_size(bullet->m_proto->m_emitter);
        base_size.y = value * texture_size->y * (base_size.y < 0.0f ? -1.0f : 1.0f);
        plugin_particle_obj_particle_set_base_size(bullet->m_particle, &base_size);
        plugin_barrage_bullet_update_shape(bullet);
        break;
    }
    case barrage_emitter_bullet_value_color_r:
        bullet->m_data.color.r = value;
        break;
    case barrage_emitter_bullet_value_color_g:
        bullet->m_data.color.g = value;
        break;
    case barrage_emitter_bullet_value_color_b:
        bullet->m_data.color.b = value;
        break;
    case barrage_emitter_bullet_value_color_a:
        bullet->m_data.color.a = value;
        break;
    case barrage_emitter_bullet_value_angle:
        plugin_particle_obj_particle_set_spin_init(
            bullet->m_particle,
            cpe_math_angle_to_radians(cpe_math_angle_regular(value)));
        break;
    case barrage_emitter_bullet_value_speed:
        bullet->m_data.speed = value;
		plugin_barrage_bullet_update_speed_angle(bullet);
        break;
    case barrage_emitter_bullet_value_speed_angle:
        bullet->m_data.speed_angle_rad = cpe_math_angle_to_radians(cpe_math_angle_regular(value));
		plugin_barrage_bullet_update_speed_angle(bullet);
		break;
    case barrage_emitter_bullet_value_acceleration:
        bullet->m_data.acceleration = value;
        break;
    case barrage_emitter_bullet_value_acceleration_angle:
        bullet->m_data.acceleration_angle = cpe_math_angle_regular(value);
        break;
    case barrage_emitter_bullet_value_x_rate:
        bullet->m_data.x_rate = value;
        break;
    case barrage_emitter_bullet_value_y_rate:
        bullet->m_data.y_rate = value;
        break;
    default:
        break;
    }
}

static void plugin_barrage_bullet_trigger_change_line(void* ctx) {
    plugin_barrage_trigger_op_t trigger_op = (plugin_barrage_trigger_op_t)ctx;
    plugin_barrage_bullet_t bullet = (plugin_barrage_bullet_t)trigger_op->m_op_target;
    plugin_barrage_env_t env = bullet->m_env;
    float percent;
    float target_value;

    assert(trigger_op->m_total_tick > 0);

    trigger_op->m_cur_tick++;

    percent = ((float)trigger_op->m_cur_tick) / (float)trigger_op->m_total_tick;

    target_value = trigger_op->m_base_value + (trigger_op->m_to_value - trigger_op->m_base_value) * percent;
        
    plugin_barrage_bullet_set_value(bullet, trigger_op->m_result_type, target_value);

    if (trigger_op->m_cur_tick < trigger_op->m_total_tick) {
        trigger_op->m_op.m_op_frame = env->m_cur_frame + 1;
        trigger_op->m_op.m_op_fun = plugin_barrage_bullet_trigger_change_line;
        trigger_op->m_op.m_op_ctx = trigger_op;
        plugin_barrage_op_enqueue(env, &trigger_op->m_op);
    }
    else {
        plugin_barrage_trigger_op_free(env, trigger_op);
    }
}

static void plugin_barrage_bullet_trigger_change_sin(void* ctx) {
    plugin_barrage_trigger_op_t trigger_op = (plugin_barrage_trigger_op_t)ctx;
    plugin_barrage_bullet_t bullet = (plugin_barrage_bullet_t)trigger_op->m_op_target;
    plugin_barrage_env_t env = bullet->m_env;
    float percent;
    float delta;
    float target_value;

    assert(trigger_op->m_total_tick > 0);

    trigger_op->m_cur_tick++;

    percent = ((float)trigger_op->m_cur_tick) / (float)trigger_op->m_total_tick;

    delta = trigger_op->m_to_value - trigger_op->m_base_value;
    
    target_value = trigger_op->m_base_value + delta * cpe_sin_radians(2 * M_PI * percent);
        
    plugin_barrage_bullet_set_value(bullet, trigger_op->m_result_type, target_value);

    if (trigger_op->m_cur_tick < trigger_op->m_total_tick) {
        trigger_op->m_op.m_op_frame = env->m_cur_frame + 1;
        trigger_op->m_op.m_op_fun = plugin_barrage_bullet_trigger_change_sin;
        trigger_op->m_op.m_op_ctx = trigger_op;
        plugin_barrage_op_enqueue(env, &trigger_op->m_op);
    }
    else {
        plugin_barrage_trigger_op_free(env, trigger_op);
    }
}

void plugin_barrage_bullet_trigger_do(plugin_barrage_bullet_t bullet, plugin_barrage_data_bullet_trigger_t trigger) {
    plugin_barrage_env_t env = bullet->m_env;
    float base_value;
    float target_value;
    plugin_barrage_op_fun_t op_fun;
    plugin_barrage_trigger_op_t trigger_op;
    float value;
    plugin_barrage_data_emitter_flip_type_t flip_type;
    
    base_value = plugin_barrage_bullet_get_value(bullet, trigger->m_data.result_type);

    switch(trigger->m_data.result_value.type) {
    case barrage_emitter_value_value:
        value = trigger->m_data.result_value.data.value;
        flip_type = bullet->m_flip_type;
        break;
    case barrage_emitter_value_calc:
        if (plugin_barrage_calc_value(&value, trigger->m_data.result_value.data.calc_type, bullet->m_emitter, bullet, env->m_module->m_em) != 0) return;
        flip_type = plugin_barrage_data_emitter_flip_type_none;
        break;
    default:
        CPE_ERROR(env->m_module->m_em, "plugin_barrage_bullet_trigger_do: unknown value type %d", trigger->m_data.result_value.type);
        return;
    }

    switch(trigger->m_data.result_type) {
    case barrage_emitter_bullet_value_angle:
    case barrage_emitter_bullet_value_speed_angle:
    case barrage_emitter_bullet_value_acceleration_angle:
        target_value = plugin_barrage_trigger_calc_angle(base_value, trigger->m_data.result_op, value, flip_type);
        break;
    default:
        target_value = plugin_barrage_trigger_calc_value(base_value, trigger->m_data.result_op, value);
        break;
    }
    
    if (base_value == target_value) {
        plugin_barrage_bullet_set_value(bullet, trigger->m_data.result_type, target_value);
        return;
    }
    
    switch(trigger->m_data.result_change_type) {
    case barrage_emitter_result_change_fix:
        plugin_barrage_bullet_set_value(bullet, trigger->m_data.result_type, target_value);
        return;
    case barrage_emitter_result_change_line:
        op_fun = plugin_barrage_bullet_trigger_change_line;
        break;
    case barrage_emitter_result_change_sin:
        op_fun = plugin_barrage_bullet_trigger_change_sin;
        break;
    default:
        CPE_ERROR(
            bullet->m_env->m_module->m_em, "plugin_barrage_bullet_trigger_do: unknown change type %d",
            trigger->m_data.result_change_type);
        return;
    }

    if (trigger->m_data.result_change_duration <= 0) {
        CPE_ERROR(
            bullet->m_env->m_module->m_em, "plugin_barrage_bullet_trigger_do: change duration %d error",
            trigger->m_data.result_change_duration);
        return;
    }

    trigger_op = plugin_barrage_trigger_op_create(env, &bullet->m_trigger_ops);
    if (trigger_op == NULL) {
        CPE_ERROR(bullet->m_env->m_module->m_em, "plugin_barrage_bullet_trigger_do: chreate trigger op fail");
        return;
    }
    
    trigger_op->m_base_value = base_value;
    trigger_op->m_to_value = target_value;
    trigger_op->m_total_tick = trigger->m_data.result_change_duration;
    trigger_op->m_cur_tick = 0;
    trigger_op->m_result_type = trigger->m_data.result_type;
    trigger_op->m_op_target = bullet;

    trigger_op->m_op.m_op_frame = env->m_cur_frame;
    trigger_op->m_op.m_op_fun = op_fun;
    trigger_op->m_op.m_op_ctx = trigger_op;
    plugin_barrage_op_enqueue(env, &trigger_op->m_op);
}
