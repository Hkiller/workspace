#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_visitor.h"
#include "cpe/cfg/cfg.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_language.h"
#include "render/model/ui_data_src.h"
#include "ui_proj_loader_i.h"

static vfs_visitor_next_op_t ui_data_proj_load_on_file(vfs_mgr_t vfs, const char * full, const char * base, void * ctx);
static int ui_data_proj_load_languages(ui_proj_loader_t loader, ui_data_mgr_t mgr, cfg_t project_cfg);

struct ui_data_proj_load_ctx {
    gd_app_context_t m_app;
    ui_data_mgr_t m_mgr;
    size_t m_root_size;
    int m_load_product;
    error_monitor_t m_em;
    struct mem_buffer m_buffer;
};


int ui_data_proj_loader_load(ui_proj_loader_t loader, ui_data_mgr_t mgr, int load_product) {
    vfs_mgr_t vfs = gd_app_vfs_mgr(ui_data_mgr_app(mgr));
    struct vfs_visitor dir_walker;
    struct ui_data_proj_load_ctx ctx;
    int ret = 0;
    char path_buf[256];
    cfg_t project_cfg = NULL;

    project_cfg = cfg_create(loader->m_alloc);
    if (project_cfg == NULL) {
        CPE_ERROR(loader->m_em, "ui_data_proj_load: alloc cfg fail!");
        return -1;
    }
    
    snprintf(path_buf, sizeof(path_buf), "%s/project.yml", loader->m_root);
    if (cfg_yaml_read_file(project_cfg, vfs, path_buf, cfg_merge_use_new, loader->m_em) != 0) {
        CPE_ERROR(loader->m_em, "convert_load_package_def: read from %s fail!", path_buf);
        return -1;
    }

    if (ui_data_proj_load_languages(loader, mgr, project_cfg) != 0) ret = -1;
    
    ui_data_proj_loader_set_load_to_data_mgr(loader, mgr);

    dir_walker.on_dir_enter = NULL;
    dir_walker.on_dir_leave = NULL;
    dir_walker.on_file = ui_data_proj_load_on_file;

    ctx.m_app = loader->m_app;
    ctx.m_mgr = mgr;
    ctx.m_root_size = strlen(loader->m_root);
    ctx.m_load_product = load_product;
    mem_buffer_init(&ctx.m_buffer, NULL);

    if (loader->m_em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, loader->m_em, cpe_error_save_last_errno, &ret);

        ctx.m_em = loader->m_em;
        vfs_search_dir(vfs, &dir_walker, &ctx, loader->m_root, 20);

        CPE_DEF_ERROR_MONITOR_REMOVE(logError, loader->m_em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);

        ctx.m_em = &logError;
        vfs_search_dir(vfs, &dir_walker, &ctx, loader->m_root, 20);
    }

    mem_buffer_clear(&ctx.m_buffer);

    cfg_free(project_cfg);
    return ret;
}

static int ui_data_proj_load_languages(ui_proj_loader_t loader, ui_data_mgr_t mgr, cfg_t project_cfg) {
    struct cfg_it language_it;
    cfg_t language_cfg;
    int rv = 0;
    
    cfg_it_init(&language_it, cfg_find_cfg(project_cfg, "languages"));
    while((language_cfg = cfg_it_next(&language_it))) {
        const char * name = cfg_as_string(language_cfg, NULL);

        if (name == NULL) {
            CPE_ERROR(loader->m_em, "ui_data_proj_load: load languages: name config format error!");
            rv = -1;
            continue;
        }

        if (ui_data_language_create(mgr, name) == NULL) {
            CPE_ERROR(loader->m_em, "ui_data_proj_load: load languages: create language %s fail!", name);
            rv = -1;
            continue;
        }
    }

    return rv;
}

static int ui_data_proj_load_meta_info(ui_data_src_t src, const char * full, struct ui_data_proj_load_ctx * ctx) {
    vfs_mgr_t vfs = gd_app_vfs_mgr(ui_data_mgr_app(ui_data_src_mgr(src)));
    const char * meta_file_name;
    vfs_file_t meta_file;
    char data_buf[64];
    ssize_t data_len;
    uint32_t id;

    mem_buffer_clear_data(&ctx->m_buffer);

    if (mem_buffer_strcat(&ctx->m_buffer, full) != 0
        || mem_buffer_strcat(&ctx->m_buffer, ".meta") != 0
        || (meta_file_name = mem_buffer_make_continuous(&ctx->m_buffer, 0)) == NULL)
    {
        CPE_ERROR(ctx->m_em, "build meta file path fail!");
        return -1;
    }

    meta_file = vfs_file_open(vfs, meta_file_name, "r");
    if (meta_file == NULL) {
        if (errno == ENOENT) {
            CPE_INFO(ctx->m_em, "meta file %s not exist!", meta_file_name);
            return 0;
        }
        else {
            CPE_ERROR(
                ctx->m_em, "open meta file %s fail, error=%d (%s)!",
                meta_file_name, errno, strerror(errno));
            return -1;
        }
    }
    
    data_len = vfs_file_read(meta_file, data_buf, sizeof(data_buf));
    if (data_len < 0) {
        CPE_ERROR(
            ctx->m_em, "read meta file %s fail, error=%d (%s)!",
            meta_file_name, errno, strerror(errno));
        vfs_file_close(meta_file);
        return -1;
    }
    data_buf[data_len] = 0;
    
    sscanf(data_buf, FMT_UINT32_T, &id);

    if (ui_data_src_set_id(src, id) != 0) {
        CPE_ERROR(ctx->m_em, "set src id %d fail!", id);
        vfs_file_close(meta_file);
        return -1;
    }

    vfs_file_close(meta_file);
    return 0;
}

static vfs_visitor_next_op_t ui_data_proj_load_on_file(vfs_mgr_t vfs, const char * full, const char * base, void * iprojut_ctx) {
    struct ui_data_proj_load_ctx * ctx = iprojut_ctx;
    ui_data_src_t src;

    const char * suffix = file_name_suffix(base);

    CPE_ERROR_SET_FILE(ctx->m_em, full);
    CPE_ERROR_SET_LINE(ctx->m_em, 0);

    if (strcmp(suffix, "ibk") == 0) {
        if ((src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_module, full))) {
            if (ui_data_proj_load_meta_info(src, full, ctx) != 0) {
                ui_data_src_free(src);
                src = NULL;
            }
        }
    }
    else if (strcmp(suffix, "frm") == 0) {
        if ((src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_sprite, full))) {
            if (ui_data_proj_load_meta_info(src, full, ctx) != 0) {
                ui_data_src_free(src);
                src = NULL;
            }
        }
    }
    else if (strcmp(suffix, "act") == 0) {
        if ((src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_action, full))) {
            if (ui_data_proj_load_meta_info(src, full, ctx) != 0) {
                ui_data_src_free(src);
                src = NULL;
            }
        }
    }
    else if (strcmp(suffix, "lay") == 0 || strcmp(suffix, "npTemplate") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_layout, full);
    }
    else if (strcmp(suffix, "particle") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_particle, full);
    }
    else if (strcmp(suffix, "spine") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_spine_skeleton, full);
    }
    else if (strcmp(suffix, "spine-state") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_spine_state_def, full);
    }
    else if (strcmp(suffix, "barrage") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_barrage, full);
    }
    else if (strcmp(suffix, "moving") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_moving_plan, full);
    }
    else if (strcmp(suffix, "chipmunk") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_chipmunk_scene, full);
    }
    else if (strcmp(suffix, "map") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_tiledmap_scene, full);
    }
    else if (strcmp(suffix, "scrollmap") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_scrollmap_scene, full);
    }
    else if (strcmp(suffix, "swf") == 0) {
        src = ui_data_src_create_file(ctx->m_mgr, ui_data_src_type_swf, full);
    }
    else {
        src = NULL;
    }

    if (ctx->m_load_product && src) {
        ui_data_src_load(src, ctx->m_em);
    }

    CPE_ERROR_SET_FILE(ctx->m_em, NULL);
    CPE_ERROR_SET_LINE(ctx->m_em, -1);

    return vfs_visitor_next_go;
}
