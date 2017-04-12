#import <UIKit/UIKit.h>
#import <Foundation/NSString.h>
#include <sys/sysctl.h>
#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "appsvr_device_backend.h"

static void appsvr_device_query_device_id(appsvr_device_module_t module, char * buf, size_t buf_size);
static void appsvr_device_query_device_model(appsvr_device_module_t module, char * buf, size_t buf_size);
uint8_t appsvr_device_device_model_to_cap(appsvr_device_module_t module, const char * machine);

void appsvr_device_backend_set_path_info(
    appsvr_device_module_t module, APPSVR_DEVICE_QUERY_PATH const * req, APPSVR_DEVICE_PATH * path_info)
{
    path_info->category = req->category;
    cpe_str_dup(path_info->path, sizeof(path_info->path), "");
}

void appsvr_device_backend_set_device_info(appsvr_device_module_t module, APPSVR_DEVICE_INFO * device_info) {
    device_info->category = appsvr_device_ios;
    cpe_str_dup(device_info->cpu_name, sizeof(device_info->cpu_name), "");
    device_info->cpu_freq = 0;
    device_info->memory_kb = 0;
    appsvr_device_query_device_id(module, device_info->device_id, sizeof(device_info->device_id));
    appsvr_device_query_device_model(module, device_info->device_model, sizeof(device_info->device_model));    
    device_info->device_cap = appsvr_device_device_model_to_cap(module, device_info->device_model);
    
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSArray *languages = [defaults objectForKey:@"AppleLanguages"];
    NSString *currentLanguage = [languages objectAtIndex:0];
    
//    NSDictionary* temp = [NSLocale componentsFromLocaleIdentifier:currentLanguage];
//    NSString * languageCode = [temp objectForKey:NSLocaleLanguageCode];
    
    if ([currentLanguage isEqualToString:@"zh-Hans-CN"]) {
        cpe_str_dup(device_info->device_language , sizeof(device_info->device_language), "cn");
    }
    else if ([currentLanguage isEqualToString:@"en"]) {
        cpe_str_dup(device_info->device_language , sizeof(device_info->device_language), "en");
    }
    else {
        cpe_str_dup(device_info->device_language , sizeof(device_info->device_language), "en");

    }
}

uint8_t appsvr_device_device_model_to_cap(appsvr_device_module_t module, const char * machine) {
    static const char * s_devices_low[] = { "iPhone2,1" };
    static const char * s_devices_medium[] = { "iPhone3,1", "iPhone3,2", "iPod4,1", "iPad1,1" };
    static const char * s_devices_high[] = { "iPhone4,1", "iPhone5,1", "iPod5,1", "iPad2,1",
                                           "iPad2,2", "iPad2,3", "iPad2,4", "iPad2,5",
                                           "iPad2,6", "iPad3,1", "iPad3,2", "iPad3,3",
                                           "iPad3,4", "iPad3,5", "iPad3,6"
    };

    for(size_t i = 0; i < CPE_ARRAY_SIZE(s_devices_low); ++i) {
        if (strcmp(machine, s_devices_low[i]) == 0) return appsvr_device_cap_low;
    }

    for(size_t i = 0; i < CPE_ARRAY_SIZE(s_devices_medium); ++i) {
        if (strcmp(machine, s_devices_medium[i]) == 0) return appsvr_device_cap_medium;
    }

    for(size_t i = 0; i < CPE_ARRAY_SIZE(s_devices_high); ++i) {
        if (strcmp(machine, s_devices_high[i]) == 0) return appsvr_device_cap_high;
    }

    return appsvr_device_cap_high;
}

void appsvr_device_query_device_id(appsvr_device_module_t module, char * buf, size_t buf_size) {
    Class cls = NSClassFromString(@"UMANUtil");
    SEL deviceIDSelector = @selector(openUDIDString);
    NSString *deviceID = nil;
    if(cls && [cls respondsToSelector:deviceIDSelector]){
        deviceID = [cls performSelector:deviceIDSelector];
    }

    if (deviceID == nil) {
        CPE_ERROR(module->m_em, "appsvr_device_query_device_id: get device id fail!");
        buf[0] = 0;
    }
    else {
        cpe_str_dup(buf, buf_size, [deviceID UTF8String]);
    }
}

void appsvr_device_query_device_model(appsvr_device_module_t module, char * buf, size_t buf_size) {
    int mib[] = { CTL_HW, HW_MACHINE };
    sysctl(mib, CPE_ARRAY_SIZE(mib), (void*)buf, &buf_size, NULL, 0);
}

int appsvr_device_backend_set_network_state(appsvr_device_module_t module, APPSVR_DEVICE_NETWORK_INFO * network_info) {
    switch([module->m_backend->m_reachability currentReachabilityStatus]) {
    case NotReachable:
        network_info->state = appsvr_device_network_none;
        break;
    case ReachableViaWiFi:
        network_info->state = appsvr_device_network_wifi;
        break;
    case ReachableViaWWAN:
        network_info->state = appsvr_device_network_wwan;
        break;
    }
    return 0;
}

