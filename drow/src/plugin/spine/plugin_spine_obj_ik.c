#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "plugin/spine/plugin_spine_utils.h"
#include "plugin_spine_obj_ik_i.h"

plugin_spine_obj_ik_t
plugin_spine_obj_ik_create(plugin_spine_obj_t obj, struct spIkConstraint * bik) {
    plugin_spine_module_t module = obj->m_module;
    plugin_spine_obj_ik_t ik;
    const char * sep;
    
    ik = TAILQ_FIRST(&module->m_free_iks);
    if (ik) {
        TAILQ_REMOVE(&module->m_free_iks, ik, m_next);
    }
    else {
        ik = mem_alloc(module->m_alloc, sizeof(struct plugin_spine_obj_ik));
        if (ik == NULL) {
            CPE_ERROR(module->m_em, "plugin_spine_obj_ik_create: alloc fail!");
            return NULL;
        }
    }

    ik->m_obj = obj;
    ik->m_ik = bik;
    ik->m_duration = 0.0f;
    ik->m_worked_time = 0.0f;
    ik->m_pos_speed = 0.0f;
    ik->m_angle_speed = 0.0f;
    ik->m_angle_min = 0.0f;
    ik->m_angle_max = 0.0f;
    
    sep = strchr(bik->data->name, '[');
    if (sep) {
        const char * end = sep + strlen(sep);
        char buf[32];
        size_t len = sep - bik->data->name;
        if (len + 1 > sizeof(ik->m_name)) len = sizeof(ik->m_name) - 1;
        memcpy(ik->m_name, bik->data->name, len);
        ik->m_name[len] = 0;

        if (cpe_str_read_arg_range(buf, sizeof(buf), sep + 1, end, "a-min", ',', '=') == 0) {
            ik->m_angle_min = atof(buf);
        }

        if (cpe_str_read_arg_range(buf, sizeof(buf), sep + 1, end, "a-max", ',', '=') == 0) {
            ik->m_angle_max = atof(buf);
        }

        if (cpe_str_read_arg_range(buf, sizeof(buf), sep + 1, end, "a-spd", ',', '=') == 0) {
            ik->m_angle_speed = atof(buf);
        }

        if (cpe_str_read_arg_range(buf, sizeof(buf), sep + 1, end, "p-spd", ',', '=') == 0) {
            ik->m_pos_speed = atof(buf);
        }
    }
    else {
        cpe_str_dup(ik->m_name, sizeof(ik->m_name), bik->data->name);
    }
    
    TAILQ_INSERT_TAIL(&obj->m_iks, ik, m_next);

    return ik;
}

void plugin_spine_obj_ik_free(plugin_spine_obj_ik_t ik) {
    plugin_spine_obj_t obj = ik->m_obj;
    plugin_spine_module_t module = obj->m_module;

    TAILQ_REMOVE(&obj->m_iks, ik, m_next);

    ik->m_obj = (void*)module;
    TAILQ_INSERT_TAIL(&module->m_free_iks, ik, m_next);
}

plugin_spine_obj_ik_t plugin_spine_obj_ik_find(plugin_spine_obj_t obj, const char * name) {
    plugin_spine_obj_ik_t ik;

    if (TAILQ_EMPTY(&obj->m_iks) && obj->m_skeleton->ikConstraintsCount > 0) {
    }
    
    TAILQ_FOREACH(ik, &obj->m_iks, m_next) {
        if (strcmp(ik->m_name, name) == 0) return ik;
    }

    return NULL;
}

const char * plugin_spine_obj_ik_name(plugin_spine_obj_ik_t ik) {
    return ik->m_name;
}

void plugin_spine_obj_ik_set_target_pos(plugin_spine_obj_ik_t ik, ui_vector_2_t pos) {
    spBone * cur = ik->m_ik->target;
    spBone * base = ik->m_ik->bones[0];
    float target_distance = cpe_math_distance(base->worldX, - base->worldY, pos->x, pos->y);
    float target_angle = cpe_math_angle(base->worldX, - base->worldY, pos->x, pos->y);

    /*有角度限制 */
    if (ik->m_angle_min < ik->m_angle_max && (ik->m_angle_max - ik->m_angle_min) < 360.0f) {
        float angle_base = - (base->data->inheritRotation ? base->parent->rotation + base->data->rotation : base->data->rotation);
        float angle_min = angle_base + ik->m_angle_min;
        float angle_max = angle_base + ik->m_angle_max;

        /* printf("xxxxxx: target=(%f,%f), angle=%f, base=%f-(%f,%f)\n", pos->x, pos->y, target_angle, angle_base, angle_min, angle_max); */
        
        if (target_angle < angle_min || target_angle > angle_max) {
            float diff_to_min = cpe_math_angle_regular(target_angle - angle_min);
            float diff_to_max = cpe_math_angle_regular(target_angle - angle_max);

            if (diff_to_min < 0.0f) diff_to_min += 360.0f;
            if (diff_to_min > 180.0f) diff_to_min = 360.0f - diff_to_min;

            if (diff_to_max < 0.0f) diff_to_max += 360.0f;
            if (diff_to_max > 180.0f) diff_to_max = 360.0f - diff_to_max;


            /* printf("     after adj=%f\n", target_angle); */
            
            assert(diff_to_min >= 0.0f);
            assert(diff_to_max >= 0.0f);

            target_angle = diff_to_min < diff_to_max ? angle_min : angle_max;
            target_angle = cpe_math_angle_regular(target_angle);
            /* printf("    result target=%f, diff-to-min=%f, diff-to-max=%f\n", target_angle, diff_to_min, diff_to_max); */
        }
    }

    ik->m_worked_time = 0.0f;
    ik->m_duration = 0.0f;    
    
    if (ik->m_angle_speed > 0.0f) {
        ik->m_move_type = plugin_spine_obj_ik_move_by_angle;
        ik->m_by_angle.m_angle_start = cpe_math_angle(base->worldX, - base->worldY, cur->worldX, -cur->worldY);
        ik->m_by_angle.m_angle_target = target_angle;
        ik->m_by_angle.m_distance_start = cpe_math_distance(base->worldX, - base->worldY, cur->worldX, -cur->worldY);
        ik->m_by_angle.m_distance_target = target_distance;
        ik->m_duration = fabs(ik->m_by_angle.m_angle_target - ik->m_by_angle.m_angle_start) / ik->m_angle_speed;

        /* printf( */
        /*     "xxxx: cur_angle=%f, target_angle=%f, speed=%f, duration=%f, angle=(%f=>%f), distance=(%f=>%f)\n", */
        /*     cur_angle, target_angle, ik->m_angle_speed, ik->m_duration, */
        /*     ik->m_by_angle.m_angle_start, ik->m_by_angle.m_angle_target, */
        /*     ik->m_by_angle.m_distance_start, ik->m_by_angle.m_distance_target); */
    }
    else if (ik->m_pos_speed > 0.0f) {
        ui_vector_2 target_pos;
        float distance;

        target_pos.x = base->worldX + cpe_cos_angle(target_angle) * target_distance;
        target_pos.y = - base->worldY + cpe_sin_angle(target_angle) * target_distance;

        distance = cpe_math_distance(cur->worldX, - cur->worldY, target_pos.x, target_pos.y);

        ik->m_move_type = plugin_spine_obj_ik_move_by_pos;
        ik->m_by_pos.m_start_pos.x = cur->worldX;
        ik->m_by_pos.m_start_pos.y = - cur->worldY;
        ik->m_by_pos.m_target_pos = target_pos;
        ik->m_duration = distance / ik->m_pos_speed;
    }

    if (ik->m_duration == 0.0f) {
        ui_vector_2 target_pos;
        target_pos.x = base->worldX + cpe_cos_angle(target_angle) * target_distance;
        target_pos.y = - base->worldY + cpe_sin_angle(target_angle) * target_distance;
        plugin_spine_ik_set_pos(ik->m_ik, &target_pos);
    }
}

void plugin_spine_obj_ik_restore(plugin_spine_obj_ik_t ik) {
    ui_vector_2 pos;

    assert(ik);
    spBone_localToWorld(
        ik->m_ik->target->parent,
        ik->m_ik->data->target->x, ik->m_ik->data->target->y,
        &pos.x, &pos.y);

    pos.y = - pos.y;

    plugin_spine_obj_ik_set_target_pos(ik, &pos);
}
    
struct spIkConstraint * plugin_spine_obj_ik_ik(plugin_spine_obj_ik_t ik) {
    return ik->m_ik;
}

uint8_t plugin_spine_obj_ik_is_runing(plugin_spine_obj_ik_t ik) {
    return ik->m_duration > 0.0f ? 1 : 0;
}

void plugin_spine_obj_ik_update(plugin_spine_obj_ik_t ik, float delta) {
    ik->m_worked_time += delta;

    if (ik->m_move_type == plugin_spine_obj_ik_move_by_pos) {
        if (ik->m_worked_time >= ik->m_duration) {
            plugin_spine_ik_set_pos(ik->m_ik, &ik->m_by_pos.m_target_pos);
        }
        else {
            ui_vector_2 target_pos;
            float percent = ik->m_worked_time / ik->m_duration;
            target_pos.x = ik->m_by_pos.m_start_pos.x + (ik->m_by_pos.m_target_pos.x - ik->m_by_pos.m_start_pos.x) * percent;
            target_pos.y = ik->m_by_pos.m_start_pos.y + (ik->m_by_pos.m_target_pos.y - ik->m_by_pos.m_start_pos.y) * percent;
            plugin_spine_ik_set_pos(ik->m_ik, &target_pos);
        /* printf("xxxxx: percent=%f, pos=(%f,%f)\n", percent, target_pos.x, target_pos.y); */
        }
    }
    else {
        spBone * base = ik->m_ik->bones[0];
        ui_vector_2 target_pos;
        float angle;
        float distance;
        
        assert(ik->m_move_type == plugin_spine_obj_ik_move_by_angle);

        if (ik->m_worked_time >= ik->m_duration) {
            angle = ik->m_by_angle.m_angle_target;
            distance = ik->m_by_angle.m_distance_target;
        }
        else {
            float percent = ik->m_worked_time / ik->m_duration;
            angle =  ik->m_by_angle.m_angle_start + (ik->m_by_angle.m_angle_target - ik->m_by_angle.m_angle_start) * percent;
            distance =  ik->m_by_angle.m_distance_start + (ik->m_by_angle.m_distance_target - ik->m_by_angle.m_distance_start) * percent;
        }

        target_pos.x = base->worldX + cpe_cos_angle(angle) * distance;
        target_pos.y = - base->worldY + cpe_sin_angle(angle) * distance;
        
        plugin_spine_ik_set_pos(ik->m_ik, &target_pos);
    }

    if (ik->m_worked_time >= ik->m_duration) {
        ik->m_duration = 0.0f;
        ik->m_worked_time = 0.0f;
    }
}

static plugin_spine_obj_ik_t plugin_spine_obj_ik_next(struct plugin_spine_obj_ik_it * it) {
    plugin_spine_obj_ik_t * data = (plugin_spine_obj_ik_t *)(it->m_data);
    plugin_spine_obj_ik_t r;

    if (*data == NULL) return NULL;

    r = *data;

    *data = TAILQ_NEXT(r, m_next);

    return r;
}

void plugin_spine_obj_iks(plugin_spine_obj_t obj, plugin_spine_obj_ik_it_t it) {
    *(plugin_spine_obj_ik_t *)(it->m_data) = TAILQ_FIRST(&obj->m_iks);
    it->next = plugin_spine_obj_ik_next;
}

void plugin_spine_obj_ik_real_free_all(plugin_spine_module_t module) {
    while(!TAILQ_EMPTY(&module->m_free_iks)) {
        plugin_spine_obj_ik_t ik = TAILQ_FIRST(&module->m_free_iks);
        TAILQ_REMOVE(&module->m_free_iks, ik, m_next);
        mem_free(module->m_alloc, ik);
    }
}

