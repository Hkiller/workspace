#include <assert.h>
#include "cpe/utils/tsort.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_group.h"
#include "render/cache/ui_cache_group.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package/plugin_package_region.h"
#include "plugin/package/plugin_package_group.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin/pack/plugin_pack_packer.h"
#include "plugin/pack/plugin_pack_language.h"
#include "convert_ctx.h"
#include "convert_language.h"
#include "convert_ctx_res_to_src.h"

static int convert_op_package_load_and_remove_ignore(convert_ctx_t ctx, plugin_package_package_t package);
static int convert_op_package_check_package_circle(convert_ctx_t ctx, plugin_package_group_t package_in_order);
static int convert_op_package_check_src_duplicate(convert_ctx_t ctx);
static int convert_op_package_check_res_duplicate(convert_ctx_t ctx);
static int convert_op_package_pack_one(convert_ctx_t ctx, plugin_package_package_t package);
static int convert_op_package_pack_one_prepaire_language(convert_ctx_t ctx, plugin_package_package_t package, plugin_pack_packer_t packer);

int convert_op_package(convert_ctx_t ctx) {
    int rv = 0;
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    struct plugin_package_region_it region_it;
    plugin_package_region_t region;
    plugin_package_group_t package_in_order;
    
    /*根据打包规则，构建package */
    if (plugin_package_manip_build(ctx->m_package_manip, cfg_find_cfg(ctx->m_package_def, "package-rules")) != 0) rv = -1;

    /*检查包的循环依赖关系 */
    package_in_order = plugin_package_group_create(ctx->m_package_module, "package_in_order");
    if (package_in_order == NULL) {
        CPE_ERROR(ctx->m_em, "pack: create package in order fail!");
        return -1;
    }
    
    if (convert_op_package_check_package_circle(ctx, package_in_order) != 0) return -1;

    /*将所有的region包含的包补充完整 */
    plugin_package_module_regions(ctx->m_package_module, &region_it);
    while((region = plugin_package_region_it_next(&region_it))) {
        if (plugin_package_group_expand_base_packages(plugin_package_region_group(region)) != 0) rv = -1;
    }
    
    /*确保所有资源加载 */
    plugin_package_module_packages(ctx->m_package_module, &package_it);
    while((package = plugin_package_package_it_next(&package_it))) {
        if (convert_op_package_load_and_remove_ignore(ctx, package) != 0) rv = -1;
        if (ui_data_src_group_collect_ress(plugin_package_package_srcs(package), plugin_package_package_resources(package)) != 0) rv = -1;

        /* do { */
        /*     struct ui_cache_res_it res_it; */
        /*     ui_cache_res_t res; */

        /*     ui_cache_group_using_resources(&res_it, plugin_package_package_resources(package)); */
        /*     while((res = ui_cache_res_it_next(&res_it))) { */
        /*         printf("package %s: res %s\n",  plugin_package_package_name(package), ui_cache_res_path(res)); */
        /*     } */
        /* } while(0); */
    }

    /*移除package中所有由基础包提供的资源 */
    plugin_package_group_packages(&package_it, package_in_order);
    while((package = plugin_package_package_it_next(&package_it))) {
        if (plugin_package_manip_remove_base_provided(package) != 0) rv = -1;

        /* do { */
        /*     struct ui_cache_res_it res_it; */
        /*     ui_cache_res_t res; */

        /*     ui_cache_group_using_resources(&res_it, plugin_package_package_resources(package)); */
        /*     while((res = ui_cache_res_it_next(&res_it))) { */
        /*         printf("package %s: res %s\n",  plugin_package_package_name(package), ui_cache_res_path(res)); */
        /*     } */
        /* } while(0); */
    }

    /*各个phase的资源中，如果有本阶段加载的公共包包含，则从phase包中移除 */
    if (plugin_package_manip_collect_extern_shared(ctx->m_global_package) != 0) rv = -1;

    if (convert_op_package_check_src_duplicate(ctx) != 0) rv = -1;
    
    /*打包所有的package */
    plugin_package_group_packages(&package_it, package_in_order);
    while((package = plugin_package_package_it_next(&package_it))) {
        if (convert_op_package_pack_one(ctx, package) != 0) rv = -1;
    }

    if (ctx->m_is_check_res_duplicate) {
        if (convert_op_package_check_res_duplicate(ctx) != 0) rv = -1;
    }
    
    plugin_package_group_free(package_in_order);
    
    return rv;
}

static uint8_t convert_op_package_is_src_in_extern(plugin_package_package_t package, ui_data_src_t src) {
    struct plugin_package_package_it extern_packages;
    plugin_package_package_t extern_package;
    
    plugin_package_package_extern_packages(package, &extern_packages);
    while((extern_package = plugin_package_package_it_next(&extern_packages))) {
        if (ui_data_src_in_group(src, plugin_package_package_srcs(extern_package))) return 1;
        if (convert_op_package_is_src_in_extern(extern_package, src)) return 1;
    }

    return 0;
}

static int convert_op_package_pack_one(convert_ctx_t ctx, plugin_package_package_t package) {
    plugin_pack_packer_t packer = NULL;
    struct ui_cache_res_it res_it;
    ui_cache_res_t packed_res;
    char buf[128];
    int rv = 0;
    ui_cache_res_t res;
    
    //printf("xxxx: pack %s\n", plugin_package_package_name(package));
    
    /*创建打包对象 */
    snprintf(buf, sizeof(buf), "packed/%s.png", plugin_package_package_name(package));
    packer = plugin_pack_packer_create(ctx->m_pack_module, buf);
    if (packer == NULL) {
        CPE_ERROR(ctx->m_em, "package %s: pack: create packer fail!", plugin_package_package_name(package));
        rv = -1;
        goto PACK_COMPLETE;
    }
    plugin_pack_packer_texture_set_limit_width(packer, ctx->m_texture_limit_width);
    plugin_pack_packer_texture_set_limit_height(packer, ctx->m_texture_limit_height);

    /*搜集需要修资源引用的src */
    if (ui_data_src_group_add_src_by_group(plugin_pack_packer_input_srcs(packer), plugin_package_package_srcs(package)) != 0) {
        CPE_ERROR(ctx->m_em, "package %s: pack: add src to packer fail!", plugin_package_package_name(package));
        rv = -1;
        goto PACK_COMPLETE;
    }
    
    /*收集打包的资源 */
    ui_cache_group_using_resources(&res_it, plugin_package_package_resources(package));
    while((res = ui_cache_res_it_next(&res_it))) {
        struct convert_ctx_res_to_src key;
        convert_ctx_res_to_src_t res_to_src;
        uint32_t res_use_count = 0;
        
        if (ui_cache_res_type(res) != ui_cache_res_type_texture) continue;

        key.m_res = res;
        for(res_to_src = cpe_hash_table_find(&ctx->m_res_to_srcs, &key);
            res_to_src;
            res_to_src = cpe_hash_table_find_next(&ctx->m_res_to_srcs, res_to_src))
        {
            if (ui_data_src_in_group(res_to_src->m_src, plugin_package_package_srcs(package))) {
                res_use_count++;
            }
            else if (convert_op_package_is_src_in_extern(package, res_to_src->m_src)) {
                res_use_count++;
                ui_data_src_group_add_src(plugin_pack_packer_input_srcs(packer), res_to_src->m_src);
            }

            /* printf( */
            /*     "package %s res %s: ==> %s\n", */
            /*     plugin_package_package_name(package), */
            /*     ui_cache_res_path(res), */
            /*     ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), res_to_src->m_src)); */
        }

        if (res_use_count) {
            if (ui_cache_group_add_res(plugin_pack_packer_pack_textures(packer), res) != 0) {
                CPE_ERROR(
                    ctx->m_em, "package %s: pack: add texture %s to packer fail!",
                    plugin_package_package_name(package), ui_cache_res_path(res));
                rv = -1;
                continue;
            }
        }
    }

    /*语言 */
    if (convert_op_package_pack_one_prepaire_language(ctx, package, packer) != 0) {
        rv = -1;
        goto PACK_COMPLETE;
    }
    
    /*打包 */
    if (plugin_pack_packer_pack(packer) != 0) rv = -1;

    /*添加打包结果资源 */
    if (ui_cache_group_add_res_by_cache_group(
            plugin_package_package_resources(package),
            plugin_pack_packer_generated_textures(packer)) != 0) rv = -1;
    
    /*移除已经打包的资源 */
    ui_cache_group_using_resources(&res_it, plugin_pack_packer_pack_textures(packer));
    while((packed_res = ui_cache_res_it_next(&res_it))) {
        ui_cache_group_remove_res(plugin_package_package_resources(package), packed_res);
        ui_cache_group_add_res(ctx->m_ignore_textures, packed_res);
    }

PACK_COMPLETE:
    if (packer) {
        plugin_pack_packer_free(packer);
    }
    
    return 0;
}

static int convert_op_package_pack_one_prepaire_language_src(
    convert_ctx_t ctx, plugin_package_package_t package, plugin_pack_packer_t packer,
    ui_data_src_group_t remove_group, ui_data_src_t base_src)
{
    uint8_t language_src_count = 0;
    convert_language_t convert_language;
    int rv = 0;
    struct ui_data_src_it using_src_it;
    ui_data_src_t using_src;

    if (!(ui_data_src_is_loaded(base_src))) {
        if (ui_data_src_load(base_src, ctx->m_em) != 0) {
            CPE_ERROR(
                ctx->m_em, "convert_prepaire_language_src_one: load src %s fail",
                ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), base_src));
            rv = -1;
        }
    }

    /* printf("xxxxx: process %s: %s\n", */
    /*        ui_data_src_type_name(ui_data_src_type(base_src)), */
    /*        ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), base_src)); */
    
    ui_data_src_using_srcs(base_src, &using_src_it);
    while((using_src = ui_data_src_it_next(&using_src_it))) {
        /* printf("xxxx: using %s: %s\n", */
        /*        ui_data_src_type_name(ui_data_src_type(using_src)), */
        /*        ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), using_src)); */
        if (convert_op_package_pack_one_prepaire_language_src(ctx, package, packer, remove_group, using_src) != 0) rv = -1;
    }
    
    /*准备语言环境 */
    TAILQ_FOREACH(convert_language, &ctx->m_languages, m_next) {
        plugin_pack_language_t pack_language;
        ui_data_src_t language_src;

        if (!convert_language->m_is_active) continue;
        
        language_src = ui_data_language_find_src(convert_language->m_data_language, base_src);
        if (language_src == NULL) continue;

        language_src_count++;
            
        pack_language = plugin_pack_language_find(packer, convert_language->m_data_language);
        if (pack_language == NULL) {
            pack_language = plugin_pack_language_create(packer, convert_language->m_data_language);
            if (pack_language == NULL) {
                CPE_ERROR(ctx->m_em, "convert_op_prepaire_language: create pack language fail!");
                rv = -1;
                continue;
            }
        }

        if (ui_data_src_group_add_src(plugin_pack_language_input_srcs(pack_language), language_src) != 0) {
            CPE_ERROR(ctx->m_em, "convert_op_prepaire_language: add language fail!");
            rv = -1;
            continue;
        }
        /* CPE_ERROR(ctx->m_em, "convert_op_prepaire_language: language add src %s %s!", */
        /*           ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), language_src), */
        /*           ui_data_src_type_name(ui_data_src_type(language_src))); */
    }

    if (language_src_count == 0) return rv;
        
    switch(ui_data_src_type(base_src)) {
    case ui_data_src_type_module:
        break;
    default:
        ui_data_src_group_add_src(remove_group, base_src);
        break;
    }

    return rv;
}

static int convert_op_package_pack_one_prepaire_language(
    convert_ctx_t ctx, plugin_package_package_t package, plugin_pack_packer_t packer)
{
    struct ui_data_src_it src_it;
    ui_data_src_t src;
    convert_language_t convert_language;
    int rv = 0;
    ui_data_src_group_t remove_group = ui_data_src_group_create(ctx->m_data_mgr);
    
    /*设置默认语言 */
    TAILQ_FOREACH(convert_language, &ctx->m_languages, m_next) {
        if (!convert_language->m_is_active) continue;
        plugin_pack_packer_set_default_language(packer, ui_data_language_name(convert_language->m_data_language));
        break;
    }

    /*准备语言相关的资源 */
    ui_data_src_group_srcs(&src_it, plugin_pack_packer_input_srcs(packer));
    while((src = ui_data_src_it_next(&src_it))) {
        if (convert_op_package_pack_one_prepaire_language_src(ctx, package, packer, remove_group, src) != 0) rv = -1;
    }

    ui_data_src_group_srcs(&src_it, remove_group);
    while((src = ui_data_src_it_next(&src_it))) {
        ui_data_src_group_remove_src(plugin_pack_packer_input_srcs(packer), src);
        ui_data_src_group_add_src(ctx->m_ignore_srcs, src);
    }
    ui_data_src_group_free(remove_group);
    
    return rv;
}

static int convert_op_package_check_package_circle(convert_ctx_t ctx, plugin_package_group_t package_in_order) {
    tsorter_str_t sorter;
    struct tsorter_str_it tsorter_it;
    struct plugin_package_package_it package_it;
    plugin_package_package_t package;
    const char * package_name;
    int rv = 0;

    sorter = tsorter_str_create(ctx->m_alloc, 0);
    if (sorter == NULL) {
        CPE_ERROR(ctx->m_em, "convert_op_check_package_circle: create sorter fail!");
        return -1;
    }
        
    plugin_package_module_packages(ctx->m_package_module, &package_it);
    while((package = plugin_package_package_it_next(&package_it))) {
        struct plugin_package_package_it base_package_it;
        plugin_package_package_t base_package;

        if (tsorter_str_add_element(sorter, plugin_package_package_name(package)) != 0) rv = -1;

        plugin_package_package_base_packages(package, &base_package_it);
        while((base_package = plugin_package_package_it_next(&base_package_it))) {
            if (tsorter_str_add_dep(sorter, plugin_package_package_name(package), plugin_package_package_name(base_package)) != 0) rv = -1;
        }
    }

    if (tsorter_str_sort(&tsorter_it, sorter) != 0) {
        CPE_ERROR(ctx->m_em, "convert_op_check_package_circle: package depend circle!");
        rv = -1;
    }

    while((package_name = tsorter_str_next(&tsorter_it))) {
        package = plugin_package_package_find(ctx->m_package_module, package_name);
        assert(package);
        plugin_package_group_add_package(package_in_order, package);
    }
    
    tsorter_str_free(sorter);

    return rv;
}

static int convert_op_package_check_src_duplicate_r(convert_ctx_t ctx, ui_data_src_t src) {
    int rv = 0;
    
    if (ui_data_src_type(src) == ui_data_src_type_dir) {
        struct ui_data_src_it child_it;
        ui_data_src_t child;
        ui_data_src_childs(&child_it, src);

        while((child = ui_data_src_it_next(&child_it))) {
            if (convert_op_package_check_src_duplicate_r(ctx, child) != 0) rv = -1;
        }
    }
    else {
        struct plugin_package_package_it package_it;
        plugin_package_package_t package;
        plugin_package_package_t used_package = NULL;
        
        plugin_package_module_packages(ctx->m_package_module, &package_it);
        while((package = plugin_package_package_it_next(&package_it))) {
            if (!ui_data_src_in_group(src, plugin_package_package_srcs(package))) continue;

            if (used_package) {
                CPE_ERROR(
                    ctx->m_em, "src %s type %s: duplicate in package %s and %s!",
                    ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src), ui_data_src_type_name(ui_data_src_type(src)),
                    plugin_package_package_name(used_package),
                    plugin_package_package_name(package));
                rv = -1;
                continue;
            }
            else {
                used_package = package;
            }
        }
    }

    return rv;
}

static int convert_op_package_check_src_duplicate(convert_ctx_t ctx) {
    convert_language_t convert_language;
    int rv = 0;
    
    TAILQ_FOREACH(convert_language, &ctx->m_languages, m_next) {
        struct ui_data_src_it src_it;
        ui_data_src_t src;

        ui_data_language_srcs(&src_it, convert_language->m_data_language);
        while((src = ui_data_src_it_next(&src_it))) {
            ui_data_src_group_add_src(ctx->m_ignore_srcs, src);
        }
    }

    if (convert_op_package_check_src_duplicate_r(ctx, ui_data_mgr_src_root(ctx->m_data_mgr)) != 0) rv = -1;

    return rv;
}


static int convert_op_package_check_res_duplicate(convert_ctx_t ctx) {
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    int rv = 0;
    
    ui_cache_manager_ress(ctx->m_cache_mgr, &res_it);
    while((res = ui_cache_res_it_next(&res_it))) {
        struct plugin_package_package_it package_it;
        plugin_package_package_t package;
        plugin_package_package_t used_package = NULL;
        
        plugin_package_module_packages(ctx->m_package_module, &package_it);
        while((package = plugin_package_package_it_next(&package_it))) {
            if (!ui_cache_res_in_group(res, plugin_package_package_resources(package))) continue;

            if (used_package) {
                CPE_ERROR(
                    ctx->m_em, "res %s: duplicate in package %s and %s!",
                    ui_cache_res_path(res),
                    plugin_package_package_name(used_package),
                    plugin_package_package_name(package));
                rv = -1;
                continue;
            }
            else {
                used_package = package;
            }
        }
    }

    return rv;
}

static int convert_op_package_load_and_remove_ignore_deps(
    convert_ctx_t ctx, plugin_package_package_t package, ui_data_src_group_t processed_group, ui_data_src_t src)
{
    struct ui_data_src_it dep_src_it;
    ui_data_src_t dep_src;
    int rv = 0;

    if (ui_data_src_in_group(src, processed_group)) return 0;
    if (ui_data_src_group_add_src(processed_group, src) != 0) return -1;

    if (!ui_data_src_is_loaded(src)) {
        if (ui_data_src_load(src, ctx->m_em) != 0) {
            CPE_ERROR(
                ctx->m_em, "convert_op_package_remove_ignore: src %s load fail",
                ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
            return -1;
        }
    }
    
    if (convert_op_remove_ignore(ctx, src) != 0) rv = -1;
    
    ui_data_src_update_using(src);

    ui_data_src_using_srcs(src, &dep_src_it);
    while((dep_src = ui_data_src_it_next(&dep_src_it))) {
        if (convert_op_package_load_and_remove_ignore_deps(ctx, package, processed_group, dep_src) != 0) {
            rv = -1;
        }
    }
    
    return rv;
}

static int convert_op_package_load_and_remove_ignore(convert_ctx_t ctx, plugin_package_package_t package) {
    ui_data_src_group_t processed_group;
    int rv = 0;
    struct ui_data_src_it src_it;
    ui_data_src_t src;

    if (ui_data_src_group_expand_dir(plugin_package_package_srcs(package)) != 0) {
        CPE_ERROR(ctx->m_em, "pack: expand dir fail!");
        return -1;
    }
        
    processed_group = ui_data_src_group_create(ctx->m_data_mgr);
    if (processed_group == NULL) {
        CPE_ERROR(ctx->m_em, "pack: create package in order fail!");
        return -1;
    }

    ui_data_src_group_srcs(&src_it, plugin_package_package_srcs(package));
    while((src = ui_data_src_it_next(&src_it))) {
        if (convert_ctx_is_src_ignore(ctx, src)) {
            CPE_ERROR(
                ctx->m_em, "convert_op_package_remove_ignore: package %s: src %s is ignore",
                plugin_package_package_name(package), ui_data_src_path_dump(gd_app_tmp_buffer(ctx->m_app), src));
            rv = -1;
            continue;
        }

        if (convert_op_package_load_and_remove_ignore_deps(ctx, package, processed_group, src) != 0) {
            rv = -1;
        }
    }

    if (ui_data_src_group_add_src_by_group(plugin_package_package_srcs(package), processed_group) != 0) rv = -1;

    ui_data_src_group_free(processed_group);
    
    return rv;
}
