#ifndef APPSVR_STATISTICS_UMENG_BACKEND_H
#define APPSVR_STATISTICS_UMENG_BACKEND_H
#include "../appsvr_umeng_module_i.h"
#include "libs/MobClick.h"
#include "libs/MobClickGameAnalytics.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_umeng_backend {
    appsvr_umeng_module_t m_module;
};
    
#ifdef __cplusplus
}
#endif
    
#endif
