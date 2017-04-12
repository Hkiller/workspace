#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_string.h"
#include "ui_cache_worker_i.h"
#include "ui_cache_task_i.h"
#include "ui_cache_res_i.h"

static void * ui_cache_worker_exec(void* arg);

ui_cache_worker_t ui_cache_worker_create(ui_cache_manager_t cache_mgr) {
    ui_cache_worker_t worker;

    worker = mem_calloc(cache_mgr->m_alloc, sizeof(struct ui_cache_worker));
    if (worker == NULL) {
        CPE_ERROR(cache_mgr->m_em, "%s: create worlker: malloc fail", ui_cache_manager_name(cache_mgr));
        return NULL;
    }

    worker->m_cacme_mgr = cache_mgr;
    worker->m_is_runing = 1;
    
    TAILQ_INSERT_TAIL(&cache_mgr->m_workers, worker, m_next_for_mgr);

    if (pthread_create(&worker->m_thread, NULL, ui_cache_worker_exec, worker) != 0) {
        CPE_ERROR(
            cache_mgr->m_em, "%s: create worlker: create thread fail errno=%d (%s)",
            ui_cache_manager_name(cache_mgr), errno, strerror(errno));
        TAILQ_REMOVE(&cache_mgr->m_workers, worker, m_next_for_mgr);
        mem_free(cache_mgr->m_alloc, worker);
        return NULL;
    }
    
    return worker;
}

void ui_cache_worker_free(ui_cache_worker_t worker) {
    ui_cache_manager_t cache_mgr = worker->m_cacme_mgr;

    /*停止线程 */
    worker->m_is_runing = 0;
    pthread_mutex_lock(&cache_mgr->m_task_mutex);
    pthread_cond_broadcast(&cache_mgr->m_task_cond);
    pthread_mutex_unlock(&cache_mgr->m_task_mutex);
    pthread_join(worker->m_thread, NULL);

    /*清理资源 */
    TAILQ_REMOVE(&cache_mgr->m_workers, worker, m_next_for_mgr);
    mem_free(cache_mgr->m_alloc, worker);
}

void ui_cache_worker_free_all(ui_cache_manager_t mgr) {
    while(!TAILQ_EMPTY(&mgr->m_workers)) {
        ui_cache_worker_free(TAILQ_FIRST(&mgr->m_workers));
    }
}

static void * ui_cache_worker_exec(void* arg) {
    ui_cache_worker_t worker = arg;
    ui_cache_manager_t cache_mgr = worker->m_cacme_mgr;

    pthread_mutex_lock(&cache_mgr->m_task_mutex);

    while(worker->m_is_runing) {
        uint16_t wp = cache_mgr->m_task_wp;
        struct ui_cache_task cur_task;
        uint16_t new_rp;

        if (cache_mgr->m_task_rp == wp) {
            pthread_cond_wait(&cache_mgr->m_task_cond, &cache_mgr->m_task_mutex);
            continue;
        }

        /*获取一个任务 */
        cur_task = cache_mgr->m_tasks[cache_mgr->m_task_rp];

        new_rp = cache_mgr->m_task_rp + 1;
        if (new_rp >= cache_mgr->m_task_capacity) {
            new_rp = 0;
        }
        cache_mgr->m_task_rp = new_rp;

        if (cur_task.m_task_type == ui_cache_task_type_load_res) {
            ui_cache_res_t res = cur_task.m_load_res.m_res;
            int r;
            
            if (cur_task.m_load_res.m_op_id != res->m_op_id) continue;
            if (res->m_load_state != ui_cache_res_wait_load) continue;
            res->m_load_state = ui_cache_res_loading;
            
            /*开始执行任务(无锁）  */
            pthread_mutex_unlock(&cache_mgr->m_task_mutex);

            if (res->m_res_type == ui_cache_res_type_texture) {
                r = ui_cache_texture_do_load(cache_mgr, res, cur_task.m_load_res.m_root[0] ? cur_task.m_load_res.m_root : NULL);
            }
            else if (res->m_res_type == ui_cache_res_type_sound) {
                r = ui_cache_sound_do_load(cache_mgr, res, cur_task.m_load_res.m_root[0] ? cur_task.m_load_res.m_root : NULL);
            }
            else if (res->m_res_type == ui_cache_res_type_font) {
                r = ui_cache_font_do_load(cache_mgr, res, cur_task.m_load_res.m_root[0] ? cur_task.m_load_res.m_root : NULL);
            }
            else {
                r = -1;
            }
            
            pthread_mutex_lock(&cache_mgr->m_task_mutex);

            /*处理执行结果（有锁） */
            assert(res->m_load_state == ui_cache_res_loading || res->m_load_state == ui_cache_res_cancel_loading);

            if (r == 0) {
                if (res->m_load_state == ui_cache_res_loading) {
                    res->m_load_state = ui_cache_res_data_loaded;
                }
            }
            else {
                res->m_load_state = ui_cache_res_load_fail;
            }

            /*向主线程发送任务 */
            if (res->m_load_state == ui_cache_res_data_loaded
                || res->m_load_state == ui_cache_res_cancel_loading)
            {
                uint16_t back_rp = cache_mgr->m_back_task_rp;
                uint16_t next_back_wp = cache_mgr->m_back_task_wp + 1;
                ui_cache_back_task_t new_back_task;

                if (next_back_wp >= cache_mgr->m_task_capacity) {
                    next_back_wp = 0;
                }

                assert(next_back_wp != back_rp);

                new_back_task = cache_mgr->m_back_tasks + cache_mgr->m_back_task_wp;
                new_back_task->m_task_type = ui_cache_task_type_load_res;
                new_back_task->m_load_res.m_res = res;

                cache_mgr->m_back_task_wp = next_back_wp;
            }
        }
    }
    
    pthread_mutex_unlock(&cache_mgr->m_task_mutex);
    
    return NULL;
}
