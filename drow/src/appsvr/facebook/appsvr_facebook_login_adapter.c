#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "gd/timer/timer_manage.h"
#include "appsvr_facebook_module_i.h"
#include "appsvr/account/appsvr_account_adapter.h"


static int appsvr_facebook_account_relogin_start(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_RELOGIN const * req) {
    appsvr_facebook_module_t module = *(appsvr_facebook_module_t*)appsvr_account_adapter_data(adapter);
	CPE_ERROR(module->m_em, "appsvr_facebook_account_relogin_start: enter!");

    return appsvr_facebook_backend_login_start(module, 1);
}

static int appsvr_facebook_account_login_start(appsvr_account_adapter_t adapter, APPSVR_ACCOUNT_LOGIN const * req) {
    appsvr_facebook_module_t module = *(appsvr_facebook_module_t*)appsvr_account_adapter_data(adapter);
	
	CPE_ERROR(module->m_em, "appsvr_facebook_account_login_start: enter!");

    return appsvr_facebook_backend_login_start(module, 0);
}


int appsvr_facebook_login_init(appsvr_facebook_module_t module) {
	CPE_ERROR(module->m_em, "appsvr_facebook_login_init: enter!");

	module->m_account_adapter = 
		appsvr_account_adapter_create(
		module->m_account_module,
        appsvr_account_facebook, "facebook",
		sizeof(appsvr_facebook_module_t), appsvr_facebook_account_login_start,appsvr_facebook_account_relogin_start);
	if (module->m_account_adapter == NULL) {
		CPE_ERROR(module->m_em, "appsvr_facebook_login_init: create adapter fail!");
		return -1;
	}

	*(appsvr_facebook_module_t*)appsvr_account_adapter_data(module->m_account_adapter) = module;

	return 0;
}

void appsvr_facebook_login_fini(appsvr_facebook_module_t module) {
	CPE_ERROR(module->m_em, "appsvr_facebook_login_fini: enter!");

	if (module->m_account_adapter) {
		appsvr_account_adapter_free(module->m_account_adapter);
		module->m_account_adapter = NULL;
	}
}
