#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dr/dr_json.h"
#include "gd/dr_store/dr_store_manage.h"
#include "gd/app_attr/app_attr_provider.h"
#include "gd/app_attr/app_attr_synchronizer.h"
#include "appsvr_chuangku_module_i.h"

static int appsvr_chuangku_on_addition_attr_sync(void * ctx, app_attr_synchronizer_t synchronizer);

int appsvr_chuangku_set_addition_attr(appsvr_chuangku_module_t module, const char * def, const char * dft) {
    dr_store_manage_t dr_store_mgr;
    LPDRMETA addition_attr_meta;
    
    if (module->m_app_attr_module == NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: no attr module!");
        return -1;
    }

    if (module->m_addition_attr_provider != NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: addition attr already setted!");
        return -1;
    }
    
    dr_store_mgr = dr_store_manage_find_nc(module->m_app, NULL);
    if (dr_store_mgr == NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: no dr_store_manage!");
        return -1;
    }

    addition_attr_meta = dr_store_manage_find_meta(dr_store_mgr, def);
    if (addition_attr_meta == NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: meta %s not exist!", def);
        return -1;
    }

    assert(module->m_addition_attr_data == NULL);
    module->m_addition_attr_data = mem_alloc(module->m_alloc, dr_meta_size(addition_attr_meta));
    if (module->m_addition_attr_data == NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: meta %s alloc data fail, size=%d!", def, (int)(dr_meta_size(addition_attr_meta)));
        return -1;
    }

    bzero(module->m_addition_attr_data, dr_meta_size(addition_attr_meta));
    dr_meta_set_defaults(module->m_addition_attr_data, dr_meta_size(addition_attr_meta), addition_attr_meta, 0);
    
    if (dft) {
        if (dr_json_read(
                module->m_addition_attr_data, dr_meta_size(addition_attr_meta),
                dft, addition_attr_meta, module->m_em)
            < 0)
        {
            CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: meta %s set default data %s fail!", def, dft);
            mem_free(module->m_alloc, module->m_addition_attr_data);
            module->m_addition_attr_data = NULL;
            return -1;
        }
    }
    
    module->m_addition_attr_provider =
        app_attr_provider_create(
            module->m_app_attr_module,
            "chuangku-addition",
            module,
            module->m_addition_attr_data, dr_meta_size(addition_attr_meta), addition_attr_meta);
    if (module->m_addition_attr_provider == NULL) {
        CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: create provider fail!");
        mem_free(module->m_alloc, module->m_addition_attr_data);
        module->m_addition_attr_data = NULL;
        return -1;
    }

    if (app_attr_synchronizer_create(
            module->m_addition_attr_provider, "all",
            NULL, appsvr_chuangku_on_addition_attr_sync) == NULL)
    {
        CPE_ERROR(module->m_em, "appsvr_chuangku_set_addition_attr: create synchronizer fail!");
        app_attr_provider_free(module->m_addition_attr_provider);
        module->m_addition_attr_provider = NULL;
        mem_free(module->m_alloc, module->m_addition_attr_data);
        module->m_addition_attr_data = NULL;
        return -1;
    }
    
    module->m_addition_attr_meta = addition_attr_meta;
    
    return 0;
}

static int appsvr_chuangku_on_addition_attr_sync(void * ctx, app_attr_synchronizer_t synchronizer) {
    appsvr_chuangku_module_t module = ctx;

    if (appsvr_chuangku_sync_addition_attr(module) != 0) {
        app_attr_synchronizer_set_done(synchronizer, 0);
    }
    else {
        app_attr_synchronizer_set_done(synchronizer, 1);
    }

    return 0;
}
