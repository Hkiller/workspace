#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/file.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin/barrage_manip/plugin_barrage_crazystorm.h"
#include "plugin/barrage/plugin_barrage_data_emitter.h"

static char * seekto(char * data, const char * target) {
    data = strstr(data, target);
    if (data == NULL) return NULL;
    return data + strlen(target);
}

static char * ignore_to_char(char * data, char sep, int n) {
    if (data == NULL) return NULL;

    while(n > 0) {
        --n;

        data = strchr(data, sep);

        if (data == NULL) return NULL;

        data += 1;
    }

    return data;
}

static char * ignore(char * data, int n) {
    return ignore_to_char(data, ',', n);
}

static char * ignore_line(char * data) {
    return ignore_to_char(data, '\n', 1);
}

static char * read_uint8(uint8_t * r, char * data) {
    if (data == NULL) return NULL;
    *r = atoi(data);
    return ignore(data, 1);
}

static char * read_uint16(uint16_t * r, char * data) {
    if (data == NULL) return NULL;
    *r = atoi(data);
    return ignore(data, 1);
}

static char * read_float(float * r, char * data) {
    if (data == NULL) return NULL;
    *r = atof(data);
    return ignore(data, 1);
}

static char * read_emitter_value(BARRAGE_RAND_EMITTER_VALUE * value, char * data) {
    if (data == NULL) return NULL;

    if (strcmp(data, "自机") == 0
        || atoi(data) == -99999)
    {
        value->type = barrage_emitter_value_calc;
        value->data.calc_type = barrage_value_calc_target_angle_locked;
    }
    else if (strcmp(data, "动态自机") == 0) {
        value->type = barrage_emitter_value_calc;
        value->data.calc_type = barrage_value_calc_target_angle;
    }
    else {
        value->type = barrage_emitter_value_value;
        value->data.value.base = atof(data);
    }

    return ignore(data, 1);
}

static char * read_bool(uint8_t * r, char * data) {
    if (data == NULL) return NULL;
    *r = memcmp(data, "True", 4) == 0 ? 1 : 0;
    return ignore(data, 1);
}

static int read_emitter_trigger_value_type(const char * name, error_monitor_t em) {
    if (strcmp(name, "当前帧") == 0) {
        return barrage_emitter_emitter_value_frame;
    }
    else if (strcmp(name, "X坐标") == 0) {
        return barrage_emitter_emitter_value_pos_x;
    }
    else if (strcmp(name, "Y坐标") == 0) {
        return barrage_emitter_emitter_value_pos_y;
    }
    else if (strcmp(name, "半径") == 0) {
        return barrage_emitter_emitter_value_pos_radius;
    }
    else if (strcmp(name, "半径方向") == 0) {
        return barrage_emitter_emitter_value_pos_angle;
    }
    else if (strcmp(name, "条数") == 0) {
        return barrage_emitter_emitter_value_count;
    }
    else if (strcmp(name, "周期") == 0) {
        return barrage_emitter_emitter_value_span;
    }
    else if (strcmp(name, "角度") == 0) {
        return barrage_emitter_emitter_value_angle;
    }
    else if (strcmp(name, "范围") == 0) {
        return barrage_emitter_emitter_value_angle_range;
    }
    else if (strcmp(name, "宽比") == 0) {
        return barrage_emitter_emitter_value_x_rate;
    }
    else if (strcmp(name, "高比") == 0) {
        return barrage_emitter_emitter_value_y_rate;
    }
    else if (strcmp(name, "不透明度") == 0) {
        return barrage_emitter_emitter_value_alpha;
    }
    else if (strcmp(name, "子弹速度") == 0) {
        return barrage_emitter_emitter_value_speed;
    }
    else if (strcmp(name, "子弹加速度") == 0) {
        return barrage_emitter_emitter_value_acceleration;
    }
    else if (strcmp(name, "子弹加速度方向") == 0) {
        return barrage_emitter_emitter_value_acceleration_angle;
    }
    else {
        CPE_ERROR(em, "plugin_barrage_import_crazystorm_emitter: not support emitter value %s!", name);
        return -1;
    }
}

static int read_emitter_trigger_condition_one(BARRAGE_EMITTER_EMITTER_CONDITION_INFO * condition_data, char * str_condition, error_monitor_t em) {
    char * sep;
    int value_type;

    if ((sep = strchr(str_condition, '<'))) {
        condition_data->condition_op = barrage_emitter_condition_op_lt;
    }
    else if ((sep = strchr(str_condition, '='))) {
        condition_data->condition_op = barrage_emitter_condition_op_eq;
    }
    else if ((sep = strchr(str_condition, '>'))) {
        condition_data->condition_op = barrage_emitter_condition_op_bg;
    }
    else {
        return -1;
    }

    *sep = 0;

    value_type = read_emitter_trigger_value_type(str_condition, em);
    if (value_type < 0) return -1;

    condition_data->condition_type = (uint8_t)value_type;
    condition_data->condition_value = atof(sep + 1);

    return 0;
}

static int read_emitter_trigger_conditions(BARRAGE_EMITTER_EMITTER_TRIGGER_INFO * trigger_data, char * str_triger, error_monitor_t em) {
    char * sep;

    if ((sep = strstr(str_triger, "且"))) {
        trigger_data->condition_compose = barrage_emitter_condition_compose_or;
        trigger_data->condition_count = 2;

        *sep = 0;
        if (read_emitter_trigger_condition_one(&trigger_data->conditions[0], str_triger, em) != 0) return -1;
        if (read_emitter_trigger_condition_one(&trigger_data->conditions[0], sep + strlen("且"), em) != 0) return -1;
    }
    else if ((sep = strstr(str_triger, "或"))) {
        trigger_data->condition_compose = barrage_emitter_condition_compose_and;
        trigger_data->condition_count = 2;

        *sep = 0;
        if (read_emitter_trigger_condition_one(&trigger_data->conditions[0], str_triger, em) != 0) return -1;
        if (read_emitter_trigger_condition_one(&trigger_data->conditions[0], sep + strlen("或"), em) != 0) return -1;
    }
    else {
        trigger_data->condition_compose = 0;
        trigger_data->condition_count = 1;
        if (read_emitter_trigger_condition_one(&trigger_data->conditions[0], str_triger, em) != 0) return -1;
    }

    return 0;
}

static int read_emitter_trigger_result(BARRAGE_EMITTER_EMITTER_TRIGGER_OP_DATA_CHANGE_VALUE * trigger_data, char * str_result, error_monitor_t em) {
    char * sep;
    char * value;
    int value_type;

    if ((sep = strstr(str_result, "变化到"))) {
        value = sep + strlen("变化到");
        trigger_data->result_op = barrage_emitter_result_op_set;
    }
    else if ((sep = strstr(str_result, "增加"))) {
        value = sep + strlen("增加");
        trigger_data->result_op = barrage_emitter_result_op_inc;
    }
    else if ((sep = strstr(str_result, "减少"))) {
        value = sep + strlen("减少");
        trigger_data->result_op = barrage_emitter_result_op_dec;
    }
    else {
        return -1;
    }

    *sep = 0;

    value_type = read_emitter_trigger_value_type(str_result, em);
    if (value_type < 0) {
        return -1;
    }

    trigger_data->result_type = value_type;

    if (strcmp(value, "自机") == 0) {
        trigger_data->result_value.type = barrage_emitter_value_calc;
        trigger_data->result_value.data.calc_type = barrage_value_calc_target_angle_locked;
    }
    else if (strcmp(value, "动态自机") == 0) {
        trigger_data->result_value.type = barrage_emitter_value_calc;
        trigger_data->result_value.data.calc_type = barrage_value_calc_target_angle;
    }
    else {
        trigger_data->result_value.type = barrage_emitter_value_value;
        trigger_data->result_value.data.value = atof(value);
    }

    return 0;
}

static char * read_emitter_triggers(ui_ed_obj_t emitter_obj, char * data, error_monitor_t em) {
    char * sep;
    char * event_group_data;
    char * event_group_end;
    uint8_t group_id = 0;

    if (data == NULL) return NULL;

    sep = strchr(data, ',');
    if (sep == NULL) return NULL;

    for(event_group_data = data, event_group_end = strchr(data, '&'); 
        event_group_end && event_group_end < sep; 
        event_group_data = event_group_end + 1, event_group_end = strchr(event_group_data, '&')
        )
    {
        char * event_data;
        char * event_data_end;

        event_group_data = ignore_to_char(event_group_data, '|', 3);
        if (event_group_data == NULL) return NULL;

        for(event_data = event_group_data, event_data_end = strchr(event_data, ';');
            event_data_end && event_data_end < event_group_end;
            event_data = event_data_end + 1, event_data_end = strchr(event_data, ';')
            )
        {
            size_t data_len = event_data_end - event_data;
            char buf[128];
            char * tmp;
            char * condition;
            char * result_op;
            char * update_way;
            ui_ed_obj_t trigger_obj;
            plugin_barrage_data_emitter_trigger_t trigger;
            BARRAGE_EMITTER_EMITTER_TRIGGER_INFO * trigger_data;

            if (data_len + 1 >= CPE_ARRAY_SIZE(buf)) return NULL;

            memcpy(buf, event_data, data_len);
            buf[data_len] = 0;

            condition = buf;
            tmp = strstr(condition, "：");
            if (tmp == NULL) return NULL;
            *tmp = 0;
            
            trigger_obj = ui_ed_obj_new(emitter_obj, ui_ed_obj_type_barrage_emitter_trigger);
            if (trigger_obj == NULL) return NULL;

            trigger = ui_ed_obj_product(trigger_obj);
            assert(trigger);

            trigger_data = ui_ed_obj_data(trigger_obj);
            assert(trigger_data);

            trigger_data->group_id = group_id;
            if (read_emitter_trigger_conditions(trigger_data, condition, em) != 0) return NULL;

            result_op = tmp + strlen("：");
                
            if (strcmp(result_op, "额外发射") == 0) {
                trigger_data->trigger_op_type = barrage_emitter_emitter_trigger_op_emit;
            }
            else {
                BARRAGE_EMITTER_EMITTER_TRIGGER_OP_DATA_CHANGE_VALUE * change_value = &trigger_data->trigger_op_data.chagne_value;
                
                trigger_data->trigger_op_type = barrage_emitter_emitter_trigger_op_change_value;

                tmp = strstr(result_op, "，");
                if (tmp == NULL) return NULL;
                *tmp = 0;
                
                if (read_emitter_trigger_result(change_value, result_op, em) != 0) return NULL;

                update_way = tmp + strlen("，");
                tmp = strstr(update_way, "，");
                if (tmp == NULL) return NULL;
                *tmp = 0;

                if (strcmp(update_way, "固定") == 0) {
                    change_value->result_change_type = barrage_emitter_result_change_fix;
                }
                else if (strcmp(update_way, "正比") == 0) {
                    change_value->result_change_type = barrage_emitter_result_change_line;
                }
                else if (strcmp(update_way, "正弦") == 0) {
                    change_value->result_change_type = barrage_emitter_result_change_sin;
                }
                else {
                    return NULL;
                }

                change_value->result_change_duration = atoi(tmp + strlen("，"));
            }

            if (plugin_barrage_data_emitter_trigger_update(trigger) != 0) return NULL;
        }

        ++group_id;
    }

    return sep + 1;
}

static int read_bullet_trigger_value_type(const char * name, error_monitor_t em) {
    if (strcmp(name, "当前帧") == 0) {
        return barrage_emitter_bullet_value_frame;
    }
    else if (strcmp(name, "X坐标") == 0) {
        return barrage_emitter_bullet_value_pos_x;
    }
    else if (strcmp(name, "Y坐标") == 0) {
        return barrage_emitter_bullet_value_pos_y;
    }
    else if (strcmp(name, "类别") == 0) {
        return barrage_emitter_bullet_value_life_circle;
    }
    else if (strcmp(name, "宽比") == 0) {
        return barrage_emitter_bullet_value_scale_x;
    }
    else if (strcmp(name, "高比") == 0) {
        return barrage_emitter_bullet_value_scale_y;
    }
    else if (strcmp(name, "R") == 0) {
        return barrage_emitter_bullet_value_color_r;
    }
    else if (strcmp(name, "G") == 0) {
        return barrage_emitter_bullet_value_color_g;
    }
    else if (strcmp(name, "B") == 0) {
        return barrage_emitter_bullet_value_color_b;
    }
    else if (strcmp(name, "A") == 0) {
        return barrage_emitter_bullet_value_color_a;
    }
    else if (strcmp(name, "朝向") == 0) {
        return barrage_emitter_bullet_value_angle;
    }
    else if (strcmp(name, "子弹速度") == 0) {
        return barrage_emitter_bullet_value_speed;
    }
    else if (strcmp(name, "子弹速度方向") == 0) {
        return barrage_emitter_bullet_value_speed_angle;
    }
    else if (strcmp(name, "子弹加速度") == 0) {
        return barrage_emitter_bullet_value_acceleration;
    }
    else if (strcmp(name, "子弹加速度方向") == 0) {
        return barrage_emitter_bullet_value_acceleration_angle;
    }
    else if (strcmp(name, "横比") == 0) {
        return barrage_emitter_bullet_value_x_rate;
    }
    else if (strcmp(name, "纵比") == 0) {
        return barrage_emitter_bullet_value_y_rate;
    }
    else {
        CPE_ERROR(em, "plugin_barrage_import_crazystorm_emitter: not support bullet event value %s!", name);
        return -1;
    }
}

static int read_bullet_trigger_condition_one(BARRAGE_EMITTER_BULLET_CONDITION_INFO * condition_data, char * str_condition, error_monitor_t em) {
    char * sep;
    int value_type;

    if ((sep = strchr(str_condition, '<'))) {
        condition_data->condition_op = barrage_emitter_condition_op_lt;
    }
    else if ((sep = strchr(str_condition, '='))) {
        condition_data->condition_op = barrage_emitter_condition_op_eq;
    }
    else if ((sep = strchr(str_condition, '>'))) {
        condition_data->condition_op = barrage_emitter_condition_op_bg;
    }
    else {
        return -1;
    }

    *sep = 0;

    value_type = read_bullet_trigger_value_type(str_condition, em);
    if (value_type < 0) return -1;

    condition_data->condition_type = (uint8_t)value_type;
    condition_data->condition_value = atof(sep + 1);

    return 0;
}

static int read_bullet_trigger_conditions(BARRAGE_EMITTER_BULLET_TRIGGER_INFO * trigger_data, char * str_triger, error_monitor_t em) {
    char * sep;

    if ((sep = strstr(str_triger, "且"))) {
        trigger_data->condition_compose = barrage_emitter_condition_compose_or;
        trigger_data->condition_count = 2;

        *sep = 0;
        if (read_bullet_trigger_condition_one(&trigger_data->conditions[0], str_triger, em) != 0) return -1;
        if (read_bullet_trigger_condition_one(&trigger_data->conditions[0], sep + strlen("且"), em) != 0) return -1;
    }
    else if ((sep = strstr(str_triger, "或"))) {
        trigger_data->condition_compose = barrage_emitter_condition_compose_and;
        trigger_data->condition_count = 2;

        *sep = 0;
        if (read_bullet_trigger_condition_one(&trigger_data->conditions[0], str_triger, em) != 0) return -1;
        if (read_bullet_trigger_condition_one(&trigger_data->conditions[0], sep + strlen("或"), em) != 0) return -1;
    }
    else {
        trigger_data->condition_compose = 0;
        trigger_data->condition_count = 1;
        if (read_bullet_trigger_condition_one(&trigger_data->conditions[0], str_triger, em) != 0) return -1;
    }

    return 0;
}

static int read_bullet_trigger_result(BARRAGE_EMITTER_BULLET_TRIGGER_INFO * trigger_data, char * str_result, error_monitor_t em) {
    char * sep;
    char * value;
    int value_type;

    if ((sep = strstr(str_result, "变化到"))) {
        value = sep + strlen("变化到");
        trigger_data->result_op = barrage_emitter_result_op_set;
    }
    else if ((sep = strstr(str_result, "增加"))) {
        value = sep + strlen("增加");
        trigger_data->result_op = barrage_emitter_result_op_inc;
    }
    else if ((sep = strstr(str_result, "减少"))) {
        value = sep + strlen("减少");
        trigger_data->result_op = barrage_emitter_result_op_dec;
    }
    else {
        return -1;
    }

    *sep = 0;

    value_type = read_bullet_trigger_value_type(str_result, em);
    if (value_type < 0) return -1;

    trigger_data->result_type = value_type;

    if (strcmp(value, "自机") == 0) {
        trigger_data->result_value.type = barrage_emitter_value_calc;
        trigger_data->result_value.data.calc_type = barrage_value_calc_target_angle_locked;
    }
    else if (strcmp(value, "动态自机") == 0) {
        trigger_data->result_value.type = barrage_emitter_value_calc;
        trigger_data->result_value.data.calc_type = barrage_value_calc_target_angle;
    }
    else {
        trigger_data->result_value.type = barrage_emitter_value_value;
        trigger_data->result_value.data.value = atof(value);
    }

    return 0;
}

static char * read_bullet_triggers(ui_ed_obj_t emitter_obj, char * data, error_monitor_t em) {
    char * sep;
    char * event_group_data;
    char * event_group_end;
    uint8_t group_id = 0;

    if (data == NULL) return NULL;

    sep = strchr(data, ',');
    if (sep == NULL) return NULL;

    for(event_group_data = data, event_group_end = strchr(data, '&'); 
        event_group_end && event_group_end < sep; 
        event_group_data = event_group_end + 1, event_group_end = strchr(event_group_data, '&')
        )
    {
        char * event_data;
        char * event_data_end;

        event_group_data = ignore_to_char(event_group_data, '|', 3);
        if (event_group_data == NULL) return NULL;

        for(event_data = event_group_data, event_data_end = strchr(event_data, ';');
            event_data_end && event_data_end < event_group_end;
            event_data = event_data_end + 1, event_data_end = strchr(event_data, ';')
            )
        {
            size_t data_len = event_data_end - event_data;
            char buf[128];
            char * tmp;
            char * condition;
            char * result_op;
            char * update_way;
            ui_ed_obj_t trigger_obj;
            plugin_barrage_data_bullet_trigger_t trigger;
            BARRAGE_EMITTER_BULLET_TRIGGER_INFO * trigger_data;

            if (data_len + 1 >= CPE_ARRAY_SIZE(buf)) return NULL;

            memcpy(buf, event_data, data_len);
            buf[data_len] = 0;

            condition = buf;
            tmp = strstr(condition, "：");
            if (tmp == NULL) return NULL;
            *tmp = 0;
            
            trigger_obj = ui_ed_obj_new(emitter_obj, ui_ed_obj_type_barrage_bullet_trigger);
            if (trigger_obj == NULL) return NULL;

            trigger = ui_ed_obj_product(trigger_obj);
            assert(trigger);

            trigger_data = ui_ed_obj_data(trigger_obj);
            assert(trigger_data);

            trigger_data->group_id = group_id;
            if (read_bullet_trigger_conditions(trigger_data, condition, em) != 0) return NULL;

            result_op = tmp + strlen("：");
            tmp = strstr(result_op, "，");
            if (tmp == NULL) return NULL;
            *tmp = 0;

            if (read_bullet_trigger_result(trigger_data, result_op, em) != 0) return NULL;

            update_way = tmp + strlen("，");
            tmp = strstr(update_way, "，");
            if (tmp == NULL) return NULL;
            *tmp = 0;

            if (strcmp(update_way, "固定") == 0) {
                trigger_data->result_change_type = barrage_emitter_result_change_fix;
            }
            else if (strcmp(update_way, "正比") == 0) {
                trigger_data->result_change_type = barrage_emitter_result_change_line;
            }
            else if (strcmp(update_way, "正弦") == 0) {
                trigger_data->result_change_type = barrage_emitter_result_change_sin;
            }
            else {
                return NULL;
            }

            trigger_data->result_change_duration = atoi(tmp +  + strlen("，"));
            if (plugin_barrage_data_bullet_trigger_update(trigger) != 0) return NULL;
        }

        ++group_id;
    }

    return sep + 1;
}

static plugin_particle_data_emitter_t
plugin_barrage_import_crazystorm_find_emitter(plugin_particle_data_t particle, uint16_t id) {
    struct plugin_particle_data_emitter_it emitter_it;
    plugin_particle_data_emitter_t emitter;
    char prefix[32];
    snprintf(prefix, sizeof(prefix), "%d_", id);
    
    plugin_particle_data_emitters(&emitter_it, particle);
    while((emitter = plugin_particle_data_emitter_it_next(&emitter_it))) {
        UI_PARTICLE_EMITTER const * emitter_data = plugin_particle_data_emitter_data(emitter);
        const char * name = plugin_particle_data_emitter_msg(emitter, emitter_data->name_id);
        if (cpe_str_start_with(name, prefix)) return emitter;
    }
    
    return NULL;
}

int plugin_barrage_import_crazystorm_emitter(
    ui_ed_mgr_t ed_mgr, const char * input_path,
    const char * to_emitter, const char * use_buletts, error_monitor_t em)
{
    ui_ed_src_t barrage_src;
    ui_ed_obj_t barrage_obj;
    struct mem_buffer data_buf;
    char * data;
    char * next_layer;
    uint16_t bullet_id;
    uint16_t value;
    ui_data_src_t particle_src;
    plugin_particle_data_t particle;
    plugin_particle_data_emitter_t use_emitter;
    
    particle_src = ui_data_src_find_by_path(ui_ed_mgr_data_mgr(ed_mgr), use_buletts, ui_data_src_type_particle);
    if (particle_src == NULL) {
        CPE_ERROR(em, "use bullets particle %s not exist!", use_buletts);
        return -1;
    }

    if (!ui_data_src_is_loaded(particle_src)) {
        if (ui_data_src_load(particle_src, em) != 0) {
            CPE_ERROR(em, "use bullets particle %s load fail!", use_buletts);
            return -1;
        }
    }
    particle = ui_data_src_product(particle_src);
    assert(particle);
    
    mem_buffer_init(&data_buf, NULL);
    if (file_load_to_buffer(&data_buf, input_path, em) < 0) {
        CPE_ERROR(em, "read from file %s fail!", input_path);
        mem_buffer_clear(&data_buf);
        return -1;
    }
    mem_buffer_append_char(&data_buf, 0);
    data = mem_buffer_make_continuous(&data_buf, 0);

    barrage_src = ui_ed_src_check_create(ed_mgr, to_emitter, ui_data_src_type_barrage);
    if (barrage_src == NULL) {
        CPE_ERROR(em, "check create emitter at %s fail!", to_emitter);
        mem_buffer_clear(&data_buf);
        return -1;
    }

    /*创建Bullet文件 */
    barrage_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(barrage_src));
    assert(barrage_obj);
    ui_ed_obj_remove_childs(barrage_obj);

    /*开始读取数据 */
    data = seekto(data, "Totalframe:");
    if (data == NULL) {
        CPE_ERROR(em, "plugin_barrage_import_crazystorm_emitter: seek to Totalframe fail!");
        goto READERRROR;
    }
    //emitter_data->frame_total = atoi(data);

    for(data = strstr(data, "Layer"), next_layer = NULL; data; data = next_layer) {
        char * layer_name;
        uint16_t total_frame;
        
        assert(cpe_str_start_with(data, "Layer"));
        data = ignore_to_char(data, ':', 1);

        next_layer = (char*)strstr(data, "Layer");

        layer_name = data;
        if (cpe_str_start_with(layer_name, "empty")) continue;

        if (next_layer) *next_layer = 0;

        data = ignore(data, 1);
        if (data == NULL) {
            CPE_ERROR(em, "plugin_barrage_import_crazystorm_emitter: read layer name fail!");
            goto READERRROR;
        }
        
        * cpe_str_trim_tail(data - 1, layer_name) = 0;

        if (strcmp(layer_name, "新图层") == 0) *layer_name = 0;

        data = ignore(data, 1);
        data = read_uint16(&total_frame, data);
        if (data == NULL) {
            CPE_ERROR(em, "plugin_barrage_import_crazystorm_emitter: read layer frame total fail!");
            goto READERRROR;
        }

        data = ignore_line(data);
        while(data && data[0] >= 20) {
            ui_ed_obj_t emitter_obj;
            BARRAGE_EMITTER_INFO * emitter_data;

            emitter_obj = ui_ed_obj_new(barrage_obj, ui_ed_obj_type_barrage_emitter);
            if (emitter_obj == NULL) {
                CPE_ERROR(em, "plugin_barrage_import_crazystorm_emitter: create emitter obj fail!");
                goto READERRROR;
            }

            emitter_data = ui_ed_obj_data(emitter_obj);

            cpe_str_dup(emitter_data->layer, sizeof(emitter_data->layer), layer_name);
            emitter_data->frame_loop = total_frame;
            
            data = ignore(data, 8);
            data = read_uint16(&emitter_data->frame_start.base, data);
            data = read_uint16(&emitter_data->frame_duration.base, data);
            data = ignore(data, 2);
            data = read_float(&emitter_data->emitter.emitter_pos_radius.base, data);
            data = read_float(&emitter_data->emitter.emitter_pos_angle.base, data);
            data = ignore(data, 1);
            data = read_uint8(&emitter_data->emitter.emitter_count.base, data);
            data = read_uint8(&emitter_data->emitter.emitter_span.base, data);
            data = read_emitter_value(&emitter_data->emitter.emitter_angle, data);
            data = ignore(data, 1);
            data = read_float(&emitter_data->emitter.emitter_angle_range.base, data);
            data = ignore(data, 6);

            /*    bullet*/
            data = read_uint16(&emitter_data->bullet.life_circle.base, data);
            data = read_uint16(&bullet_id, data);

            use_emitter = plugin_barrage_import_crazystorm_find_emitter(particle, bullet_id);
            if (use_emitter == NULL) {
                CPE_ERROR(em, "plugin_barrage_import_crazystorm_emitter: use bullet %d not exist in bullets particle %s!", bullet_id, use_buletts);
                goto READERRROR;
            }
    
            snprintf(
                emitter_data->bullet.proto, sizeof(emitter_data->bullet.proto),
                "%s#%s", use_buletts, plugin_particle_data_emitter_msg(use_emitter, plugin_particle_data_emitter_data(use_emitter)->name_id));
            data = read_float(&emitter_data->bullet.scale.x.base, data);
            data = read_float(&emitter_data->bullet.scale.y.base, data);

            data = read_uint16(&value, data);
            emitter_data->bullet.color.r = value / 255.0f;
            data = read_uint16(&value, data);
            emitter_data->bullet.color.g = value / 255.0f;
            data = read_uint16(&value, data);
            emitter_data->bullet.color.b = value / 255.0f;
            data = read_uint16(&value, data);
            emitter_data->bullet.color.a = value / 100.0f;

            data = read_float(&emitter_data->bullet.angle.base, data);
            data = ignore(data, 1);
            data = read_bool(&emitter_data->bullet.angle_to_speed, data);
            data = read_float(&emitter_data->bullet.speed.base, data);
            data = ignore(data, 2);
            data = read_float(&emitter_data->bullet.acceleration.base, data);
            data = read_float(&emitter_data->bullet.acceleration_angle.base, data);
            data = ignore(data, 1);
            data = read_float(&emitter_data->bullet.x_rate.base, data);
            data = read_float(&emitter_data->bullet.y_rate.base, data);
            data = ignore(data, 6);

            data = read_emitter_triggers(emitter_obj, data, em);
            data = read_bullet_triggers(emitter_obj, data, em);

            if (data == NULL) {
                CPE_ERROR(em, "plugin_barrage_import_crazystorm_emitter: no data after read triggers!");
                goto READERRROR;
            }

            data = read_float(&emitter_data->emitter.pos.x.adj, data);
            data = read_float(&emitter_data->emitter.pos.y.adj, data);
            data = read_float(&emitter_data->emitter.emitter_pos_radius.adj, data);
            data = read_float(&emitter_data->emitter.emitter_pos_angle.adj, data);    
            data = read_uint8(&emitter_data->emitter.emitter_count.adj, data);    
            data = read_uint8(&emitter_data->emitter.emitter_span.adj, data);
            if (emitter_data->emitter.emitter_angle.type == barrage_emitter_value_value) {
                data = read_float(&emitter_data->emitter.emitter_angle.data.value.adj, data);
            }
            else {
                data = ignore(data, 1);
            }
            data = read_float(&emitter_data->emitter.emitter_angle_range.adj, data);    

            data = read_float(&emitter_data->emitter.speed.adj, data);
            data = read_float(&emitter_data->emitter.speed_angle.adj, data);
            data = read_float(&emitter_data->emitter.acceleration.adj, data);
            data = read_float(&emitter_data->emitter.acceleration_angle.adj, data);

            data = read_float(&emitter_data->bullet.angle.adj, data);
            data = read_float(&emitter_data->bullet.speed.adj, data);
            data = ignore(data, 1);
            data = read_float(&emitter_data->bullet.acceleration.adj, data);
            data = read_float(&emitter_data->bullet.acceleration_angle.adj, data);

            data = ignore_line(data);
        }

        if (next_layer) *next_layer = 'L'; /*restore*/
    }
    
    ui_ed_src_touch(barrage_src);
    mem_buffer_clear(&data_buf);
    return 0;

READERRROR:
    mem_buffer_clear(&data_buf);
    return -1;
}
