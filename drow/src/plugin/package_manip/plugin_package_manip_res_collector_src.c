#include <assert.h>
#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_src_group.h"
#include "render/model/ui_data_src.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "plugin_package_manip_res_collector_i.h"
#include "plugin_package_manip_src_convertor_i.h"

static int plugin_package_manip_res_collector_src_do(
    void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg, cfg_calc_context_t args)
{
    plugin_package_manip_t manip = ctx;
    const char * src_def;
    const char * src_value;
    struct mem_buffer buffer;
    ui_data_src_group_t src_group = NULL;
    plugin_package_manip_src_convertor_t convertor = NULL;
    cfg_t convertor_cfg;
    int rv = -1;

    mem_buffer_init(&buffer, manip->m_alloc);
    
    src_def = cfg_get_string(res_cfg, "src", NULL);
    if (src_def == NULL) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: src: src not configured",
            plugin_package_package_name(package));
        goto COLLECT_COMPLETE;
    }

    if ((src_value = plugin_package_manip_calc(manip, &buffer, src_def, args)) == NULL) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: calc from %s fail",
            plugin_package_package_name(package),  src_def);
        goto COLLECT_COMPLETE;
    }

    if (src_value[0] == 0) {
        rv = 0;
        goto COLLECT_COMPLETE;
    }
    
    convertor_cfg = cfg_find_cfg(res_cfg, "convertor");
    if (convertor_cfg) {
        const char * convertor_type = cfg_get_string(convertor_cfg, "type", NULL);

        if (convertor_type == NULL) {
            CPE_ERROR(
                manip->m_em, "package: %s: res collect: src: src convertor type not configured",
                plugin_package_package_name(package));
            goto COLLECT_COMPLETE;
        }            
        
        convertor = plugin_package_manip_src_convertor_find(manip, convertor_type);
        if (convertor == NULL) {
            CPE_ERROR(
                manip->m_em, "package: %s: res collect: src: src convertor %s unknown",
                plugin_package_package_name(package), convertor_type);
            goto COLLECT_COMPLETE;
        }
        
        src_group = ui_data_src_group_create(plugin_package_module_data_mgr(manip->m_package_module));
        if (src_group == NULL) {
            CPE_ERROR(
                manip->m_em, "package: %s: res collect: src: create src group fail",
                plugin_package_package_name(package));
            goto COLLECT_COMPLETE;
        }
    }
    else {
        src_group = plugin_package_package_srcs(package);
    }
        
    if (plugin_package_manip_collect_src_by_res(src_group, src_value) != 0) {
        CPE_ERROR(
            manip->m_em, "package: %s: res collect: src: collect src from src %s fail",
            plugin_package_package_name(package), src_value);
        goto COLLECT_COMPLETE;
    }

    if (convertor) {
        const char * output = cfg_get_string(convertor_cfg, "output", NULL);
        struct ui_data_src_it src_it;
        ui_data_src_t target_parent_src = NULL;
        ui_data_src_t src;

        if (output) {
            const char * output_value = plugin_package_manip_calc(manip, &buffer, output, args);
            if (output_value == NULL) {
                CPE_ERROR(
                    manip->m_em, "package: %s: res collect: src: calc convertor output from %s fail",
                    plugin_package_package_name(package),  output);
                goto COLLECT_COMPLETE;
            }
            
            target_parent_src = ui_data_src_find_by_path(plugin_package_module_data_mgr(manip->m_package_module), output_value, ui_data_src_type_dir);
            if (target_parent_src == NULL) {
                target_parent_src = ui_data_src_create_relative(plugin_package_module_data_mgr(manip->m_package_module), ui_data_src_type_dir, output_value);
                if (target_parent_src == NULL) {
                    CPE_ERROR(
                        manip->m_em, "package: %s: res collect: src: create convertor target src dir %s fail",
                        plugin_package_package_name(package), output);
                    goto COLLECT_COMPLETE;
                }
            }
        }
        
        assert(src_group);
        ui_data_src_group_srcs(&src_it, src_group);

        while((src = ui_data_src_it_next(&src_it))) {
            ui_data_src_t target_src;
            
            if (ui_data_src_type(src) != convertor->m_from_src_type) {
                //ui_data_src_group_add_src(plugin_package_package_srcs(package), src);
                continue;
            }

            target_src = ui_data_src_child_find(
                target_parent_src ? target_parent_src : ui_data_src_parent(src),
                ui_data_src_data(src), convertor->m_to_src_type);
            if (target_src) {
                if (ui_data_src_in_group(target_src, plugin_package_package_srcs(package))) {
                    continue;
                }
                else {
                    CPE_ERROR(
                        manip->m_em, "package: %s: res collect: src: convertor %s create target src %s already exist fail",
                        plugin_package_package_name(package), convertor->m_name,
                        ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), target_src));
                    goto COLLECT_COMPLETE;
                }
            }
                
            target_src = ui_data_src_create_child(
                target_parent_src ? target_parent_src : ui_data_src_parent(src),
                convertor->m_to_src_type, ui_data_src_data(src));
            if (target_src == NULL) {
                CPE_ERROR(
                    manip->m_em, "package: %s: res collect: src: convertor %s create target src fail",
                    plugin_package_package_name(package), convertor->m_name);
                goto COLLECT_COMPLETE;
            }

            if (convertor->m_fun(convertor->m_ctx, src, target_src, convertor_cfg) != 0) {
                CPE_ERROR(
                    manip->m_em, "package: %s: res collect: src: convertor %s process src %s %s fail",
                    plugin_package_package_name(package), convertor->m_name,
                    ui_data_src_type_name(ui_data_src_type(src)),
                    ui_data_src_path_dump(gd_app_tmp_buffer(manip->m_app), src));
                ui_data_src_free(target_src);
                goto COLLECT_COMPLETE;
            }

            ui_data_src_group_add_src(plugin_package_package_srcs(package), target_src);
        }
    }

    rv = 0;
    
COLLECT_COMPLETE:
    if (src_group && src_group != plugin_package_package_srcs(package)) ui_data_src_group_free(src_group);
    mem_buffer_clear(&buffer);
    return rv;
}

static int plugin_package_manip_res_collector_src(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg) {
    plugin_package_manip_t manip = ctx;
    const char * search;
    
    if ((search = cfg_get_string(res_cfg, "search", NULL))) {
        struct cfg_calc_context args = { package_cfg, NULL };
        return plugin_package_manip_collect_search(
            manip, package, res_cfg, package_cfg, &args,
            search, plugin_package_manip_res_collector_src_do, manip);
    }
    else {
        struct cfg_calc_context calc_ctx;
        calc_ctx.m_cfg = package_cfg;
        calc_ctx.m_next = NULL;
        return plugin_package_manip_res_collector_src_do(ctx, package, res_cfg, package_cfg, &calc_ctx);
    }
}

int plugin_package_manip_create_res_collector_src(plugin_package_manip_t manip) {
    plugin_package_manip_res_collector_t collector =
        plugin_package_manip_res_collector_create(
            manip, "src", plugin_package_manip_res_collector_src, manip);
    if (collector == NULL) return -1;
    return 0;
}
