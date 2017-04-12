#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app_attr/app_attr_provider.h"
#include "appsvr_haoxin_module_i.h"

int appsvr_haoxin_attr_provider_init(appsvr_haoxin_module_t module) {
    if (module->m_app_attr_module == NULL) return 0;
    
    module->m_attr_provider =
        app_attr_provider_create(
            module->m_app_attr_module,
            "haoxin",
            module,
            &module->m_attr_data, sizeof(module->m_attr_data), module->m_attr_meta);
    if (module->m_attr_provider == NULL) {
        CPE_ERROR(module->m_em, "appsvr_haoxin_attr_provider_init: create provider fail!");
        return -1;
    }

    app_attr_provider_set_attrs_wait_all(module->m_attr_provider);
    
    return 0;
}

void appsvr_haoxin_attr_provider_fini(appsvr_haoxin_module_t module) {
    if(module->m_attr_provider) {
        app_attr_provider_free(module->m_attr_provider);
        module->m_attr_provider = NULL;
    }
}

int appsvr_haoxin_notify_support_exit_game(appsvr_haoxin_module_t module, uint8_t is_support) {
    module->m_attr_data.support_exit_game = is_support;
    return app_attr_provider_set_attrs_changed(module->m_attr_provider, "support_exit_game");
}

int appsvr_haoxin_notify_support_more_game(appsvr_haoxin_module_t module, uint8_t is_support) {
    module->m_attr_data.support_more_game = is_support;
    return app_attr_provider_set_attrs_changed(module->m_attr_provider, "support_more_game");
}

int appsvr_haoxin_notify_payscreen(appsvr_haoxin_module_t module, const char * payscreen) {
    cpe_str_dup(module->m_attr_data.payscreen, sizeof(module->m_attr_data.payscreen), payscreen);
    return app_attr_provider_set_attrs_changed(module->m_attr_provider, "payscreen");
}
