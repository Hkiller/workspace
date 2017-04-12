#ifndef APPSVR_STATISTICS_WEIXIN_IOS_H
#define APPSVR_STATISTICS_WEIXIN_IOS_H
#include "plugin/app_env/plugin_app_env_monitor.h"
#include "../appsvr_weixin_module_i.h"
#include "appsvr_weixin_delegate.h"

#ifdef __cplusplus
extern "C" {
#endif

struct appsvr_weixin_backend {
    plugin_app_env_monitor_t m_url_handler;
    appsvr_weixin_delegate * m_delegate;
};

#ifdef __cplusplus
}
#endif
    
#endif
