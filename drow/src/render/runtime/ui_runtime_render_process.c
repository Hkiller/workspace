#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "render/cache/ui_cache_res.h"
#include "render/runtime/ui_runtime_render_cmd.h"
#include "ui_runtime_render_i.h"
#include "ui_runtime_render_backend_i.h"
#include "ui_runtime_render_queue_i.h"
#include "ui_runtime_render_cmd_i.h"
#include "ui_runtime_render_worker_i.h"

void ui_runtime_render_begin(ui_runtime_render_t render, uint8_t * have_pre_frame) {
    ui_runtime_render_worker_t worker = render->m_worker;
    
    /*上一帧还在绘制则需要等待 */
    if (render->m_commit_state == ui_runtime_render_commit_state_commit) {
        assert(worker);

        pthread_mutex_lock(&worker->m_mutex);
        while(render->m_commit_state == ui_runtime_render_commit_state_commit) {
            pthread_cond_wait(&worker->m_cond, &worker->m_mutex);
        }
        pthread_mutex_unlock(&worker->m_mutex);
    }
    
    if (have_pre_frame) {
        *have_pre_frame = render->m_commit_state == ui_runtime_render_commit_state_done ? 1 : 0;
    }

    /*清理上一帧返分配的资源 */
    if (render->m_commit_state == ui_runtime_render_commit_state_done
        || render->m_commit_state == ui_runtime_render_commit_state_skip)
    {
        ui_runtime_render_clear_commit(render);
    }

    /*设置好状态，等待绘制命令 */
    render->m_commit_state = ui_runtime_render_commit_state_prepaire;
}

void ui_runtime_render_done(ui_runtime_render_t render) {
    assert(render->m_commit_state == ui_runtime_render_commit_state_prepaire);

    render->m_commit_state = ui_runtime_render_commit_state_commit;

    if (render->m_worker) {
        ui_runtime_render_worker_signal(render->m_worker);
    }
    else {
        ui_runtime_render_do_commit(render);
        render->m_commit_state = ui_runtime_render_commit_state_done;
    }
}

static void ui_runtime_render_do_commit_queue(ui_runtime_render_t render, ui_runtime_render_backend_t backend, ui_runtime_render_queue_t queue) {
    size_t i;

    ui_runtime_render_queue_sort(queue);
    ui_runtime_render_queue_state_save(queue);

    for(i = 0; i < CPE_ARRAY_SIZE(queue->m_groups); ++i) {
        ui_runtime_render_cmd_list_t * cmd_list;
        ui_runtime_render_cmd_t cmd;

        backend->m_commit_group_being(backend->m_ctx, render, (ui_runtime_render_queue_group_t)i);
        
        cmd_list = &queue->m_groups[i];
        TAILQ_FOREACH(cmd, cmd_list, m_next) {
            if (cmd->m_cmd_type == ui_runtime_render_cmd_group) {
                ui_runtime_render_do_commit_queue(render, backend, cmd->m_sub_queue);
            }
            else {
                backend->m_commit_cmd(backend->m_ctx, render, cmd);
            }
        }

        backend->m_commit_group_done(backend->m_ctx, render, (ui_runtime_render_queue_group_t)i);
    }
    
    ui_runtime_render_queue_state_restore(queue);
}

void ui_runtime_render_do_commit(ui_runtime_render_t render) {
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_backend_t backend = module->m_render_backend;

    assert(render->m_commit_state == ui_runtime_render_commit_state_commit);

    backend->m_commit_being(backend->m_ctx, render);
    ui_runtime_render_do_commit_queue(render, backend, render->m_queue_stack[0]);
    backend->m_commit_done(backend->m_ctx, render, &backend->m_last_commit_statics);
}

void ui_runtime_render_clear_commit(ui_runtime_render_t render) {
    ui_runtime_render_queue_clear(render->m_default_queue);
    render->m_data_buf_wp = 0;

    ui_cache_res_free_deleting(render->m_module->m_cache_mgr);
    
    render->m_commit_state = ui_runtime_render_commit_state_clear;

    if (render->m_module->m_render_backend) {
        render->m_last_commit_statics = render->m_module->m_render_backend->m_last_commit_statics;
    }

    assert(render->m_queue_count == 1);
    assert(render->m_cmd_count == 0);
}
