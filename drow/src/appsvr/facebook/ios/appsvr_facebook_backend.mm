#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "plugin/app_env/ios/plugin_app_env_ios.h"
#include "appsvr/account/appsvr_account_adapter.h"
#include "appsvr_facebook_backend.h"
#include "../appsvr_facebook_permission_i.h"

static uint8_t appsvr_facebook_backend_open_url(
    void * ctx, void * application, void * url, void * sourceApplication, void * annotation);

int appsvr_facebook_backend_init(appsvr_facebook_module_t module) {
    appsvr_facebook_backend_t backend = (appsvr_facebook_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_facebook_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_facebook_backend_init: alloc fail!");
        return -1;
    }
    backend->m_module = module;
    backend->m_login_mgr = NULL;
    
    if (plugin_app_env_ios_register_delegate(module->m_app_env, module, appsvr_facebook_backend_open_url) != 0) {
        CPE_ERROR(module->m_em, "appsvr_facebook_backend_init: register delegate fail!");
        mem_free(module->m_alloc, backend);
        return -1;
    }

    @try {
        [FBSDKAppEvents activateApp];
    
        [[FBSDKApplicationDelegate sharedInstance]
            application:(UIApplication *) plugin_app_env_ios_application(module->m_app_env)
            didFinishLaunchingWithOptions: [NSDictionary dictionaryWithObjectsAndKeys: nil]];
    }
    @catch(NSException * e) {
        CPE_ERROR(
            module->m_em, "appsvr_facebook_backend_init: catch exceptin %s: reasion=%s",
            [e.name UTF8String], [e.reason UTF8String]);
        plugin_app_env_ios_unregister_delegate(module->m_app_env, module);
        mem_free(module->m_alloc, backend);
        return -1;
    }

    module->m_backend = backend;
    return 0;
}

void appsvr_facebook_backend_fini(appsvr_facebook_module_t module) {
    plugin_app_env_ios_unregister_delegate(module->m_app_env, module);

    mem_free(module->m_alloc, module->m_backend);
    module->m_backend = NULL;
}

int appsvr_facebook_backend_login_start(appsvr_facebook_module_t module, uint8_t is_relogin) {
    appsvr_facebook_backend_t backend = module->m_backend;
    appsvr_facebook_permission_t permission;

    assert(backend);

    if (backend->m_login_mgr == NULL) {
        backend->m_login_mgr = [[FBSDKLoginManager alloc] init];
        if (backend->m_login_mgr == NULL) {
            CPE_ERROR(module->m_em, "appsvr_facebook_backend_login_start: alloc FBSDKLoginManager fail!");
            return -1;
        }
    }

    @try {
        NSMutableArray* permisions = [[NSMutableArray alloc] init];

        TAILQ_FOREACH(permission, &module->m_permissions, m_next) {
            [permisions addObject: [[NSString alloc] initWithUTF8String: permission->m_permission]];
        }
    
        [backend->m_login_mgr
            logInWithReadPermissions: permisions
            handler:^(FBSDKLoginManagerLoginResult *result, NSError *error) {

                APPSVR_ACCOUNT_LOGIN_RESULT login_result;
                bzero(&login_result, sizeof(login_result));
    
                if (error) {  
                    login_result.result = appsvr_account_login_failed;
                }
                else if (result.isCancelled) {  
                    login_result.result = appsvr_account_login_canceled;
                }
                else {  
                    if (result.token.tokenString == nil) {
                        CPE_ERROR(module->m_em, "appsvr_facebook_backend: login success, but no token!!!");
                        login_result.result = appsvr_account_login_failed;
                    }  
                    else {
                        appsvr_facebook_permission_t permission;
                        const char * token;

                        token = [result.token.tokenString UTF8String];
                        appsvr_facebook_set_token(module, token);
                        CPE_INFO(module->m_em, "appsvr_facebook_backend: login success, token-len=%d, token=%s", strlen(token), token);

                        NSLog(@"token %@:", result.token.tokenString);

                        TAILQ_FOREACH(permission, &module->m_permissions, m_next) {
                            if ([result.grantedPermissions containsObject: [[NSString alloc] initWithUTF8String: permission->m_permission]]) {
                                permission->m_is_gaint = 1;
                            }
                            else {
                                CPE_ERROR(module->m_em, "appsvr_facebook_backend: permission %s not gaint", permission->m_permission);
                            }
                        }

                        login_result.result = appsvr_account_login_success;
                        snprintf(login_result.token, sizeof(login_result.token), "%s", token);
                    }
                }

                appsvr_account_adapter_notify_login_result(module->m_account_adapter, &login_result);
            }];

        return 0;
    }
    @catch(NSException * e) {
        CPE_ERROR(
            module->m_em, "appsvr_facebook_backend_login_start: catch exceptin %s: reasion=%s",
            [e.name UTF8String],
            [e.reason UTF8String]);
        return -1;
    }
}

uint8_t appsvr_facebook_backend_open_url(void * ctx, void * application, void * url, void * sourceApplication, void * annotation) {
    return [[FBSDKApplicationDelegate sharedInstance] application: (UIApplication*)application
                                                   openURL: (NSURL *)url
                                         sourceApplication: (NSString *)sourceApplication
                                                annotation: (id)annotation]
        ? 1
        : 0;
}

void appsvr_facebook_on_suspend(appsvr_facebook_module_t module) {
}

void appsvr_facebook_on_resume(appsvr_facebook_module_t module) {
      [FBSDKAppEvents activateApp];
}
