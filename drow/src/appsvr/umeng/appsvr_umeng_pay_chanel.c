#include "cpe/pal/pal_stdlib.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/utils/string_utils.h"
#include "appsvr_umeng_pay_chanel.h"

appsvr_umeng_pay_chanel_t
appsvr_umeng_pay_chanel_create(appsvr_umeng_module_t module, uint8_t service, uint8_t chanel_id) {
    appsvr_umeng_pay_chanel_t pay_chanel;

    pay_chanel = mem_alloc(module->m_alloc, sizeof(struct appsvr_umeng_pay_chanel));
    if (pay_chanel == NULL) {
        CPE_ERROR(module->m_em, "appsvr_umeng_pay_chanel: alloc fail!");
        return NULL;
    }
    
    pay_chanel->m_module = module;
    pay_chanel->m_service = service;
    pay_chanel->m_id = chanel_id;
    TAILQ_INSERT_TAIL(&module->m_pay_chanels, pay_chanel, m_next);
    
    return pay_chanel;
}

void appsvr_umeng_pay_chanel_free(appsvr_umeng_pay_chanel_t chanel) {
    appsvr_umeng_module_t module = chanel->m_module;

    TAILQ_REMOVE(&module->m_pay_chanels, chanel, m_next);

    mem_free(module->m_alloc, chanel);
}

appsvr_umeng_pay_chanel_t
appsvr_umeng_pay_chanel_find(appsvr_umeng_module_t module, uint8_t service) {
    appsvr_umeng_pay_chanel_t chanel;

    TAILQ_FOREACH(chanel, &module->m_pay_chanels, m_next) {
        if (chanel->m_service == service) return chanel;
    }

    return NULL;
}

int appsvr_umeng_load_pay_chanels(appsvr_umeng_module_t module, cfg_t cfg) {
    struct cfg_it data_it;
    cfg_t data_cfg;
    
    cfg_it_init(&data_it, cfg_find_cfg(cfg, "pay-chanels"));
    while((data_cfg = cfg_it_next(&data_it))) {
        uint8_t chanel_id;
        
        data_cfg = cfg_child_only(data_cfg);
        if (data_cfg == NULL) {
            CPE_ERROR(module->m_em, "appsvr_umeng_load_pah_chanels: format error!");
            return -1;
        }

        if (cfg_try_as_uint8(data_cfg, &chanel_id) != 0) {
            CPE_ERROR(module->m_em, "appsvr_umeng_load_pah_chanels: chanel %s read id fail!", cfg_name(data_cfg));
            return -1;
        }

        if (appsvr_umeng_pay_chanel_create(module, atoi(cfg_name(data_cfg)), chanel_id) == NULL) {
            CPE_ERROR(module->m_em, "appsvr_umeng_load_pah_chanels: chanel %s create fail!", cfg_name(data_cfg));
            return -1;
        }
    }

    return 0;
}



