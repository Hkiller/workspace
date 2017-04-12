#ifndef UI_DATA_MGR_I_H
#define UI_DATA_MGR_I_H
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/memory.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/error.h"
#include "cpe/xcalc/xcalc_types.h"
#include "gd/app/app_context.h"
#include "render/model/ui_model_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(ui_data_language_list, ui_data_language) ui_data_language_list_t;
typedef TAILQ_HEAD(ui_data_src_src_list, ui_data_src_src) ui_data_src_src_list_t;
typedef TAILQ_HEAD(ui_data_src_res_list, ui_data_src_res) ui_data_src_res_list_t;
typedef TAILQ_HEAD(ui_data_src_list, ui_data_src) ui_data_src_list_t;
typedef TAILQ_HEAD(ui_data_src_user_list, ui_data_src_user) ui_data_src_user_list_t;
typedef TAILQ_HEAD(ui_data_src_group_item_list, ui_data_src_group_item) ui_data_src_group_item_list_t;
typedef TAILQ_HEAD(ui_data_evt_collector_list, ui_data_evt_collector) ui_data_evt_collector_list_t;

typedef struct ui_product_type * ui_product_type_t;

struct ui_product_type {
    void * product_create_ctx;
    ui_data_product_create_fun_t product_create;
    void * product_free_ctx;
    ui_data_product_free_fun_t product_free;
    void * product_load_ctx;
    product_load_fun_t product_load;
    void * product_save_ctx;
    product_save_fun_t product_save;
    product_remove_fun_t product_remove;
    product_using_src_update_using_fun_t product_update_usings;
    ui_data_src_list_t srcs;
};

struct ui_data_mgr {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    uint8_t m_debug;
    ui_cache_manager_t m_cache_mgr;
    ui_data_language_t m_active_language;
    ui_data_language_list_t m_languages;
    struct ui_product_type m_product_types[UI_DATA_SRC_TYPE_MAX - UI_DATA_SRC_TYPE_MIN];
    xcomputer_t m_computer;
    
    struct mem_buffer m_dump_buffer;

    LPDRMETA m_meta_collision;    
    LPDRMETA m_meta_img_block;
    LPDRMETA m_meta_frame;
    LPDRMETA m_meta_frame_img;
    LPDRMETA m_meta_actor;
    LPDRMETA m_meta_actor_layer;
    LPDRMETA m_meta_actor_frame;
    LPDRMETA m_meta_control;    
    LPDRMETA m_meta_control_anim;
    LPDRMETA m_meta_control_anim_frame;    
    LPDRMETA m_meta_control_addition;
    LPDRMETA m_meta_control_object_url;
    LPDRMETA m_meta_object_url;
    
    /*src相关数据 */
    ui_data_src_t m_src_root;
    struct cpe_hash_table m_srcs_by_name;
    struct cpe_hash_table m_srcs_by_id;

    /*module相关数据 */
    struct cpe_hash_table m_img_blocks;

    /*sprite相关数据 */
    struct cpe_hash_table m_frames;

    /*action相关数据 */
    struct cpe_hash_table m_actors;
    
    /*layout相关数据 */
    struct cpe_hash_table m_controls_by_id;

    /**/
    ui_data_evt_collector_list_t m_evt_collectors;
    
    ui_data_src_user_list_t m_free_src_users;
    ui_data_src_src_list_t m_free_src_srcs;
    ui_data_src_res_list_t m_free_src_ress;
};

#define ui_product_type_of(__mgr, __type) ((__mgr)->m_product_types + ((__type) - UI_DATA_SRC_TYPE_MIN))

#ifdef __cplusplus
}
#endif

#endif
