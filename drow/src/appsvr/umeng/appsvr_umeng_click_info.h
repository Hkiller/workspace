#ifndef APPSVR_STATISTICS_UMENG_CLICK_INFO_H
#define APPSVR_STATISTICS_UMENG_CLICK_INFO_H
#include "appsvr_umeng_module_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_umeng_click_info {
    appsvr_umeng_module_t m_module;
    struct cpe_hash_entry m_hh;
    const char * m_page_name;
    const char * m_control_name;
    const char * m_id;
    const char * m_value;
    const char * m_attrs;
};

appsvr_umeng_click_info_t
appsvr_umeng_click_info_create(
    appsvr_umeng_module_t module, const char * page_name, const char * control_name,
    const char * id, const char * value, const char * attrs);

void appsvr_umeng_click_info_free(appsvr_umeng_click_info_t click_info);

int appsvr_umeng_load_click_infos(appsvr_umeng_module_t module, cfg_t cfg);
    
void appsvr_umeng_click_info_free_all(appsvr_umeng_module_t module);
    
uint32_t appsvr_umeng_click_info_hash(appsvr_umeng_click_info_t click_info);
int appsvr_umeng_click_info_eq(appsvr_umeng_click_info_t l, appsvr_umeng_click_info_t r);
    
#ifdef __cplusplus
}
#endif
    
#endif
