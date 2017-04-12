#ifndef UI_MODEL_TOOL_CONVERT_CTX_H
#define UI_MODEL_TOOL_CONVERT_CTX_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "cpe/cfg/cfg.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_group.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package_manip/plugin_package_manip_types.h"
#include "plugin/pack/plugin_pack_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct convert_ctx * convert_ctx_t;
typedef struct convert_ctx_res_to_src * convert_ctx_res_to_src_t;
typedef struct convert_language * convert_language_t;
typedef struct convert_pack_phase * convert_pack_phase_t;

typedef TAILQ_HEAD(convert_language_list, convert_language) convert_language_list_t;
typedef TAILQ_HEAD(convert_pack_phase_list, convert_pack_phase) convert_pack_phase_list_t;    
    
struct convert_ctx {
    gd_app_context_t m_app;
    error_monitor_t m_em;
    mem_allocrator_t m_alloc;
    char * m_output;
    char * m_root;
    const char * m_pack_way;
    uint32_t m_texture_limit_width;
    uint32_t m_texture_limit_height;
    const char * m_texture_compress;
    uint8_t m_is_full;
    uint8_t m_is_check_res_duplicate;
    uint8_t m_language_count;
    convert_language_list_t m_languages;
    ui_data_mgr_t m_data_mgr;
    ui_cache_manager_t m_cache_mgr;
    plugin_pack_module_t m_pack_module;
    plugin_package_module_t m_package_module;
    plugin_package_manip_t m_package_manip;
    cfg_t m_package_def;
    cfg_t m_todo;
    
    cfg_t m_runing;
    ui_cache_group_t m_ignore_textures;
    ui_data_src_group_t m_ignore_srcs;

    /*tiny png*/
    const char * m_tinypng_accounts;
    uint8_t m_tinypng_init;
    char * m_tinypng_cache;
    cfg_t m_tinypng_data;
    
    /*pack*/
    plugin_package_package_t m_global_package;
    convert_pack_phase_list_t m_pack_phases;

    /**/
    struct cpe_hash_table m_res_to_srcs;
};

/*init*/
int convert_ctx_init(
    convert_ctx_t ctx,
    gd_app_context_t app, const char * input, const char * to,
    uint32_t texture_limit_width, uint32_t texture_limit_height, 
    const char * texture_compress, const char * tinypng_accounts,
    const char * * languages, uint32_t language_count,
    const char * pack_way,
    uint8_t is_full,
    error_monitor_t em);
void convert_ctx_fini(convert_ctx_t ctx);
    
/*load*/
int convert_load_etc(convert_ctx_t ctx);
int convert_load_ui_def(convert_ctx_t ctx);
int convert_load_res_cfg(convert_ctx_t ctx);
int convert_load_package_def(convert_ctx_t ctx);
int convert_load_todo(convert_ctx_t ctx);
int convert_load_textures(convert_ctx_t ctx);
int convert_load_from_phases(convert_ctx_t ctx);
int convert_load_from_click_audio(convert_ctx_t ctx);

/*save */
int convert_save_runing(convert_ctx_t ctx);
int convert_save_languages(convert_ctx_t ctx);
int convert_save_packages(convert_ctx_t ctx);
int convert_save_summary(convert_ctx_t ctx);

/*prepaire*/
int convert_prepaire_update_refs(convert_ctx_t ctx);
int convert_prepaire_src_using_res(convert_ctx_t ctx);

/*package*/
int convert_op_package(convert_ctx_t ctx);
int convert_op_remove_ignore(convert_ctx_t ctx, ui_data_src_t src);
    
/*build */
int convert_build_language(convert_ctx_t ctx);

int convert_rename_texture_src(convert_ctx_t ctx);
int convert_op_todo_search(convert_ctx_t ctx);
    
/*check*/
int convert_check_resources(convert_ctx_t ctx);
int convert_check_srcs(convert_ctx_t ctx);
int convert_check_ignores(convert_ctx_t ctx);
int convert_check_packages_no_region(convert_ctx_t ctx);


/*spack*/
const char * convert_save_package_spack_prepaire(
    convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx);
    
int convert_save_package_spack_commit(
    convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx, uint8_t success);
    
/*tiny png*/
int convert_tinypng_convert(convert_ctx_t ctx, mem_buffer_t input, mem_buffer_t output, uint8_t * use_cache);
    
/*utils*/
int convert_save_src(convert_ctx_t ctx, const char * output, ui_data_src_t src);
int convert_save_res(convert_ctx_t ctx, const char * output, ui_cache_res_t res);

uint8_t convert_ctx_is_res_ignore(convert_ctx_t ctx, ui_cache_res_t res);
uint8_t convert_ctx_is_src_ignore(convert_ctx_t ctx, ui_data_src_t src);
uint8_t convert_ctx_is_src_ignore_by_path(convert_ctx_t ctx, const char * res_path);
    
#ifdef __cplusplus
}
#endif

#endif
