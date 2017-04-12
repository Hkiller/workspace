#ifndef DROW_PLUGIN_PACKAGE_MANIP_SRC_CONVERTOR_H
#define DROW_PLUGIN_PACKAGE_MANIP_SRC_CONVERTOR_H
#include "plugin_package_manip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*plugin_package_manip_src_convertor_fun_t)(void * ctx, ui_data_src_t from_src, ui_data_src_t to_src, cfg_t args);

plugin_package_manip_src_convertor_t
plugin_package_manip_src_convertor_create(
    plugin_package_manip_t manip, const char * name,
    ui_data_src_type_t from_src,
    ui_data_src_type_t to_src,
    plugin_package_manip_src_convertor_fun_t fun, void * ctx);

void plugin_package_manip_src_convertor_free(plugin_package_manip_src_convertor_t convertor);

plugin_package_manip_src_convertor_t
plugin_package_manip_src_convertor_find(plugin_package_manip_t manip, const char * name);

ui_data_src_type_t plugin_package_manip_src_convertor_from_src_type(plugin_package_manip_src_convertor_t convertor);
ui_data_src_type_t plugin_package_manip_src_convertor_to_src_type(plugin_package_manip_src_convertor_t convertor);    

int plugin_package_manip_src_convertor_convert(
    plugin_package_manip_src_convertor_t convertor, ui_data_src_t from_src, ui_data_src_t to_src);

#ifdef __cplusplus
}
#endif

#endif
