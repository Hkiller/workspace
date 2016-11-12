#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_ops.h"

static void cfg_do_merge_struct(cfg_t cfg, cfg_t source, cfg_policy_t policy, error_monitor_t em);
static void cfg_do_merge_struct_child(cfg_t cfg, cfg_t child, cfg_policy_t policy, error_monitor_t em);
static void cfg_do_copy_seq(cfg_t cfg, cfg_t source, error_monitor_t em);

static void cfg_do_merge_struct(cfg_t cfg, cfg_t source, cfg_policy_t policy, error_monitor_t em) {
    struct cfg_it it;
    cfg_t child;

    if (cfg == NULL) return;

    assert(cfg->m_type == CPE_CFG_TYPE_STRUCT);
    assert(source->m_type == CPE_CFG_TYPE_STRUCT);

    cfg_it_init(&it, source);
    while((child = cfg_it_next(&it))) {
        cfg_do_merge_struct_child(cfg, child, policy, em);
    }
}

static void cfg_do_merge_struct_child(cfg_t cfg, cfg_t child, cfg_policy_t policy, error_monitor_t em) {
    assert(cfg != child);

    switch(child->m_type) {
    case CPE_CFG_TYPE_SEQUENCE:
        cfg_do_copy_seq(
            cfg_struct_add_seq(cfg, cfg_name(child), policy),
            child, em);
        break;
    case CPE_CFG_TYPE_STRUCT:
        cfg_do_merge_struct(
            cfg_struct_add_struct(cfg, cfg_name(child), policy),
            child, policy, em);
        break;
    case CPE_CFG_TYPE_INT8:
        cfg_struct_add_int8(
            cfg, cfg_name(child),
            dr_ctype_read_int8(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_UINT8:
        cfg_struct_add_uint8(
            cfg, cfg_name(child),
            dr_ctype_read_uint8(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_INT16:
        cfg_struct_add_int16(
            cfg, cfg_name(child),
            dr_ctype_read_int16(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_UINT16:
        cfg_struct_add_uint16(
            cfg, cfg_name(child),
            dr_ctype_read_uint16(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_INT32:
        cfg_struct_add_int32(
            cfg, cfg_name(child),
            dr_ctype_read_int32(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_UINT32:
        cfg_struct_add_uint32(
            cfg, cfg_name(child),
            dr_ctype_read_uint32(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_INT64:
        cfg_struct_add_int64(
            cfg, cfg_name(child),
            dr_ctype_read_int64(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_UINT64:
        cfg_struct_add_uint64(
            cfg, cfg_name(child),
            dr_ctype_read_uint64(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_STRING:
        cfg_struct_add_string(
            cfg, cfg_name(child),
            (const char *)cfg_data(child),
            policy);
        break;
    case CPE_CFG_TYPE_FLOAT:
        cfg_struct_add_float(
            cfg, cfg_name(child),
            dr_ctype_read_float(cfg_data(child), child->m_type),
            policy);
        break;
    case CPE_CFG_TYPE_DOUBLE:
        cfg_struct_add_double(
            cfg, cfg_name(child),
            dr_ctype_read_double(cfg_data(child), child->m_type),
            policy);
        break;
    default:
        CPE_ERROR(em, "cfg_merge_struct: unknown child type %d!", child->m_type);
        break;
    }
}

static void cfg_do_copy_seq(cfg_t cfg, cfg_t source, error_monitor_t em) {
    struct cfg_it it;
    cfg_t child;

    if (cfg == NULL) return;

    assert(cfg->m_type == CPE_CFG_TYPE_SEQUENCE);
    assert(source->m_type == CPE_CFG_TYPE_SEQUENCE);

    cfg_it_init(&it, source);
    while((child = cfg_it_next(&it))) {
        switch(child->m_type) {
        case CPE_CFG_TYPE_SEQUENCE:
            cfg_do_copy_seq(
                cfg_seq_add_seq(cfg),
                child, em);
            break;
        case CPE_CFG_TYPE_STRUCT:
            cfg_do_merge_struct(
                cfg_seq_add_struct(cfg),
                child, cfg_merge_use_new, em);
            break;
        case CPE_CFG_TYPE_INT8:
            cfg_seq_add_int8(
                cfg,
                dr_ctype_read_int8(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_UINT8:
            cfg_seq_add_uint8(
                cfg,
                dr_ctype_read_uint8(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_INT16:
            cfg_seq_add_int16(
                cfg,
                dr_ctype_read_int16(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_UINT16:
            cfg_seq_add_uint16(
                cfg,
                dr_ctype_read_uint16(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_INT32:
            cfg_seq_add_int32(
                cfg,
                dr_ctype_read_int32(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_UINT32:
            cfg_seq_add_uint32(
                cfg,
                dr_ctype_read_uint32(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_INT64:
            cfg_seq_add_int64(
                cfg,
                dr_ctype_read_int64(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_UINT64:
            cfg_seq_add_uint64(
                cfg,
                dr_ctype_read_uint64(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_STRING:
            cfg_seq_add_string(
                cfg,
                (const char *)cfg_data(child));
            break;
        case CPE_CFG_TYPE_FLOAT:
            cfg_seq_add_float(
                cfg,
                dr_ctype_read_float(cfg_data(child), child->m_type));
            break;
        case CPE_CFG_TYPE_DOUBLE:
            cfg_seq_add_double(
                cfg,
                dr_ctype_read_double(cfg_data(child), child->m_type));
            break;
        default:
            CPE_ERROR(em, "cfg_merge_seq: unknown child type %d!", child->m_type);
            break;
        }
    }
}

int cfg_merge(cfg_t cfg, cfg_t source, cfg_policy_t policy, error_monitor_t em) {
    cfg_t parent;

    assert(cfg);
    if (source == NULL) return 0;

    if (cfg->m_type == source->m_type) {
        if (cfg->m_type == CPE_CFG_TYPE_STRUCT) {
            cfg_do_merge_struct(cfg, source, policy, em);
            return 0;
        }

        if (policy == cfg_merge_use_exist) return 0;

        assert(policy == cfg_merge_use_new || policy == cfg_replace);

        if (cfg->m_type == CPE_CFG_TYPE_SEQUENCE) {
            if (policy == cfg_replace) {
                cfg_seq_fini((struct cfg_seq *)cfg);
            }

            cfg_do_copy_seq(cfg, source, em);
        }
        else {
            memcpy(cfg_data(cfg), cfg_data(source), dr_type_size(cfg->m_type));
        }

        return 0;
    }

    if ((parent = cfg_parent(cfg))) {
        if (parent->m_type == CPE_CFG_TYPE_STRUCT) {
            cfg_do_merge_struct_child(parent, source, policy, em);
        }
        else {
            assert(parent->m_type == CPE_CFG_TYPE_SEQUENCE);
        }
    }

    CPE_ERROR(
        em, "cfg_merge: con`t merge type %s to %s", 
        dr_type_name(source->m_type), dr_type_name(cfg->m_type));

    return -1;
}
