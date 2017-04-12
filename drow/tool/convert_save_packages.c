#include <assert.h>
#include "cpe/utils/tsort.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_file.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_group.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package/plugin_package_region.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "convert_ctx.h"

typedef struct convert_packer * convert_packer_t;
static int convert_save_package_summary(convert_ctx_t ctx, const char * output_path, plugin_package_package_t package);
static int convert_save_package_one(convert_ctx_t ctx, mem_buffer_t path_buffer, convert_packer_t packer, plugin_package_package_t package);

/*native*/
static const char * convert_save_package_native_prepaire(
    convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx);
static int convert_save_package_native_commit(
    convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx, uint8_t success);

/*packer*/
static struct convert_packer {
    const char * m_name;
    const char * (*package_prepaire)(convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx);
    int (*package_commit)(convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx, uint8_t success);
} s_packers[] = {
    { "native", convert_save_package_native_prepaire, convert_save_package_native_commit },
    { "spack", convert_save_package_spack_prepaire, convert_save_package_spack_commit },
};

int convert_save_packages(convert_ctx_t ctx) {
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    plugin_package_region_t region;
    plugin_package_group_t saved_packages;
    int rv = 0;
    struct mem_buffer path_buffer;
    char * output_path;
    convert_packer_t packer = NULL;

    if (ctx->m_pack_way == NULL) {
        packer = &s_packers[0];
    }
    else {
        uint8_t i;
        for(i = 0; i < CPE_ARRAY_SIZE(s_packers); ++i) {
            if (strcmp(s_packers[i].m_name, ctx->m_pack_way) == 0) {
                packer = &s_packers[i];
            }
        }

        if (packer == NULL) {
            CPE_ERROR(ctx->m_em, "convert_save_packages: pack way %s not support!", ctx->m_pack_way);
            return -1;
        }
    }
    
    mem_buffer_init(&path_buffer, NULL);

    mem_buffer_printf(&path_buffer, "%s/packages", ctx->m_output);
    output_path = mem_buffer_make_continuous(&path_buffer, 0);
    if (dir_rm_recursion(output_path, ctx->m_em, ctx->m_alloc) != 0) rv = -1;

    saved_packages = plugin_package_group_create(ctx->m_package_module, "saved_packages");
    if (saved_packages == NULL) {
        CPE_ERROR(ctx->m_em, "convert_save_packages: create saved packages group fail");
        mem_buffer_clear(&path_buffer);
        return -1;
    }

    if (ctx->m_is_full) {
        struct plugin_package_region_it region_it;
        
        plugin_package_module_regions(ctx->m_package_module, &region_it);
        while((region = plugin_package_region_it_next(&region_it))) {
            
            plugin_package_group_packages(&package_it, plugin_package_region_group(region));
            while((package = plugin_package_package_it_next(&package_it))) {
                if (plugin_package_package_is_in_group(package,  saved_packages)) continue;

                plugin_package_group_add_package(saved_packages, package);
                if (convert_save_package_one(ctx, &path_buffer, packer, package) != 0) rv = -1;
            }
        }
    }
    else {
        struct cfg_it dft_region_it;
        cfg_t dft_region;

        cfg_it_init(&dft_region_it, cfg_find_cfg(ctx->m_package_def, "default-regions"));
        while((dft_region = cfg_it_next(&dft_region_it))) {
            const char * region_name = cfg_as_string(dft_region, NULL);

            if (region_name == NULL) {
                CPE_ERROR(ctx->m_em, "convert_save_packages: default region format error");
                rv = -1;
                continue;
            }

            region = plugin_package_region_find(ctx->m_package_module, region_name);
            if (region == NULL) {
                CPE_ERROR(ctx->m_em, "convert_save_packages: region %s not exist", region_name);
                rv = -1;
                continue;
            }

            plugin_package_group_packages(&package_it, plugin_package_region_group(region));
            while((package = plugin_package_package_it_next(&package_it))) {
                if (plugin_package_package_is_in_group(package,  saved_packages)) continue;

                plugin_package_group_add_package(saved_packages, package);
                if (convert_save_package_one(ctx, &path_buffer, packer, package) != 0) rv = -1;
            }
        }
    }
    
    plugin_package_group_free(saved_packages);
    
    mem_buffer_clear(&path_buffer);
    
    return rv;
}

static int convert_save_package_summary_base_packages_add(
    convert_ctx_t ctx, plugin_package_package_t package, cfg_t base_seq)
{
    struct plugin_package_package_it base_package_it;
    plugin_package_package_t base_package;
    int rv = 0;
    
    plugin_package_package_base_packages(package, &base_package_it);
    while((base_package = plugin_package_package_it_next(&base_package_it))) {
        if (cfg_seq_add_string(base_seq, plugin_package_package_name(base_package)) == NULL) rv = -1;
    }
    
    return rv;
}

static int convert_save_package_summary_srcs_add(
    convert_ctx_t ctx, plugin_package_package_t package, cfg_t src_root, ui_data_src_group_t group, const char * language)
{
    tsorter_str_t sorter;
    struct tsorter_str_it sorter_it;
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    int rv = 0;
    const char * in_sorter_src_id;
    
    sorter = tsorter_str_create(ctx->m_alloc, 1);
    if (sorter == NULL) {
        CPE_ERROR(ctx->m_em, "convert_save_package_summary_srcs: create sorter fail!");
        return -1;
    }
    
    ui_data_src_group_srcs(&src_it, plugin_package_package_srcs(package));
    while((src = ui_data_src_it_next(&src_it))) {
        char src_id[64];
        struct ui_data_src_it dep_src_it;
        ui_data_src_t dep_src;
        
        snprintf(src_id, sizeof(src_id), "%p", src);
        if (tsorter_str_add_element(sorter, src_id) != 0) {
            rv = -1;
            continue;
        }
        
        ui_data_src_using_srcs(src, &dep_src_it);
        while((dep_src = ui_data_src_it_next(&dep_src_it))) {
            char dep_src_id[64];

            if (!ui_data_src_in_group(dep_src, plugin_package_package_srcs(package))) continue;
            
            snprintf(dep_src_id, sizeof(dep_src_id), "%p", dep_src);

            if (tsorter_str_add_dep(sorter, src_id, dep_src_id) != 0) rv = -1;
        }
    }

    if (tsorter_str_sort(&sorter_it, sorter) != 0) {
        CPE_ERROR(ctx->m_em, "convert_save_package_summary_srcs: src depend circle!");
        rv = -1;
    }

    while((in_sorter_src_id = tsorter_str_next(&sorter_it))) {
        cfg_t src_info;
        
        sscanf(in_sorter_src_id, "%p", &src);

        src_info = cfg_seq_add_struct(src_root);

        cfg_struct_add_string(src_info, "path", ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), cfg_replace);
        cfg_struct_add_uint8(src_info, "type", ui_data_src_type(src), cfg_replace);

        if (ui_data_src_id(src) != (uint32_t)-1) {
            cfg_struct_add_uint32(src_info, "id", ui_data_src_id(src), cfg_replace);
        }
    }
    
    tsorter_str_free(sorter);
    
    return rv;
}

static int convert_save_package_summary_resources_add(
    convert_ctx_t ctx, plugin_package_package_t package, cfg_t res_seq, ui_cache_group_t group, const char * language)
{
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    int rv = 0;
    
    ui_cache_group_using_resources(&res_it, plugin_package_package_resources(package));
    while((res = ui_cache_res_it_next(&res_it))) {
        const char * path = ui_cache_res_path(res);
        cfg_t file_info;

        file_info = cfg_seq_add_struct(res_seq);
        cfg_struct_add_uint8(file_info, "type", ui_cache_res_type(res), cfg_merge_use_new);
        if (language) cfg_struct_add_string(file_info, "language", language, cfg_merge_use_new);
        
        if (ui_cache_res_type(res) == ui_cache_res_type_texture) {
            char * update_path;
            
            if (ui_cache_res_load_state(res) != ui_cache_res_loaded) {
                if (ui_cache_res_load_sync(res, NULL)) {
                    CPE_ERROR(
                        ctx->m_em, "save pachage %s: save summary: res %s load fail!",
                        plugin_package_package_name(package), path);
                    rv = -1;
                    cfg_free(file_info);
                    continue;
                }
            }

            /*修改图片后缀名 */
            mem_buffer_clear_data(gd_app_tmp_buffer(ctx->m_app));
            update_path = mem_buffer_strdup(gd_app_tmp_buffer(ctx->m_app), path);

            if (file_name_rename_suffix(update_path, mem_buffer_size(gd_app_tmp_buffer(ctx->m_app)), "pzd") != 0) {
                CPE_ERROR(
                    ctx->m_em, "save pachage %s: save summary: res %s rename to pzd fail!",
                    plugin_package_package_name(package), path);
                rv = -1;
                cfg_free(file_info);
                continue;
            }
            
            cfg_struct_add_string(file_info, "path", update_path, cfg_merge_use_new);
            cfg_struct_add_uint32(file_info, "width", ui_cache_texture_width(res), cfg_merge_use_new);
            cfg_struct_add_uint32(file_info, "height", ui_cache_texture_height(res), cfg_merge_use_new);
        }
        else {
            cfg_struct_add_string(file_info, "path", path, cfg_merge_use_new);
        }
    }

    return rv;
}

static int convert_save_package_summary(convert_ctx_t ctx, const char * output_path, plugin_package_package_t package) {
    struct mem_buffer path_buff;
    struct mem_buffer write_buff;
    vfs_file_t output_file = NULL;
    cfg_t package_cfg = NULL;
    cfg_t base_seq;
    cfg_t src_seq;
    cfg_t res_seq;
    ssize_t size;
    int rv = 0;

    mem_buffer_init(&path_buff, NULL);
    mem_buffer_init(&write_buff, NULL);

    /*构建 */
    package_cfg = cfg_create(ctx->m_alloc);
    if (package_cfg == NULL) {
        CPE_ERROR(
            ctx->m_em, "save pachage %s: save summary: create cfg fail",
            plugin_package_package_name(package));
        rv = -1;
        goto SAVE_ERROR;
    }

    base_seq = cfg_struct_add_seq(package_cfg, "base-packages", cfg_replace);
    assert(base_seq);
    src_seq = cfg_struct_add_seq(package_cfg, "src-root", cfg_replace);
    assert(src_seq);
    res_seq = cfg_struct_add_seq(package_cfg, "resources", cfg_replace);
    assert(res_seq);

    /*构建公共资源 */
    if (convert_save_package_summary_base_packages_add(ctx, package, base_seq) != 0) rv = -1;
    if (convert_save_package_summary_srcs_add(ctx, package, src_seq, plugin_package_package_srcs(package), NULL) != 0) rv = -1;
    if (convert_save_package_summary_resources_add(ctx, package, res_seq, plugin_package_package_resources(package), NULL) != 0) rv = -1;

    /*构建语言相关资源 */
    
    /*存盘 */
    size = cfg_bin_write_to_buffer(&write_buff, package_cfg, ctx->m_em);
    if (size < 0) {
        CPE_ERROR(ctx->m_em, "convert_ctx_save_runing: write to bin fail, rv=%d!", (int)size);
        rv = -1;
        goto SAVE_ERROR;
    }
    assert(mem_buffer_size(&write_buff) == (size_t)size);

    mem_buffer_printf(&path_buff, "%s/package.bin", output_path);

    output_file = vfs_file_open(gd_app_vfs_mgr(ctx->m_app), mem_buffer_make_continuous(&path_buff, 0), "wb");
    if (output_file == NULL) {
        CPE_ERROR(
            ctx->m_em, "save pachage %s: save summary: create file %s fail!",
            plugin_package_package_name(package), 
            mem_buffer_make_continuous(&path_buff, 0));
        rv = -1;
        goto SAVE_ERROR;
    }

    if (vfs_file_write_from_buffer(output_file, &write_buff) < 0) {
        CPE_ERROR(
            ctx->m_em, "save pachage %s: save summary: write to %s fail!",
            plugin_package_package_name(package), 
            mem_buffer_make_continuous(&path_buff, 0));
        rv = -1;
        goto SAVE_ERROR;
    }

    /* do { */
    /*     struct vfs_write_stream ws; */
    /*     snprintf(path_buf, sizeof(path_buf), "%s/runtime.yml", ctx->m_output); */
    /*     output_file = vfs_file_open(gd_app_vfs_mgr(ctx->m_app), path_buf, "wb"); */
    /*     vfs_write_stream_init(&ws, output_file); */
    /*     cfg_yaml_write((write_stream_t)&ws, ctx->m_runing, ctx->m_em); */
    /*     vfs_file_close(output_file); */
    /* } while(0); */
    
SAVE_ERROR:    
    if (package_cfg) cfg_free(package_cfg);
    if (output_file) vfs_file_close(output_file);
    mem_buffer_clear(&write_buff);
    mem_buffer_clear(&path_buff);
    
    return rv;
}

static int convert_save_package_one(convert_ctx_t ctx, mem_buffer_t path_buffer, convert_packer_t packer, plugin_package_package_t package) {
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    int rv = 0;
    const char * output_path;
    void * packer_ctx = NULL;
    
    output_path = packer->package_prepaire(ctx, path_buffer, package, &packer_ctx);
    if (output_path == NULL) return -1;

    /*保持Package的基本信息 */
    if (convert_save_package_summary(ctx, output_path, package) != 0) {
        rv = -1;
        goto PACKAGE_ONE_COMPLETE;
    }
        
    /*保存Src */
    ui_data_src_group_srcs(&src_it, plugin_package_package_srcs(package));
    while((src = ui_data_src_it_next(&src_it))) {
        if (convert_save_src(ctx, output_path, src) != 0) {
            rv = -1;
            continue;
        }
    }

    /*保存Texture */
    ui_cache_group_using_resources(&res_it, plugin_package_package_resources(package));
    while((res = ui_cache_res_it_next(&res_it))) {
        if (convert_save_res(ctx, output_path, res) != 0) {
            rv = -1;
            continue;
        }
    }

PACKAGE_ONE_COMPLETE:
    if (packer->package_commit(ctx, path_buffer, package, &packer_ctx, rv == 0 ? 1 : 0) != 0) rv = -1;
    
    return rv;
}

/*native*/
static const char * convert_save_package_native_prepaire(
    convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx)
{
    const char * output_path;
    
    mem_buffer_printf(path_buffer, "%s/packages/%s", ctx->m_output, plugin_package_package_name(package));
    output_path = mem_buffer_make_continuous(path_buffer, 0);
    if (vfs_dir_mk_recursion(gd_app_vfs_mgr(ctx->m_app), output_path) != 0) {
        CPE_ERROR(ctx->m_em, "save pachage %s: mkdir %s fail", plugin_package_package_name(package), output_path);
        return NULL;
    }
    
    return output_path;
}

static int convert_save_package_native_commit(
    convert_ctx_t ctx, mem_buffer_t path_buffer, plugin_package_package_t package, void * * builder_ctx, uint8_t success)
{
    return 0;
}
