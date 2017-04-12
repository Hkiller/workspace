#include <assert.h>
#include "appsvr_facebook_backend.hpp"

int appsvr_facebook_backend_init(appsvr_facebook_module_t module) {
    return 0;
}

void appsvr_facebook_backend_fini(appsvr_facebook_module_t module) {
}

int appsvr_facebook_backend_login_start(appsvr_facebook_module_t module, uint8_t is_relogin){
    return 0;
}

void appsvr_facebook_on_suspend(appsvr_facebook_module_t module) {
}

void appsvr_facebook_on_resume(appsvr_facebook_module_t module) {
}
