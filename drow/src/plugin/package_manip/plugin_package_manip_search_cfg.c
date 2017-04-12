#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin_package_manip_search_i.h"

static uint8_t plugin_package_manip_collect_search_process_check_condition(
    plugin_package_manip_search_t search, cfg_t cfg, cfg_calc_context_t args, const char * condition)
{
    struct cfg_calc_context check_arg = { cfg, args };
    uint8_t result;

    if (cfg_try_calc_bool(search->m_manip->m_computer, &result, condition, &check_arg, search->m_manip->m_em) != 0) {
        CPE_ERROR(
            search->m_manip->m_em,
            "plugin_package_manip_collect_search_process_check_condition: calc condition %s fail!",
            condition);
        return -1;
    }

    return result;
}
        
static int plugin_package_manip_collect_search_process_child(
    plugin_package_manip_search_t search, cfg_t cfg, cfg_calc_context_t args, char * root, char * child_name, char * p)
{
    char * condition_begin;
    char * condition_end;
    size_t name_len;
    struct cfg_it child_it;
    cfg_t  child_cfg;
    int rv = 0;
    uint8_t is_match_prefix;
    
    if (child_name[0] == 0) return plugin_package_manip_collect_search_process_entry(search, cfg, args, root, p);
    
    condition_begin = strchr(child_name, '[');
    if (condition_begin) {
        condition_end = (char*)cpe_str_char_not_in_pair(condition_begin + 1, ']', "{[(", ")]}");
        if (condition_end == NULL) {
            CPE_ERROR(search->m_manip->m_em, "plugin_package_manip_collect_search_process_child: child_name %s format error!", child_name);
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
                    && !plugin_package_manip_collect_search_process_check_condition(search, seq_cfg, args, condition_begin + 1)) continue;
                if (plugin_package_manip_collect_search_process_entry(search, seq_cfg, args, root, p) != 0) rv = -1;
            }
        }
        else {
            if (condition_begin
                && !plugin_package_manip_collect_search_process_check_condition(search, child_cfg, args, condition_begin + 1)) continue;
            if (plugin_package_manip_collect_search_process_entry(search, child_cfg, args, root, p) != 0) rv = -1;
        }
    }
        
    if (is_match_prefix) child_name[name_len - 1] = '*';

    if (condition_begin) *condition_begin = '[';
    if (condition_end) *condition_end = ']';
    
    return rv;
}

int plugin_package_manip_collect_search_process_entry(plugin_package_manip_search_t search, cfg_t cfg, cfg_calc_context_t args, char * root, char * p) {
    if (cfg_type(cfg) == CPE_CFG_TYPE_SEQUENCE) {
        struct cfg_it child_it;
        cfg_t  child_cfg;
        int rv = 0;
            
        cfg_it_init(&child_it, cfg);
        while((child_cfg = cfg_it_next(&child_it))) {
            if (plugin_package_manip_collect_search_process_entry(search, child_cfg, args, root, p) != 0) {
                rv = -1;
            }
        }

        return rv;
    }
    else {
        if (p[0] == 0) {
            if (args == NULL || cfg != args->m_cfg) {
                struct cfg_calc_context commit_args = { cfg, args };
                return plugin_package_manip_search_commit(search, &commit_args);
            }
            else {
                return plugin_package_manip_search_commit(search, args);
            }
        }
        else {
            char * sep;

            sep = strchr(p, '.');
            if (sep) {
                int rv;

                *sep = 0;
                rv = plugin_package_manip_collect_search_process_child(search, cfg, args, root, p, sep + 1);
                *sep = '.';

                return rv;
            }
            else {
                return plugin_package_manip_collect_search_process_child(search, cfg, args, root, p, p + strlen(p));
            }
        }
    }
}
