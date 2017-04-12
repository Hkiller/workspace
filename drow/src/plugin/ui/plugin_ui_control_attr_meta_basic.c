#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dr/dr_data_value.h"
#include "plugin_ui_control_attr_meta_i.h"
#include "plugin_ui_control_i.h"
#include "plugin_ui_page_i.h"

/*name*/
static int plugin_ui_control_attr_name_set(plugin_ui_control_t control, dr_value_t value) {
    plugin_ui_env_t env = control->m_page->m_env;
    const char * str_value = dr_value_to_string(&env->m_module->m_dump_buffer, value, NULL);
    plugin_ui_control_set_name(control, str_value);
    return 0;
}

static int plugin_ui_control_attr_name_get(plugin_ui_control_t control, dr_value_t value) {
    value->m_type = CPE_DR_TYPE_STRING;
    value->m_meta = NULL;
    value->m_data = (void*)plugin_ui_control_name(control);
    value->m_size = strlen(value->m_data) + 1;
    return 0;
}

/*enable*/

/*visible*/

/*alpha*/

/*user-text*/
static int plugin_ui_control_attr_user_text_set(plugin_ui_control_t control, dr_value_t value) {
    plugin_ui_env_t env = control->m_page->m_env;
    const char * str_value = dr_value_to_string(&env->m_module->m_dump_buffer, value, NULL);
    plugin_ui_control_set_user_text(control, str_value);
    return 0;
}

static int plugin_ui_control_attr_user_text_get(plugin_ui_control_t control, dr_value_t value) {
    value->m_type = CPE_DR_TYPE_STRING;
    value->m_meta = NULL;
    value->m_data = (void*)plugin_ui_control_user_text(control);
    value->m_size = strlen(value->m_data) + 1;
    return 0;
}

int plugin_ui_control_attr_meta_create_basics(plugin_ui_control_meta_t meta) {
    if (plugin_ui_control_attr_meta_create(
            meta, "name",
            plugin_ui_control_attr_name_set, plugin_ui_control_attr_name_get) == NULL
        ||
        plugin_ui_control_attr_meta_create(
            meta, "user-text",
            plugin_ui_control_attr_user_text_set, plugin_ui_control_attr_user_text_get) == NULL
        )
    {
        CPE_ERROR(meta->m_module->m_em, "plugin_ui_control_attr_meta_create_basics: create fail!");
        return -1;
    }

    return 0;
}
