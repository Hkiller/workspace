#include "cpe/pal/pal_string.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_read.h"
#include "cfg_internal_ops.h"

static int cfg_apply_modify_set(cfg_t cfg, cfg_t modify_info, error_monitor_t em) {
    char buf[256];
    const char * path = cfg_name(modify_info);
    cfg_t last_cfg;
    cfg_t old_cfg;

    last_cfg = cfg_check_or_create(cfg, path, em, buf, sizeof(buf));
    if (last_cfg == NULL) return -1;

    old_cfg = cfg_find_cfg(last_cfg, buf);
    if (old_cfg) cfg_free(old_cfg);

    switch(cfg_type(modify_info)) {
    case CPE_CFG_TYPE_SEQUENCE: {
        cfg_t seq = cfg_add_seq(cfg, path, em);
        if (seq == NULL) return -1;

        if (cfg_merge(seq, modify_info, cfg_replace, em) != 0) {
            return -1;
        }

        break;
    }
    case CPE_CFG_TYPE_STRUCT: {
        cfg_t st = cfg_add_struct(cfg, path, em);
        if (st == NULL) return -1;

        if (cfg_merge(st, modify_info, cfg_replace, em) != 0) {
            return -1;
        }

        break;
    }
    case CPE_CFG_TYPE_INT8:
        if (cfg_add_int8(last_cfg, buf, cfg_as_int8(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_UINT8:
        if (cfg_add_uint8(last_cfg, buf, cfg_as_uint8(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_INT16:
        if (cfg_add_int16(last_cfg, buf, cfg_as_int16(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_UINT16:
        if (cfg_add_uint16(last_cfg, buf, cfg_as_uint16(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_INT32:
        if (cfg_add_int32(last_cfg, buf, cfg_as_int32(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_UINT32:
        if (cfg_add_uint32(last_cfg, buf, cfg_as_uint32(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_INT64:
        if (cfg_add_int64(last_cfg, buf, cfg_as_int64(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_UINT64:
        if (cfg_add_uint64(last_cfg, buf, cfg_as_uint64(modify_info, 0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_STRING:
        if (cfg_add_string(last_cfg, buf, cfg_as_string(modify_info, ""), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_FLOAT:
        if (cfg_add_float(last_cfg, buf, cfg_as_float(modify_info, 0.0), em) == NULL) {
            return -1;
        }
        break;
    case CPE_CFG_TYPE_DOUBLE:
        if (cfg_add_double(last_cfg, buf, cfg_as_double(modify_info, 0.0), em) == NULL) {
            return -1;
        }
        break;
    default:
        CPE_ERROR(em, "cfg_apply_modify_set: %s: unknown type %d!", path, cfg_type(modify_info));
        return -1;
    }

    return 0;
}

int cfg_apply_modify(cfg_t cfg, cfg_t modify_info, error_monitor_t em) {
    const char * op_name;
    struct cfg_it chils;
    cfg_t with_cfg, op_cfg;
    
    int rv = 0;

    with_cfg = cfg_find_cfg(modify_info, "with");
    if (with_cfg) {
        if (cfg_type(with_cfg) != CPE_DR_TYPE_STRING) {
            CPE_ERROR(em, "cfg_apply_modify: with require string value!");
            return -1;
        }

        cfg = cfg_find_cfg(cfg, cfg_as_string(with_cfg, NULL));
    }

    if (cfg == NULL) return 0;

    cfg_it_init(&chils, modify_info);
    while((op_cfg = cfg_it_next(&chils))) {
        op_name = cfg_name(op_cfg);

        if (strcmp(op_name, "with") == 0) continue;

        if (strcmp(op_name, "set") == 0) {
            switch(cfg_type(op_cfg)) {
            case CPE_CFG_TYPE_SEQUENCE: {
                struct cfg_it set_childs;
                cfg_it_init(&set_childs, op_cfg);

                while((op_cfg = cfg_it_next(&set_childs))) {
                    op_cfg = cfg_child_only(op_cfg);
                    if (op_cfg == NULL) {
                        CPE_ERROR(em, "cfg_apply_modify: set: sequence child format error!");
                        rv = -1;
                    }
                    else {
                        if (cfg_apply_modify_set(cfg, op_cfg, em) != 0) rv = -1;
                    }
                }
                break;
            }
            case CPE_CFG_TYPE_STRUCT: {
                struct cfg_it set_childs;
                cfg_it_init(&set_childs, op_cfg);

                while((op_cfg = cfg_it_next(&set_childs))) {
                    if (cfg_apply_modify_set(cfg, op_cfg, em) != 0) rv = -1;
                }
                break;
            }
            default:
                if (cfg_apply_modify_set(cfg, op_cfg, em) != 0) rv = -1;
                break;
            }
        }
        else {
            CPE_ERROR(em, "cfg_apply_modify: unknown op name %s!", op_name);
            rv = -1;
        }
    }

    return rv;
}


int cfg_apply_modify_seq(cfg_t cfg, cfg_t modify_info, error_monitor_t em) {
    int r = 0;
    struct cfg_it it;
    cfg_t child;

    cfg_it_init(&it, modify_info);

    while((child = cfg_it_next(&it))) {
        if (cfg_apply_modify(cfg, child, em) != 0) {
            r = -1;
        }
    }

    return r;
}


