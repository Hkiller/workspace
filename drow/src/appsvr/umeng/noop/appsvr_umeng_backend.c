#include "../appsvr_umeng_executor.h"

int appsvr_umeng_backend_init(appsvr_umeng_module_t module) {
    return 0;
}

void appsvr_umeng_backend_fini(appsvr_umeng_module_t module) {
}

void appsvr_umeng_on_page_begin(appsvr_umeng_module_t module, const char * page_name) {
}

void appsvr_umeng_on_page_end(appsvr_umeng_module_t module, const char * page_name) {
}

void appsvr_umeng_on_pause(appsvr_umeng_module_t module) {
}

void appsvr_umeng_on_resume(appsvr_umeng_module_t module) {
}

int appsvr_umeng_executor_backend_init(appsvr_umeng_executor_t executor) {
    return 0;
}

void appsvr_umeng_executor_backend_exec(appsvr_umeng_executor_t executor, dr_data_source_t data_source) {
}

void appsvr_umeng_executor_backend_fini(appsvr_umeng_executor_t executor) {
}

void appsvr_umeng_on_event(appsvr_umeng_module_t module, const char * id, uint32_t count, const char * attrs) {

}
