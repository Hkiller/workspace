#ifndef PLUGIN_PACKAGE_MANIP_SEARCH_I_H
#define PLUGIN_PACKAGE_MANIP_SEARCH_I_H
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin_package_manip_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_manip_search {
    plugin_package_manip_t m_manip;
    plugin_package_manip_search_map_list_t m_maps;
    char * m_def;
    plugin_package_package_t m_package;
    cfg_t m_res_cfg;
    cfg_t m_package_cfg;
    plugin_package_manip_collect_op_fun_t m_op;
    void * m_ctx;

    /*runtime*/
    plugin_package_manip_search_map_t m_cur_map;
};

plugin_package_manip_search_t
plugin_package_manip_search_create(plugin_package_manip_t manip);
void plugin_package_manip_search_free(plugin_package_manip_search_t search);

int plugin_package_manip_search_commit(
    plugin_package_manip_search_t search, cfg_calc_context_t args);
    
/*op*/    
int plugin_package_manip_collect_search_src(
    plugin_package_manip_search_t search, const char * str_obj_type, char * path, cfg_t rule_cfg, cfg_calc_context_t args);

int plugin_package_manip_collect_search_process_entry(
    plugin_package_manip_search_t search, cfg_t cfg, cfg_calc_context_t args, char * root, char * p);
    
#ifdef __cplusplus
}
#endif

#endif 
