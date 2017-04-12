#include <assert.h>
#include <stdio.h>
#include "cpe/utils/math_ex.h"
#include "plugin_barrage_emitter_i.h"
#include "plugin_barrage_data_emitter_i.h"
#include "plugin_barrage_trigger_op_i.h"
#include "plugin_barrage_utils_i.h"

static void plugin_barrage_emitter_do_sync_trigger(plugin_barrage_emitter_t emitter);
    
static uint8_t plugin_barrage_emitter_trigger_check_one(plugin_barrage_emitter_t emitter, BARRAGE_EMITTER_EMITTER_CONDITION_INFO const * condition) {
    switch(condition->condition_type) {
    case barrage_emitter_emitter_value_frame:
        return plugin_barrage_trigger_check_value_uint(emitter->m_barrage->m_data.frame, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_pos_x:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.emitter.pos.x, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_pos_y:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.emitter.pos.y, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_pos_radius:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.emitter.emitter_pos_radius.base, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_pos_angle:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.emitter.emitter_pos_angle.base, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_count:
        return plugin_barrage_trigger_check_value_uint(emitter->m_data.emitter.emitter_count.base, condition->condition_op, (uint8_t)condition->condition_value);
    case barrage_emitter_emitter_value_span:
        return plugin_barrage_trigger_check_value_uint(emitter->m_data.emitter.emitter_span.base, condition->condition_op, (uint8_t)condition->condition_value);
    case barrage_emitter_emitter_value_angle:
        if (emitter->m_data.emitter.emitter_angle.type == barrage_emitter_value_value) {
            return plugin_barrage_trigger_check_value_float(emitter->m_data.emitter.emitter_angle.data.value.base, condition->condition_op, condition->condition_value);
        }
        else {
            return 0;
        }
    case barrage_emitter_emitter_value_angle_range:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.emitter.emitter_angle_range.base, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_x_rate:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.bullet.x_rate.base, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_y_rate:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.bullet.y_rate.base, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_alpha:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.bullet.color.a, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_speed:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.bullet.speed.base, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_acceleration:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.bullet.acceleration.base, condition->condition_op, condition->condition_value);
    case barrage_emitter_emitter_value_acceleration_angle:
        return plugin_barrage_trigger_check_value_float(emitter->m_data.bullet.acceleration_angle.base, condition->condition_op, condition->condition_value);
    default:
        CPE_ERROR(
            emitter->m_barrage->m_group->m_env->m_module->m_em,
            "plugin_barrage_emitter_trigger_check_one: unknown conditin type %d", condition->condition_type);
        return 0;
    }
}

uint8_t plugin_barrage_emitter_trigger_check(plugin_barrage_emitter_t emitter, plugin_barrage_data_emitter_trigger_t trigger) {
    uint8_t i;

    if (trigger->m_data.condition_count == 0) return 1;
    if (trigger->m_data.condition_count == 1) return plugin_barrage_emitter_trigger_check_one(emitter, &trigger->m_data.conditions[0]);

    switch(trigger->m_data.condition_compose) {
    case barrage_emitter_condition_compose_or:
        for(i = 0; i < trigger->m_data.condition_count; ++i) {
            if (plugin_barrage_emitter_trigger_check_one(emitter, &trigger->m_data.conditions[i])) return 1;
        }
        return 0;
    case barrage_emitter_condition_compose_and:
        for(i = 0; i < trigger->m_data.condition_count; ++i) {
            if (!plugin_barrage_emitter_trigger_check_one(emitter, &trigger->m_data.conditions[i])) return 0;
        }
        return 1;
    default:
        return 0;
    }
}

static float plugin_barrage_emitter_get_value(plugin_barrage_emitter_t emitter, uint8_t value_type) {
    switch(value_type) {
    case barrage_emitter_emitter_value_pos_x:
        return emitter->m_data.emitter.pos.x;
    case barrage_emitter_emitter_value_pos_y:
        return emitter->m_data.emitter.pos.y;
    case barrage_emitter_emitter_value_pos_radius:
        return emitter->m_data.emitter.emitter_pos_radius.base;
    case barrage_emitter_emitter_value_pos_angle:
        return emitter->m_data.emitter.emitter_pos_angle.base;
    case barrage_emitter_emitter_value_count:
        return emitter->m_data.emitter.emitter_count.base;
    case barrage_emitter_emitter_value_span:
        return emitter->m_data.emitter.emitter_span.base;
    case barrage_emitter_emitter_value_angle:
        switch(emitter->m_data.emitter.emitter_angle.type) {
        case barrage_emitter_value_value:
            return emitter->m_data.emitter.emitter_angle.data.value.base;
        case barrage_emitter_value_calc: {
            float v;
            if (plugin_barrage_calc_value(
                    &v, emitter->m_data.emitter.emitter_angle.data.calc_type,
                    emitter, NULL, emitter->m_barrage->m_group->m_env->m_module->m_em) != 0)
            {
                return 0.0f;
            }
            else {
                return v;
            }
        }
        default:
            return 0.0f;
        }
    case barrage_emitter_emitter_value_angle_range:
        return emitter->m_data.emitter.emitter_angle_range.base;
    case barrage_emitter_emitter_value_x_rate:
        return emitter->m_data.bullet.x_rate.base;
    case barrage_emitter_emitter_value_y_rate:
        return emitter->m_data.bullet.y_rate.base;
    case barrage_emitter_emitter_value_alpha:
        return emitter->m_data.bullet.color.a;
    case barrage_emitter_emitter_value_speed:
        return emitter->m_data.bullet.speed.base;
    case barrage_emitter_emitter_value_acceleration:
        return emitter->m_data.bullet.acceleration.base;
    case barrage_emitter_emitter_value_acceleration_angle:
        return emitter->m_data.bullet.acceleration_angle.base;
    default:
        return 0.0f;
    }
}

static void plugin_barrage_emitter_set_value(plugin_barrage_emitter_t emitter, uint8_t value_type, float value) {
    switch(value_type) {
    case barrage_emitter_emitter_value_pos_x:
        emitter->m_data.emitter.pos.x = value;
        break;
    case barrage_emitter_emitter_value_pos_y:
        emitter->m_data.emitter.pos.y = value;
        break;
    case barrage_emitter_emitter_value_pos_radius:
        emitter->m_data.emitter.emitter_pos_radius.base = value;
        break;
    case barrage_emitter_emitter_value_pos_angle:
        emitter->m_data.emitter.emitter_pos_angle.base = cpe_math_angle_regular(value);
        break;
    case barrage_emitter_emitter_value_count:
        emitter->m_data.emitter.emitter_count.base = (uint8_t)value;
        emitter->m_data.next_emitter_count = plugin_barrage_calc_uint8(&emitter->m_data.emitter.emitter_count);
        plugin_barrage_emitter_do_sync_trigger(emitter);
        break;
    case barrage_emitter_emitter_value_span:
        emitter->m_data.emitter.emitter_span.base = (uint8_t)value;
        emitter->m_data.next_emitter_span = plugin_barrage_calc_uint8(&emitter->m_data.emitter.emitter_span);
        plugin_barrage_emitter_do_sync_trigger(emitter);
        break;
    case barrage_emitter_emitter_value_angle:
        emitter->m_data.emitter.emitter_angle.type = barrage_emitter_value_value;
        emitter->m_data.emitter.emitter_angle.data.value.base = cpe_math_angle_regular(value);
        break;
    case barrage_emitter_emitter_value_angle_range:
        emitter->m_data.emitter.emitter_angle_range.base = value;
        break;
    case barrage_emitter_emitter_value_x_rate:
        emitter->m_data.bullet.x_rate.base = value;
        break;
    case barrage_emitter_emitter_value_y_rate:
        emitter->m_data.bullet.y_rate.base = value;
        break;
    case barrage_emitter_emitter_value_alpha:
        emitter->m_data.bullet.color.a = value;
        break;
    case barrage_emitter_emitter_value_speed:
        emitter->m_data.bullet.speed.base = value;
        break;
    case barrage_emitter_emitter_value_acceleration:
        emitter->m_data.bullet.acceleration.base = value;
        break;
    case barrage_emitter_emitter_value_acceleration_angle:
        emitter->m_data.bullet.acceleration_angle.base = value;
        break;
    default:
        CPE_ERROR(
            emitter->m_barrage->m_group->m_env->m_module->m_em,
            "plugin_barrage_emitter_set_value: unknown value type %d", value_type);
        break;
    }
}

static void plugin_barrage_emitter_trigger_change_line(void* ctx) {
    plugin_barrage_trigger_op_t trigger_op = (plugin_barrage_trigger_op_t)ctx;
    plugin_barrage_emitter_t emitter = (plugin_barrage_emitter_t)trigger_op->m_op_target;
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;
    float percent;
    float target_value;

    assert(emitter->m_is_working);
    assert(trigger_op->m_total_tick > 0);

    trigger_op->m_cur_tick++;

    percent = ((float)trigger_op->m_cur_tick) / (float)trigger_op->m_total_tick;

    target_value = trigger_op->m_base_value + (trigger_op->m_to_value - trigger_op->m_base_value) * percent;
        
    plugin_barrage_emitter_set_value(emitter, trigger_op->m_result_type, target_value);

    if (trigger_op->m_cur_tick < trigger_op->m_total_tick) {
        trigger_op->m_op.m_op_frame = env->m_cur_frame + 1;
        trigger_op->m_op.m_op_fun = plugin_barrage_emitter_trigger_change_line;
        trigger_op->m_op.m_op_ctx = trigger_op;
        plugin_barrage_op_enqueue(env, &trigger_op->m_op);
    }
    else {
        plugin_barrage_trigger_op_free(env, trigger_op);
    }
}

static void plugin_barrage_emitter_trigger_change_sin(void* ctx) {
    plugin_barrage_trigger_op_t trigger_op = (plugin_barrage_trigger_op_t)ctx;
    plugin_barrage_emitter_t emitter = (plugin_barrage_emitter_t)trigger_op->m_op_target;
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;
    float percent;
    float delta;
    float target_value;

    assert(emitter->m_is_working);
    assert(trigger_op->m_total_tick > 0);

    trigger_op->m_cur_tick++;

    percent = ((float)trigger_op->m_cur_tick) / (float)trigger_op->m_total_tick;

    delta = trigger_op->m_to_value - trigger_op->m_base_value;
    
    target_value = trigger_op->m_base_value + delta * cpe_sin_radians(2 * M_PI * percent);
        
    plugin_barrage_emitter_set_value(emitter, trigger_op->m_result_type, target_value);

    if (trigger_op->m_cur_tick < trigger_op->m_total_tick) {
        trigger_op->m_op.m_op_frame = env->m_cur_frame + 1;
        trigger_op->m_op.m_op_fun = plugin_barrage_emitter_trigger_change_sin;
        trigger_op->m_op.m_op_ctx = trigger_op;
        plugin_barrage_op_enqueue(env, &trigger_op->m_op);
    }
    else {
        plugin_barrage_trigger_op_free(env, trigger_op);
    }
}

void plugin_barrage_emitter_trigger_do(plugin_barrage_emitter_t emitter, plugin_barrage_data_emitter_trigger_t trigger) {
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;
    
    assert(emitter->m_is_working);

    switch(trigger->m_data.trigger_op_type) {
    case barrage_emitter_emitter_trigger_op_change_value: {
        BARRAGE_EMITTER_EMITTER_TRIGGER_OP_DATA_CHANGE_VALUE const * change_value = &trigger->m_data.trigger_op_data.chagne_value;
        float base_value;
        float target_value;
        plugin_barrage_op_fun_t op_fun;
        plugin_barrage_trigger_op_t trigger_op;
        float value;
        plugin_barrage_data_emitter_flip_type_t flip_type;

        base_value = plugin_barrage_emitter_get_value(emitter, change_value->result_type);

        switch(change_value->result_value.type) {
        case barrage_emitter_value_value:
            value = change_value->result_value.data.value;
            flip_type = emitter->m_flip_type;
            break;
        case barrage_emitter_value_calc:
            if (plugin_barrage_calc_value(&value, change_value->result_value.data.calc_type, emitter, NULL, env->m_module->m_em) != 0) {
                return;
            }
            break;
        default:
            CPE_ERROR(env->m_module->m_em, "plugin_barrage_emitter_trigger_do: unknown value type %d", change_value->result_value.type);
            return;
        }

        switch(change_value->result_type) {
        case barrage_emitter_emitter_value_pos_angle:
        case barrage_emitter_emitter_value_angle:
            target_value = plugin_barrage_trigger_calc_angle(base_value, change_value->result_op, value, flip_type);
            break;
        default:
            target_value = plugin_barrage_trigger_calc_value(base_value, change_value->result_op, value);
            break;
        }

        if (base_value == target_value) {
            plugin_barrage_emitter_set_value(emitter, change_value->result_type, target_value);
            return;
        }
    
        switch(change_value->result_change_type) {
        case barrage_emitter_result_change_fix:
            plugin_barrage_emitter_set_value(emitter, change_value->result_type, target_value);
            return;
        case barrage_emitter_result_change_line:
            op_fun = plugin_barrage_emitter_trigger_change_line;
            break;
        case barrage_emitter_result_change_sin:
            op_fun = plugin_barrage_emitter_trigger_change_sin;
            break;
        default:
            CPE_ERROR(
                emitter->m_barrage->m_group->m_env->m_module->m_em, "plugin_barrage_emitter_trigger_do: unknown change type %d",
                change_value->result_change_type);
            return;
        }

        if (change_value->result_change_duration <= 0) {
            CPE_ERROR(
                emitter->m_barrage->m_group->m_env->m_module->m_em, "plugin_barrage_emitter_trigger_do: change duration %d error",
                change_value->result_change_duration);
            return;
        }

        trigger_op = plugin_barrage_trigger_op_create(env, &emitter->m_trigger_ops);
        if (trigger_op == NULL) {
            CPE_ERROR(emitter->m_barrage->m_group->m_env->m_module->m_em, "plugin_barrage_emitter_trigger_do: chreate trigger op fail");
            return;
        }
    
        trigger_op->m_base_value = base_value;
        trigger_op->m_to_value = target_value;
        trigger_op->m_total_tick = change_value->result_change_duration;
        trigger_op->m_cur_tick = 0;
        trigger_op->m_result_type = change_value->result_type;
        trigger_op->m_op_target = emitter;

        trigger_op->m_op.m_op_frame = env->m_cur_frame;
        trigger_op->m_op.m_op_fun = op_fun;
        trigger_op->m_op.m_op_ctx = trigger_op;
        plugin_barrage_op_enqueue(env, &trigger_op->m_op);
        break;
    }
    case barrage_emitter_emitter_trigger_op_emit:
        plugin_barrage_emitter_emit(emitter);
        break;
    case barrage_emitter_emitter_trigger_op_restore:
        break;
    default:
        break;
    }
}

static void plugin_barrage_emitter_do_sync_trigger(plugin_barrage_emitter_t emitter) {
    plugin_barrage_env_t env = emitter->m_barrage->m_group->m_env;

    assert(emitter->m_data.frame_complete > 0);

    if (emitter->m_emitter_op.m_op_fun != NULL) {
        plugin_barrage_op_dequeue(env, &emitter->m_emitter_op);
    }
    
    if (emitter->m_data.next_emitter_span > 0 && emitter->m_data.next_emitter_count > 0) {
        uint8_t emitter_span;
        uint8_t emitter_left_span;

        emitter_span = emitter->m_barrage->m_emitter_adj > 0.0f
            ? (emitter->m_data.next_emitter_span / emitter->m_barrage->m_emitter_adj)
            : 1;
        if (emitter_span < 1) emitter_span = 1;

        if (emitter->m_barrage->m_data.frame <= emitter->m_data.frame_start) {
            emitter_left_span = (emitter->m_data.frame_start - emitter->m_barrage->m_data.frame) + emitter_span;
        }
        else {
            emitter_left_span = (emitter->m_barrage->m_data.frame - emitter->m_data.frame_start + 1) % emitter_span;
            if (emitter_left_span) emitter_left_span = emitter_span - emitter_left_span;
        }

        /* printf("xxxxx: sync trigger, cur-frame=%d, config-span=%d, real-span=%d, delay=%d\n", */
        /*        emitter->m_barrage->m_data.frame, emitter->m_data.next_emitter_span, */
        /*        emitter_span, emitter_left_span); */
        
        emitter->m_emitter_op.m_op_frame = env->m_cur_frame + emitter_left_span;
        emitter->m_emitter_op.m_op_fun = plugin_barrage_emitter_trigger_once;
        emitter->m_emitter_op.m_op_ctx = emitter;
        plugin_barrage_op_enqueue(env, &emitter->m_emitter_op);
    }
}
