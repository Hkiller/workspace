#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/error.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cfg_internal_types.h"

static int cfg_cmp_i(cfg_t l, cfg_t r, int policy, error_monitor_t em, cfg_t root, mem_buffer_t buffer);

static int cfg_cmp_struct(cfg_t l, cfg_t r, int policy, error_monitor_t em, cfg_t root, mem_buffer_t buffer) {
    int ret;
    cfg_it_t l_it;
    cfg_t l_child;
    cfg_it_t r_it;
    cfg_t r_child;

    ret = 0;

    cfg_it_init(&l_it, l);
    cfg_it_init(&r_it, r);

    l_child = cfg_it_next(&l_it);
    r_child = cfg_it_next(&r_it);

    while(l_child && r_child && (ret == 0 || em)) {
        int name_r = strcmp(cfg_name(l_child), cfg_name(r_child));
        if (name_r < 0) {
            if ((policy & CFG_CMP_POLICY_R_STRUCT_LEAK) != CFG_CMP_POLICY_R_STRUCT_LEAK) {
                mem_buffer_clear_data(buffer);
                CPE_ERROR(
                    em, "%s: entry %s not exsit in right",
                    cfg_path(buffer, l, root),
                    cfg_name(l_child));
                if (ret == 0) ret = 1;
            }

            l_child = cfg_it_next(&l_it);
        }
        else if (name_r > 0) {
            if ((policy & CFG_CMP_POLICY_L_STRUCT_LEAK) != CFG_CMP_POLICY_L_STRUCT_LEAK) {
                mem_buffer_clear_data(buffer);
                CPE_ERROR(
                    em, "%s: entry %s not exsit in left",
                    cfg_path(buffer, l, root),
                    cfg_name(r_child));
                if (ret == 0) ret = -1;
            }
            r_child = cfg_it_next(&r_it);
        }
        else {
            int part = cfg_cmp_i(l_child, r_child, policy, em, root, buffer);
            if (ret == 0 && part != 0) ret = part;
            
            l_child = cfg_it_next(&l_it);
            r_child = cfg_it_next(&r_it);
        }
    }

    if ((policy & CFG_CMP_POLICY_R_STRUCT_LEAK) != CFG_CMP_POLICY_R_STRUCT_LEAK) {
        for(; l_child && (ret == 0 || em); l_child = cfg_it_next(&l_it)) {
            mem_buffer_clear_data(buffer);
            CPE_ERROR(
                em, "%s: entry %s not exsit in right",
                cfg_path(buffer, l, root),
                cfg_name(l_child));

            if (ret == 0) ret = 1;
        }
    }

    if ((policy & CFG_CMP_POLICY_L_STRUCT_LEAK) != CFG_CMP_POLICY_L_STRUCT_LEAK) {
        for(; r_child && (ret == 0 || em); r_child = cfg_it_next(&r_it)) {
            mem_buffer_clear_data(buffer);
            CPE_ERROR(
                em, "%s: entry %s not exsit in left",
                cfg_path(buffer, l, root),
                cfg_name(r_child));
            if (ret == 0) ret = -1;
        }
    }

    return ret;
}

static int cfg_cmp_sequence(cfg_t l, cfg_t r, int policy, error_monitor_t em, cfg_t root, mem_buffer_t buffer) {
    int ret;
    cfg_it_t l_it;
    cfg_t l_child;
    cfg_it_t r_it;
    cfg_t r_child;

    ret = 0;

    if (cfg_seq_count(l) != cfg_seq_count(r)) {
        mem_buffer_clear_data(buffer);
        CPE_ERROR(
            em, "%s: sequence count mismatch %d and %d",
            cfg_path(buffer, l, root),
            (int)cfg_seq_count(l),
            (int)cfg_seq_count(r));
        if (ret == 0) ret = cfg_seq_count(l) - cfg_seq_count(r);
    }

    cfg_it_init(&l_it, l);
    cfg_it_init(&r_it, r);

    while((l_child = cfg_it_next(&l_it))
          && (r_child = cfg_it_next(&r_it))
          && (ret == 0 || em))
    {
        int part = cfg_cmp_i(l_child, r_child, policy, em, root, buffer);
        if (ret == 0 && part != 0) ret = part;
    }

    return ret;
}

static int cfg_cmp_i(cfg_t l, cfg_t r, int policy, error_monitor_t em, cfg_t root, mem_buffer_t buffer) {
    switch(l->m_type) {
    case CPE_CFG_TYPE_STRUCT: {
        if (r->m_type != CPE_CFG_TYPE_STRUCT) {
            mem_buffer_clear_data(buffer);
            CPE_ERROR(
                em, "%s: compaire %s to %s",
                cfg_path(buffer, l, root),
                "struct",
                (r->m_type == CPE_CFG_TYPE_SEQUENCE ? "sequence" : dr_type_name(r->m_type)));

            return l->m_type < r->m_type;
        }
        else {
            return cfg_cmp_struct(l, r, policy, em, root, buffer);
        }
    }
    case CPE_CFG_TYPE_SEQUENCE: {
        if (r->m_type != CPE_CFG_TYPE_SEQUENCE) {
            mem_buffer_clear_data(buffer);
            CPE_ERROR(
                em, "%s: compaire %s to %s",
                cfg_path(buffer, l, root),
                "sequence",
                (r->m_type == CPE_CFG_TYPE_STRUCT ? "struct" : dr_type_name(r->m_type)));

            return l->m_type < r->m_type;
        }
        else {
            return cfg_cmp_sequence(l, r, policy, em, root, buffer);
        }
    }
    default: {
        if (r->m_type == CPE_CFG_TYPE_STRUCT) {
            mem_buffer_clear_data(buffer);

            CPE_ERROR(
                em, "%s: compaire %s to %s",
                cfg_path(buffer, l, root),
                dr_type_name(r->m_type),
                "struct");

            return l->m_type < r->m_type;
        }
        else if (r->m_type == CPE_CFG_TYPE_SEQUENCE) {
            mem_buffer_clear_data(buffer);
            CPE_ERROR(
                em, "%s: compaire %s to %s",
                cfg_path(buffer, l, root),
                dr_type_name(r->m_type),
                "sequence");

            return l->m_type < r->m_type;
        }
        else {
            int ret = dr_ctype_cmp(cfg_data(l), l->m_type, cfg_data(r), r->m_type);
            if (ret != 0) {
                char lValueBuf[128];
                char rValueBuf[128];
                struct write_stream_mem stream;
                int pr;

                write_stream_mem_init(&stream, lValueBuf, sizeof(lValueBuf));
                pr = dr_ctype_print_to_stream((write_stream_t)&stream, cfg_data(l), l->m_type, 0);
                lValueBuf[pr > 0 ? pr : 0] = 0;

                write_stream_mem_init(&stream, rValueBuf, sizeof(rValueBuf));
                pr = dr_ctype_print_to_stream((write_stream_t)&stream, cfg_data(r), r->m_type, 0);
                rValueBuf[pr > 0 ? pr : 0] = 0;

                mem_buffer_clear_data(buffer);
                CPE_ERROR(
                    em, "%s: %s %s than %s",
                    cfg_path(buffer, l, root),
                    lValueBuf,
                    ret > 0 ? "bigger" : "smaller",
                    rValueBuf);
            }
            return ret;
        }
    }
    }
}

int cfg_cmp(cfg_t l, cfg_t r, int policy, error_monitor_t em) {
    int ret;
    struct mem_buffer buffer;

    assert(l);
    assert(r);

    mem_buffer_init(&buffer, 0);

    ret = cfg_cmp_i(l, r, policy, em, l, &buffer);

    mem_buffer_clear(&buffer);

    return ret;
}
