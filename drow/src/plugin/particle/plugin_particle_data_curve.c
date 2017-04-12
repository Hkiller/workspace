#include "cpe/pal/pal_strings.h"
#include "cpe/utils/math_ex.h"
#include "cpe/utils/algorithm.h"
#include "gd/app/app_context.h"
#include "plugin_particle_data_i.h"

plugin_particle_data_curve_t
plugin_particle_data_curve_create(plugin_particle_data_t particle, uint16_t curve_id) {
    plugin_particle_module_t module = particle->m_module;
    plugin_particle_data_curve_t curve;

    if (curve_id == 0) curve_id = particle->m_curve_max_id + 1;

    curve = mem_alloc(module->m_alloc, sizeof(struct plugin_particle_data_curve));
    if (curve == NULL) {
        CPE_ERROR(module->m_em, "plugin_particle_data_curve_create: alloc fail!");
        return NULL;
    }

    curve->m_particle = particle;
    curve->m_id = curve_id;
    curve->m_point_count = 0;
    curve->m_point_capacity = CPE_ARRAY_SIZE(curve->m_point_buf);
    curve->m_points_external = 0;
    curve->m_points = curve->m_point_buf;

    cpe_hash_entry_init(&curve->m_hh);
    if (cpe_hash_table_insert_unique(&particle->m_curves, curve) != 0) {
        CPE_ERROR(
            module->m_em, "particle %s curve %d duplicate!",
            ui_data_src_path_dump(gd_app_tmp_buffer(module->m_app), particle->m_src), curve_id);
        mem_free(module->m_alloc, curve);
        return NULL;
    }
    
    if (curve_id > particle->m_curve_max_id) particle->m_curve_max_id = curve_id;
    return curve;
}

void plugin_particle_data_curve_free(plugin_particle_data_curve_t curve) {
    plugin_particle_module_t module = curve->m_particle->m_module;

    if (curve->m_points != curve->m_point_buf && curve->m_points_external) {
        mem_free(module->m_alloc, curve->m_points);
    }

    cpe_hash_table_remove_by_ins(&curve->m_particle->m_curves, curve);
    
    mem_free(module->m_alloc, curve);
}

void plugin_particle_data_curve_free_all(plugin_particle_data_t particle) {
    struct cpe_hash_it curve_it;
    plugin_particle_data_curve_t curve;

    cpe_hash_it_init(&curve_it, &particle->m_curves);

    curve = cpe_hash_it_next(&curve_it);
    while (curve) {
        plugin_particle_data_curve_t next = cpe_hash_it_next(&curve_it);
        plugin_particle_data_curve_free(curve);
        curve = next;
    }
}

plugin_particle_data_curve_t
plugin_particle_data_curve_find(plugin_particle_data_t particle, uint16_t curve_id) {
    struct plugin_particle_data_curve key;
    key.m_id = curve_id;
    return cpe_hash_table_find(&particle->m_curves, &key);
}
    
uint16_t plugin_particle_data_curve_id(plugin_particle_data_curve_t curve) {
    return curve->m_id;
}

uint16_t plugin_particle_data_curve_point_count(plugin_particle_data_curve_t curve) {
    return curve->m_point_count;
}

UI_CURVE_POINT * plugin_particle_data_curve_point_at(plugin_particle_data_curve_t curve, uint16_t pos) {
    assert(pos < curve->m_point_count);
    return &curve->m_points[pos];
}

UI_CURVE_POINT * plugin_particle_data_curve_point_append(plugin_particle_data_curve_t curve) {
    UI_CURVE_POINT * r;
    
    if (curve->m_point_count + 1 > curve->m_point_capacity) {
        plugin_particle_module_t module = curve->m_particle->m_module;
        uint16_t new_capacity = curve->m_point_capacity < 16 ? 16 : curve->m_point_capacity * 2;
        UI_CURVE_POINT * new_buf = mem_alloc(module->m_alloc, sizeof(UI_CURVE_POINT) * new_capacity);
        if (new_buf == NULL) {
            CPE_ERROR(module->m_em, "plugin_particle_data_curve_point_append: alloc new buff fail, new-capacity=%d!", new_capacity);
            return NULL;
        }

        memcpy(new_buf, curve->m_points, sizeof(UI_CURVE_POINT) * curve->m_point_count);

        if (curve->m_points != curve->m_point_buf && curve->m_points_external) {
            mem_free(module->m_alloc, curve->m_points);
        }
        curve->m_points = new_buf;
        curve->m_point_capacity = new_capacity;
        curve->m_points_external = 0;
    }

    assert(curve->m_point_count + 1 <= curve->m_point_capacity);

    r = &curve->m_points[curve->m_point_count++];
    bzero(r, sizeof(*r));
    r->curve_id = curve->m_id;
    return r;
}

uint32_t plugin_particle_data_curve_hash(plugin_particle_data_curve_t curve) {
    return curve->m_id;
}

int plugin_particle_data_curve_eq(plugin_particle_data_curve_t l, plugin_particle_data_curve_t r) {
    return l->m_id == r->m_id;
}

static int plugin_particle_data_curve_point_cmp(const void * l, const void * r) {
    float rv = ((UI_CURVE_POINT const *)l)->key - ((UI_CURVE_POINT const *)r)->key;
    return rv < 0.0f
        ? -1
        : rv > 0.0f
        ? 1
        : 0;
}

static float plugin_particle_data_curve_sample_i(float key, UI_CURVE_POINT const * l, UI_CURVE_POINT const * r) {
    switch(l->interp) {
    case ui_curve_interp_mode_break:
        return l->ret;
    case ui_curve_interp_mode_linear: {
        float percent = (key - l->key) / (r->key - l->key);
        return l->ret * (1.0f - percent) + r->ret * percent;
    }
    case ui_curve_interp_mode_bezier: {
        const float time = (key - l->key) / (r->key - l->key);
        const float negt = 1 - time;
        float d = fabs(l->ret - r->ret);
        float p0 = l->ret;
        float p1 = l->ret + l->leave_tan * d;
        float p2 = r->ret + r->enter_tan * d;
        float p3 = r->ret;
        return p0 * negt * negt * negt + 3.0f * p1 * time * negt * negt + 3.0f * p2 * time * time * negt + p3 * time * time * time;
    }
    default:
        return 3.402823466e+38f; // FLT_MAX
    }
}

float plugin_particle_data_curve_sample(plugin_particle_data_curve_t chanel, float i_key) {
    UI_CURVE_POINT const * point;
    UI_CURVE_POINT const * pre_point;
    UI_CURVE_POINT key;
    int idx;

    if (chanel->m_point_count == 0) return 0.0f;

    key.key = i_key;
    point = cpe_lower_bound((void*)&chanel->m_points[0], chanel->m_point_count, &key, sizeof(UI_CURVE_POINT), plugin_particle_data_curve_point_cmp);
    idx = point - chanel->m_points;

    if (idx >= chanel->m_point_count) return chanel->m_points[chanel->m_point_count - 1].ret;
    if (idx == 0) return chanel->m_points[0].ret;
    
    pre_point = point - 1;

    return plugin_particle_data_curve_sample_i(i_key, pre_point, point);
}


