#ifndef UI_CACHE_MANAGER_I_H
#define UI_CACHE_MANAGER_I_H
#include <pthread.h>
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/hash.h"
#include "cpe/utils/buffer.h"
#include "render/cache/ui_cache_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ui_cache_color * ui_cache_color_t;
typedef struct ui_cache_task * ui_cache_task_t;
typedef struct ui_cache_back_task * ui_cache_back_task_t;
typedef TAILQ_HEAD(ui_cache_res_list, ui_cache_res) ui_cache_res_list_t;
typedef TAILQ_HEAD(ui_cache_group_list, ui_cache_group) ui_cache_group_list_t;
typedef TAILQ_HEAD(ui_cache_res_ref_list, ui_cache_res_ref) ui_cache_res_ref_list_t;
typedef TAILQ_HEAD(ui_cache_worker_list, ui_cache_worker) ui_cache_worker_list_t;

struct ui_cache_manager {
    gd_app_context_t m_app;
    mem_allocrator_t m_alloc;
    error_monitor_t m_em;
    int m_debug;
    uint8_t m_texture_data_buff_keep;
    uint8_t m_texture_scale;
    
    struct mem_buffer m_dump_buffer;

    struct cpe_hash_table m_colors;
    
    ui_cache_res_list_t m_ress;
    struct cpe_hash_table m_texturies_by_path;
    ui_cache_res_list_t m_deleting_ress;

    ui_cache_group_list_t m_groups;
    ui_cache_res_plugin_t m_res_plugins[3];

    /*worker相关数据 */
    pthread_mutex_t m_task_mutex;
    pthread_cond_t m_task_cond;
    ui_cache_worker_list_t m_workers;

    uint16_t m_task_capacity;
    
    /*发送任务到worker的队列 */
    struct ui_cache_task * m_tasks;
    uint16_t m_task_wp;
    uint16_t m_task_rp;

    /*从Worker返回主线程处理的任务队列 */
    uint16_t m_back_task_wp;
    uint16_t m_back_task_rp;
    struct ui_cache_back_task * m_back_tasks;
};

#ifdef __cplusplus
}
#endif

#endif
