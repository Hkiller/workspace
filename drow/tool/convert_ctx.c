#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "cpe/utils/file.h"
#include "convert_ctx.h"
#include "plugin/pack/plugin_pack_module.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "convert_ctx_res_to_src.h"
#include "convert_language.h"
#include "convert_pack_phase.h"

int convert_ctx_init(
    convert_ctx_t ctx,
    gd_app_context_t app, const char * input, const char * to,
    uint32_t texture_limit_width, uint32_t texture_limit_height,
    const char * texture_compress, const char * tinypng_accounts,
    const char * * languages, uint32_t language_count,
    const char * pack_way,
    uint8_t is_full,
    error_monitor_t em)
{
    uint32_t i;
    char buf[128];
    struct ui_data_language_it language_it;
    ui_data_language_t data_language = NULL;
    convert_language_t convert_language;

    ctx->m_app = app;
    ctx->m_em = em;
    ctx->m_alloc = gd_app_alloc(app);
    ctx->m_output = cpe_str_mem_dup(ctx->m_alloc, to);
    file_name_normalize(ctx->m_output);
    ctx->m_root = cpe_str_mem_dup(ctx->m_alloc, input);
    file_name_normalize(ctx->m_root);
    ctx->m_pack_way = pack_way;
    ctx->m_texture_limit_width = texture_limit_width;
    ctx->m_texture_limit_height = texture_limit_height;
    ctx->m_texture_compress = texture_compress;
    ctx->m_is_full = is_full;
    ctx->m_is_check_res_duplicate = 0;
    ctx->m_data_mgr = ui_data_mgr_find_nc(app, NULL);
    assert(ctx->m_data_mgr);
    ctx->m_pack_module = plugin_pack_module_find_nc(app, NULL);
    ctx->m_cache_mgr = ui_cache_manager_find_nc(app, NULL);

    ctx->m_package_module = plugin_package_module_find_nc(app, NULL);
    plugin_package_module_set_control_res(ctx->m_package_module, 0);
    ctx->m_package_manip = plugin_package_manip_find_nc(app, NULL);
    ctx->m_language_count = 0;
    
    assert(ctx->m_cache_mgr);

    ctx->m_tinypng_accounts = tinypng_accounts;
    ctx->m_tinypng_init = 0;
    ctx->m_tinypng_cache = NULL;
    ctx->m_tinypng_data = NULL;

    ctx->m_package_def = cfg_create(NULL);
    assert(ctx->m_package_def);

    ctx->m_todo = cfg_create(NULL);
    assert(ctx->m_todo);
    
    ctx->m_runing = gd_app_cfg(app);
    assert(ctx->m_runing);
    
    ctx->m_ignore_textures = ui_cache_group_create(ctx->m_cache_mgr);
    assert(ctx->m_ignore_textures);

    ctx->m_ignore_srcs = ui_data_src_group_create(ctx->m_data_mgr);
    assert(ctx->m_ignore_srcs);

    TAILQ_INIT(&ctx->m_languages);
    TAILQ_INIT(&ctx->m_pack_phases);

    ctx->m_global_package = NULL;

    ui_data_languages(&language_it, ctx->m_data_mgr);
    while((data_language = ui_data_language_it_next(&language_it))) {
        if (convert_language_create(ctx, data_language) == NULL) {
            convert_ctx_fini(ctx);
            return -1;
        }
    }
    
    if (language_count > 0) {
        for(i = 0; i < language_count; ++i) {
            convert_language = convert_language_find_by_name(ctx, languages[i]);
            if (convert_language == NULL) {
                CPE_ERROR(ctx->m_em, "convert_language_create: language %s unknown!", languages[i]);
                convert_ctx_fini(ctx);
                return -1;
            }

            convert_language->m_is_active = 1;
        }
    }
    else {
        TAILQ_FOREACH(convert_language, &ctx->m_languages, m_next) {
            convert_language->m_is_active = 1;
        }
    }

    snprintf(buf, sizeof(buf), "shared/common");
    ctx->m_global_package = plugin_package_package_create(ctx->m_package_module, buf, plugin_package_package_loaded);
    if (ctx->m_global_package == NULL) {
        CPE_ERROR(ctx->m_em, "convert_pack_phase_pack: create global packer fail!");
        convert_ctx_fini(ctx);
        return -1;
    }

    if (cpe_hash_table_init(
            &ctx->m_res_to_srcs,
            ctx->m_alloc,
            (cpe_hash_fun_t) convert_ctx_res_to_src_hash,
            (cpe_hash_eq_t) convert_ctx_res_to_src_eq,
            CPE_HASH_OBJ2ENTRY(convert_ctx_res_to_src, m_hh),
            -1) != 0)
    {
        CPE_ERROR(ctx->m_em, "convert_pack_phase_pack: init hash table fail!");
        plugin_package_package_free(ctx->m_global_package);
        convert_ctx_fini(ctx);
        return -1;
    }

    return 0;
}

void convert_ctx_fini(convert_ctx_t ctx) {
    convert_ctx_res_to_src_free_all(ctx);
    
    while(!TAILQ_EMPTY(&ctx->m_languages)) {
        convert_language_free(TAILQ_FIRST(&ctx->m_languages));
    }
    assert(ctx->m_language_count == 0);

    while(!TAILQ_EMPTY(&ctx->m_pack_phases)) {
        convert_pack_phase_free(TAILQ_FIRST(&ctx->m_pack_phases));
    }

    if (ctx->m_tinypng_cache) {
        mem_free(ctx->m_alloc, ctx->m_tinypng_cache);
        ctx->m_tinypng_cache = NULL;
    }

    if (ctx->m_tinypng_data) {
        cfg_free(ctx->m_tinypng_data);
        ctx->m_tinypng_data = NULL;
    }

    cfg_free(ctx->m_package_def);
    cfg_free(ctx->m_todo);
    ui_cache_group_free(ctx->m_ignore_textures);
    ui_data_src_group_free(ctx->m_ignore_srcs);

    if (ctx->m_global_package) {
        plugin_package_package_free(ctx->m_global_package);
        ctx->m_global_package = NULL;
    }

    if (ctx->m_root) {
        mem_free(ctx->m_alloc, ctx->m_root);
        ctx->m_root = NULL;
    }

    if (ctx->m_output) {
        mem_free(ctx->m_alloc, ctx->m_output);
        ctx->m_output = NULL;
    }
    
    cpe_hash_table_fini(&ctx->m_res_to_srcs);
}

uint8_t convert_ctx_is_res_ignore(convert_ctx_t ctx, ui_cache_res_t res) {
    struct cfg_it ignore_cfg_it;
    cfg_t ignore_cfg;
    const char * path = ui_cache_res_path(res);

    cfg_it_init(&ignore_cfg_it, cfg_find_cfg(ctx->m_package_def, "ignore"));
    while((ignore_cfg = cfg_it_next(&ignore_cfg_it))) {
        const char * ignore = cfg_as_string(ignore_cfg, NULL);
        if (ignore == NULL) continue;

        if (cpe_str_start_with(path, ignore) && (*(path + strlen(ignore)) == '/')) return 1;
    }

    return 0;
}

uint8_t convert_ctx_is_src_ignore(convert_ctx_t ctx, ui_data_src_t src) {
    return convert_ctx_is_src_ignore_by_path(ctx, ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
}

uint8_t convert_ctx_is_src_ignore_by_path(convert_ctx_t ctx, const char * res_path) {
    struct cfg_it ignore_cfg_it;
    cfg_t ignore_cfg;

    cfg_it_init(&ignore_cfg_it, cfg_find_cfg(ctx->m_package_def, "ignore"));
    while((ignore_cfg = cfg_it_next(&ignore_cfg_it))) {
        const char * ignore = cfg_as_string(ignore_cfg, NULL);
        if (ignore == NULL) continue;

        if (cpe_str_start_with(res_path, ignore) && (*(res_path + strlen(ignore)) == '/')) return 1;
    }

    return 0;
}
