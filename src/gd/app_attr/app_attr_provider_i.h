#ifndef APP_ATTR_PROVIDER_I_H
#define APP_ATTR_PROVIDER_I_H
#include "gd/app_attr/app_attr_provider.h"
#include "app_attr_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_attr_provider {
    app_attr_module_t m_module;
    TAILQ_ENTRY(app_attr_provider) m_next;
    app_attr_attr_list_t m_attrs;
    app_attr_synchronizer_list_t m_synchronizers;
    char m_name[32];
    void * m_ctx;
    void * m_data;
    size_t m_data_size;
    LPDRMETA m_data_meta;
};
    
#ifdef __cplusplus
}
#endif

#endif
