#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/stream.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/pal/pal_stdio.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_quaternion.h"
#include "render/utils/ui_matrix_4x4.h"
#include "render/utils/ui_vector_3.h"
#include "render/utils/ui_vector_2.h"

int ui_transform_cmp(ui_transform_t l, ui_transform_t r) {
    int rv;
        
    if ((rv = cpe_float_cmp(l->m_m4.m14, r->m_m4.m14, UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->m_m4.m24, r->m_m4.m24, UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->m_m4.m34, r->m_m4.m34, UI_FLOAT_PRECISION))) return rv;

    if ((rv = cpe_float_cmp(l->m_s.x, r->m_s.x, UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->m_s.y, r->m_s.y, UI_FLOAT_PRECISION))) return rv;
    if ((rv = cpe_float_cmp(l->m_s.z, r->m_s.z, UI_FLOAT_PRECISION))) return rv;

    if (l->m_r_p) {
        if (!r->m_r_p) return -1;

        assert(l->m_init_p);
        assert(r->m_init_p);
        return ui_matrix_4x4_cmp(&l->m_m4, &r->m_m4);
    }
    else {
        if (r->m_r_p) return 1;
    }

    return 0;
}

void ui_transform_set_pos_3(ui_transform_t transform, ui_vector_3_t pos) {
    cpe_assert_float_sane(pos->x);
    cpe_assert_float_sane(pos->y);
    cpe_assert_float_sane(pos->z);
    
    transform->m_m4.m14 = pos->x;
    transform->m_m4.m24 = pos->y;
    transform->m_m4.m34 = pos->z;
}

void ui_transform_adj_by_pos_3(ui_transform_t transform, ui_vector_3_t pos) {
    cpe_assert_float_sane(pos->x);
    cpe_assert_float_sane(pos->y);
    cpe_assert_float_sane(pos->z);

    transform->m_m4.m14 += pos->x;
    transform->m_m4.m24 += pos->y;
    transform->m_m4.m34 += pos->z;
}

void ui_transform_get_pos_3(ui_transform_t transform, ui_vector_3_t pos) {
    pos->x = transform->m_m4.m14;
    pos->y = transform->m_m4.m24;
    pos->z = transform->m_m4.m34;
}

void ui_transform_set_pos_2(ui_transform_t transform, ui_vector_2_t pos) {
    cpe_assert_float_sane(pos->x);
    cpe_assert_float_sane(pos->y);
    
    transform->m_m4.m14 = pos->x;
    transform->m_m4.m24 = pos->y;
}

void ui_transform_adj_by_pos_2(ui_transform_t transform, ui_vector_2_t pos) {
    cpe_assert_float_sane(pos->x);
    cpe_assert_float_sane(pos->y);
    
    transform->m_m4.m14 += pos->x;
    transform->m_m4.m24 += pos->y;
}

void ui_transform_get_pos_2(ui_transform_t transform, ui_vector_2_t pos) {
    pos->x = transform->m_m4.m14;
    pos->y = transform->m_m4.m24;
}

void ui_transform_set_quation(ui_transform_t transform, ui_quaternion_t q) {
    if (ui_quaternion_cmp(q, &UI_QUATERNION_IDENTITY) == 0) {
        transform->m_r_p = 0;
        transform->m_init_p = 0;
        return;
    }
    else {
        ui_vector_3 pos = UI_VECTOR_3_INITLIZER(transform->m_m4.m14, transform->m_m4.m24, transform->m_m4.m34);

        if (ui_vector_3_cmp(&transform->m_s, &UI_VECTOR_3_IDENTITY) != 0) {
            ui_matrix_4x4 ms;

            ui_matrix_4x4_set_scale_vector_3(&ms, &transform->m_s);
            ui_quaternion_to_rotation_matrix_4x4(q, &transform->m_m4);
            ui_matrix_4x4_inline_cross_product(&transform->m_m4, &ms);
        }
        else {
            ui_quaternion_to_rotation_matrix_4x4(q, &transform->m_m4);
        }

        transform->m_m4.m14 = pos.x;
        transform->m_m4.m24 = pos.y;
        transform->m_m4.m34 = pos.z;

        transform->m_r_p = 1;
        transform->m_init_p = 1;
    }
}

void ui_transform_set_scale(ui_transform_t transform, ui_vector_3_t s) {
    
    cpe_assert_float_sane(s->x);
    cpe_assert_float_sane(s->y);
    cpe_assert_float_sane(s->z);
    //assert(cpe_float_cmp(s->x, 0.0f, UI_FLOAT_PRECISION) != 0);
    //assert(cpe_float_cmp(s->y, 0.0f, UI_FLOAT_PRECISION) != 0);
    //assert(cpe_float_cmp(s->z, 0.0f, UI_FLOAT_PRECISION) != 0);
    assert(cpe_float_cmp(transform->m_s.x, 0.0f, UI_FLOAT_PRECISION) != 0);
    assert(cpe_float_cmp(transform->m_s.y, 0.0f, UI_FLOAT_PRECISION) != 0);
    assert(cpe_float_cmp(transform->m_s.z, 0.0f, UI_FLOAT_PRECISION) != 0);

    if (transform->m_r_p == 0) {
        transform->m_s = *s;
        transform->m_init_p = 0;
        return;
    }

    assert(transform->m_init_p);

    if (ui_vector_3_cmp(&transform->m_s, s) != 0) {
        ui_vector_3 pos = UI_VECTOR_3_INITLIZER(transform->m_m4.m14, transform->m_m4.m24, transform->m_m4.m34);
        ui_matrix_4x4 ms;
        ui_vector_3 adj_s;

        adj_s.x = s->x / transform->m_s.x;
        adj_s.y = s->y / transform->m_s.y;
        adj_s.z = s->z / transform->m_s.z;

        assert(transform->m_init_p);

        ui_matrix_4x4_set_scale_vector_3(&ms, &adj_s);

        ui_matrix_4x4_inline_cross_product(&transform->m_m4, &ms);

        transform->m_m4.m14 = pos.x;
        transform->m_m4.m24 = pos.y;
        transform->m_m4.m34 = pos.z;
        transform->m_s = *s;
    }
}

void ui_transform_normalize_scale(ui_transform_t transform) {
    ui_vector_3 s;

    s.x = transform->m_s.x < 0.0f ? -1.0f : 1.0f;
    s.y = transform->m_s.y < 0.0f ? -1.0f : 1.0f;
    s.z = transform->m_s.z < 0.0f ? -1.0f : 1.0f;

    ui_transform_set_scale(transform, &s);
}

uint8_t ui_transform_scale_zero(ui_transform_t transform) {
    return (cpe_float_cmp(transform->m_s.x, 0.0f, UI_FLOAT_PRECISION) == 0
        || cpe_float_cmp(transform->m_s.y, 0.0f, UI_FLOAT_PRECISION) == 0
        || cpe_float_cmp(transform->m_s.z, 0.0f, UI_FLOAT_PRECISION) == 0
        )
        ? 1
        : 0;
}

void ui_transform_set_quation_scale(ui_transform_t transform, ui_quaternion_t q, ui_vector_3_t s) {
    transform->m_s = *s;
    transform->m_r_p = ui_quaternion_cmp(q, &UI_QUATERNION_IDENTITY) == 0 ? 0 : 1;
    transform->m_init_p = 0;
    
    cpe_assert_float_sane(s->x);
    cpe_assert_float_sane(s->y);
    cpe_assert_float_sane(s->z);

    if (transform->m_r_p) {
        ui_vector_3 pos = UI_VECTOR_3_INITLIZER(transform->m_m4.m14, transform->m_m4.m24, transform->m_m4.m34);

        if (ui_vector_3_cmp(s, &UI_VECTOR_3_IDENTITY) != 0) {
            ui_matrix_4x4 ms;
            ui_matrix_4x4_set_scale_vector_3(&ms, s);
            ui_quaternion_to_rotation_matrix_4x4(q, &transform->m_m4);
            ui_matrix_4x4_inline_cross_product(&transform->m_m4, &ms);
        }
        else {
            ui_quaternion_to_rotation_matrix_4x4(q, &transform->m_m4);
        }
    
        transform->m_m4.m14 = pos.x;
        transform->m_m4.m24 = pos.y;
        transform->m_m4.m34 = pos.z;

        transform->m_init_p = 1;
    }
}

ui_matrix_4x4_t ui_transform_calc_matrix_4x4(ui_transform_t transform) {
    if (!transform->m_init_p) {
        assert(transform->m_r_p == 0);

        cpe_assert_float_sane(transform->m_s.x);
        cpe_assert_float_sane(transform->m_s.y);
        cpe_assert_float_sane(transform->m_s.z);
        
        transform->m_m4.m11 = transform->m_s.x;  transform->m_m4.m12 = 0.0f, transform->m_m4.m13 = 0.0f;
        transform->m_m4.m21 = 0.0f; transform->m_m4.m22 = transform->m_s.y;  transform->m_m4.m23 = 0.0f;
        transform->m_m4.m31 = 0.0f; transform->m_m4.m32 = 0.0f; transform->m_m4.m33 = transform->m_s.z;
        transform->m_m4.m41 = 0.0f; transform->m_m4.m42 = 0.0f; transform->m_m4.m43 = 0.0f; transform->m_m4.m44 = 1.0f;

        transform->m_init_p = 1;
    }

    return &transform->m_m4;
}

void ui_transform_adj_by_flip(ui_transform_t transform, ui_vector_3_t flip) {
    ui_vector_3 s = UI_VECTOR_3_INITLIZER(transform->m_s.x * flip->x, transform->m_s.y * flip->y, transform->m_s.z * flip->z);
    ui_transform_set_scale(transform, &s);
}

void ui_transform_adj_by_parent(ui_transform_t transform, ui_transform_t parent) {
    if (!parent->m_r_p && ui_vector_3_cmp(&parent->m_s, &UI_VECTOR_3_IDENTITY) == 0) {
        transform->m_m4.m14 += parent->m_m4.m14;
        transform->m_m4.m24 += parent->m_m4.m24;
        transform->m_m4.m34 += parent->m_m4.m34;
    }
    else {
        ui_matrix_4x4 v;
        ui_matrix_4x4_cross_product(&v, ui_transform_calc_matrix_4x4(parent), ui_transform_calc_matrix_4x4(transform));
        transform->m_m4 = v;

        transform->m_r_p = (transform->m_r_p || parent->m_r_p) ? 1 : 0;
        transform->m_init_p = 1;

        transform->m_s.x *= parent->m_s.x;
        transform->m_s.y *= parent->m_s.y;
        transform->m_s.z *= parent->m_s.z;
    }
}

void ui_transform_cross_product(ui_transform_t to, ui_transform_t l, ui_transform_t r) {
    if (!l->m_r_p && ui_vector_3_cmp(&l->m_s, &UI_VECTOR_3_IDENTITY) == 0) {
        *to = *r;
        to->m_m4.m14 += l->m_m4.m14;
        to->m_m4.m24 += l->m_m4.m24;
        to->m_m4.m34 += l->m_m4.m34;
    }
    else {
        ui_matrix_4x4_cross_product(&to->m_m4, ui_transform_calc_matrix_4x4(l), ui_transform_calc_matrix_4x4(r));

        to->m_r_p = (l->m_r_p || r->m_r_p) ? 1 : 0;
        to->m_init_p = 1;

        to->m_s.x = l->m_s.x * r->m_s.x;
        to->m_s.y = l->m_s.y * r->m_s.y;
        to->m_s.z = l->m_s.z * r->m_s.z;
    }
}

void ui_transform_reverse(ui_transform_t to, ui_transform_t origin) {
    assert(origin->m_s.x != 0.0f);
    assert(origin->m_s.y != 0.0f);
    assert(origin->m_s.z != 0.0f);

    to->m_s.x = 1.0 / origin->m_s.x;
    to->m_s.y = 1.0 / origin->m_s.y;
    to->m_s.z = 1.0 / origin->m_s.z;

    to->m_r_p = origin->m_r_p;
    if (origin->m_r_p) {
        assert(origin->m_init_p);
        to->m_init_p = 1;
        ui_matrix_4x4_invers(&to->m_m4, &origin->m_m4);
    }
    else {
        to->m_init_p = 0;
        to->m_m4.m14 = - origin->m_m4.m14 * to->m_s.x;
        to->m_m4.m24 = - origin->m_m4.m24 * to->m_s.y;
        to->m_m4.m34 = - origin->m_m4.m34 * to->m_s.z;
    }
}

void ui_transform_inline_reverse(ui_transform_t t) {
    assert(t->m_s.x != 0.0f);
    assert(t->m_s.y != 0.0f);
    assert(t->m_s.z != 0.0f);
    
    t->m_s.x = 1.0 / t->m_s.x;
    t->m_s.y = 1.0 / t->m_s.y;
    t->m_s.z = 1.0 / t->m_s.z;

    if (t->m_r_p) {
        assert(t->m_init_p);
        ui_matrix_4x4_inline_invers(&t->m_m4);
    }
    else {
        t->m_init_p = 0;
        t->m_m4.m14 = - t->m_m4.m14 * t->m_s.x;
        t->m_m4.m24 = - t->m_m4.m24 * t->m_s.y;
        t->m_m4.m34 = - t->m_m4.m34 * t->m_s.z;
    }
}

void ui_transform_adj_vector_2(ui_transform_t transform, ui_vector_2_t to, ui_vector_2_t i) {
    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(transform);
    ui_matrix_4x4_adj_vector_2(m, to, i);
}

void ui_transform_inline_adj_vector_2(ui_transform_t transform, ui_vector_2_t v) {
    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(transform);
    ui_matrix_4x4_inline_adj_vector_2(m, v);
}

void ui_transform_adj_vector_2_no_t(ui_transform_t transform, ui_vector_2_t to, ui_vector_2_t i) {
    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(transform);
    ui_vector_3 pos = UI_VECTOR_3_INITLIZER(m->m14, m->m24, m->m34);

    m->m14 = 0.0f;
    m->m24 = 0.0f;
    m->m34 = 0.0f;

    ui_matrix_4x4_adj_vector_2(m, to, i);
        
    m->m14 = pos.x;
    m->m24 = pos.y;
    m->m34 = pos.z;
}

void ui_transform_inline_adj_vector_2_no_t(ui_transform_t transform, ui_vector_2_t v) {
    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(transform);
    ui_vector_3 pos = UI_VECTOR_3_INITLIZER(m->m14, m->m24, m->m34);

    m->m14 = 0.0f;
    m->m24 = 0.0f;
    m->m34 = 0.0f;

    cpe_assert_float_sane(v->x);
    cpe_assert_float_sane(v->y);

    ui_matrix_4x4_inline_adj_vector_2(m, v);
        
    m->m14 = pos.x;
    m->m24 = pos.y;
    m->m34 = pos.z;
}

void ui_transform_reverse_adj_vector_2(ui_transform_t transform, ui_vector_2_t to, ui_vector_2_t i) {
    *to = *i;
    
    to->x -= transform->m_m4.m14;
    to->y -= transform->m_m4.m24;

    to->x /= transform->m_s.x;
    to->y /= transform->m_s.y;
}

void ui_transform_inline_reverse_adj_vector_2(ui_transform_t transform, ui_vector_2_t v) {
    v->x -= transform->m_m4.m14;
    v->y -= transform->m_m4.m24;

    v->x /= transform->m_s.x;
    v->y /= transform->m_s.y;
}

void ui_transform_adj_vector_3_no_t(ui_transform_t transform, ui_vector_3_t to, ui_vector_3_t i) {
    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(transform);
    ui_vector_3 pos = UI_VECTOR_3_INITLIZER(m->m14, m->m34, m->m34);

    m->m14 = 0.0f;
    m->m24 = 0.0f;
    m->m34 = 0.0f;

    ui_matrix_4x4_adj_vector_3(m, to, i);
        
    m->m14 = pos.x;
    m->m24 = pos.y;
    m->m34 = pos.z;
}

void ui_transform_inline_adj_vector_3_no_t(ui_transform_t transform, ui_vector_3_t v) {
    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(transform);
    ui_vector_3 pos = UI_VECTOR_3_INITLIZER(m->m14, m->m34, m->m34);

    m->m14 = 0.0f;
    m->m24 = 0.0f;
    m->m34 = 0.0f;

    ui_matrix_4x4_inline_adj_vector_3(m, v);
        
    m->m14 = pos.x;
    m->m24 = pos.y;
    m->m34 = pos.z;
}

void ui_transform_adj_vector_3(ui_transform_t transform, ui_vector_3_t to, ui_vector_3_t i) {
    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(transform);
    ui_matrix_4x4_adj_vector_3(m, to, i);
}

void ui_transform_inline_adj_vector_3(ui_transform_t transform, ui_vector_3_t v) {
    ui_matrix_4x4_t m = ui_transform_calc_matrix_4x4(transform);
    ui_matrix_4x4_inline_adj_vector_3(m, v);
}

float ui_transform_calc_angle_z_rad(ui_transform_t transform) {
    ui_vector_2 r = UI_VECTOR_2_POSITIVE_UNIT_X;
    ui_transform_inline_adj_vector_2_no_t(transform, &r);
    return cpe_math_radians(0.0f, 0.0f, r.x, r.y);
}

void ui_transform_print(write_stream_t s, ui_transform_t t, const char * prefix) {
    stream_printf(
        s, "%st: [ %3.2f, %3.2f, %3.2f ]\n",
        prefix, t->m_m4.m14, t->m_m4.m24, t->m_m4.m34);

    stream_printf(
        s, "%ss: [ %3.2f, %3.2f, %3.2f ]\n",
        prefix, t->m_s.x, t->m_s.y, t->m_s.z);

    if (t->m_r_p) {
        char buf[64];
        
        assert(t->m_init_p);
        
        stream_printf(s, "%sm:\n");

        snprintf(buf, sizeof(buf), "%s    ", prefix);
        ui_matrix_4x4_print(s, &t->m_m4, buf);
    }
}

const char * ui_transform_dump(mem_buffer_t buffer, ui_transform_t t) {
    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    mem_buffer_clear_data(buffer);
    ui_transform_print((write_stream_t)&ws, t, "    ");
    mem_buffer_append_char(buffer, 0);
    return mem_buffer_make_continuous(buffer, 0);
}

void ui_transform_print_2d(write_stream_t s, ui_transform_t t) {
    stream_printf(
        s, "pos=(%f,%f), scale=(%f,%f), angle=%f",
        t->m_m4.m14, t->m_m4.m24, t->m_s.x, t->m_s.y,
        cpe_math_radians_to_angle(ui_transform_calc_angle_z_rad(t)));
}

const char * ui_transform_dump_2d(mem_buffer_t buffer, ui_transform_t t) {
    struct write_stream_buffer ws = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    mem_buffer_clear_data(buffer);
    ui_transform_print_2d((write_stream_t)&ws, t);
    mem_buffer_append_char(buffer, 0);
    return mem_buffer_make_continuous(buffer, 0);
}

ui_transform UI_TRANSFORM_IDENTITY = {
    UI_VECTOR_3_INITLIZER(1.0f, 1.0f, 1.0f),
    0,
    0,
    UI_INIT_ZERO_MATRIX_4X4
};

ui_transform UI_TRANSFORM_ZERO = {
    UI_VECTOR_3_INITLIZER(0.0f, 0.0f, 0.0f),
    0,
    0,
    UI_INIT_ZERO_MATRIX_4X4
};

