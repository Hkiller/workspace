#include "../appsvr_notify_device_module_i.h"

int appsvr_notify_device_backend_init(appsvr_notify_device_module_t module) {
    return 0;
}

void appsvr_notify_device_backend_fini(appsvr_notify_device_module_t module) {
}

int appsvr_notify_device_install_schedule(void * ctx, appsvr_notify_schedule_t schedule){
    return 0;
}

int appsvr_notify_device_update_schedule(void * ctx, appsvr_notify_schedule_t schedule){
    return 0;
}

void appsvr_notify_device_uninstall_schedule(void * ctx, appsvr_notify_schedule_t schedule){

}

void appsvr_notify_device_on_suspend(appsvr_notify_device_module_t module) {
}

void appsvr_notify_device_on_resume(appsvr_notify_device_module_t module) {
}
