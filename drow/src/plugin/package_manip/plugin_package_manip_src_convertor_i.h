#ifndef PLUGIN_PACKAGE_MANIP_SRC_CONVERTOR_I_H
#define PLUGIN_PACKAGE_MANIP_SRC_CONVERTOR_I_H
#include "plugin/package_manip/plugin_package_manip_src_convertor.h"
#include "plugin_package_manip_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_package_manip_src_convertor {
    plugin_package_manip_t m_manip;
    TAILQ_ENTRY(plugin_package_manip_src_convertor) m_next;
    char m_name[32];
    ui_data_src_type_t m_from_src_type;
    ui_data_src_type_t m_to_src_type;
    plugin_package_manip_src_convertor_fun_t m_fun;
    void * m_ctx;
};

#ifdef __cplusplus
}
#endif

#endif 
