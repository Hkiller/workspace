#ifndef UI_PLUGIN_PACKAGE_REGION_H
#define UI_PLUGIN_PACKAGE_REGION_H
#include "plugin_package_types.h"

#ifdef __cplusplus
extern "C" {
#endif

plugin_package_region_t plugin_package_region_create(plugin_package_module_t module, const char * name);
void plugin_package_region_free(plugin_package_region_t region);

plugin_package_region_t plugin_package_region_find(plugin_package_module_t module, const char * name);
    
plugin_package_module_t plugin_package_region_module(plugin_package_region_t region);
plugin_package_group_t plugin_package_region_group(plugin_package_region_t region);
const char * plugin_package_region_name(plugin_package_region_t region);    

/*it*/
struct plugin_package_region_it {
    plugin_package_region_t (*next)(plugin_package_region_it_t it);
    char m_data[64];
};

#define plugin_package_region_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif

