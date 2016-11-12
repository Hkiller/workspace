#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/cfg/cfg.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "usf/logic/logic_manage.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_context.h"
#include "usf/logic_use/logic_data_build.h"

logic_data_t logic_data_build(logic_context_t context, LPDRMETA meta, cfg_t cfg, error_monitor_t em) {
    logic_data_t des;
    struct cfg_it child_cfgs;
    cfg_t arg_cfg;

    des = logic_context_data_get_or_create(context, meta, 0);
    if (des == NULL) {
        CPE_ERROR(em, "logic_data_build %s: create data fail!", dr_meta_name(meta));
        return NULL;
    }

    cfg_it_init(&child_cfgs, cfg);
    while((arg_cfg = cfg_it_next(&child_cfgs))) {
        const char * arg_name;
        const char * arg_path;
        int des_data_off;
        LPDRMETAENTRY des_entry;
        char * sep;
        size_t root_name_len;
        char root_name[64];
        logic_data_t src;
        int src_data_off;
        LPDRMETAENTRY src_entry;

        arg_cfg = cfg_child_only(arg_cfg);

        arg_name = cfg_name(arg_cfg);
        arg_path = cfg_as_string(arg_cfg, NULL);

        if (arg_name == NULL || arg_path == NULL) {
            CPE_ERROR(em, "logic_data_build %s: arg config format error!", dr_meta_name(meta));
            logic_data_free(des);
            return NULL;
        }

        des_data_off = -1;
        des_entry = dr_meta_find_entry_by_path_ex(meta, arg_name, &des_data_off);
        if (des_entry == NULL) {
            CPE_ERROR(em, "logic_data_build %s: set arg %s: not exist in meta", dr_meta_name(meta), arg_name);
            logic_data_free(des);
            return NULL;
        }

        sep = strchr(arg_path, '.');
        if (sep == NULL) {
            CPE_ERROR(
                em, "logic_data_build %s: set arg %s: path (%s) format error!",
                dr_meta_name(meta), arg_name, arg_path);
            continue;
        } 

        root_name_len = sep - arg_path;
        if ((root_name_len + 1) > sizeof(root_name)) {
            CPE_ERROR(
                em, "logic_data_build %s: set arg %s: path %s root name too long!", dr_meta_name(meta), arg_name, arg_path);
            continue;
        }

        memcpy(root_name, arg_path, root_name_len);
        root_name[root_name_len] = 0;

        src = logic_context_data_find(context, root_name);
        if (src == NULL) {
            CPE_ERROR(
                em, "logic_data_build %s: set arg %s: root %s not exist!", dr_meta_name(meta), arg_name, root_name);
            continue;
        }

        src_data_off = -1;
        src_entry = dr_meta_find_entry_by_path_ex(logic_data_meta(src), sep + 1, &src_data_off);
        if (src_entry == NULL) {
            CPE_ERROR(
                em, "logic_data_build %s: set arg %s: can`t find entry at %s from %s!", dr_meta_name(meta), arg_name, sep + 1, root_name);
            continue;
        }

        switch(dr_entry_type(des_entry)) {
        case CPE_DR_TYPE_STRUCT:
            if (dr_entry_type(src_entry) != CPE_DR_TYPE_STRUCT) {
                CPE_ERROR(
                    em, "logic_data_build %s: set arg %s: not support set %s from %s!",
                    dr_meta_name(meta), arg_name,
                    dr_type_name(dr_entry_type(des_entry)),
                    dr_type_name(dr_entry_type(src_entry)));
            }
            else {
                if (dr_meta_copy_same_entry(
                        ((char*)logic_data_data(des)) + des_data_off,
                        logic_data_capacity(des) - des_data_off,
                        dr_entry_ref_meta(des_entry),
                        ((char *)logic_data_data(src)) + src_data_off,
                        logic_data_capacity(src) - src_data_off,
                        dr_entry_ref_meta(src_entry),
                        0, NULL) < 0)
                {
                    CPE_ERROR(
                        em, "logic_data_build %s: set arg %s: %s from %s fail!",
                        dr_meta_name(meta), arg_name,
                        dr_type_name(dr_entry_type(des_entry)),
                        dr_type_name(dr_entry_type(src_entry)));
                }
            }
            break;
        case CPE_DR_TYPE_UNION:
            CPE_ERROR(
                em, "logic_data_build %s: set arg %s: not support set union yet!", dr_meta_name(meta), arg_name);
            break;
        default:
            if (dr_entry_type(src_entry) > CPE_DR_TYPE_COMPOSITE) {
                if (dr_entry_set_from_ctype(
                        ((char*)logic_data_data(des)) + des_data_off,
                        ((char *)logic_data_data(src)) + src_data_off,
                        dr_entry_type(src_entry),
                        des_entry,
                        em) != 0)
                {
                    CPE_ERROR(
                        em, "logic_data_build %s: set arg %s: %s from %s fail!",
                        dr_meta_name(meta), arg_name,
                        dr_type_name(dr_entry_type(des_entry)),
                        dr_type_name(dr_entry_type(src_entry)));
                }
            }
            else {
                CPE_ERROR(
                    em, "logic_data_build %s: set arg %s: not support set %s from %s!",
                    dr_meta_name(meta), arg_name,
                    dr_type_name(dr_entry_type(des_entry)),
                    dr_type_name(dr_entry_type(src_entry)));
            }
            break;
        }
    }

    return des;
}

