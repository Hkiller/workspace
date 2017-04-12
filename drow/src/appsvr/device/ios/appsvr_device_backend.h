#ifndef APPSVR_STATISTICS_DEVIE_BACKEND_H
#define APPSVR_STATISTICS_DEVIE_BACKEND_H
#include "../appsvr_device_module_i.h"
#import "Reachability.h"

#ifdef __cplusplus
extern "C" {
#endif

@interface appsvr_device_backend_adapter : NSObject
@end

struct appsvr_device_backend {
    appsvr_device_backend_adapter * m_adapter;
    Reachability * m_reachability;
};

#ifdef __cplusplus
}
#endif
    
#endif
