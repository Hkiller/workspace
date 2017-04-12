#include "cpe/vfs/vfs_dir.h"
#include "render/model/ui_data_evt_collector.h"
#include "convert_ctx.h"

int convert_load_etc(convert_ctx_t ctx) {
    char path_buf[512];
    
    snprintf(path_buf, sizeof(path_buf), "%s/../etc", ctx->m_root);
    if (cfg_read_dir(gd_app_cfg(ctx->m_app), gd_app_vfs_mgr(ctx->m_app), path_buf, cfg_merge_use_new, ctx->m_em, gd_app_alloc(ctx->m_app)) != 0) {
        CPE_ERROR(ctx->m_em, "convert_ctx_load_etc: read from %s fail!", path_buf);
        return -1;
    }

    snprintf(path_buf, sizeof(path_buf), "%s/../meta/data", ctx->m_root);
    if (vfs_dir_exist(gd_app_vfs_mgr(ctx->m_app), path_buf)) {
        cfg_t meta_cfg = cfg_struct_add_struct(gd_app_cfg(ctx->m_app), "meta", cfg_replace);
        if (meta_cfg == NULL) {
            CPE_ERROR(ctx->m_em, "convert_ctx_load_etc: create meta cfg fail!");
            return -1;
        }

        if (cfg_read_dir(meta_cfg, gd_app_vfs_mgr(ctx->m_app), path_buf, cfg_merge_use_new, ctx->m_em, gd_app_alloc(ctx->m_app)) != 0) {
            CPE_ERROR(ctx->m_em, "convert_ctx_load_etc: read from %s fail!", path_buf);
            return -1;
        }
    }
    
    return 0;
}

int convert_load_ui_def(convert_ctx_t ctx) {
    char path_buf[512];
    cfg_t ui_root = cfg_struct_add_struct(ctx->m_runing, "ui", cfg_merge_use_new);
    
    snprintf(path_buf, sizeof(path_buf), "%s/runtime/ui", ctx->m_root);
    if (cfg_read_dir(ui_root, gd_app_vfs_mgr(ctx->m_app), path_buf, cfg_merge_use_new, ctx->m_em, gd_app_alloc(ctx->m_app)) != 0) {
        CPE_ERROR(ctx->m_em, "convert_ctx_load_ui_def: read from %s fail!", path_buf);
        return -1;
    }

    return 0;
}

int convert_load_package_def(convert_ctx_t ctx) {
    char path_buf[512];
    struct cfg_it evt_res_it;
    cfg_t evt_res;
    int rv = 0;
    
    snprintf(path_buf, sizeof(path_buf), "%s/package.yml", ctx->m_root);
    if (cfg_yaml_read_file(ctx->m_package_def, gd_app_vfs_mgr(ctx->m_app), path_buf, cfg_merge_use_new, ctx->m_em) != 0) {
        CPE_ERROR(ctx->m_em, "convert_load_package_def: read from %s fail!", path_buf);
        return -1;
    }

    cfg_it_init(&evt_res_it, cfg_find_cfg(ctx->m_package_def, "evt-resources"));
    while((evt_res = cfg_it_next(&evt_res_it))) {
        const char * evt_name = cfg_name(evt_res);
        const char * evt_arg = cfg_as_string(evt_res, NULL);

        if (evt_arg == NULL) {
            CPE_ERROR(ctx->m_em, "convert_load_package_def: evt-resources format error!");
            rv = -1;
            continue;
        }

        if (ui_data_evt_collector_create(ctx->m_data_mgr, evt_name, evt_arg) == NULL) {
            CPE_ERROR(ctx->m_em, "convert_load_package_def: create convertor %s ==> %s fail!", evt_name, evt_arg);
            rv = -1;
            continue;
        }
    }
    
    return rv;
}

int convert_load_todo(convert_ctx_t ctx) {
    char path_buf[512];
    
    snprintf(path_buf, sizeof(path_buf), "%s/todo.yml", ctx->m_root);
    if (cfg_yaml_read_file(ctx->m_todo, gd_app_vfs_mgr(ctx->m_app), path_buf, cfg_merge_use_new, ctx->m_em) != 0) {
        CPE_ERROR(ctx->m_em, "convert_load_todo: read from %s fail!", path_buf);
        return -1;
    }

    return 0;
}
