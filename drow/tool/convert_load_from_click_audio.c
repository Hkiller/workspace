#include <assert.h>
#include "convert_ctx.h"

int convert_load_from_click_audio(convert_ctx_t ctx) {
    struct cfg_it category_it;
    cfg_t category_cfg;
    
    cfg_it_init(&category_it, cfg_find_cfg(ctx->m_runing, "ui.ui-center.action-audio"));
    while((category_cfg = cfg_it_next(&category_it))) {
        const char * click_audio = cfg_get_string(category_cfg, "res", NULL);
        if (click_audio && click_audio[0]) {
            ui_cache_group_add_res_by_path(plugin_package_package_resources(ctx->m_global_package), click_audio);
        }
    }
    
    return 0;
}

