#include "cpe/cfg/cfg_read.h"
#include "cpe/cfg/cfg_calc.h"
#include "gd/app/app_context.h"
#include "plugin/package/plugin_package_package.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "plugin/package_manip/plugin_package_manip_res_collector.h"
#include "plugin/package_manip/plugin_package_manip_utils.h"
#include "ui/app_manip/ui_app_manip_src.h"
#include "ui_sprite_manip_i.h"

static int plugin_package_manip_res_collector_popup(void * ctx, plugin_package_package_t package, cfg_t res_cfg, cfg_t package_cfg) {
    ui_sprite_manip_t module = ctx;
    struct mem_buffer buffer;
    const char * popup_name_def;
    const char * popup_name;
    struct cfg_calc_context calc_ctx;
    cfg_t popup_cfg;

    popup_name_def = cfg_get_string(res_cfg, "popup", NULL);
    if (popup_name_def == NULL) {
        CPE_ERROR(
            module->m_em, "package: %s: popup collect: popup not configured",
            plugin_package_package_name(package));
        return -1;
    }
    
    calc_ctx.m_cfg = package_cfg;
    calc_ctx.m_next = NULL;

    mem_buffer_init(&buffer, module->m_alloc);
    if ((popup_name = plugin_package_manip_calc(module->m_package_manip, &buffer, popup_name_def, &calc_ctx)) == NULL) {
        CPE_ERROR(
            module->m_em, "package: %s: popup collect: calc from %s fail",
            plugin_package_package_name(package),  popup_name_def);
        mem_buffer_clear(&buffer);
        return -1;
    }

    popup_cfg = cfg_find_cfg(gd_app_cfg(module->m_app), "ui.popups");
    popup_cfg = cfg_find_cfg(popup_cfg, popup_name);
    if (popup_cfg == NULL) {
        CPE_ERROR(
            module->m_em, "package: %s: popup collect: popup %s not exist",
            plugin_package_package_name(package),  popup_name);
        mem_buffer_clear(&buffer);
        return -1;
    }
    mem_buffer_clear(&buffer);
    
    return ui_app_manip_collect_src_from_page(
        plugin_package_package_srcs(package),
        plugin_package_package_resources(package),
        popup_cfg, module->m_em);
}

int ui_sprite_manip_res_collector_popup_regist(ui_sprite_manip_t module) {
    plugin_package_manip_res_collector_t collector =
        plugin_package_manip_res_collector_create(
            module->m_package_manip, "popup", plugin_package_manip_res_collector_popup, module);
    if (collector == NULL) return -1;
    return 0;
}

void  ui_sprite_manip_res_collector_popup_unregist(ui_sprite_manip_t module) {
    plugin_package_manip_res_collector_t collector = plugin_package_manip_res_collector_find(module->m_package_manip, "popup");
    if (collector) {
        plugin_package_manip_res_collector_free(collector);
    }
}
