#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "appsvr_yomobads_backend.hpp"
#include "appsvr/ad/appsvr_ad_action.h"

int appsvr_yomobads_backend_init(appsvr_yomobads_module_t module) {
	return 0;
}

void appsvr_yomobads_backend_fini(appsvr_yomobads_module_t module) {
}

void appsvr_yomobads_on_suspend(appsvr_yomobads_module_t module) {
}

void appsvr_yomobads_on_resume(appsvr_yomobads_module_t module) {
}

void appsvr_yomobads_on_pause(appsvr_yomobads_module_t module, uint8_t is_pause) {
}

int appsvr_yomobads_backend_open_start(appsvr_yomobads_module_t module, const char* sceneID) {
	return 0;
}

void appsvr_yomobads_on_activity_result(appsvr_yomobads_module_t module,uint32_t requestCode, uint32_t resultCode){
}

void appsvr_yomobads_on_activity_start(appsvr_yomobads_module_t module) {
}

void appsvr_yomobads_on_activity_destroy(appsvr_yomobads_module_t module) {
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
