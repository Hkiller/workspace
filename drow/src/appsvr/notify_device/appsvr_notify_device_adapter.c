#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/timer/timer_manage.h"
#include "appsvr_notify_device_module_i.h"
#include "appsvr/notify/appsvr_notify_adapter.h"

int appsvr_notify_device_adapter_init(appsvr_notify_device_module_t module) {
    module->m_notify_adapter = 
        appsvr_notify_adapter_create(
            module->m_notify_module, "device",
            module,
            appsvr_notify_device_install_schedule,
            appsvr_notify_device_update_schedule,
            appsvr_notify_device_uninstall_schedule,
            module->m_tags);
	if (module->m_notify_adapter == NULL) {
		CPE_ERROR(module->m_em, "appsvr_notify_device_adapter_init: create adapter fail!");
		return -1;
	}

	return 0;
}

void appsvr_notify_device_adapter_fini(appsvr_notify_device_module_t module) {
	if (module->m_notify_adapter) {
        appsvr_notify_adapter_free(module->m_notify_adapter);
		module->m_notify_adapter = NULL;
	}
}
