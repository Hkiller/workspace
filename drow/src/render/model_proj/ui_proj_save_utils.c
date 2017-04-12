#include <errno.h>
#include <math.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_math.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_proj_save_utils.h"
#include "ui_proj_utils.h"

static const char * ui_data_proj_float_to_str(char * buf, size_t capacity, float v);

int ui_data_proj_save_gen_meta_file(const char * root, ui_data_src_t src, error_monitor_t em) {
    struct mem_buffer path_buff;
    struct write_stream_buffer path_stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&path_buff);
    vfs_file_t fp = NULL;
    struct vfs_write_stream fs;
    int rv = -1;
    
    mem_buffer_init(&path_buff, NULL);

    /*构造路径 */
    stream_printf((write_stream_t)&path_stream, root);
    if (root[strlen(root) - 1] != '/') {
        stream_printf((write_stream_t)&path_stream, "/");
    }
    ui_data_src_path_print((write_stream_t)&path_stream, src);
    stream_printf((write_stream_t)&path_stream, ".%s.meta", ui_data_proj_postfix(ui_data_src_type(src)));
    stream_putc((write_stream_t)&path_stream, 0);

    fp = vfs_file_open(gd_app_vfs_mgr(ui_data_mgr_app(ui_data_src_mgr(src))), mem_buffer_make_continuous(&path_buff, 0), "w");
    if (fp == NULL) {
        CPE_ERROR(em, "open file \"%s\" fail, errno=%d (%s)!", (const char*)mem_buffer_make_continuous(&path_buff, 0), errno, strerror(errno));
        goto COMPLETE;
    }

    vfs_write_stream_init(&fs, fp);

    stream_printf((write_stream_t)&fs, "%u\n", ui_data_src_id(src));

    ui_data_src_path_print((write_stream_t)&fs, ui_data_src_parent(src));
    stream_printf((write_stream_t)&fs, "/\n");

    ui_data_src_data_print((write_stream_t)&fs, src);
    stream_printf((write_stream_t)&fs, ".%s\n", ui_data_proj_postfix(ui_data_src_type(src)));
    
    rv = 0;

COMPLETE:
    if (fp) vfs_file_close(fp);
    mem_buffer_clear(&path_buff);

    return rv;
}

void ui_data_proj_save_rect(write_stream_t s, int level, const char * tag, UI_RECT const * rect) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s LT=\"%d\" TP=\"%d\" RT=\"%d\" BM=\"%d\" />\n",
        tag, rect->lt, rect->tp, rect->rt, rect->bm);
}

void ui_data_proj_save_bool(write_stream_t s, int level, const char * tag, uint8_t value) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s>%s</%s>\n", tag, value ? "True" : "False", tag);
}

void ui_data_proj_save_int(write_stream_t s, int level, const char * tag, int value) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s>%d</%s>\n", tag, value, tag);
}

void ui_data_proj_save_str(write_stream_t s, int level, const char * tag, const char * value) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s>%s</%s>\n", tag, value, tag);
}

void ui_data_proj_save_float(write_stream_t s, int level, const char * tag, float value) {
    char buf[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s>%s</%s>\n",
        tag, ui_data_proj_float_to_str(buf, sizeof(buf), value), tag);
}

void ui_data_proj_save_color(write_stream_t s, int level, const char * tag, UI_COLOR const * color) {
    char buf1[32]; char buf2[32]; char buf3[32]; char buf4[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s A=\"%s\" R=\"%s\" G=\"%s\" B=\"%s\" />\n",
        tag,
        ui_data_proj_float_to_str(buf1, sizeof(buf1), color->a),
        ui_data_proj_float_to_str(buf2, sizeof(buf2), color->r),
        ui_data_proj_float_to_str(buf3, sizeof(buf3), color->g),
        ui_data_proj_float_to_str(buf4, sizeof(buf4), color->b));
}

void ui_data_proj_save_vector_2(write_stream_t s, int level, const char * tag, UI_VECTOR_2 const * vec) {
    char buf1[32]; char buf2[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s x=\"%s\" y=\"%s\" />\n",
        tag,
        ui_data_proj_float_to_str(buf1, sizeof(buf1), vec->value[0]),
        ui_data_proj_float_to_str(buf2, sizeof(buf2), vec->value[1]));
}

void ui_data_proj_save_vector_3(write_stream_t s, int level, const char * tag, UI_VECTOR_3 const * vec) {
    char buf1[32]; char buf2[32]; char buf3[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s x=\"%s\" y=\"%s\" z=\"%s\" />\n",
        tag,
        ui_data_proj_float_to_str(buf1, sizeof(buf1), vec->value[0]),
        ui_data_proj_float_to_str(buf2, sizeof(buf3), vec->value[1]),
        ui_data_proj_float_to_str(buf3, sizeof(buf3), vec->value[2]));
}

void ui_data_proj_save_vector_4(write_stream_t s, int level, const char * tag, UI_VECTOR_4 const * vec) {
    char buf1[32]; char buf2[32]; char buf3[32]; char buf4[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s x=\"%s\" y=\"%s\" z=\"%s\" w=\"%s\"/>\n",
        tag,
        ui_data_proj_float_to_str(buf1, sizeof(buf1), vec->value[0]),
        ui_data_proj_float_to_str(buf2, sizeof(buf3), vec->value[1]),
        ui_data_proj_float_to_str(buf3, sizeof(buf3), vec->value[2]),
        ui_data_proj_float_to_str(buf4, sizeof(buf4), vec->value[3]));
}

void ui_data_proj_save_unit(write_stream_t s, int level, const char * tag, UI_UNIT const * value) {
    char buf1[32]; char buf2[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s k=\"%s\" b=\"%s\" />\n",
        tag,
        ui_data_proj_float_to_str(buf1, sizeof(buf1), value->k),
        ui_data_proj_float_to_str(buf2, sizeof(buf2), value->b));
}

void ui_data_proj_save_unit_vector_2(write_stream_t s, int level, const char * tag, UI_UNIT_VECTOR_2 const * vec) {
    char buf1[32]; char buf2[32]; char buf3[32]; char buf4[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s xk=\"%s\" xb=\"%s\" yk=\"%s\" yb=\"%s\" />\n",
        tag,
        ui_data_proj_float_to_str(buf1, sizeof(buf1), vec->x.k),
        ui_data_proj_float_to_str(buf2, sizeof(buf2), vec->x.b),
        ui_data_proj_float_to_str(buf3, sizeof(buf3), vec->y.k),
        ui_data_proj_float_to_str(buf4, sizeof(buf4), vec->y.b));
}

void ui_data_proj_save_unit_rect(write_stream_t s, int level, const char * tag, UI_UNIT_RECT const * rect) {
    char buf1[32]; char buf2[32]; char buf3[32]; char buf4[32]; char buf5[32]; char buf6[32]; char buf7[32]; char buf8[32];

    stream_putc_count(s, ' ', level * 4);
    stream_printf(
        s, "<%s ltk=\"%s\" ltb=\"%s\" tpk=\"%s\" tpb=\"%s\" rtk=\"%s\" rtb=\"%s\" bmk=\"%s\" bmb=\"%s\" />\n",
        tag,
        ui_data_proj_float_to_str(buf1, sizeof(buf1), rect->lt.x.k),
        ui_data_proj_float_to_str(buf2, sizeof(buf2), rect->lt.x.b),
        ui_data_proj_float_to_str(buf3, sizeof(buf3), rect->lt.y.k),
        ui_data_proj_float_to_str(buf4, sizeof(buf4), rect->lt.y.b),
        ui_data_proj_float_to_str(buf5, sizeof(buf5), rect->rb.x.k),
        ui_data_proj_float_to_str(buf6, sizeof(buf6), rect->rb.x.b),
        ui_data_proj_float_to_str(buf7, sizeof(buf7), rect->rb.y.k),
        ui_data_proj_float_to_str(buf8, sizeof(buf8), rect->rb.y.b));
}

void ui_data_proj_save_obj_start(write_stream_t s, int level, const char * tag) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s>\n", tag);
}

void ui_data_proj_save_obj_end(write_stream_t s, int level, const char * tag) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "</%s>\n", tag);
}

void ui_data_proj_save_obj_null(write_stream_t s, int level, const char * tag) {
    stream_putc_count(s, ' ', level * 4);
    stream_printf(s, "<%s />\n", tag);
}

void ui_data_proj_save_trans(write_stream_t s, int level, UI_TRANS const * trans) {
    ui_data_proj_save_float(s, level, "LocalAngle", trans->local_trans.angle);
    ui_data_proj_save_float(s, level, "WorldAngle", trans->world_trans.angle);
    ui_data_proj_save_vector_3(s, level, "LocalAnglePivot", &trans->local_trans.angle_pivot);
    ui_data_proj_save_vector_3(s, level, "WorldAnglePivot", &trans->world_trans.angle_pivot);
    ui_data_proj_save_int(s, level, "LocalFlips", trans->local_trans.flips);
    ui_data_proj_save_int(s, level, "WorldFlips", trans->world_trans.flips);
    ui_data_proj_save_vector_3(s, level, "LocalScale", &trans->local_trans.scale);
    ui_data_proj_save_vector_3(s, level, "WorldScale", &trans->world_trans.scale);
    ui_data_proj_save_vector_3(s, level, "LocalTrans", &trans->local_trans.trans);
    ui_data_proj_save_vector_3(s, level, "WorldTrans", &trans->world_trans.trans);
    ui_data_proj_save_color(s, level, "BColor", &trans->background);
    ui_data_proj_save_int(s, level, "Filter", trans->filter);
    ui_data_proj_save_int(s, level, "TexEnv", trans->tex_env);
    ui_data_proj_save_int(s, level, "SrcABM", trans->src_abm);
    ui_data_proj_save_int(s, level, "DstABM", trans->dst_abm);
    ui_data_proj_save_obj_start(s, level, "OLInfo");
    ui_data_proj_save_ol(s, level + 1, &trans->ol);
    ui_data_proj_save_obj_end(s, level, "OLInfo");
}

void ui_data_proj_save_ol(write_stream_t s, int level, UI_OL const * ol) {
    ui_data_proj_save_bool(s, level, "Outline", ol->enable);
    ui_data_proj_save_int(s, level, "OutlineWidth", ol->width);
    ui_data_proj_save_color(s, level, "OutlineColor", &ol->color);
}

static const char * ui_data_proj_float_to_str(char * buf, size_t capacity, float v) {
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

int ui_data_proj_remove_file(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, ui_data_proj_postfix(ui_data_src_type(src)), em);
}
