#ifndef APPSVR_STATISTICS_APPLE_PURCHASE_ANDROID_H
#define APPSVR_STATISTICS_APPLE_PURCHASE_ANDROID_H
#include "cpe/pal/pal_queue.h"
#include "plugin/app_env/android/plugin_app_env_android.hpp"
#include "../appsvr_apple_purchase_module_i.h"
#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_apple_purchase_backend {
    appsvr_apple_purchase_module_t m_module;
};

#ifdef __cplusplus
}
#endif
    
#endif
