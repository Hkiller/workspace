#ifndef UI_DATA_SRC_INTERNAL_H
#define UI_DATA_SRC_INTERNAL_H
#include "render/model/ui_data_src.h"
#include "ui_data_mgr_i.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ui_data_src {
    /*全局管理相关数据 */
    ui_data_mgr_t m_mgr;
    struct cpe_hash_entry m_hh_for_name;
    struct cpe_hash_entry m_hh_for_id;
    ui_data_src_user_list_t m_users;

    /*多语言 */
    ui_data_language_t m_language;
    ui_data_src_t m_base_src;
    ui_data_src_t m_language_src;
    TAILQ_ENTRY(ui_data_src) m_next_for_language;
    
    /*树形关系数据 */
    ui_data_src_t m_parent;
    ui_data_src_list_t m_childs;
    TAILQ_ENTRY(ui_data_src) m_next_for_parent;
    TAILQ_ENTRY(ui_data_src) m_next_for_type;

    /*使用关系数据 */
    uint8_t m_using_loaded;
    ui_data_src_src_list_t m_using_srcs;
    ui_data_src_src_list_t m_user_srcs;

    /*资源引用 */
    ui_data_src_res_list_t m_using_ress;

    /*分组数据 */
    ui_data_src_group_item_list_t m_items;

    /*strings*/
    ui_data_src_strings_state_t m_strings_state;
    void * m_strings;
    
    /*业务数据 */
    ui_data_src_type_t m_type;
    ui_data_src_load_state_t m_state;
    char * m_data; 
    void * m_product;
    uint32_t m_id;
};

ui_data_src_t ui_data_src_create_i(ui_data_mgr_t mgr, ui_data_src_t parent, ui_data_src_type_t type, const char * data);
    
uint32_t ui_data_src_hash(ui_data_src_t src);
int ui_data_src_eq(ui_data_src_t l, ui_data_src_t r);

uint32_t ui_data_src_id_hash(const ui_data_src_t src);
int ui_data_src_id_eq(const ui_data_src_t l, const ui_data_src_t r);

#ifdef __cplusplus
}
#endif

#endif
