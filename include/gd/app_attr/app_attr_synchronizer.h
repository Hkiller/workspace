#ifndef GD_APP_ATTR_SYNCHRONIZER_H
#define GD_APP_ATTR_SYNCHRONIZER_H
#include "app_attr_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct app_attr_synchronizer_it {
    app_attr_synchronizer_t (*next)(struct app_attr_synchronizer_it * it);
    char m_data[64];
};

typedef int (*app_attr_synchronizer_start_fun_t)(void * ctx, app_attr_synchronizer_t synchronizer);
    
app_attr_synchronizer_t
app_attr_synchronizer_create(
    app_attr_provider_t provider, const char * name,
    void * attrs, app_attr_synchronizer_start_fun_t sync_start_fun);

void app_attr_synchronizer_free(app_attr_synchronizer_t synchronizer);

int app_attr_synchronizer_set_done(app_attr_synchronizer_t synchronizer, uint8_t success);
    
void app_attr_provider_synchronizers(app_attr_provider_t provider, app_attr_synchronizer_it_t synchronizer_it);

#define app_attr_synchronizer_it_next(it) ((it)->next ? (it)->next(it) : NULL)
    
#ifdef __cplusplus
}
#endif

#endif
