#ifndef GD_APP_ATTR_ATTR_H
#define GD_APP_ATTR_ATTR_H
#include "app_attr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_attr_attr_it {
    app_attr_attr_t (*next)(struct app_attr_attr_it * it);
    char m_data[64];
};

app_attr_attr_t app_attr_attr_find(app_attr_module_t module, const char * name);
app_attr_attr_t app_attr_attr_find_in_provider(app_attr_provider_t provider, const char * name);
    
const char * app_attr_attr_name(app_attr_attr_t attr);
void * app_attr_attr_data(app_attr_attr_t attr);

void app_attr_provider_attrs(app_attr_provider_t provider, app_attr_attr_it_t attr_it);
void app_attr_module_attrs(app_attr_module_t module, app_attr_attr_it_t attr_it);    

#define app_attr_attr_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
