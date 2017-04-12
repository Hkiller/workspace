#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_context.h"
#include "appsvr_yomobads_delegate.h"
#include "appsvr/ad/appsvr_ad_action.h"

struct appsvr_yomobads_backend {
    YomobadsDelegate * m_delegate;
};

int appsvr_yomobads_backend_open_start(appsvr_yomobads_module_t yomobads, const char* sceneID){
    char product_buf[64];
    snprintf(product_buf, sizeof(product_buf), "%s", sceneID);
    NSString *scene_id = [NSString stringWithUTF8String: product_buf];

    [yomobads->m_backend->m_delegate showAd: scene_id];
    
    return 0;
}

int appsvr_yomobads_backend_init(appsvr_yomobads_module_t yomobads) {
    cfg_t global_cfg = cfg_find_cfg(gd_app_cfg(yomobads->m_app), "args");
    cfg_t yomobads_cfg = cfg_find_cfg(global_cfg, "yomobads");
    cfg_t ios_cfg = cfg_find_cfg(yomobads_cfg, "ios");
    
    if (appsvr_yomobads_module_set_app_id(yomobads, cfg_get_string(ios_cfg, "app-id", NULL)) != 0)
    {
        CPE_ERROR(yomobads->m_em, "appsvr_yomobads: set config data fail!");
        return -1;
    }
    
    
     yomobads->m_backend
         = (appsvr_yomobads_backend_t)mem_alloc(yomobads->m_alloc, sizeof(struct appsvr_yomobads_backend));
     if (yomobads->m_backend == NULL) {
         CPE_ERROR(yomobads->m_em, "appsvr_yomobads: alloc backend fail!");
         return -1;
     }
     yomobads->m_backend->m_delegate = [[YomobadsDelegate alloc] initWithModule: yomobads];

    NSString *app_id = [NSString stringWithUTF8String: yomobads->m_app_id];
    
    [TGSDK setDebugModel:YES];
    [TGSDK initialize:app_id
             callback:^(BOOL success, id tag, NSDictionary* result){
             }];
    
    [TGSDK setRewardVideoADDelegate:yomobads->m_backend->m_delegate];
    [TGSDK setADDelegate:yomobads->m_backend->m_delegate];
    [TGSDK preloadAd:yomobads->m_backend->m_delegate];
    
     return 0;
}

void appsvr_yomobads_backend_fini(appsvr_yomobads_module_t yomobads) {
    assert(yomobads->m_backend);

    yomobads->m_backend->m_delegate = nil;
    
    mem_free(yomobads->m_alloc, yomobads->m_backend);
    yomobads->m_backend = NULL;
}

void appsvr_yomobads_on_suspend(appsvr_yomobads_module_t module) {
    CPE_ERROR(module->m_em, "appsvr_yomobads_on_suspend: enter!");
    return ;
}

void appsvr_yomobads_on_resume(appsvr_yomobads_module_t module) {
    CPE_ERROR(module->m_em, "appsvr_yomobads_on_resume: enter!");
    return ;
}

int appsvr_yomobads_read_adaction_data(appsvr_yomobads_module_t module,cfg_t cfg){
    struct cfg_it scene_it;
    cfg_t scene_cfg;
    cfg_it_init(&scene_it, cfg_find_cfg(cfg, "ios.seans"));
    while((scene_cfg = cfg_it_next(&scene_it))) {
        struct cfg_it bind_to_it;
        const char * scene_id;
        cfg_t bind_to_cfg;
        size_t id_len;

        scene_id  = cfg_get_string(scene_cfg, "id", NULL);
        if (scene_id == NULL) {
            APP_CTX_ERROR(module->m_app, "appsvr_yomobads_module_app_init:scene id not configured!");
            return -1;
        }

        id_len = strlen(scene_id) + 1;

        cfg_it_init(&bind_to_it, cfg_find_cfg(scene_cfg, "bind-to"));
        while((bind_to_cfg = cfg_it_next(&bind_to_it))) {
            const char * action_name;
            appsvr_ad_action_t action;

            action_name  = cfg_as_string(bind_to_cfg, NULL);
            if (action_name == NULL) {
                APP_CTX_ERROR(module->m_app, "appsvr_yomobads_module_app_init:bind-to format error!");
                return -1;
            }

            action = appsvr_ad_action_create(module->m_ad_adapter, action_name, id_len);
            if (action == NULL) {
                APP_CTX_ERROR(module->m_app, "appsvr_yomobads_module_app_init:action create fail!!", action_name);
                return -1;
            }

            memcpy(appsvr_ad_action_data(action), scene_id, id_len);
        }
    }

    return 0;
}
