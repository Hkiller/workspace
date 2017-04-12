#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "render/model/ui_data_src_group.h"
#include "render/model/ui_data_src.h"
#include "render/cache/ui_cache_group.h"
#include "render/cache/ui_cache_res.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_region.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin_package_manip_i.h"
#include "plugin_package_manip_res_collector_i.h"

static int plugin_package_manip_build_process_child(
    plugin_package_manip_t manip, const char * type_name, cfg_t rule_cfg,
    cfg_t cfg, char * root, char * child_name, char * p);

static int plugin_package_manip_build_process_entry(
    plugin_package_manip_t manip, const char * type_name, cfg_t rule_cfg,
    cfg_t cfg, char * root, char * p);

static int plugin_package_manip_build_base_packages(
    plugin_package_manip_t manip, const char * type_name,
    cfg_t rule_cfg, cfg_t package_cfg, const char * pacakge_name,
    plugin_package_group_t * base_package_group)
{
    struct cfg_it base_package_it;
    cfg_t base_package_cfg;
    int rv = 0;
    struct mem_buffer calc_buffer;
    const char * v;
    
    *base_package_group = 0;

    mem_buffer_init(&calc_buffer, manip->m_alloc);
    
    /*构造基础包列表 */
    cfg_it_init(&base_package_it, cfg_find_cfg(rule_cfg, "base-packages"));
    while((base_package_cfg = cfg_it_next(&base_package_it))) {
        const char * base_package_name = cfg_as_string(base_package_cfg, NULL);
        char base_package_name_buf[64];
        plugin_package_package_t base_package;
        struct cfg_calc_context arg = { package_cfg, NULL };

        if (base_package_name == NULL) {
            CPE_ERROR(manip->m_em, "package: %s: base package configure format error", type_name);
            rv = -1;
            continue;
        }

        v = plugin_package_manip_calc(manip, &calc_buffer, base_package_name, &arg);
        if (v == NULL) {
            CPE_ERROR(manip->m_em, "package: %s: base package: calc from %s fail", type_name,  base_package_name);
            rv = -1;
            continue;
        }
        base_package_name = v;

        if (strrchr(base_package_name, '/') == NULL) {
            snprintf(base_package_name_buf, sizeof(base_package_name_buf), "%s/%s", type_name, base_package_name);
            base_package_name = base_package_name_buf;
        }
        
        base_package = plugin_package_package_find(manip->m_package_module, base_package_name);
        if (base_package == NULL) {
            base_package = plugin_package_package_create(manip->m_package_module, base_package_name, plugin_package_package_loaded);
            if (base_package == NULL) {
                CPE_ERROR(manip->m_em, "package: %s: base package %s create fail", type_name, base_package_name);
                rv = -1;
                continue;
            }
        }

        if (*base_package_group == NULL) {
            *base_package_group = plugin_package_group_create(manip->m_package_module, "basepackages");
            if (*base_package_group == NULL) {
                CPE_ERROR(manip->m_em, "package: %s: create base package group fail", type_name);
                rv = -1;
                break;
            }
        }
        
        if (plugin_package_group_add_package(*base_package_group, base_package) != 0) rv = -1;
    }

    mem_buffer_clear(&calc_buffer);
    
    return rv;
}

static int plugin_package_manip_build_region(
    plugin_package_manip_t manip, const char * type_name, cfg_t rule_cfg, cfg_t package_cfg, plugin_package_package_t package)
{
    struct cfg_it region_it;
    cfg_t region_cfg;
    struct mem_buffer calc_buffer;
    int rv = 0;
    const char * v;

    mem_buffer_init(&calc_buffer, manip->m_alloc);

    cfg_it_init(&region_it, cfg_find_cfg(rule_cfg, "regions"));
    while((region_cfg = cfg_it_next(&region_it))) {
        const char * region_name = cfg_as_string(region_cfg, NULL);
        struct cfg_calc_context arg = { package_cfg, NULL };
        plugin_package_region_t region;

        if (region_name == NULL) {
            CPE_ERROR(manip->m_em, "package: %s: region configure format error", type_name);
            rv = -1;
            continue;
        }
        
        v = plugin_package_manip_calc(manip, &calc_buffer, region_name, &arg);
        if (v == NULL) {
            CPE_ERROR(manip->m_em, "package: %s: region: calc from %s fail", type_name,  region_name);
            rv = -1;
            continue;
        }
        region_name = v;

        region = plugin_package_region_find(manip->m_package_module, region_name);
        if (region == NULL) {
            region = plugin_package_region_create(manip->m_package_module, region_name);
            if (region == NULL) {
                CPE_ERROR(manip->m_em, "package: %s: region: create region %s fail", type_name,  region_name);
                rv = -1;
                continue;
            }
        }

        plugin_package_group_add_package(plugin_package_region_group(region), package);
    }

    mem_buffer_clear(&calc_buffer);
    
    return rv;
}

static int plugin_package_manip_build_package(
    plugin_package_manip_t manip, const char * type_name, const char * name, cfg_t rule_cfg, cfg_t package_cfg)
{
    plugin_package_package_t package;
    struct cfg_it resource_cfg_it;
    cfg_t resource_cfg;
    int rv = 0;
    plugin_package_group_t base_package_group = NULL;

    package = plugin_package_package_find(manip->m_package_module, name);
    if (package == NULL) {
        package = plugin_package_package_create(manip->m_package_module, name, plugin_package_package_loaded);
        if (package == NULL) return -1;
    }

    cfg_it_init(&resource_cfg_it, cfg_find_cfg(rule_cfg, "resources"));
    while((resource_cfg = cfg_it_next(&resource_cfg_it))) {
        plugin_package_manip_res_collector_t collector;
        const char * collector_type;

        collector_type = cfg_get_string(resource_cfg, "type", NULL);
        if (collector_type == NULL) {
            CPE_ERROR(manip->m_em, "package: %s: res collector type not configured", name);
            rv = -1;
            continue;
        }
        
        collector = plugin_package_manip_res_collector_find(manip, collector_type);
        if(collector == NULL) {
            CPE_ERROR(manip->m_em, "package: %s: res collector type %s not support", name, collector_type);
            rv = -1;
            continue;
        }

        if (collector->m_fun(collector->m_ctx, package, resource_cfg, package_cfg) != 0) {
            rv = -1;
            continue;
        }
    }

    if (plugin_package_manip_build_region(manip, type_name, rule_cfg, package_cfg, package) != 0) rv = -1;
    
    if (plugin_package_manip_build_base_packages(
            manip, type_name, rule_cfg, package_cfg, name, &base_package_group) != 0) rv = -1;
    if (base_package_group) {
        struct plugin_package_package_it base_package_it;
        plugin_package_package_t base_package;
        
        plugin_package_group_packages(&base_package_it, base_package_group);
        while((base_package = plugin_package_package_it_next(&base_package_it))) {
            if (plugin_package_package_add_base_package(package, base_package) != 0) rv = -1;
        }

        plugin_package_group_free(base_package_group);
    }

    if (plugin_package_manip_remove_base_provided(package) != 0) rv = -1;

    return rv;
}

static uint8_t plugin_package_manip_check_prefix(const char * name, const char * prefix) {
    if (prefix == NULL) return 1;

    if (prefix[0] == '^') {
        return cpe_str_start_with(name, prefix + 1) ? 0 : 1;
    }
    else {
        return cpe_str_start_with(name, prefix) ? 1 : 0;
    }
}

static int plugin_package_manip_build_found_one(plugin_package_manip_t manip, const char * type_name, cfg_t rule_cfg, cfg_t package_cfg) {
    const char * generator_prefix = cfg_get_string(rule_cfg, "prefix", NULL);
    const char * generator_entry = cfg_get_string(rule_cfg, "entry", NULL);
    const char * name = cfg_get_string(rule_cfg, "name", NULL);
    const char * tag = cfg_get_string(rule_cfg, "tag", NULL);
    char package_name[64];
    char buf[32];
    int rv = 0;

    /*根据tag过滤掉不需要的包 */
    if (tag && cfg_find_cfg(package_cfg, tag) == NULL) return 0;

    if (name == NULL) {
        if (generator_entry) {
            cfg_t entry_cfg = cfg_find_cfg(package_cfg, generator_entry);
            if (entry_cfg == NULL) {
                CPE_ERROR(manip->m_em, "package: %s: entry %s not exist", type_name, generator_entry);
                return -1;
            }
        
            name = cfg_as_string_cvt(entry_cfg, NULL, buf, sizeof(buf));
            if (name == NULL) {
                CPE_ERROR(manip->m_em, "package: %s: entry %s get value string fail", type_name, generator_entry);
                return -1;
            }
        }
        else {
            if (cfg_is_value(package_cfg)) {
                name = cfg_as_string_cvt(package_cfg, NULL, buf, sizeof(buf));
                if (name == NULL) {
                    CPE_ERROR(manip->m_em, "package: %s: entry %s get value string fail", type_name, generator_entry);
                    return -1;
                }
            }
            else {
                name = cfg_name(package_cfg);
            }
        }
    }
    else {
        struct cfg_calc_context calc_arg = { package_cfg, NULL };
        const char * new_name;

        new_name = plugin_package_manip_calc(manip, gd_app_tmp_buffer(manip->m_app), name, &calc_arg);
        if (new_name == NULL) {
            CPE_ERROR(manip->m_em, "plugin_package_manip_build: calc name from %s fail!", name);
            return -1;
        }

        name = new_name;
    }
    
    assert(name);
    if (name[0] == 0) return 0;
    if (!plugin_package_manip_check_prefix(name, generator_prefix)) return 0;
                
    snprintf(package_name, sizeof(package_name), "%s/%s", type_name, name);
    if (plugin_package_manip_build_package(manip, type_name, package_name, rule_cfg, package_cfg) != 0) rv = -1;

    return rv;
}

static uint8_t plugin_package_manip_build_process_check_condition(plugin_package_manip_t manip, cfg_t cfg, const char * condition) {
    struct cfg_calc_context check_arg = { cfg, NULL };
    uint8_t result;

    if (cfg_try_calc_bool(manip->m_computer, &result, condition, &check_arg, manip->m_em) != 0) {
        CPE_ERROR(
            manip->m_em,
            "plugin_package_manip_build: calc condition %s fail!",
            condition);
        return -1;
    }

    return result;
}
        

static int plugin_package_manip_build_process_child(
    plugin_package_manip_t manip, const char * type_name, cfg_t rule_cfg,
    cfg_t cfg, char * root, char * child_name, char * p)
{
    char * condition_begin;
    char * condition_end;
    size_t name_len;
    struct cfg_it child_it;
    cfg_t  child_cfg;
    int rv = 0;
    uint8_t is_match_prefix;
    
    if (child_name[0] == 0) return plugin_package_manip_build_process_entry(manip, type_name, rule_cfg, cfg, root, p);
    
    condition_begin = strchr(child_name, '[');
    if (condition_begin) {
        condition_end = (char*)cpe_str_char_not_in_pair(condition_begin + 1, ']', "{[(", ")]}");
        if (condition_end == NULL) {
            CPE_ERROR(manip->m_em, "plugin_package_manip_build_process_child: child_name %s format error!", child_name);
            return -1;
        }
    }
    else {
        condition_end = NULL;
    }
    
    if (condition_begin) *condition_begin = 0;
    if (condition_end) *condition_end = 0;

    name_len = strlen(child_name);

    is_match_prefix = child_name[name_len - 1] == '*';
    if (is_match_prefix) child_name[name_len - 1] = 0;
    
    cfg_it_init(&child_it, cfg);
    while((child_cfg = cfg_it_next(&child_it))) {
        const char * name = cfg_name(child_cfg);
        if (! (
                is_match_prefix
                ? cpe_str_start_with(name, child_name)
                : (strcmp(name, child_name) == 0 ? 1 : 0) ) ) continue;

        if (cfg_type(child_cfg) == CPE_CFG_TYPE_SEQUENCE) {
            struct cfg_it seq_it;
            cfg_t  seq_cfg;

            cfg_it_init(&seq_it, child_cfg);
            while((seq_cfg = cfg_it_next(&seq_it))) {
                if (condition_begin
                    && !plugin_package_manip_build_process_check_condition(manip, seq_cfg, condition_begin + 1)) continue;
                if (plugin_package_manip_build_process_entry(manip, type_name, rule_cfg, seq_cfg, root, p) != 0) rv = -1;
            }
        }
        else {
            if (condition_begin
                && !plugin_package_manip_build_process_check_condition(manip, child_cfg, condition_begin + 1)) continue;
            if (plugin_package_manip_build_process_entry(manip, type_name, rule_cfg, child_cfg, root, p) != 0) rv = -1;
        }
    }
        
    if (is_match_prefix) child_name[name_len - 1] = '*';

    if (condition_begin) *condition_begin = '[';
    if (condition_end) *condition_end = ']';
    
    return rv;
}

int plugin_package_manip_build_process_entry(
    plugin_package_manip_t manip, const char * type_name, cfg_t rule_cfg,
    cfg_t cfg, char * root, char * p)
{
    if (cfg_type(cfg) == CPE_CFG_TYPE_SEQUENCE) {
        struct cfg_it child_it;
        cfg_t  child_cfg;
        int rv = 0;
            
        cfg_it_init(&child_it, cfg);
        while((child_cfg = cfg_it_next(&child_it))) {
            if (plugin_package_manip_build_process_entry(manip, type_name, rule_cfg, child_cfg, root, p) != 0) {
                rv = -1;
            }
        }

        return rv;
    }
    else {
        if (p[0] == 0) {
            return plugin_package_manip_build_found_one(manip, type_name, rule_cfg, cfg);
        }
        else {
            char * sep;

            sep = strchr(p, '.');
            if (sep) {
                int rv;

                *sep = 0;
                rv = plugin_package_manip_build_process_child(manip, type_name, rule_cfg, cfg, root, p, sep + 1);
                *sep = '.';

                return rv;
            }
            else {
                return plugin_package_manip_build_process_child(manip, type_name, rule_cfg, cfg, root, p, p + strlen(p));
            }
        }
    }
}

static int plugin_package_manip_build_one_type(plugin_package_manip_t manip, cfg_t rule_cfg) {
    int rv = 0;
    struct cfg_it package_rule_cfg_it;
    cfg_t package_rule_cfg;
    const char * type_name = cfg_name(rule_cfg);

    cfg_it_init(&package_rule_cfg_it, rule_cfg);
    while((package_rule_cfg = cfg_it_next(&package_rule_cfg_it))) {
        const char * generator_path = cfg_get_string(package_rule_cfg, "path", NULL);
        const char * name = cfg_get_string(package_rule_cfg, "name", NULL);
        
        if (generator_path) {
            char * path_buf = cpe_str_mem_dup(manip->m_alloc, generator_path);
            if (plugin_package_manip_build_process_entry(manip, type_name, package_rule_cfg, gd_app_cfg(manip->m_app), path_buf, path_buf) != 0) rv = -1;
            mem_free(manip->m_alloc, path_buf);
        }
        else if (name) {
            char package_name[64];
            snprintf(package_name, sizeof(package_name), "%s/%s", type_name, name);
            if (plugin_package_manip_build_package(
                    manip, type_name, package_name,
                    package_rule_cfg, gd_app_cfg(manip->m_app)) != 0) rv = -1;
        }
        else {
            CPE_ERROR(manip->m_em, "package: no path or name configured");
            rv = -1;
        }
    }

    return rv;
}


int plugin_package_manip_build(plugin_package_manip_t manip, cfg_t cfg) {
    struct cfg_it rule_cfg_it;
    cfg_t rule_cfg;
    int rv = 0;

    cfg_it_init(&rule_cfg_it, cfg);
    while((rule_cfg = cfg_it_next(&rule_cfg_it))) {
        rule_cfg = cfg_child_only(rule_cfg);
        if (rule_cfg == NULL) {
            CPE_ERROR(manip->m_em, "package: package list format error!");
            rv = -1;
            continue;
        }
            
        if (plugin_package_manip_build_one_type(manip, rule_cfg) != 0) rv = -1;
    }
    
    return rv;
}
