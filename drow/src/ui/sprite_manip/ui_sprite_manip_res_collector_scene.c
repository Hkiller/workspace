#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "gd/app/app_context.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "plugin/package_manip/plugin_package_manip_res_collector.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "ui/app_manip/ui_app_manip_src.h"
#include "ui_sprite_manip_i.h"

static int plugin_package_manip_res_collector_scene_do(
    void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg, cfg_calc_context_t args)
{
    ui_sprite_manip_t module = ctx;
    const char * path_def;
    cfg_t scene_cfg;

    path_def = cfg_get_string(res_cfg, "path", NULL);
    if (path_def) {
        struct mem_buffer buffer;
        const char * path;
        
        mem_buffer_init(&buffer, module->m_alloc);
        if ((path = plugin_package_manip_calc(module->m_package_manip, &buffer, path_def, args)) == NULL) {
            CPE_ERROR(
                module->m_em, "package: %s: scene collect: calc from %s fail",
                plugin_package_package_name(package),  path_def);
            mem_buffer_clear(&buffer);
            return -1;
        }

        if (path[0] == '^') {
            scene_cfg = cfg_find_cfg(gd_app_cfg(module->m_app), path + 1);
        }
        else {
            scene_cfg = cfg_find_cfg(args->m_cfg, path);
        }

        mem_buffer_clear(&buffer);
    }
    else {
        scene_cfg = args->m_cfg;
    }

    return ui_app_manip_collect_src_from_scene(
        plugin_package_package_srcs(package),
        plugin_package_package_resources(package),
        scene_cfg, module->m_em);
}

static int plugin_package_manip_res_collector_scene(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg) {
    ui_sprite_manip_t module = ctx;
    plugin_package_manip_t manip = module->m_package_manip;
    const char * search;
    
    if ((search = cfg_get_string(res_cfg, "search", NULL))) {
        struct cfg_calc_context args = { package_cfg, NULL };
        struct mem_buffer path_buffer;
        const char * search_path;
        int rv;
        
        mem_buffer_init(&path_buffer, NULL);
        search_path = plugin_package_manip_calc(manip, &path_buffer, search, &args);
        if (search_path == NULL) {
            mem_buffer_clear(&path_buffer);
            CPE_ERROR(
                module->m_em, "package: %s: scene collect: calc search from %s fail",
                plugin_package_package_name(package), search);
            mem_buffer_clear(&path_buffer);
            return -1;
        }

        rv = plugin_package_manip_collect_search(
            manip, package, res_cfg, package_cfg, &args,
            search_path, plugin_package_manip_res_collector_scene_do, manip);

        mem_buffer_clear(&path_buffer);

        return rv;
    }
    else {
        struct cfg_calc_context calc_ctx;
        calc_ctx.m_cfg = package_cfg;
        calc_ctx.m_next = NULL;
        return plugin_package_manip_res_collector_scene_do(ctx, package, res_cfg, package_cfg, &calc_ctx);
    }
}

int ui_sprite_manip_res_collector_scene_regist(ui_sprite_manip_t module) {
    plugin_package_manip_res_collector_t collector =
        plugin_package_manip_res_collector_create(
            module->m_package_manip, "scene", plugin_package_manip_res_collector_scene, module);
    if (collector == NULL) return -1;
    return 0;
}

void  ui_sprite_manip_res_collector_scene_unregist(ui_sprite_manip_t module) {
    plugin_package_manip_res_collector_t collector = plugin_package_manip_res_collector_find(module->m_package_manip, "scene");
    if (collector) {
        plugin_package_manip_res_collector_free(collector);
    }
}
