#include <assert.h>
#include <errno.h>
#include "cpe/utils/time_utils.h"
#include "ui_runtime_render_worker_i.h"

static void * ui_runtime_render_worker_exec(void* arg);

ui_runtime_render_worker_t
ui_runtime_render_worker_create(
    ui_runtime_render_t context,
    void * ctx,
    ui_runtime_render_worker_begin_fun_t begin_fun,
    ui_runtime_render_worker_end_fun_t end_fun, uint8_t with_thread)
{
    ui_runtime_module_t module = context->m_module;
    ui_runtime_render_worker_t worker;

    if (context->m_worker) {
        CPE_ERROR(module->m_em, "%s: create context worker: already have worker!", ui_runtime_module_name(module));
        return NULL;
    }

    worker = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render_worker));
    if (worker == NULL) {
        CPE_ERROR(module->m_em, "%s: create context worker: alloc fail!", ui_runtime_module_name(module));
        return NULL;
    }
    
    worker->m_context = context;
    worker->m_ctx = ctx;
    worker->m_with_thread = with_thread;
    worker->m_is_exit = 0;
    worker->m_is_rending = 0;
    worker->m_begin_fun = begin_fun;
    worker->m_end_fun = end_fun;
    context->m_worker = worker;

    if (pthread_mutex_init(&worker->m_mutex, NULL) != 0) {
        CPE_ERROR(module->m_em, "%s: create context worker: begin mutex fail, errno=%d (%s)!",
                  ui_runtime_module_name(module), errno, strerror(errno));
        context->m_worker = NULL;
        mem_free(module->m_alloc, worker);
        return NULL;
    }

    if (pthread_cond_init(&worker->m_cond, NULL) != 0) {
        CPE_ERROR(module->m_em, "%s: create context worker: begin cond fail, errno=%d (%s)!",
                  ui_runtime_module_name(module), errno, strerror(errno));
        pthread_mutex_destroy(&worker->m_mutex);
        context->m_worker = NULL;
        mem_free(module->m_alloc, worker);
        return NULL;
    }
    

    if (with_thread) {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        if (pthread_create(&worker->m_thread, &attr, ui_runtime_render_worker_exec, worker) != 0) {
            CPE_ERROR(module->m_em, "%s: create context worker: create thread fail, errno=%d (%s)!",
                      ui_runtime_module_name(module), errno, strerror(errno));
            pthread_attr_destroy(&attr);
            pthread_cond_destroy(&worker->m_cond);
            pthread_mutex_destroy(&worker->m_mutex);
            context->m_worker = NULL;
            mem_free(module->m_alloc, worker);
            return NULL;
        }

        pthread_attr_destroy(&attr);
    }
    
    return worker;
}

void ui_runtime_render_worker_free(ui_runtime_render_worker_t worker) {
    ui_runtime_render_t context = worker->m_context;

    assert(context->m_worker == worker);

    context->m_worker = NULL;

    worker->m_is_exit = 1;
    if (worker->m_with_thread) {
        ui_runtime_render_worker_signal(worker);
        pthread_join(worker->m_thread, NULL);
        assert(!worker->m_is_rending);
    }
    else {
        if(worker->m_is_rending) {
            pthread_mutex_lock(&worker->m_mutex);
            if(worker->m_is_rending) {
                pthread_cond_broadcast(&worker->m_cond);
                while(worker->m_is_rending) {
                    pthread_cond_wait(&worker->m_cond, &worker->m_mutex);
                }
            }
            pthread_mutex_unlock(&worker->m_mutex);
        }
    }

    pthread_cond_destroy(&worker->m_cond);
    pthread_mutex_destroy(&worker->m_mutex);
    
    mem_free(context->m_module->m_alloc, worker);
}

ui_runtime_render_worker_t
ui_runtime_render_worker_get(ui_runtime_render_t context) {
    return context->m_worker;
}

void ui_runtime_render_worker_signal(ui_runtime_render_worker_t worker) {
    pthread_mutex_lock(&worker->m_mutex);
    pthread_cond_broadcast(&worker->m_cond);
    pthread_mutex_unlock(&worker->m_mutex);
}

void ui_runtime_render_work_run(ui_runtime_render_worker_t worker, uint32_t loop_count, uint32_t timespan_ms) {
    ui_runtime_render_t render = worker->m_context;
    struct timespec time_to_wait;

    if (timespan_ms) {
        struct timeval now;
        gettimeofday(&now, NULL);

        time_to_wait.tv_sec = time(0) + timespan_ms / 1000;
        time_to_wait.tv_nsec = (now.tv_usec + 1000UL * (timespan_ms % 1000)) * 1000UL;
        time_to_wait.tv_sec += time_to_wait.tv_nsec / (1000UL * 1000UL * 1000UL);
        time_to_wait.tv_nsec %= (1000UL * 1000UL * 1000UL);
    }

    pthread_mutex_lock(&worker->m_mutex);
    assert(!worker->m_is_rending);
    worker->m_is_rending = 1;
    
    while(!worker->m_is_exit) {
        if (render->m_commit_state != ui_runtime_render_commit_state_commit) {
            if (timespan_ms > 0) {
                int rv = pthread_cond_timedwait(&worker->m_cond, &worker->m_mutex, &time_to_wait);
                if (rv != 0) {
                    if (rv == ETIMEDOUT) break;
                }
            }
            else {
                pthread_cond_wait(&worker->m_cond, &worker->m_mutex);
            }
        }

        if (render->m_commit_state != ui_runtime_render_commit_state_commit) continue;

        pthread_mutex_unlock(&worker->m_mutex);

        if (worker->m_begin_fun) {
            if (worker->m_begin_fun(worker->m_ctx) != 0) {
                pthread_mutex_lock(&worker->m_mutex);

                assert(render->m_commit_state == ui_runtime_render_commit_state_commit);
                render->m_commit_state = ui_runtime_render_commit_state_skip;
                pthread_cond_broadcast(&worker->m_cond);
                continue;
            }
        }

        ui_runtime_render_do_commit(render);
        
        if (worker->m_end_fun) worker->m_end_fun(worker->m_ctx);
        
        pthread_mutex_lock(&worker->m_mutex);

        assert(render->m_commit_state == ui_runtime_render_commit_state_commit);
        render->m_commit_state = ui_runtime_render_commit_state_done;
        pthread_cond_broadcast(&worker->m_cond);

        if (loop_count > 0 && --loop_count == 0) break;
    }
    assert(worker->m_is_rending == 1);
    
    worker->m_is_rending = 0;
    if (worker->m_is_exit) {
        pthread_cond_broadcast(&worker->m_cond);
    }
    
    pthread_mutex_unlock(&worker->m_mutex);
}

static void * ui_runtime_render_worker_exec(void* arg) {
    ui_runtime_render_worker_t worker = arg;
    ui_runtime_render_work_run(worker, 0, 0);
    return NULL;
}
