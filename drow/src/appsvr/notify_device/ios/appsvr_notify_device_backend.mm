#import <UIKit/UIKit.h>
#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "plugin/app_env/ios/plugin_app_env_ios.h"
#include "appsvr/notify/appsvr_notify_schedule.h"
#include "../appsvr_notify_device_module_i.h"

int appsvr_notify_device_backend_init(appsvr_notify_device_module_t module) {
    UIApplication * application = (UIApplication *) plugin_app_env_ios_application(module->m_app_env);

    [application registerUserNotificationSettings: [UIUserNotificationSettings settingsForTypes:UIUserNotificationTypeAlert|UIUserNotificationTypeBadge|UIUserNotificationTypeSound categories:nil]];

    [application cancelAllLocalNotifications];
    
    return 0;
}

void appsvr_notify_device_backend_fini(appsvr_notify_device_module_t module) {
}

int appsvr_notify_device_install_schedule(void * ctx, appsvr_notify_schedule_t schedule) {
    appsvr_notify_device_module_t module = (appsvr_notify_device_module_t)ctx;

    UIApplication * application = (UIApplication *) plugin_app_env_ios_application(module->m_app_env);

    UILocalNotification * notification = [[UILocalNotification alloc] init];
    if (notification == NULL) {
        CPE_ERROR(module->m_em, "appsvr_notify_device_install_schedule: alloc notification fail!");
        return -1;
    }

    notification.fireDate = [NSDate dateWithTimeIntervalSince1970: appsvr_notify_schedule_start_time(schedule)];
    notification.timeZone = [NSTimeZone defaultTimeZone];
    
    notification.repeatInterval = appsvr_notify_schedule_repeat_time(schedule);
    
    notification.soundName = UILocalNotificationDefaultSoundName;
    
    notification.alertBody = [NSString stringWithUTF8String: appsvr_notify_schedule_title(schedule)];
    notification.applicationIconBadgeNumber = 1;

    char id_buf[64];
    snprintf(id_buf, sizeof(id_buf), "%d", appsvr_notify_schedule_id(schedule));
    notification.userInfo = [NSDictionary dictionaryWithObject: [NSString stringWithUTF8String: id_buf]
                                                        forKey: @"id"];

    [application scheduleLocalNotification: notification];
    
    return 0;
}

int appsvr_notify_device_update_schedule(void * ctx, appsvr_notify_schedule_t schedule) {
    return 0;
}

void appsvr_notify_device_uninstall_schedule(void * ctx, appsvr_notify_schedule_t schedule) {
    appsvr_notify_device_module_t module = (appsvr_notify_device_module_t)ctx;
    UIApplication * application = (UIApplication *) plugin_app_env_ios_application(module->m_app_env);
    NSArray * array = [application scheduledLocalNotifications];
    char id_buf[64];

    snprintf(id_buf, sizeof(id_buf), "%d", appsvr_notify_schedule_id(schedule));
    NSString * notify_id = [NSString stringWithUTF8String: id_buf];

    //便利这个数组 根据 key 拿到我们想要的 UILocalNotification
    for (UILocalNotification * loc in array) {
        if ([[loc.userInfo objectForKey:@"id"] isEqualToString:notify_id]) {
            //取消 本地推送
            [[UIApplication sharedApplication] cancelLocalNotification:loc];
            CPE_ERROR(module->m_em, "appsvr_notify_device_uninstall_schedule: schedule %d uninstalled!", appsvr_notify_schedule_id(schedule));
        }
    }
}

void appsvr_notify_device_on_suspend(appsvr_notify_device_module_t module) {
    return ;
}

void appsvr_notify_device_on_resume(appsvr_notify_device_module_t module) {
    UIApplication * application = (UIApplication *) plugin_app_env_ios_application(module->m_app_env);
    application.applicationIconBadgeNumber = 0;
    return ;
}
