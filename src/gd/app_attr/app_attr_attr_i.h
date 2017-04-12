#ifndef GD_APP_ATTR_ATTR_I_H
#define GD_APP_ATTR_ATTR_I_H
#include "gd/app_attr/app_attr_attr.h"
#include "app_attr_provider_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum app_attr_attr_state {
    app_attr_attr_readable,
    app_attr_attr_waiting,
    app_attr_attr_error,
} app_attr_attr_state_t;

struct app_attr_attr {
    app_attr_provider_t m_provider;
    TAILQ_ENTRY(app_attr_attr) m_next_for_provider;
    struct cpe_hash_entry m_hh;

    const char * m_name;
    app_attr_attr_state_t m_state;
    LPDRMETAENTRY m_entry;
    uint32_t m_start_pos;
    app_attr_attr_binding_list_t m_synchronizers;
    app_attr_attr_binding_list_t m_requests;
};

app_attr_attr_t app_attr_attr_create(app_attr_provider_t provider, const char * name, LPDRMETAENTRY entry, uint32_t start_pos);
void app_attr_attr_free(app_attr_attr_t attr);

void app_attr_attr_set_readable(app_attr_attr_t attr);
    
uint32_t app_attr_attr_hash(app_attr_attr_t attr);
int app_attr_attr_eq(app_attr_attr_t l, app_attr_attr_t r);
    
#ifdef __cplusplus
}
#endif

#endif
