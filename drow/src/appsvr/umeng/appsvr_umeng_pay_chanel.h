#ifndef APPSVR_STATISTICS_UMENG_PAY_CHANEL_H
#define APPSVR_STATISTICS_UMENG_PAY_CHANEL_H
#include "appsvr_umeng_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_umeng_pay_chanel {
    appsvr_umeng_module_t m_module;
    TAILQ_ENTRY(appsvr_umeng_pay_chanel) m_next;
    uint8_t m_service;
    uint8_t m_id;
};
    
void appsvr_umeng_pay_chanel_free(appsvr_umeng_pay_chanel_t chanel);
int appsvr_umeng_load_pay_chanels(appsvr_umeng_module_t module, cfg_t cfg);

appsvr_umeng_pay_chanel_t appsvr_umeng_pay_chanel_find(appsvr_umeng_module_t module, uint8_t service);

#ifdef __cplusplus
}
#endif
    
#endif
