#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "plugin/app_env/plugin_app_env_module.h"
#include "appsvr_device_backend.h"

@interface appsvr_device_backend_adapter() {
    appsvr_device_module_t m_module;
}
@end

@implementation appsvr_device_backend_adapter

- (id) initWithModule: (appsvr_device_module_t)module
{
    self->m_module = module;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(reachabilityChanged) name:kReachabilityChangedNotification object:nil];
    return self;
}

- (void)reachabilityChanged
{
    APPSVR_DEVICE_NETWORK_INFO network_info;
    if (appsvr_device_backend_set_network_state(self->m_module, &network_info) == 0) {
        plugin_app_env_send_notification(self->m_module->m_app_env,  self->m_module->m_meta_network_info, &network_info, sizeof(network_info));
    }
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:kReachabilityChangedNotification object:nil];
}

@end

int appsvr_device_backend_init(appsvr_device_module_t module) {
    appsvr_device_backend_t backend;

    backend = (appsvr_device_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_device_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_device_backend_init: alloc fail!");
        return -1;
    }

    backend->m_adapter = [[appsvr_device_backend_adapter alloc] initWithModule: module];
    backend->m_reachability = [Reachability reachabilityForInternetConnection];
    [backend->m_reachability startNotifier];

    module->m_backend = backend;
    
    return 0;
}

void appsvr_device_backend_fini(appsvr_device_module_t module) {
    appsvr_device_backend_t backend = module->m_backend;

    [backend->m_reachability stopNotifier];
    backend->m_adapter = nil;

    mem_free(module->m_alloc, backend);
    module->m_backend = NULL;
}
