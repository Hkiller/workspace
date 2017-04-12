#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_manage.h"
#include "cpe/cfg/cfg_calc.h"
#include "cpe/dr/dr_cfg.h"
#include "render/model/ui_data_src_group.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_search.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin_package_manip_search_i.h"

static uint8_t plugin_package_manip_collect_search_src_name_match(ui_data_src_t src, char * name) {
    char * last;
    uint8_t rv;
    
    if (name[0] == 0) return 1;

    last = name + strlen(name) - 1;
    if (*last == '*') {
        *last = 0;
        rv = cpe_str_start_with(ui_data_src_data(src), name);
        *last = '*';
    }
    else {
        rv = strcmp(ui_data_src_data(src), name) == 0 ? 1 : 0;
    }

    return rv;
}

static uint8_t plugin_package_manip_collect_search_src_need_process(ui_data_src_t src, char * path) {
    char * sep = strchr(path, '/');
    uint8_t rv;

    if (src == NULL) return 0;
    
    if (sep == NULL) return plugin_package_manip_collect_search_src_name_match(src, path);

    if (!plugin_package_manip_collect_search_src_name_match(src, sep + 1)) return 0;

    *sep = 0;
    rv = plugin_package_manip_collect_search_src_need_process(ui_data_src_parent(src), path);
    *sep = '/';

    return 1;
}

static int plugin_package_manip_collect_search_src_parse_arg_one(
    plugin_package_manip_search_t search, const char * parse_arg_name, cfg_t arg_cfg, char * value)
{
    char * sep;

    sep = strchr(value, '=');
    if (sep == NULL) {
        CPE_ERROR(
            search->m_manip->m_em, "plugin_package_manip_collect_search: parse-args arg %s: value format error!", parse_arg_name);
        return -1;
    }

    * cpe_str_trim_tail(sep, value) = 0;

    if (cfg_struct_add_string(arg_cfg, value, cpe_str_trim_head(sep + 1), cfg_replace) == NULL) {
        CPE_ERROR(
            search->m_manip->m_em, "plugin_package_manip_collect_search: parse-args arg %s: add to struct fail!", parse_arg_name);
        return -1;
    }

    return 0;
}

static int plugin_package_manip_collect_search_src_parse_args(plugin_package_manip_search_t search, cfg_t obj_cfg, cfg_t rule_cfg) {
    struct cfg_it parse_arg_it;
    cfg_t parse_arg;
    int rv = 0;
    
    cfg_it_init(&parse_arg_it, cfg_find_cfg(rule_cfg, "parse-args"));
    while((parse_arg = cfg_it_next(&parse_arg_it))) {
        const char * parse_arg_name = cfg_as_string(parse_arg, NULL);
        cfg_t parse_arg_cfg;
        void * value_buf_origin;
        char * value_buf;
        char * sep;
        
        if (parse_arg_name == NULL) {
            CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_collect_search: parse-args config format error!");
            rv = -1;
            continue;
        }

        parse_arg_cfg = cfg_find_cfg(obj_cfg, parse_arg_name);
        if (parse_arg_cfg == NULL) continue;

        if (cfg_as_string(parse_arg_cfg, NULL) == NULL) {
            CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_collect_search: parse-args arg %s format error!", parse_arg_name);
            rv = -1;
            continue;
        }
        
        value_buf_origin = cpe_str_mem_dup(search->m_manip->m_alloc, cfg_as_string(parse_arg_cfg, NULL));
        if (value_buf_origin == NULL) {
            CPE_ERROR(
                search->m_manip->m_em, "plugin_package_manip_collect_search: parse-args arg %s dup value %s fail!",
                parse_arg_name, cfg_as_string(parse_arg_cfg, NULL));
            rv = -1;
            continue;
        }

        /*重新创建节点 */
        parse_arg_cfg = cfg_struct_add_struct(
            cfg_parent(parse_arg_cfg),
            strrchr(parse_arg_name, '.') ? (strrchr(parse_arg_name, '.') + 1) : parse_arg_name,
            cfg_replace);
        if (parse_arg_cfg == NULL) {
            CPE_ERROR(
                search->m_manip->m_em, "plugin_package_manip_collect_search: parse-args arg %s create struct new node fail!",
                parse_arg_name);
            mem_free(search->m_manip->m_alloc, value_buf_origin);
            rv = -1;
            continue;
        }

        value_buf = cpe_str_trim_head(value_buf_origin);

        /*读取类型分割 */
        sep = strchr(value_buf, ':');
        if (sep) {
            * cpe_str_trim_tail(sep, value_buf) = 0;

            parse_arg_cfg = cfg_struct_add_struct(parse_arg_cfg, value_buf, cfg_replace);
            if (parse_arg_cfg == NULL) {
                CPE_ERROR(
                    search->m_manip->m_em, "plugin_package_manip_collect_search: parse-args arg %s create type struct fail!",
                    parse_arg_name);
                mem_free(search->m_manip->m_alloc, value_buf);
                rv = -1;
                continue;
            }

            value_buf = cpe_str_trim_head(sep + 1);
        }
        
        while((sep = strchr(value_buf, ','))) {
            * cpe_str_trim_tail(sep, value_buf) = 0;
            if (plugin_package_manip_collect_search_src_parse_arg_one(search, parse_arg_name, parse_arg_cfg, value_buf) != 0) rv = -1;
            value_buf = cpe_str_trim_head(sep + 1);
        }
        * cpe_str_trim_tail(value_buf + strlen(value_buf), value_buf) = 0;

        if (value_buf[0]) {
            if (plugin_package_manip_collect_search_src_parse_arg_one(search, parse_arg_name, parse_arg_cfg, value_buf) != 0) rv = -1;
        }

        mem_free(search->m_manip->m_alloc, value_buf_origin);
    }

    return rv;
}

int plugin_package_manip_collect_search_src(
    plugin_package_manip_search_t search, const char * str_obj_type, char * path, cfg_t rule_cfg, cfg_calc_context_t args)
{
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    int rv = 0;
    ui_ed_obj_type_t obj_type;

    obj_type = ui_ed_obj_type_from_name(str_obj_type);
    if (obj_type == 0) {
        CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_collect_search: obj type %s unknown", str_obj_type);
        return -1;
    }
    
    ui_data_src_group_srcs(&src_it, plugin_package_package_srcs(search->m_package));
    while((src = ui_data_src_it_next(&src_it))) {
        ui_ed_search_t ed_search;
        ui_ed_obj_t ed_obj;

        if (!plugin_package_manip_collect_search_src_need_process(src, path)) continue;

        ed_search = ui_ed_search_create(search->m_manip->m_ed_mgr);
        if (ed_search == NULL) {
            CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_collect_search: create ed search fail");
            rv = -1;
            continue;
        }

        if (ui_ed_search_add_obj_type(ed_search, obj_type) != 0) {
            CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_collect_search: ed search add obj type fail");
            rv = -1;
            continue;
        }
        
        /* printf("xxxxx: package %s: src %s\n", */
        /*        plugin_package_package_name(search->m_package), */
        /*        ui_data_src_path_dump(gd_app_tmp_buffer(search->m_manip->m_app), src)); */
        if (ui_ed_search_add_root(ed_search, src) != 0) {
            CPE_ERROR(
                search->m_manip->m_em, "plugin_package_manip_collect_search: add obj root %s fail",
                ui_data_src_path_dump(gd_app_tmp_buffer(search->m_manip->m_app), src));
            rv = -1;
            continue;
        }

        while((ed_obj = ui_ed_obj_search_next(ed_search))) {
            cfg_t obj_cfg = NULL;
            struct cfg_calc_context commit_arg;
            
            obj_cfg = cfg_create(search->m_manip->m_alloc);
            if (obj_cfg == NULL) {
                CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_collect_search: create obj cfg fail!");
                rv = -1;
                break;
            }
            
            if (dr_cfg_write(obj_cfg, ui_ed_obj_data(ed_obj), ui_ed_obj_data_meta(ed_obj), search->m_manip->m_em) != 0) {
                CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_collect_search: write obj data to cfg fail!");
                rv = -1;
                cfg_free(obj_cfg);
                break;
            }

            if (plugin_package_manip_collect_search_src_parse_args(search, obj_cfg, rule_cfg) != 0) {
                rv = -1;
                cfg_free(obj_cfg);
                continue;
            }
            
            //printf("xxxxx: obj %s\n", cfg_dump_inline(obj_cfg, gd_app_tmp_buffer(search->m_manip->m_app)));
            
            commit_arg.m_cfg = obj_cfg;
            commit_arg.m_next = args;
            if (plugin_package_manip_search_commit(search, &commit_arg) != 0) rv = -1;
            
            cfg_free(obj_cfg);
        }

        ui_ed_search_free(ed_search);
    }
    
    return rv;
}
