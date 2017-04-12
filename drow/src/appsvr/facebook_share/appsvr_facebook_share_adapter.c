#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/timer/timer_manage.h"
#include "appsvr/share/appsvr_share_adapter.h"
#include "appsvr_facebook_share_module_i.h"

static int appsvr_facebook_share_commit(void * ctx, appsvr_share_request_t req) {
    appsvr_facebook_share_module_t module = ctx;
    return appsvr_facebook_share_backend_commit(module, req);
}


int appsvr_facebook_share_commit_init(appsvr_facebook_share_module_t module) {
	CPE_ERROR(module->m_em, "appsvr_facebook_share_commit_init: enter!");

	module->m_share_adapter = 
		appsvr_share_adapter_create(
		module->m_share_module, "facebook_share",
		module, appsvr_facebook_share_commit);
	if (module->m_share_adapter == NULL) {
		CPE_ERROR(module->m_em, "appsvr_facebook_share_commit_init: create adapter fail!");
		return -1;
	}

    return 0;
}

void appsvr_facebook_share_commit_fini(appsvr_facebook_share_module_t module) {
	CPE_ERROR(module->m_em, "appsvr_facebook_share_commit_fini: enter!");

	if (module->m_share_adapter) {
		appsvr_share_adapter_free(module->m_share_adapter);
		module->m_share_adapter = NULL;
	}
}
