#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "ui_cache_task_i.h"
#include "ui_cache_res_i.h"
#include "ui_cache_res_plugin_i.h"
#include "ui_cache_res_plugin_i.h"
#include "ui_cache_pixel_buf_i.h"

int ui_cache_add_load_res_task(ui_cache_manager_t cache_mgr, ui_cache_res_t res, const char * root) {
    uint16_t rp = cache_mgr->m_task_rp;
    uint16_t next_wp = cache_mgr->m_task_wp + 1;
    ui_cache_task_t new_task;
    
    if (next_wp >= cache_mgr->m_task_capacity) {
        next_wp = 0;
    }
    
    if (next_wp == rp) {
        CPE_ERROR(
            cache_mgr->m_em, "%s: add load res task: task overflow, capacity=%d, rp=%d, wp=%d",
            ui_cache_manager_name(cache_mgr), cache_mgr->m_task_capacity, rp, cache_mgr->m_task_wp);
        return -1;
    }

    /*将任务放入队列 */
    new_task = cache_mgr->m_tasks + cache_mgr->m_task_wp;

    assert(res->m_load_state == ui_cache_res_not_load || res->m_load_state == ui_cache_res_load_fail);
    res->m_load_state = ui_cache_res_wait_load;
    res->m_op_id++;

    new_task->m_task_type = ui_cache_task_type_load_res;
    new_task->m_load_res.m_op_id = res->m_op_id;
    new_task->m_load_res.m_res = res;
    if (root) {
        cpe_str_dup(new_task->m_load_res.m_root, sizeof(new_task->m_load_res.m_root), root);
    }
    else {
        new_task->m_load_res.m_root[0] = 0;
    }

    /*唤醒工作线程 */
    cache_mgr->m_task_wp = next_wp;
    
    pthread_mutex_lock(&cache_mgr->m_task_mutex);
    pthread_cond_broadcast(&cache_mgr->m_task_cond);
    pthread_mutex_unlock(&cache_mgr->m_task_mutex);

    return 0;
}

ptr_int_t ui_cache_task_main_thread_process(void * ctx, ptr_int_t arg, float delta_s) {
    ui_cache_manager_t cache_mgr = ctx;
    uint16_t protect_count = 128;
    uint16_t i;

    for(i = 0; i < protect_count && cache_mgr->m_back_task_rp != cache_mgr->m_back_task_wp; ++i) {
        uint16_t wp = cache_mgr->m_task_wp;
        struct ui_cache_back_task cur_task;
        uint16_t new_rp;

        if (cache_mgr->m_back_task_rp == wp) break;

        /*获取一个任务 */
        cur_task = cache_mgr->m_back_tasks[cache_mgr->m_back_task_rp];

        new_rp = cache_mgr->m_back_task_rp + 1;
        if (new_rp >= cache_mgr->m_task_capacity) {
            new_rp = 0;
        }
        cache_mgr->m_back_task_rp = new_rp;

        switch(cur_task.m_task_type) {
        case ui_cache_task_type_load_res: {
            ui_cache_res_t res = cur_task.m_load_res.m_res;
            if (res->m_load_state == ui_cache_res_data_loaded) {
                ui_cache_res_plugin_t plugin;
                
                if (res->m_res_type == ui_cache_res_type_texture) {
                    assert(res->m_texture.m_data_buff);
                    if (ui_cache_texture_load_from_buf(res, res->m_texture.m_data_buff) != 0) {
                        CPE_ERROR(cache_mgr->m_em, "res %s: load from buf fail", res->m_path);
                        ui_cache_pixel_buf_free(res->m_texture.m_data_buff);
                        res->m_texture.m_data_buff = NULL;
                        res->m_load_result = ui_cache_res_internal_error;
                    }
                    else {
                        res->m_load_state = ui_cache_res_loaded;
                    }
                }
                else {
                    res->m_load_state = ui_cache_res_loaded;
                }
    
                plugin = ui_cache_res_plugin_find_by_type(cache_mgr, res->m_res_type);
                if (plugin && plugin->m_on_load) {
                    if (plugin->m_on_load(plugin->m_ctx, res) != 0) {
                        CPE_ERROR(
                            cache_mgr->m_em, "%s: res %s: plugin %s do load fail",
                            ui_cache_manager_name(cache_mgr), res->m_path, plugin->m_name);
                        res->m_load_state = ui_cache_res_data_loaded;
                        ui_cache_res_do_unload(res, 0);
                        return -1;
                    }
                }

                if (res->m_mgr->m_debug) {
                    CPE_INFO(res->m_mgr->m_em, "res %s: loaded(async)", res->m_path);
                }
            }
            else if (res->m_load_state == ui_cache_res_cancel_loading) {
                res->m_load_state = ui_cache_res_not_load;

                if (res->m_mgr->m_debug) {
                    CPE_INFO(res->m_mgr->m_em, "res %s: load cancel(async)", res->m_path);
                }
    
                ui_cache_res_do_unload(res, 0);
            }
            break;
        }
        default:
            break;
        }
    }

    return 0;
}
