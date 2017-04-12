#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "gd/app/app_context.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "plugin/package_manip/plugin_package_manip_res_collector.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "ui/app_manip/ui_app_manip_src.h"
#include "ui_sprite_manip_i.h"

static int plugin_package_manip_res_collector_page(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg) {
    ui_sprite_manip_t module = ctx;
    struct mem_buffer buffer;
    const char * page_name_def;
    const char * page_name;
    struct cfg_calc_context calc_ctx;
    cfg_t page_cfg;

    page_name_def = cfg_get_string(res_cfg, "page", NULL);
    if (page_name_def == NULL) {
        CPE_ERROR(
            module->m_em, "package: %s: page collect: page not configured",
            plugin_package_package_name(package));
        return -1;
    }
    
    calc_ctx.m_cfg = package_cfg;
    calc_ctx.m_next = NULL;

    mem_buffer_init(&buffer, module->m_alloc);
    if ((page_name = plugin_package_manip_calc(module->m_package_manip, &buffer, page_name_def, &calc_ctx)) == NULL) {
        CPE_ERROR(
            module->m_em, "package: %s: page collect: calc from %s fail",
            plugin_package_package_name(package),  page_name_def);
        mem_buffer_clear(&buffer);
        return -1;
    }

    page_cfg = cfg_find_cfg(gd_app_cfg(module->m_app), "ui.pages");
    page_cfg = cfg_find_cfg(page_cfg, page_name);
    if (page_cfg == NULL) {
        CPE_ERROR(
            module->m_em, "package: %s: page collect: page %s not exist",
            plugin_package_package_name(package),  page_name);
        mem_buffer_clear(&buffer);
        return -1;
    }
    mem_buffer_clear(&buffer);
    
    return ui_app_manip_collect_src_from_page(
        plugin_package_package_srcs(package),
        plugin_package_package_resources(package),
        page_cfg, module->m_em);
}

int ui_sprite_manip_res_collector_page_regist(ui_sprite_manip_t module) {
    plugin_package_manip_res_collector_t collector =
        plugin_package_manip_res_collector_create(
            module->m_package_manip, "page", plugin_package_manip_res_collector_page, module);
    if (collector == NULL) return -1;
    return 0;
}

void  ui_sprite_manip_res_collector_page_unregist(ui_sprite_manip_t module) {
    plugin_package_manip_res_collector_t collector = plugin_package_manip_res_collector_find(module->m_package_manip, "page");
    if (collector) {
        plugin_package_manip_res_collector_free(collector);
    }
}
