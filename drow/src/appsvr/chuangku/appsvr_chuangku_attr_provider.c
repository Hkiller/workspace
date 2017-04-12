#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app_attr/app_attr_provider.h"
#include "appsvr_chuangku_module_i.h"
#include "gd/app_attr/app_attr_synchronizer.h"
static int appsvr_chuangku_on_attr_sync(void * ctx, app_attr_synchronizer_t synchronizer);

int appsvr_chuangku_attr_provider_init(appsvr_chuangku_module_t module) {
    if (module->m_app_attr_module == NULL) return 0;
    
    module->m_attr_provider =
        app_attr_provider_create(
            module->m_app_attr_module,
            "chuangku",
            module,
            &module->m_attr_data, sizeof(module->m_attr_data), module->m_attr_meta);
    if (module->m_attr_provider == NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_attr_provider_init: create provider fail!");
        return -1;
    }

    if (app_attr_synchronizer_create(
        module->m_attr_provider, "attr",
        NULL, appsvr_chuangku_on_attr_sync) == NULL)
    {
        CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: create synchronizer fail!");
        app_attr_provider_free(module->m_attr_provider);
        module->m_attr_provider = NULL;
        return -1;
    }

    app_attr_provider_set_attrs_wait_all(module->m_attr_provider);
    
    return 0;
}

void appsvr_chuangku_attr_provider_fini(appsvr_chuangku_module_t module) {
    if(module->m_attr_provider) {
        app_attr_provider_free(module->m_attr_provider);
        module->m_attr_provider = NULL;
    }
}

int appsvr_chuangku_notify_support_exit_game(appsvr_chuangku_module_t module, uint8_t is_support) {
    module->m_attr_data.support_exit_game = is_support;
    return app_attr_provider_set_attrs_changed(module->m_attr_provider, "support_exit_game");
}

int appsvr_chuangku_notify_support_more_game(appsvr_chuangku_module_t module, uint8_t is_support) {
    module->m_attr_data.support_more_game = is_support;
    return app_attr_provider_set_attrs_changed(module->m_attr_provider, "support_more_game");
}

static int appsvr_chuangku_on_attr_sync(void * ctx, app_attr_synchronizer_t synchronizer) {
    appsvr_chuangku_module_t module = ctx;

    if (appsvr_chuangku_sync_attr(module) != 0) {
        app_attr_synchronizer_set_done(synchronizer, 0);
    }
    else {
        app_attr_synchronizer_set_done(synchronizer, 1);
    }

    return 0;
}