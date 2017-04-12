#include <assert.h>
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_group.h"
#include "render/model/ui_data_src.h"
#include "convert_ctx_res_to_src.h"

static int convert_prepaire_src_using_res_one(ui_data_src_t src, convert_ctx_t ctx, ui_cache_group_t res_group) {
    int rv = 0;
    
    if(ui_data_src_type(src) == ui_data_src_type_dir) {
        struct ui_data_src_it child_it;
        ui_data_src_t child;
        ui_data_src_childs(&child_it, src);

        while((child = ui_data_src_it_next(&child_it))) {
            if (convert_prepaire_src_using_res_one(child, ctx, res_group) != 0) rv = -1;
        }
    }
    else {
        ui_cache_group_clear(res_group);

        if (ui_data_src_collect_ress(src, res_group) != 0) {
            CPE_ERROR(
                ctx->m_em, "convert_prepaire_src_using_res: collect resources from %s fail!",
                ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
            rv = -1;
        }
        else {
            struct ui_cache_res_it res_it;
            ui_cache_res_t res;
        
            ui_cache_group_using_resources(&res_it, res_group);
            while((res = ui_cache_res_it_next(&res_it))) {
                if (convert_ctx_res_to_src_check_create(ctx, res, src) == NULL) {
                    CPE_ERROR(ctx->m_em, "convert_prepaire_src_using_res: crate fail!");
                    rv = -1;
                }
            }
        }
    }

    return rv;
}

int convert_prepaire_src_using_res(convert_ctx_t ctx) {
    ui_cache_group_t res_group;
    int rv;
    
    res_group = ui_cache_group_create(ctx->m_cache_mgr);
    if (res_group == NULL) {
        CPE_ERROR(ctx->m_em, "convert_prepaire_src_using_res_one: create res group fail!");
        return -1;
    }

    rv = convert_prepaire_src_using_res_one(ui_data_mgr_src_root(ctx->m_data_mgr), ctx, res_group);
    
    ui_cache_group_free(res_group);

    return rv;
}
