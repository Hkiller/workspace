#include <assert.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_context.h"
#include "plugin/ui/plugin_ui_env.h"
#include "plugin/ui/plugin_ui_env_action.h"
#include "plugin/ui/plugin_ui_page.h"
#include "plugin/ui/plugin_ui_control.h"
#include "plugin/ui/plugin_ui_control_calc.h"
#include "appsvr_umeng_click_info.h"

static void appsvr_umeng_on_click(void * ctx, plugin_ui_control_t from_control, plugin_ui_event_t event) {
    appsvr_umeng_module_t module = ctx;
    appsvr_umeng_click_info_t click_info, click_info_next;
    struct appsvr_umeng_click_info key;
    
    if (from_control == NULL) return;
    
    key.m_page_name = plugin_ui_page_name(plugin_ui_control_page(from_control));
    key.m_control_name = plugin_ui_control_name(from_control);

    for(click_info = cpe_hash_table_find(&module->m_click_infos, &key);
        click_info;
        click_info = click_info_next)
    {
        char id[32];
        uint32_t value;
        const char * attrs;
        const char * str_id;
        
        click_info_next = cpe_hash_table_find_next(&module->m_click_infos, click_info);

        str_id = plugin_ui_control_calc_str_with_dft(&module->m_dump_buffer, click_info->m_id, from_control, NULL, NULL);
        if (str_id == NULL) {
            CPE_ERROR(
                module->m_em, "umeng: on click %s.%s: calc id %s fail!",
                click_info->m_page_name, click_info->m_control_name, click_info->m_id);
            continue;
        }
        cpe_str_dup(id, sizeof(id), str_id);

        if (plugin_ui_control_try_calc_uint32(&value, click_info->m_value, from_control, NULL, module->m_em) != 0) {
            CPE_ERROR(
                module->m_em, "umeng: on click %s.%s: calc value %s fail!",
                click_info->m_page_name, click_info->m_control_name, click_info->m_value);
            continue;
        }

        if (click_info->m_attrs) {
            attrs = plugin_ui_control_calc_str_with_dft(&module->m_dump_buffer, click_info->m_attrs, from_control, NULL, NULL);
        }
        else {
            attrs = NULL;
        }

        appsvr_umeng_on_event(module, id, value, attrs);        
    }
}

int appsvr_umeng_click_monitor_init(appsvr_umeng_module_t module) {
    plugin_ui_env_t cur_env;
    
    if (module->m_ui_module == NULL) return 0;

    cur_env = plugin_ui_env_first(module->m_ui_module);
    if (cur_env == NULL) return 0;

    assert(module->m_click_monitor == NULL);
    module->m_click_monitor =
        plugin_ui_env_action_create(
            cur_env, plugin_ui_event_mouse_click, appsvr_umeng_on_click, module);
    if (module->m_click_monitor == NULL) {
        CPE_ERROR(module->m_em, "appsvr_umeng_click_monitor_init: create click monitor fail!");
        return -1;
    }

    return 0;
}

void appsvr_umeng_click_monitor_fini(appsvr_umeng_module_t module) {
    if (module->m_ui_module) {
        assert(module->m_click_monitor);
        plugin_ui_env_action_free(module->m_click_monitor);
        module->m_click_monitor = NULL;
    }
}

