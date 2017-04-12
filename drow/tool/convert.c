#include <assert.h>
#include "cpe/pal/pal_unistd.h"
#include "cpe/utils/string_utils.h"
#include "ops.h"
#include "convert_ctx.h"

int do_convert_model(
    gd_app_context_t app, 
    const char * input, const char * to, const char * format,
    uint32_t texture_limit_width, uint32_t texture_limit_height, 
    const char * texture_compress, const char * tinypng_accounts,
    const char * * languages, uint32_t language_count,
    const char * pack_way,
    uint8_t is_full,
    error_monitor_t em)
{
    ui_data_mgr_t data_mgr = ui_data_mgr_find_nc(app, NULL);
    assert(data_mgr);

    if (strcmp(format, "proj") == 0) {
        if (ui_data_mgr_save(data_mgr, to) != 0) {
            CPE_ERROR(em, "convert_model: proj: format %s save error!", format);
            return -1;
        }
    }
    else if (strcmp(format, "bin") == 0) {
        char output_dir[256];
        struct convert_ctx ctx;
        int rv = 0;
        
        chdir(input);

        cpe_str_dup(output_dir, sizeof(output_dir), to);
        if (output_dir[0] && output_dir[strlen(output_dir) - 1] != '/') {
            output_dir[strlen(output_dir) + 1] = 0;
            output_dir[strlen(output_dir)] = '/';
        }

        /*加载数据 */
        if (convert_ctx_init(
                &ctx, app, input, output_dir,
                texture_limit_width, texture_limit_height,
                texture_compress, tinypng_accounts,
                languages, language_count, pack_way, is_full, em) != 0) return -1;
        
        if (convert_load_textures(&ctx) != 0) rv = -1;
        if (convert_load_etc(&ctx) != 0) rv = -1;
        if (convert_load_ui_def(&ctx) != 0) rv = -1;
        if (convert_load_package_def(&ctx) != 0) rv = -1;
        if (convert_load_todo(&ctx) != 0) rv = -1;
        if (convert_load_res_cfg(&ctx) != 0) rv = -1;
        if (convert_load_from_phases(&ctx) != 0) rv = -1;
        if (convert_load_from_click_audio(&ctx) != 0) rv = -1;
        
        /*打包前资源预处理 */
        if (convert_prepaire_update_refs(&ctx) != 0) rv = -1;
        if (convert_prepaire_src_using_res(&ctx) != 0) rv = -1;
        
        /*打包资源 */
        if (convert_op_package(&ctx) != 0) rv = -1;

        /*构建运行数据 */
        if (convert_build_language(&ctx) != 0) rv = -1;
        
        /*检查 */
        if (convert_op_todo_search(&ctx) != 0) rv = -1;
        if (convert_check_srcs(&ctx) != 0) rv = -1;
        if (convert_check_resources(&ctx) != 0) rv = -1;
        if (convert_check_ignores(&ctx) != 0) rv = -1;
        convert_check_packages_no_region(&ctx);

        /*修改数据并保存数据 */
        if (convert_rename_texture_src(&ctx) != 0) rv = -1;
        if (convert_save_packages(&ctx) != 0) rv = -1;
        if (convert_save_languages(&ctx) != 0) rv = -1;
        if (convert_save_runing(&ctx) != 0) rv = -1;
        if (convert_save_summary(&ctx) != 0) rv = -1;

        convert_ctx_fini(&ctx);
        return rv;
    }
    else {
        CPE_ERROR(em, "convert_model: format %s unknown!", format);
        return -1;
    }

    return 0;
}
