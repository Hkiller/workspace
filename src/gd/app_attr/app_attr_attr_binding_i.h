#ifndef GD_APP_ATTR_ATTR_BINDING_I_H
#define GD_APP_ATTR_ATTR_BINDING_I_H
#include "app_attr_attr_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum app_attr_attr_binding_type {
    app_attr_attr_binding_synchronizer,
    app_attr_attr_binding_request,
} app_attr_attr_binding_type_t;
        
struct app_attr_attr_binding {
    app_attr_attr_binding_type_t m_type;
    app_attr_attr_t m_attr;
    TAILQ_ENTRY(app_attr_attr_binding) m_next_for_attr;
    void * m_product;
    TAILQ_ENTRY(app_attr_attr_binding) m_next_for_product;
};

app_attr_attr_binding_t app_attr_attr_binding_create(app_attr_attr_binding_type_t binding_type, app_attr_attr_t attr, void * product);
void app_attr_attr_binding_free(app_attr_attr_binding_t attr_binding);
void app_attr_attr_binding_real_free(app_attr_attr_binding_t attr_binding);
    
#ifdef __cplusplus
}
#endif

#endif
