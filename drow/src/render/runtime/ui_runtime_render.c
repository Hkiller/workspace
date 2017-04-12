#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_texture.h"
#include "render/runtime/ui_runtime_render_cmd.h"
#include "ui_runtime_render_i.h"
#include "ui_runtime_render_state_i.h"
#include "ui_runtime_render_camera_i.h"
#include "ui_runtime_render_program_i.h"
#include "ui_runtime_render_program_state_i.h"
#include "ui_runtime_render_queue_i.h"
#include "ui_runtime_render_material_i.h"
#include "ui_runtime_render_cmd_i.h"
#include "ui_runtime_render_backend_i.h"
#include "ui_runtime_render_worker_i.h"

ui_runtime_render_t
ui_runtime_render_create(ui_runtime_module_t module, uint32_t buf_capacity) {
    ui_runtime_render_t render;

    if (module->m_render) {
        CPE_ERROR(module->m_em, "%s: render: create: already exist", ui_runtime_module_name(module));
        return NULL;
    }
    
    render = mem_alloc(module->m_alloc, sizeof(struct ui_runtime_render) + buf_capacity);
    if (render == NULL) {
        CPE_ERROR(module->m_em, "%s: render: create: alloc fail, capacity=%d", ui_runtime_module_name(module), buf_capacity);
        return NULL;
    }

    render->m_module = module;
    render->m_is_depth_test_for_2d = 0;

    bzero(&render->m_last_commit_statics, sizeof(render->m_last_commit_statics));

    render->m_inited = 0;
    render->m_worker = NULL;
    render->m_commit_state = ui_runtime_render_commit_state_clear;
    
    render->m_clear_color.r = 0.9f;
    render->m_clear_color.g = 0.9f;
    render->m_clear_color.b = 1.0f;
    render->m_clear_color.a = 1.0f;

    render->m_view_size = UI_VECTOR_2_ZERO;
    bzero(&render->m_matrix_stacks, sizeof(render->m_matrix_stacks));

    render->m_scissor_buf = NULL;
    render->m_scissor_size = 0;
    render->m_scissor_capacity = 0;
    
    render->m_active_camera = NULL;
    TAILQ_INIT(&render->m_cameras);

    render->m_cmd_count = 0;
    render->m_queue_count = 0;
    render->m_default_queue = NULL;
    TAILQ_INIT(&render->m_queues);
    render->m_queue_stack_count = 0;
    render->m_queue_stack_capacity = 0;
    render->m_queue_stack = NULL;

    TAILQ_INIT(&render->m_programs);
    bzero(render->m_buildin_programs, sizeof(render->m_buildin_programs));

    TAILQ_INIT(&render->m_materials);

    render->m_data_buf = render + 1;
    render->m_data_buf_capacity = buf_capacity;
    render->m_data_buf_wp = 0;

    render->m_render_state = ui_runtime_render_state_create(render, NULL);
    if (render->m_render_state == NULL) {
        CPE_ERROR(module->m_em, "%s: render: create: create root state fail", ui_runtime_module_name(module));
        mem_free(module->m_alloc, render);
        return NULL;
    }

    if (ui_runtime_render_matrix_reset(render) != 0) {
        CPE_ERROR(module->m_em, "%s: render: create: init matrix fail", ui_runtime_module_name(module));
        ui_runtime_render_state_free(render->m_render_state);
        mem_free(module->m_alloc, render);
        return NULL;
    }
    
    if (module->m_render_backend) {
        if (ui_runtime_render_init(render) != 0) {
            uint8_t i;

            for(i = 0; i < CPE_ARRAY_SIZE(render->m_matrix_stacks); ++i) {
                mem_free(module->m_alloc, render->m_matrix_stacks[i].m_buf);
            }
            
            ui_runtime_render_state_free(render->m_render_state);
            mem_free(module->m_alloc, render);
            return NULL;
        }
    }

    module->m_render = render;

    return render;
}

void ui_runtime_render_free(ui_runtime_render_t render) {
    ui_runtime_module_t module = render->m_module;
    uint8_t i;
    
    if (render->m_worker) {
        ui_runtime_render_worker_free(render->m_worker);
        render->m_worker = NULL;
    }
        
    assert(render == module->m_render);
    module->m_render = NULL;

    if (render->m_inited) ui_runtime_render_fini(render, 0);
    assert(!render->m_inited);
    assert(render->m_default_queue == NULL);

    while(!TAILQ_EMPTY(&render->m_cameras)) {
        ui_runtime_render_camera_free(TAILQ_FIRST(&render->m_cameras));
    }
    assert(render->m_active_camera == NULL);
           
    assert(render->m_render_state);
    ui_runtime_render_state_free(render->m_render_state);
    render->m_render_state = NULL;

    if (render->m_queue_stack) {
        mem_free(module->m_alloc, render->m_queue_stack);
        render->m_queue_stack = NULL;
        render->m_queue_stack_capacity = 0;
    }

    for(i = 0; i < CPE_ARRAY_SIZE(render->m_matrix_stacks); ++i) {
        mem_free(module->m_alloc, render->m_matrix_stacks[i].m_buf);
    }

    if (render->m_scissor_buf) {
        mem_free(module->m_alloc, render->m_scissor_buf);
    }

    mem_free(module->m_alloc, render);
}

gd_app_context_t ui_runtime_render_app(ui_runtime_render_t render) {
    return render->m_module->m_app;
}

ui_runtime_module_t ui_runtime_render_module(ui_runtime_render_t render) {
    return render->m_module;
}

ui_cache_manager_t ui_runtime_render_cache_mgr(ui_runtime_render_t render) {
    return render->m_module->m_cache_mgr;
}

ui_data_mgr_t ui_runtime_render_data_mgr(ui_runtime_render_t render) {
    return render->m_module->m_data_mgr;
}

ui_runtime_render_worker_t ui_runtime_render_worker(ui_runtime_render_t render) {
    return render->m_worker;
}

ui_runtime_render_statistics_t ui_runtime_render_last_commit_statistics(ui_runtime_render_t render) {
    return &render->m_last_commit_statics;
}

ui_runtime_render_commit_state_t ui_runtime_render_commit_state(ui_runtime_render_t render) {
    return render->m_commit_state;
}

ui_color_t ui_runtime_render_clear_color(ui_runtime_render_t render) {
    return &render->m_clear_color;
}

void ui_render_render_set_clear_color(ui_runtime_render_t render, ui_color_t color) {
    render->m_clear_color = *color;
}

ui_runtime_render_t ui_runtime_module_render(ui_runtime_module_t module) {
    return module->m_render;
}

uint8_t ui_runtime_render_is_depth_test_for_2d(ui_runtime_render_t render) {
    return render->m_is_depth_test_for_2d;
}

void ui_runtime_render_set_is_depth_test_for_2d(ui_runtime_render_t render, uint8_t b) {
    render->m_is_depth_test_for_2d = b;
}

ui_vector_2_t ui_runtime_render_view_size(ui_runtime_render_t render) {
    return &render->m_view_size;
}

void ui_runtime_render_set_view_size(ui_runtime_render_t render, ui_vector_2_t view_size) {
    if (ui_vector_2_cmp(&render->m_view_size, view_size) == 0) return;

    if (render->m_view_size.x > 0.0f && render->m_view_size.y > 0.0f) {
        /*ignore resize*/
        return;
    }
    
    render->m_view_size = *view_size;
    if (render->m_active_camera) {
        ui_runtime_render_camera_update(render->m_active_camera);
    }
}

ui_runtime_render_state_t ui_runtime_render_render_state(ui_runtime_render_t render) {
    return render->m_render_state;
}

uint32_t ui_runtime_render_data_buf_capacity(ui_runtime_render_t render) {
    return render->m_data_buf_capacity;
}

int ui_runtime_render_copy_buf(ui_runtime_render_t render, ui_runtime_render_buff_use_t o_buff_use, ui_runtime_render_buff_use_t i_buff_use) {
    uint32_t buf_size;
    void * buf;
    
    if (i_buff_use->m_data_source != ui_runtime_render_buff_source_inline) {
        * o_buff_use = *i_buff_use;
        return 0;
    }

    /*已经是内部的数据块了 */
    if (i_buff_use->m_inline.m_buf >= render->m_data_buf
        && (i_buff_use->m_inline.m_buf < (void*)((char*)render->m_data_buf + render->m_data_buf_wp)))
    {
        * o_buff_use = *i_buff_use;
        return 0;
    }
        
    buf_size = ui_runtime_render_buff_type_stride(i_buff_use->m_inline.m_e_type) * i_buff_use->m_inline.m_count;
    if (buf_size > (render->m_data_buf_capacity - render->m_data_buf_wp)) {
        CPE_ERROR(
            render->m_module->m_em,
            "ui_runtime_render_copy_buf: not-enough-buf, capacity=%d, used=%d, require=%d",
            render->m_data_buf_capacity, render->m_data_buf_wp, buf_size);
        return -1;
    }

    buf = ((char*)render->m_data_buf) + render->m_data_buf_wp;
    memcpy(buf, i_buff_use->m_inline.m_buf, buf_size);

    CPE_PAL_ALIGN_DFT(buf_size);
    render->m_data_buf_wp += buf_size;
    
    * o_buff_use = * i_buff_use;
    o_buff_use->m_inline.m_buf = buf;
    
    return 0;
}

void * ui_runtime_render_alloc_buf(ui_runtime_render_t render, ui_runtime_render_buff_type_t e_type, uint32_t count) {
    uint32_t buf_size;
    void * buf;
    
    buf_size = ui_runtime_render_buff_type_stride(e_type) * count;
    if (buf_size > (render->m_data_buf_capacity - render->m_data_buf_wp)) {
        CPE_ERROR(
            render->m_module->m_em,
            "ui_runtime_render_alloc_buf: not-enough-buf, capacity=%d, used=%d, require=%d",
            render->m_data_buf_capacity, render->m_data_buf_wp, buf_size);
        return NULL;
    }

    buf = ((char*)render->m_data_buf) + render->m_data_buf_wp;

    CPE_PAL_ALIGN_DFT(buf_size);
    render->m_data_buf_wp += buf_size;
        
    return buf;
}

void * ui_runtime_render_append_buf(ui_runtime_render_t render, ui_runtime_render_buff_use_t buff_use, uint32_t append_count) {
    uint32_t e_size;
    uint32_t buf_size;
    uint32_t begin_pos;
    void * buf;

    assert(buff_use->m_data_source == ui_runtime_render_buff_source_inline);

    if (buff_use->m_inline.m_buf < render->m_data_buf
        || (buff_use->m_inline.m_buf >= (void*)((char*)render->m_data_buf + render->m_data_buf_wp)))
    {
        CPE_ERROR(render->m_module->m_em, "ui_runtime_render_resize_buf: input-buf is not inline buf");
        return NULL;
    }

    e_size = ui_runtime_render_buff_type_stride(buff_use->m_inline.m_e_type);
    buf_size = e_size * buff_use->m_inline.m_count;
    CPE_PAL_ALIGN_DFT(buf_size);

    begin_pos = (char *)buff_use->m_inline.m_buf - (char*)render->m_data_buf;
    if (begin_pos + buf_size != render->m_data_buf_wp) {
        CPE_ERROR(
            render->m_module->m_em,
            "ui_runtime_render_alloc_buf: buf-range=[%d~%d), wp=%d, mismatch!", begin_pos, begin_pos + buf_size, render->m_data_buf_wp);
        return NULL;
    }

    buf_size = e_size * (buff_use->m_inline.m_count + append_count);
    if (begin_pos + buf_size > render->m_data_buf_capacity) {
        CPE_ERROR(
            render->m_module->m_em,
            "ui_runtime_render_alloc_buf: not-enough-buf, capacity=%d, used=%d, wp=%d, require=%d",
            render->m_data_buf_capacity, begin_pos, render->m_data_buf_wp, buf_size);
        return NULL;
    }
        
    CPE_PAL_ALIGN_DFT(buf_size);
    render->m_data_buf_wp = begin_pos + buf_size;

    buf = ((char*)render->m_data_buf) + begin_pos + e_size * buff_use->m_inline.m_count;
    buff_use->m_inline.m_count += append_count;
    
    return buf;
}

int ui_runtime_render_set_buildin_program_state(
    ui_runtime_render_t render, ui_runtime_render_program_buildin_t buildin, ui_runtime_render_program_state_t program_state)
{
    ui_runtime_module_t module = render->m_module;

    if (buildin >= ui_runtime_render_program_buildin_count) {
        CPE_ERROR(module->m_em, "ui_runtime: program bind buildin: buildin %d unkonwn", buildin);
        return -1;
    }

    if (render->m_buildin_programs[buildin] != NULL) {
        CPE_ERROR(module->m_em, "ui_runtime: program bind buildin: buildin %d already have bind program", buildin);
        return -1;
    }

    render->m_buildin_programs[buildin] = program_state;
    return 0;
}

int ui_runtime_render_init(ui_runtime_render_t render) {
    ui_runtime_module_t module = render->m_module;
    ui_runtime_render_backend_t backend = module->m_render_backend;
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    
    if (backend == NULL) {
        CPE_ERROR(module->m_em, "%s: render: init: backend not exist!", ui_runtime_module_name(module));
        return -1;
    }

    if (render->m_inited) {
        ui_runtime_render_fini(render, 1);
    }
    
    if (backend->m_render_bind(backend->m_ctx, render) != 0) {
        CPE_ERROR(module->m_em, "%s: render: init: backend bind fail!", ui_runtime_module_name(module));
        return -1;
    }

    if (render->m_view_size.x > 0.0f && render->m_view_size.y > 0.0f) {
        ui_runtime_render_camera_t camera;
        TAILQ_FOREACH(camera, &render->m_cameras, m_next) {
            ui_runtime_render_camera_update(camera);
            if (camera == render->m_active_camera) {
                ui_runtime_render_matrix_load(render, ui_runtime_render_matrix_stack_projection, &camera->m_transform);
                ui_runtime_render_matrix_load_identity(render, ui_runtime_render_matrix_stack_modelview);
            }
        }
    }

    if (render->m_default_queue == NULL) {
        render->m_default_queue = ui_runtime_render_queue_create(render);
        if (render->m_default_queue == NULL) {
            CPE_ERROR(module->m_em, "%s: render: init: create default queue fail!", ui_runtime_module_name(module));
            backend->m_render_unbind(backend->m_ctx, render);
            return -1;
        }

        if (ui_runtime_render_queue_push(render->m_default_queue) != 0) {
            CPE_ERROR(module->m_em, "%s: render: init: push default queue fail!", ui_runtime_module_name(module));
            ui_runtime_render_queue_free(render->m_default_queue);
            render->m_default_queue = NULL;
            backend->m_render_unbind(backend->m_ctx, render);
            return -1;
        }
    }

    ui_cache_manager_ress(module->m_cache_mgr, &res_it);
    while((res = ui_cache_res_it_next(&res_it))) {
        if (ui_cache_res_type(res) != ui_cache_res_type_texture) continue;
        if (ui_cache_res_load_state(res) == ui_cache_res_loaded) continue;
        if (ui_cache_texture_data_buf(res) == NULL) continue;
        
        if (ui_cache_res_load_sync(res, NULL) != 0) {
            CPE_ERROR(module->m_em, "%s: render: auto load runtime texture fail!", ui_runtime_module_name(module));
        }
    }

    render->m_inited = 1;
    
    return 0;
}

void ui_runtime_render_fini(ui_runtime_render_t render, uint8_t is_external_unloaded) {
    ui_runtime_module_t module = render->m_module;
    struct ui_cache_res_it res_it;
    ui_cache_res_t res;
    uint8_t i;

    if (!render->m_inited) return;

    assert(module->m_render_backend);
    
    /*清理上一帧的资源 */
    ui_runtime_render_begin(render, NULL);
    assert(render->m_commit_state == ui_runtime_render_commit_state_prepaire);
    render->m_commit_state = ui_runtime_render_commit_state_clear;

    /*清理所有texture */
    ui_cache_manager_ress(module->m_cache_mgr, &res_it);
    while((res = ui_cache_res_it_next(&res_it))) {
        if (ui_cache_res_type(res) != ui_cache_res_type_texture) continue;
        if (ui_cache_res_load_state(res) != ui_cache_res_loaded) continue;

        ui_cache_res_unload(res, is_external_unloaded);
        CPE_ERROR(
            module->m_em, "%s: render: unload externan %s, state=%s!",
            ui_runtime_module_name(module), ui_cache_res_path(res),
            ui_cache_res_load_state_to_str(ui_cache_res_load_state(res)));
    }
        
    /*清理所有的内置渲染程序 */
    for(i = 0; i < CPE_ARRAY_SIZE(render->m_buildin_programs); ++i) {
        if (render->m_buildin_programs[i]) {
            ui_runtime_render_program_state_free(render->m_buildin_programs[i]);
            render->m_buildin_programs[i] = NULL;
        }
    }

    /*清理所有的渲染程序 */
    while(!TAILQ_EMPTY(&render->m_programs)) {
        ui_runtime_render_program_free(TAILQ_FIRST(&render->m_programs));
    }
    bzero(render->m_buildin_programs, sizeof(render->m_buildin_programs));

    while(!TAILQ_EMPTY(&render->m_queues)) {
        ui_runtime_render_queue_free(TAILQ_FIRST(&render->m_queues));
    }
    assert(render->m_default_queue == NULL);
    render->m_queue_stack_count = 0;
    
    while(!TAILQ_EMPTY(&render->m_materials)) {
        ui_runtime_render_material_free(TAILQ_FIRST(&render->m_materials));
    }

    module->m_render_backend->m_render_unbind(module->m_render_backend->m_ctx, render);

    render->m_inited = 0;
}
