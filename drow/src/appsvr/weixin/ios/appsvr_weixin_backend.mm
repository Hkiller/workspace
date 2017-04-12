#import <assert.h>
#import "cpe/pal/pal_stdlib.h"
#import "cpe/utils/string_utils.h"
#import "gd/app/app_context.h"
#import "appsvr_weixin_backend.h"
#import "protocol/plugin/app_env/app_env_pro.h"

static int appsvr_weixin_backend_handl_url(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size);

int appsvr_weixin_backend_init(appsvr_weixin_module_t module) {
    appsvr_weixin_backend_t backend;
    uint16_t component_pos;

    backend = (appsvr_weixin_backend_t)mem_alloc(module->m_alloc, sizeof(struct appsvr_weixin_backend));
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "appsvr_weixin_backend_create: alloc fail!");
        return -1;
    }
    
    backend->m_url_handler = 
        plugin_app_env_monitor_create(
            module->m_app_env, "app_env_handle_url", module, appsvr_weixin_backend_handl_url, NULL);
    if (backend->m_url_handler == NULL) {
        CPE_ERROR(module->m_em, "appsvr_weixin_backend_create: create url handler fail!");
        mem_free(module->m_alloc, backend);
        return -1;
    }

    backend->m_delegate = [[appsvr_weixin_delegate alloc] initWithModule: module];
    
    [WXApi registerApp : [NSString stringWithUTF8String: module->m_appid] ];

    module->m_backend = backend;
    
    return 0;
}

void appsvr_weixin_backend_fini(appsvr_weixin_module_t module) {
    appsvr_weixin_backend_t backend = module->m_backend;

    assert(backend);

    backend->m_delegate = NULL;
    
    plugin_app_env_monitor_free(backend->m_url_handler);

    mem_free(module->m_alloc, backend);
    module->m_backend = NULL;
}

int appsvr_weixin_backend_start_login(appsvr_weixin_module_t module, uint8_t is_relogin) {
    char session[32];
    snprintf(session, sizeof(session), "%d", module->m_login_session);
    
    SendAuthReq* req =[[[SendAuthReq alloc ] init ] autorelease ];
    req.scope = [NSString stringWithUTF8String: module->m_scope];
    req.state = [NSString stringWithUTF8String: session];
    [WXApi sendReq:req];
    
    return 0;
}

static int appsvr_weixin_backend_handl_url(void * ctx, LPDRMETA req_meta, void const * req_data, size_t req_size) {
    /*
    appsvr_weixin_module_t module = (appsvr_weixin_module_t)ctx;
    APP_ENV_HANDLE_URL const * req = (APP_ENV_HANDLE_URL const *)req_data;

    // CPE_ERROR(module->m_em, "xxxx: source=%s", req->source_app);
    // CPE_ERROR(module->m_em, "xxxx: url=%s", req->url);
    
    if (strcmp(req->source_app, "com.tencent.xin") != 0) return 0;

    NSURL * url = [NSURL URLWithString: [NSString stringWithUTF8String: req->url]];

    return [WXApi handleOpenURL:url delegate: module->m_backend->m_delegate] ? 1 : 0;
     */
}
