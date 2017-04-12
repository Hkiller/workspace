#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "plugin_package_manip_search_i.h"
#include "plugin_package_manip_search_map_i.h"

plugin_package_manip_search_t plugin_package_manip_search_create(plugin_package_manip_t manip) {
    plugin_package_manip_search_t search;

    search = mem_alloc(manip->m_alloc, sizeof(struct plugin_package_manip_search));
    if (search == NULL) {
        CPE_ERROR(manip->m_em, "plugin_package_manip_search_create: alloc fail!");
        return NULL;
    }

    bzero(search, sizeof(*search));
    
    search->m_manip = manip;
    TAILQ_INIT(&search->m_maps);
    
    return search;
}

void plugin_package_manip_search_free(plugin_package_manip_search_t search) {
    plugin_package_manip_t manip = search->m_manip;

    while(!TAILQ_EMPTY(&search->m_maps)) {
        plugin_package_manip_search_map_free(TAILQ_FIRST(&search->m_maps));
    }
    
    if (search->m_def) {
        mem_free(manip->m_alloc, search->m_def);
    }
    
    mem_free(manip->m_alloc, search);
}

int plugin_package_manip_collect_search(
    plugin_package_manip_t manip, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg, cfg_calc_context_t args,
    const char * path, plugin_package_manip_collect_op_fun_t op, void * ctx)
{
    plugin_package_manip_search_t search;
    char * sep;
    int rv = 0;
    
    search = plugin_package_manip_search_create(manip);
    if (search == NULL) return -1;

    search->m_package = package;
    search->m_res_cfg = res_cfg;
    search->m_package_cfg = package_cfg;
    search->m_op = op;
    search->m_ctx = ctx;
    
    path = cpe_str_trim_head((char*)path), 

    sep = (char*)strrchr(path, '|');
    if (sep) {
        search->m_def = cpe_str_mem_dup_range(manip->m_alloc, path, cpe_str_trim_tail((char*)sep, path));
    }
    else {
        search->m_def = cpe_str_mem_dup_range(manip->m_alloc, path, cpe_str_trim_tail((char*)path + strlen(path), path));
    }

    if (search->m_def == NULL) {
        CPE_ERROR(manip->m_em, "plugin_package_manip_search: alloc fail!");
        plugin_package_manip_search_free(search);
        return -1;
    }

    while(sep) {
        char * begin = cpe_str_trim_head(sep + 1);
        plugin_package_manip_search_map_t map;

        map = plugin_package_manip_search_map_create(search);
        if(map == NULL) {
            plugin_package_manip_search_free(search);
            return -1;
        }

        sep = strchr(begin, '|');

        if (sep) {
            map->m_def = cpe_str_mem_dup_range(manip->m_alloc, begin, cpe_str_trim_tail(sep, begin));
        }
        else {
            map->m_def = cpe_str_mem_dup_range(manip->m_alloc, begin, cpe_str_trim_tail(begin + strlen(begin), begin));
        }
    }

    if (search->m_def[0] == '*') {
        sep = strchr(search->m_def, '@');
        if (sep == NULL) {
            rv = plugin_package_manip_collect_search_src(search, search->m_def + 1, search->m_def + strlen(search->m_def), res_cfg, args);
        }
        else {
            *sep = 0;
            rv = plugin_package_manip_collect_search_src(search, search->m_def + 1, sep + 1, res_cfg, args);
        }
    }
    else {
        if (search->m_def[0] == '^') {
            if (plugin_package_manip_collect_search_process_entry(search, gd_app_cfg(manip->m_app), args, search->m_def + 1, search->m_def + 1) != 0) {
                rv = -1;
            }
        }
        else {
            if (plugin_package_manip_collect_search_process_entry(search, search->m_package_cfg, args, search->m_def, search->m_def) != 0) {
                rv = -1;
            }
        }
    }
    
    return rv;
}

int plugin_package_manip_search_commit(plugin_package_manip_search_t search, cfg_calc_context_t args) {
    int rv = 0;
    if (search->m_cur_map == NULL) {
        rv = search->m_op(search->m_ctx, search->m_package, search->m_res_cfg, search->m_package_cfg, args);
    }
    else {
        plugin_package_manip_search_map_t save_map = search->m_cur_map;
        search->m_cur_map = TAILQ_NEXT(search->m_cur_map, m_next);

        if (save_map->m_def[0] == '^') {
            rv = plugin_package_manip_collect_search_process_entry(
                search, gd_app_cfg(search->m_manip->m_app), args, save_map->m_def + 1, save_map->m_def + 1);
        }
        else {
            rv = plugin_package_manip_collect_search_process_entry(search, search->m_package_cfg, args, save_map->m_def, save_map->m_def);
        }
        
        search->m_cur_map = save_map;
    }
    
    return rv;
}
