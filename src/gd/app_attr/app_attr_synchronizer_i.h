#ifndef APP_ATTR_SYNCHRONIZER_I_H
#define APP_ATTR_SYNCHRONIZER_I_H
#include "gd/app_attr/app_attr_synchronizer.h"
#include "app_attr_provider_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum app_attr_synchronizer_state {
    app_attr_synchronizer_idle,
    app_attr_synchronizer_trigger,
    app_attr_synchronizer_runing,
} app_attr_synchronizer_state_t;
    
struct app_attr_synchronizer {
    app_attr_provider_t m_provider;
    TAILQ_ENTRY(app_attr_synchronizer) m_next_for_provider;
    app_attr_synchronizer_state_t m_state;
    TAILQ_ENTRY(app_attr_synchronizer) m_next_for_state;
    char m_name[32];
    app_attr_synchronizer_start_fun_t m_sync_start_fun;
    app_attr_attr_binding_list_t m_attrs;
};

void app_attr_synchronizer_set_state(app_attr_synchronizer_t synchronizer, app_attr_synchronizer_state_t state);
void app_attr_synchronizer_tick(app_attr_module_t module);
const char * app_attr_synchronizer_state_str(app_attr_synchronizer_state_t state);
    
#ifdef __cplusplus
}
#endif

#endif
