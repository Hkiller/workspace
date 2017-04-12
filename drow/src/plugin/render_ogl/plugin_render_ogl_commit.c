#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/bitarry.h"
#include "render/utils/ui_color.h"
#include "render/runtime/ui_runtime_render.h"
#include "render/runtime/ui_runtime_render_cmd.h"
#include "plugin_render_ogl_module_i.h"
#include "plugin_render_ogl_cache_i.h"
#include "plugin_render_ogl_commit_batch_2d.h"
#include "plugin_render_ogl_commit_batch_3d.h"

static void plugin_render_ogl_commit_flush_2d(plugin_render_ogl_module_t module) {
    plugin_render_ogl_batch_2d_commit(module);    
}

static void plugin_render_ogl_commit_flush_3d(plugin_render_ogl_module_t module) {
    plugin_render_ogl_batch_3d_commit(module);
}

static void plugin_render_ogl_commit_flush(plugin_render_ogl_module_t module) {
    plugin_render_ogl_commit_flush_2d(module);
    plugin_render_ogl_commit_flush_3d(module);
}

void plugin_render_ogl_commit_begin(void * ctx, ui_runtime_render_t render) {
    plugin_render_ogl_module_t module = ctx;

    bzero(&module->m_statistics, sizeof(module->m_statistics));

    if (cpe_ba_get(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_scissor)) {
        plugin_render_ogl_set_scissor(module, NULL);
    }
    
    plugin_render_ogl_set_view_point(module, ui_runtime_render_view_size(render));
    plugin_render_ogl_clear(module, ui_runtime_render_clear_color(render));
}

void plugin_render_ogl_commit_done(void * ctx, ui_runtime_render_t render, ui_runtime_render_statistics_t statistics) {
    plugin_render_ogl_module_t module = ctx;
    
    glFlush();
    
    *statistics = module->m_statistics;
}

void plugin_render_ogl_commit_group_begin(void * ctx, ui_runtime_render_t render, ui_runtime_render_queue_group_t cmd_group_type) {
    switch(cmd_group_type) {
    case ui_runtime_render_queue_group_neg:
    case ui_runtime_render_queue_group_zero:
    case ui_runtime_render_queue_group_pos:
        if(ui_runtime_render_is_depth_test_for_2d(render)) {
            /* glEnable(GL_DEPTH_TEST); */
            /* glDepthMask(GL_TRUE); */
            /* glEnable(GL_BLEND); */
            /* RenderState::StateBlock::_defaultState->setDepthTest(true); */
            /* RenderState::StateBlock::_defaultState->setDepthWrite(true); */
            /* RenderState::StateBlock::_defaultState->setBlend(true); */
        }
        else {
            /* glDisable(GL_DEPTH_TEST); */
            /* glDepthMask(GL_FALSE); */
            /* glEnable(GL_BLEND); */
            /* RenderState::StateBlock::_defaultState->setDepthTest(false); */
            /* RenderState::StateBlock::_defaultState->setDepthWrite(false); */
            /* RenderState::StateBlock::_defaultState->setBlend(true); */
        }
        /* glDisable(GL_CULL_FACE); */
        //RenderState::StateBlock::_defaultState->setCullFace(false);
        break;
    case ui_runtime_render_queue_group_3d:
    case ui_runtime_render_queue_group_3d_transparent:
        /* glEnable(GL_DEPTH_TEST); */
        /* glDepthMask(GL_FALSE); */
        /* glEnable(GL_BLEND); */
        /* glEnable(GL_CULL_FACE); */
        /* RenderState::StateBlock::_defaultState->setDepthTest(true); */
        /* RenderState::StateBlock::_defaultState->setDepthWrite(false); */
        /* RenderState::StateBlock::_defaultState->setBlend(true); */
        /* RenderState::StateBlock::_defaultState->setCullFace(true); */
        break;
    default:
        break;
    }
}

void plugin_render_ogl_commit_group_done(void * ctx, ui_runtime_render_t render, ui_runtime_render_queue_group_t cmd_group_type) {
    plugin_render_ogl_module_t module = ctx;
    
    plugin_render_ogl_commit_flush(module);
}

void plugin_render_ogl_commit_cmd(void * ctx, ui_runtime_render_t render, ui_runtime_render_cmd_t cmd) {
    plugin_render_ogl_module_t module = ctx;
    
    switch(ui_runtime_render_cmd_type(cmd)) {
    case ui_runtime_render_cmd_triangles:
        /* flush other queues */
        plugin_render_ogl_commit_flush_3d(module);

        /* flush own queue when buffer is full */
        if (ui_runtime_render_cmd_vertex_count(cmd) > (module->m_capacity_vertex_vbo_size - module->m_batch_2d->m_queued_triangle_vertex_count)
            || ui_runtime_render_cmd_index_count(cmd) > (module->m_capacity_index_vbo_size - module->m_batch_2d->m_queued_triangle_index_count)
            )
        {
            plugin_render_ogl_batch_2d_commit(module);
        }

        plugin_render_ogl_batch_2d_queue_cmd(module, cmd);
        break;
    default:
        CPE_ERROR(module->m_em, "plugin_render_ogl_commit_cmd: not support command type %s", ui_runtime_render_cmd_type_str(cmd));
    }

    module->m_statistics.m_cmd_count++;
}
