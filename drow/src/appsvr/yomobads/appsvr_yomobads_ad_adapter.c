#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "appsvr/ad/appsvr_ad_module.h"
#include "appsvr/ad/appsvr_ad_adapter.h"
#include "appsvr/ad/appsvr_ad_action.h"
#include "appsvr/ad/appsvr_ad_request.h"
#include "appsvr_yomobads_module_i.h"

static int appsvr_yomobads_ad_start(void * ctx, appsvr_ad_request_t request, appsvr_ad_action_t action) {
    appsvr_yomobads_module_t module = (appsvr_yomobads_module_t)ctx;
    const char * scene_id = appsvr_ad_action_data(action);
    module->m_request_id = appsvr_ad_request_id(request);
    return appsvr_yomobads_backend_open_start(module,scene_id);
}

static void appsvr_yomobads_ad_cancel(void * ctx, appsvr_ad_request_t request, appsvr_ad_action_t action) {
    
}

int appsvr_yomobads_ad_init(appsvr_yomobads_module_t module) {
	module->m_ad_adapter =
		appsvr_ad_adapter_create(
		module->m_ad_module, "yomobads",
        module, appsvr_yomobads_ad_start, appsvr_yomobads_ad_cancel);

	return 0;
}

void appsvr_yomobads_ad_fini(appsvr_yomobads_module_t module) {
    if (module->m_request_id) {
        appsvr_ad_request_t request = appsvr_ad_request_find_by_id(module->m_ad_module, module->m_request_id);
        if (request) {
            appsvr_ad_request_set_result(request, appsvr_ad_start_started);
        }
        module->m_request_id = 0;
    }

	if (module->m_ad_adapter) {
		appsvr_ad_adapter_free(module->m_ad_adapter);
		module->m_ad_adapter = NULL;
	}
}
