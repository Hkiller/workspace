#ifndef GD_APP_ATTR_PROVIDER_H
#define GD_APP_ATTR_PROVIDER_H
#include "app_attr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_attr_provider_it {
    app_attr_provider_t (*next)(struct app_attr_provider_it * it);
    char m_data[64];
};

app_attr_provider_t
app_attr_provider_create(
    app_attr_module_t module, const char * name,
    void * ctx,
    void * data, size_t data_size, LPDRMETA data_meta);

void app_attr_provider_free(app_attr_provider_t provider);

int app_attr_provider_set_attrs_wait(app_attr_provider_t provider, const char * attrs);
void app_attr_provider_set_attrs_wait_all(app_attr_provider_t provider);    
int app_attr_provider_set_attrs_changed(app_attr_provider_t provider, const char * attrs);

void app_attr_module_providers(app_attr_module_t module, app_attr_provider_it_t provider_it);

#define app_attr_provider_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
