#ifndef PLUGIN_UI_PAGE_I_H
#define PLUGIN_UI_PAGE_I_H
#include "plugin/ui/plugin_ui_page.h"
#include "plugin_ui_control_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct plugin_ui_page {
    plugin_ui_env_t m_env;
    struct cpe_hash_entry m_hh_for_env;
    TAILQ_ENTRY(plugin_ui_page) m_next_for_env;
    
    plugin_ui_page_meta_t m_page_meta;
    TAILQ_ENTRY(plugin_ui_page) m_next_for_meta;

    uint8_t m_is_in_visible_queue;
    TAILQ_ENTRY(plugin_ui_page) m_next_for_visible_queue;

    uint8_t m_hiding;
    TAILQ_ENTRY(plugin_ui_page) m_next_for_hiding;

    plugin_ui_page_load_policy_t m_load_policy;
    
    plugin_ui_popup_t m_visible_in_popup;
    plugin_ui_popup_list_t m_created_popups;
    plugin_ui_state_node_page_list_t m_visible_in_states;

    plugin_ui_control_binding_list_t m_need_process_bindings;

    uint8_t m_force_change;
    uint8_t m_changed;

    char * m_load_from;
    ui_data_src_t m_src;
    uint16_t m_control_count;
    uint8_t m_modal;

    LPDRMETA m_page_data_meta;
    void * m_page_data;
    uint32_t m_page_data_size;

    /*事件接收器 */
    plugin_ui_page_eh_list_t m_ehs;

    /*事件接收器 */
    plugin_ui_page_plugin_list_t m_plugins;

    /*属性值插接口 */
    plugin_ui_page_slot_list_t m_slots;
    
    /*和phase的绑定关系 */
    plugin_ui_phase_use_page_list_t m_used_by_phases;

    /*创建中的控件 */
    plugin_ui_control_list_t m_building_controls;

    /*control的处理函数绑定 */
    plugin_ui_control_action_list_t m_control_actions;

    /*页面相关package管理 */
    plugin_package_group_t m_packages;
    plugin_package_group_t m_packages_r;
    
    /*root控件数据，必须最后一个，后面跟业务数据 */
    struct plugin_ui_control m_control;
};

void plugin_ui_page_update_area(const plugin_ui_page_t page);
void plugin_ui_page_update_visible(plugin_ui_page_t page, uint8_t visible);
void plugin_ui_page_do_hide(plugin_ui_page_t page);

int plugin_ui_control_meta_page_regist(plugin_ui_module_t module);
void plugin_ui_control_meta_page_unregist(plugin_ui_module_t module);
    
uint32_t plugin_ui_page_hash(const plugin_ui_page_t page);
int plugin_ui_page_eq(const plugin_ui_page_t l, const plugin_ui_page_t r);

#ifdef __cplusplus
}
#endif

#endif
