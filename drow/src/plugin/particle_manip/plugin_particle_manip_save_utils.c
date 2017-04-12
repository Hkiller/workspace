#include <errno.h>
#include <math.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_math.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_file.h"
#include "cpe/utils/stream_buffer.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_mgr.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin_particle_manip_save_utils.h"
#include "plugin_particle_manip_utils.h"

static const char * plugin_particle_manip_proj_float_to_str(char * buf, size_t capacity, float v);

void plugin_particle_manip_proj_save_bool(write_stream_t s, int level, const char * tag, uint8_t value) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s>%s</%s>\n", tag, value ? "True" : "False", tag);
}

void plugin_particle_manip_proj_save_int(write_stream_t s, int level, const char * tag, int value) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s>%d</%s>\n", tag, value, tag);
}

void plugin_particle_manip_proj_save_str(write_stream_t s, int level, const char * tag, const char * value) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s>%s</%s>\n", tag, value, tag);
}

void plugin_particle_manip_proj_save_float(write_stream_t s, int level, const char * tag, float value) {
    char buf[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s>%s</%s>\n",
        tag, plugin_particle_manip_proj_float_to_str(buf, sizeof(buf), value), tag);
}

void plugin_particle_manip_proj_save_obj_start(write_stream_t s, int level, const char * tag) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s>\n", tag);
}

void plugin_particle_manip_proj_save_obj_end(write_stream_t s, int level, const char * tag) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "</%s>\n", tag);
}

static const char * plugin_particle_manip_proj_float_to_str(char * buf, size_t capacity, float v) {
    char * sep;
    char * p;
    int n;

    n = snprintf(buf, capacity, "%.7f", v);

    sep = strchr(buf, '.');
    if (sep == NULL) return buf;

    for(p = buf + n; p > sep; --p) {
        if (*(p - 1) != '0') break;
    }

    if (sep + 1 == p) {
        *sep = 0;
    }
    else {
        *p = 0;
    }

    return buf;
}

void plugin_particle_manip_proj_save_curve_chanel(write_stream_t s, int level, const char * tag, plugin_particle_data_t particle, uint16_t chanel_id) {
    plugin_particle_data_curve_t chanel;
    uint16_t i;
    char buf1[32]; char buf2[32]; char buf3[32]; char buf4[32];
    uint16_t point_count;

    chanel =  plugin_particle_data_curve_find(particle, chanel_id);
    if (chanel == NULL) return;
    
    point_count = plugin_particle_data_curve_point_count(chanel);
    if (point_count == 0) return;

    plugin_particle_manip_proj_save_obj_start(s, level, tag);

    for(i = 0; i < point_count; ++i) {
        UI_CURVE_POINT const * point = plugin_particle_data_curve_point_at(chanel, i);
        float angle_l = atan(point->enter_tan) / (float)M_PI * 180.0f;
        float angle_r = atan(point->leave_tan) / (float)M_PI * 180.0f;

        stream_putc_count(s, ' ', (level + 1) * 4);
        stream_printf(
            s, "<CurveKey Mode=\"%d\" Key=\"%s\" Ret=\"%s\" Enter=\"%s\" Leave=\"%s\" />\n",
            point->interp, 
            plugin_particle_manip_proj_float_to_str(buf1, sizeof(buf1), point->key),
            plugin_particle_manip_proj_float_to_str(buf2, sizeof(buf2), point->ret),
            plugin_particle_manip_proj_float_to_str(buf3, sizeof(buf3), angle_l),
            plugin_particle_manip_proj_float_to_str(buf4, sizeof(buf4), angle_r));
    }

    plugin_particle_manip_proj_save_obj_end(s, level, tag);
}
