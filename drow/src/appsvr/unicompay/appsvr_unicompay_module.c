#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_module.h"
#include "gd/app/app_log.h"
#include "appsvr/payment/appsvr_payment_module.h"
#include "appsvr_unicompay_module_i.h"

static void appsvr_unicompay_module_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_appsvr_unicompay_module = {
    "appsvr_unicompay_module",
    appsvr_unicompay_module_clear
};

static struct {
    const char * name; 
    int (*init)(appsvr_unicompay_module_t module);
    void (*fini)(appsvr_unicompay_module_t module);
} s_auto_reg_products[] = {
    { "backend", appsvr_unicompay_backend_init, appsvr_unicompay_backend_fini }
    , { "payment-adapter", appsvr_unicompay_payment_adapter_init, appsvr_unicompay_payment_adapter_fini }
};

appsvr_unicompay_module_t
appsvr_unicompay_module_create(
    gd_app_context_t app, uint8_t debug,
    appsvr_payment_module_t payment_module,
    mem_allocrator_t alloc, const char * name, error_monitor_t em)
{
    appsvr_unicompay_module_t module;
    nm_node_t module_node;
    uint16_t component_pos;
    
    if (name == NULL) name = "appsvr_unicompay_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct appsvr_unicompay_module));
    if (module_node == NULL) return NULL;

    module = (appsvr_unicompay_module_t)nm_node_data(module_node);

    module->m_app = app; 
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;
    module->m_payment_module = payment_module;
    module->m_url = NULL;
    module->m_app_id = NULL;
    module->m_chanel = NULL;
    module->m_appv_key = NULL;
    module->m_platp_key = NULL;
    module->m_free_pay_product_id = NULL;
    module->m_backend = NULL;
    module->m_adapter = NULL;

    mem_buffer_init(&module->m_dump_buffer, alloc);

    for(component_pos = 0; component_pos < CPE_ARRAY_SIZE(s_auto_reg_products); ++component_pos) {
        if (s_auto_reg_products[component_pos].init(module) != 0) {
            CPE_ERROR(module->m_em, "appsvr_unicompay_module_create: regist product %s fail!", s_auto_reg_products[component_pos].name);
            for(; component_pos > 0; component_pos--) {
                s_auto_reg_products[component_pos - 1].fini(module);
            }

            mem_buffer_clear(&module->m_dump_buffer);
            nm_node_free(module_node);
            return NULL;
        }
    }

    nm_node_set_type(module_node, &s_nm_node_type_appsvr_unicompay_module);

    return module;
}

static void appsvr_unicompay_module_clear(nm_node_t node) {
    appsvr_unicompay_module_t module = nm_node_data(node);
    uint16_t component_pos;

    for(component_pos = CPE_ARRAY_SIZE(s_auto_reg_products); component_pos > 0; component_pos--) {
        s_auto_reg_products[component_pos - 1].fini(module);
    }
    assert(module->m_backend == NULL);
    assert(module->m_adapter == NULL);

    if (module->m_app_id) {
        mem_free(module->m_alloc, module->m_app_id);
        module->m_app_id = NULL;
    }

    if (module->m_url) {
        mem_free(module->m_alloc, module->m_url);
        module->m_url = NULL;
    }
    
    if (module->m_chanel) {
        mem_free(module->m_alloc, module->m_chanel);
        module->m_chanel = NULL;
    }

    if (module->m_appv_key) {
        mem_free(module->m_alloc, module->m_appv_key);
        module->m_appv_key = NULL;
    }
    
    if (module->m_platp_key) {
        mem_free(module->m_alloc, module->m_platp_key);
        module->m_platp_key = NULL;
    }

    if (module->m_free_pay_product_id) {
        mem_free(module->m_alloc, module->m_free_pay_product_id);
        module->m_free_pay_product_id = NULL;
    }
    
    mem_buffer_clear(&module->m_dump_buffer);
}

void appsvr_unicompay_module_free(appsvr_unicompay_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_appsvr_unicompay_module) return;
    nm_node_free(module_node);
}

appsvr_unicompay_module_t
appsvr_unicompay_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "appsvr_unicompay_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_appsvr_unicompay_module) return NULL;
    return (appsvr_unicompay_module_t)nm_node_data(node);
}

const char * appsvr_unicompay_module_name(appsvr_unicompay_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

int appsvr_unicompay_module_set_url(appsvr_unicompay_module_t adapter, const char * url) {
    char url_buf[128];

    if (url == NULL) {
        CPE_ERROR(adapter->m_em, "unicompay: url can`t set to NULL");
        return -1;
    }
    
    if (adapter->m_url) {
        mem_free(adapter->m_alloc, adapter->m_url);
    }

    snprintf(url_buf, sizeof(url_buf), "%s.unicompay", url);
    adapter->m_url = cpe_str_mem_dup(adapter->m_alloc, url_buf);
    
    return adapter->m_url ? 0 : -1;
}

int appsvr_unicompay_module_set_app_id(appsvr_unicompay_module_t adapter, const char * app_id) {
    if (app_id == NULL) {
        CPE_ERROR(adapter->m_em, "unicompay: app id can`t set to NULL");
        return -1;
    }
    
    if (adapter->m_app_id) {
        mem_free(adapter->m_alloc, adapter->m_app_id);
    }

    adapter->m_app_id = cpe_str_mem_dup(adapter->m_alloc, app_id);
    return adapter->m_app_id ? 0 : -1;
}

int appsvr_unicompay_module_set_chanel(appsvr_unicompay_module_t adapter, const char * chanel) {
    if (adapter->m_chanel) {
        mem_free(adapter->m_alloc, adapter->m_chanel);
    }

    if (chanel) {
        adapter->m_chanel = cpe_str_mem_dup(adapter->m_alloc, chanel);
    }
    else {
        adapter->m_chanel = NULL;
    }

    return 0;
}

int appsvr_unicompay_module_set_appv_key(appsvr_unicompay_module_t adapter, const char * appv_key) {
    if (appv_key == NULL) {
        CPE_ERROR(adapter->m_em, "unicompay: appv key can`t set to NULL");
        return -1;
    }
    
    if (adapter->m_appv_key) {
        mem_free(adapter->m_alloc, adapter->m_appv_key);
    }

    adapter->m_appv_key = cpe_str_mem_dup(adapter->m_alloc, appv_key);
    return adapter->m_appv_key ? 0 : -1;
}

int appsvr_unicompay_module_set_platp_key(appsvr_unicompay_module_t adapter, const char * platp_key) {
    if (platp_key == NULL) {
        CPE_ERROR(adapter->m_em, "unicompay: platp key can`t set to NULL");
        return -1;
    }
    
    if (adapter->m_platp_key) {
        mem_free(adapter->m_alloc, adapter->m_platp_key);
    }

    adapter->m_platp_key = cpe_str_mem_dup(adapter->m_alloc, platp_key);
    return adapter->m_platp_key ? 0 : -1;
}

int appsvr_unicompay_module_set_free_pay_product_id(appsvr_unicompay_module_t adapter, const char * free_pay_product_id) {
    if (adapter->m_free_pay_product_id) {
        mem_free(adapter->m_alloc, adapter->m_free_pay_product_id);
    }

    if (free_pay_product_id) {
        adapter->m_free_pay_product_id = cpe_str_mem_dup(adapter->m_alloc, free_pay_product_id);
        return adapter->m_free_pay_product_id ? 0 : -1;
    }
    else {
        adapter->m_free_pay_product_id = NULL;
        return 0;
    }
}


EXPORT_DIRECTIVE
int appsvr_unicompay_module_app_init(gd_app_context_t app, gd_app_module_t module, cfg_t cfg) {
    appsvr_payment_module_t payment_module;
    appsvr_unicompay_module_t unicompay;

    payment_module = appsvr_payment_module_find_nc(app, NULL);
    if (payment_module == NULL) {
        APP_CTX_ERROR(app, "unicompay: load: no payment module");
        return -1;
    }

    unicompay =
        appsvr_unicompay_module_create(
            app, cfg_get_uint8(cfg, "debug", 0),
            payment_module,
            gd_app_alloc(app),
            gd_app_module_name(module),
            gd_app_em(app));
    if (unicompay == NULL) return -1;

    return 0;
}

EXPORT_DIRECTIVE
void appsvr_unicompay_module_app_fini(gd_app_context_t app, gd_app_module_t module) {
    appsvr_unicompay_module_t appsvr_unicompay_module;

    appsvr_unicompay_module = appsvr_unicompay_module_find_nc(app, gd_app_module_name(module));
    if (appsvr_unicompay_module) {
        appsvr_unicompay_module_free(appsvr_unicompay_module);
    }
}
