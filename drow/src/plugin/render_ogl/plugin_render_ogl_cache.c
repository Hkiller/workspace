#include <assert.h>
#include "cpe/utils/bitarry.h"
#include "cpe/pal/pal_strings.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_res_plugin.h"
#include "plugin_render_ogl_cache_i.h"
#include "plugin_render_ogl_texture_i.h"
#include "plugin_render_ogl_utils.h"

int plugin_render_ogl_cache_init(plugin_render_ogl_module_t module) {
    plugin_render_ogl_cache_t cache;
    uint32_t i;

    cache = mem_alloc(module->m_alloc, sizeof(struct plugin_render_ogl_cache));
    if (cache == NULL) {
        CPE_ERROR(module->m_em, "plugin_render_ogl_cache_init: alloc fail!");
        return -1;
    }

    cache->m_view_point.x = cache->m_view_point.y = -1.0f;
    cache->m_clear_color.a = cache->m_clear_color.b = cache->m_clear_color.g = cache->m_clear_color.r = -1.0f;
    
    cache->m_program = (GLuint)-1;
    cache->m_active_texture = (uint8_t)-1;
    
    for(i = 0; i < CPE_ARRAY_SIZE(cache->m_textures); i++) {
        cache->m_textures[i] = (GLuint)-1;
    }
    bzero(&cache->m_state, sizeof(cache->m_state));
    cache->m_vao = 0;
    cache->m_attr_flag = 0;

    for(i = 0; i < CPE_ARRAY_SIZE(cache->m_buffs); ++i) {
        cache->m_buffs[i] = 0;
    }
    
    module->m_cache = cache;

    plugin_render_ogl_set_blend(module, NULL);
    
    return 0;
}

void plugin_render_ogl_cache_fini(plugin_render_ogl_module_t module) {
    mem_free(module->m_alloc, module->m_cache);
    module->m_cache = NULL;
}

void plugin_render_ogl_state_save(void * ctx, ui_runtime_render_state_data_t state_data) {
    plugin_render_ogl_module_t module = ctx;
    *state_data = module->m_cache->m_state;
}

void plugin_render_ogl_state_restore(void * ctx, ui_runtime_render_state_data_t queue) {
    /* if (ogl_queue->m_is_cull_enabled) { */
    /*     //RenderState::StateBlock::_defaultState->setCullFace(true); */
    /* } */
    /* else { */
    /*     //RenderState::StateBlock::_defaultState->setCullFace(false); */
    /* } */

    /* if (ogl_queue->m_is_depth_enabled) { */
    /*     //RenderState::StateBlock::_defaultState->setDepthTest(true); */
    /* } */
    /* else { */
    /*     //RenderState::StateBlock::_defaultState->setDepthTest(false); */
    /* } */
    
    /* //RenderState::StateBlock::_defaultState->setDepthWrite(_isDepthEnabled); */
}

void plugin_render_ogl_set_view_point(plugin_render_ogl_module_t module, ui_vector_2_t sz) {
    if (module->m_cache->m_view_point.x != sz->x || module->m_cache->m_view_point.y != sz->y) {
        module->m_cache->m_view_point = *sz;
        glViewport(0, 0, sz->x, sz->y);
        
        if (module->m_debug) {
            CPE_INFO(module->m_em, "ogl: glViewport %f,%f", sz->x, sz->y);
        }
    }
}

void plugin_render_ogl_clear(plugin_render_ogl_module_t module, ui_color_t c) {
    if (ui_color_cmp(&module->m_cache->m_clear_color, c) != 0) {
        module->m_cache->m_clear_color = *c;
        glClearColor(c->r, c->g, c->b, c->a);
        
        if (module->m_debug) {
            CPE_INFO(module->m_em, "ogl: glClearColor (%f,%f,%f,%f)", c->r, c->g, c->b, c->a);
        }
    }
    
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE);
}

void plugin_render_ogl_active_vao(plugin_render_ogl_module_t module, GLuint vao) {
#if DROW_RENDER_USE_VAO        
    if (module->m_capacity_supports_shareable_vao) {
        if (module->m_cache->m_vao != vao) {
            module->m_cache->m_vao = vao;
            glBindVertexArrayOES(vao);

            if (module->m_debug) {
                CPE_INFO(module->m_em, "ogl: glBindVertexArray %d", vao);
            }
        }
    }
#endif        
}

void plugin_render_ogl_bind_buffer(plugin_render_ogl_module_t module, plugin_render_ogl_buffer_type_t t, GLuint buffer) {
    assert(t < CPE_ARRAY_SIZE(module->m_cache->m_buffs));
    
    if (module->m_cache->m_buffs[t] != buffer) {
        module->m_cache->m_buffs[t] = buffer;
        switch(t) {
        case plugin_render_ogl_buffer_array:
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            if (module->m_debug >= 2) {
                CPE_INFO(module->m_em, "ogl: glBindBuffer vertex ==> %d", buffer);
            }
            break;
        case plugin_render_ogl_buffer_element:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            if (module->m_debug >= 2) {
                CPE_INFO(module->m_em, "ogl: glBindBuffer element ==> %d", buffer);
            }
            break;
        default:
            assert(0);
        };
    }
}

void plugin_render_ogl_use_program(plugin_render_ogl_module_t module, GLuint program) {
    if (module->m_cache->m_program != program) {
        module->m_cache->m_program = program;
        glUseProgram(program);
    }
}

static uint8_t plugin_render_ogl_active_texture(plugin_render_ogl_module_t module, uint8_t index) {
    if(module->m_cache->m_active_texture != index) {
        module->m_cache->m_active_texture = index;
        glActiveTexture(GL_TEXTURE0 + index);
        if (module->m_debug >= 2) {
            CPE_INFO(module->m_em, "ogl: glActiveTexture %d", index);
        }
        return 1;
    }
    else {
        return 0;
    }
}

	/* glUseProgram(ogl_program->m_program_id); */
void plugin_render_ogl_enable_program_attrs(plugin_render_ogl_module_t module, uint32_t flags) {
    uint8_t i;

    for(i = 0; i < ui_runtime_render_program_attr_max; i++) {
        cpe_ba_value_t enabled = cpe_ba_get(&flags, i);
        cpe_ba_value_t enabled_before = cpe_ba_get(&module->m_cache->m_attr_flag, i);

        if(enabled != enabled_before) {
            if(enabled) {
                glEnableVertexAttribArray(i);
                if (module->m_debug >= 2) {
                    CPE_INFO(module->m_em, "ogl: vertex attrib array %d ==> enable", i);
                }
            }
            else {
                glDisableVertexAttribArray(i);
                if (module->m_debug >= 2) {
                    CPE_INFO(module->m_em, "ogl: vertex attrib array %d ==> disable", i);
                }
            }
        }
    }

    module->m_cache->m_attr_flag = flags;
}

void plugin_render_ogl_set_texture_filter(plugin_render_ogl_module_t module, GLenum pname, ui_runtime_render_texture_filter_t filter) {
    switch(filter) {
    case ui_runtime_render_filter_linear:
        glTexParameteri(GL_TEXTURE_2D, pname, GL_LINEAR);
        break;
    case ui_runtime_render_filter_nearest:
        glTexParameteri(GL_TEXTURE_2D, pname, GL_NEAREST);
        break;
    }
}

void plugin_render_ogl_set_texture_wrapping(plugin_render_ogl_module_t module, GLenum pname, ui_runtime_render_texture_wrapping_t wrap) {
    switch(wrap) {
    case ui_runtime_render_texture_repeat:
        glTexParameteri(GL_TEXTURE_2D, pname, GL_REPEAT );
        break;
    case ui_runtime_render_texture_mirrored_repeat:
        glTexParameteri(GL_TEXTURE_2D, pname, GL_MIRRORED_REPEAT );
        break;
    case ui_runtime_render_texture_clamp_to_edge:
        glTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP_TO_EDGE );
        break;
    case ui_runtime_render_texture_clamp_to_border:
        glTexParameteri(GL_TEXTURE_2D, pname, GL_CLAMP_TO_EDGE );
        break;
    }
}

void plugin_render_ogl_bind_texture_for_manip(plugin_render_ogl_module_t module, ui_cache_res_t res, uint8_t index) {
    plugin_render_ogl_texture_t ogl_texture = (plugin_render_ogl_texture_t)ui_cache_res_plugin_data(res);
    GLuint texture_id = ogl_texture->m_texture_id;

    assert(index < CPE_ARRAY_SIZE(module->m_cache->m_textures));

    if (plugin_render_ogl_active_texture(module, index)
        || module->m_cache->m_textures[index] != texture_id)
    {
        module->m_cache->m_textures[index] = texture_id;
		glBindTexture(GL_TEXTURE_2D, texture_id);

        if (module->m_debug >= 2) {
            CPE_INFO(module->m_em, "ogl: glBindTexture %d ==> %d", index, texture_id);
        }
    }
}

void plugin_render_ogl_bind_texture(
    plugin_render_ogl_module_t module,
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t index)
{
    plugin_render_ogl_texture_t ogl_texture = (plugin_render_ogl_texture_t)ui_cache_res_plugin_data(res);
    GLuint texture_id = ogl_texture->m_texture_id;
    
    assert(index < CPE_ARRAY_SIZE(module->m_cache->m_textures));

	if (plugin_render_ogl_active_texture(module, index) || module->m_cache->m_textures[index] != texture_id) {
        module->m_cache->m_textures[index] = texture_id;
		glBindTexture(GL_TEXTURE_2D, texture_id);

        if (module->m_debug >= 2) {
            CPE_INFO(module->m_em, "ogl: glBindTexture %d ==> %d", index, texture_id);
        }
    }

    if (ogl_texture->m_min_filter != min_filter) {
        plugin_render_ogl_set_texture_filter(module, GL_TEXTURE_MIN_FILTER, min_filter);
        ogl_texture->m_min_filter = min_filter;
    }

    if (ogl_texture->m_mag_filter != mag_filter) {
        plugin_render_ogl_set_texture_filter(module, GL_TEXTURE_MAG_FILTER, mag_filter);
        ogl_texture->m_mag_filter = mag_filter;
    }

    if (ogl_texture->m_wrap_s != wrap_s) {
        plugin_render_ogl_set_texture_wrapping(module, GL_TEXTURE_WRAP_S, wrap_s);
        ogl_texture->m_wrap_s = wrap_s;
    }

    if (ogl_texture->m_wrap_t != wrap_t) {
        plugin_render_ogl_set_texture_wrapping(module, GL_TEXTURE_WRAP_T, wrap_t);
        ogl_texture->m_wrap_t = wrap_t;
    }

    if (ui_cache_texture_is_dirty(res)) {
        plugin_render_ogl_texture_upload(res);
    }

    DROW_RENDER_OGL_ERROR_DEBUG(module);
}

static GLenum plugin_render_ogl_blend_factor_to_gl(ui_runtime_render_blend_factor_t factor) {
    switch(factor) {
    case ui_runtime_render_dst_alpha:
        return GL_DST_ALPHA;
    case ui_runtime_render_dst_color:
        return GL_DST_COLOR;
    case ui_runtime_render_one:
        return GL_ONE;
    case ui_runtime_render_one_minus_dst_alpha:
        return GL_ONE_MINUS_DST_ALPHA;
    case ui_runtime_render_one_minus_dst_color:
        return GL_ONE_MINUS_DST_COLOR;
    case ui_runtime_render_one_minus_src_alpha:
        return GL_ONE_MINUS_SRC_ALPHA;
    case ui_runtime_render_one_minus_src_color:
        return GL_ONE_MINUS_SRC_COLOR;
    case ui_runtime_render_src_alpha:
        return GL_SRC_ALPHA;
    case ui_runtime_render_src_color:
        return GL_SRC_COLOR;
    case ui_runtime_render_zero:
        return GL_ZERO;
    default:
        assert(0);
        return GL_ZERO;
    }
}

void plugin_render_ogl_set_blend(plugin_render_ogl_module_t module, ui_runtime_render_blend_t blend) {
    if (cpe_ba_get(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_blend)
        && ((blend == NULL && module->m_cache->m_state.m_blend_on == 0)
            || (blend && module->m_cache->m_state.m_blend_on
                && (module->m_cache->m_state.m_blend.m_src_factor == blend->m_src_factor
                    && module->m_cache->m_state.m_blend.m_dst_factor == blend->m_dst_factor)
                ))) return;
        
    if (blend == NULL) {
        glDisable(GL_BLEND);
    }
    else {
        if (module->m_cache->m_state.m_blend_on == 0) {
            glEnable(GL_BLEND);
        }
        
        glBlendFunc(
            plugin_render_ogl_blend_factor_to_gl(blend->m_src_factor),
            plugin_render_ogl_blend_factor_to_gl(blend->m_dst_factor));
    }

    cpe_ba_set(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_blend, cpe_ba_true);
    if (blend) {
        module->m_cache->m_state.m_blend_on = 1;
        module->m_cache->m_state.m_blend = *blend;
    }
    else {
        module->m_cache->m_state.m_blend_on = 0;
    }
}

void plugin_render_ogl_set_scissor(plugin_render_ogl_module_t module, ui_rect_t scissor) {
    uint8_t scissor_on = scissor ? 1 : 0;
    if (cpe_ba_get(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_scissor)
        && module->m_cache->m_state.m_scissor_on == scissor_on
        && (scissor_on == 0
            || ui_rect_cmp(&module->m_cache->m_state.m_scissor, scissor) == 0)) return;

    if (scissor == NULL) {
        glDisable(GL_SCISSOR_TEST);
    }
    else {
        if (module->m_cache->m_state.m_scissor_on == 0) {
            glEnable(GL_SCISSOR_TEST);
        }

        if (ui_rect_cmp(&module->m_cache->m_state.m_scissor, scissor) != 0) {
            glScissor(
                scissor->lt.x, scissor->lt.y,
                ui_rect_width(scissor), ui_rect_height(scissor));
        }
    }

    cpe_ba_set(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_scissor, cpe_ba_true);
    module->m_cache->m_state.m_scissor_on = scissor_on;
    if (scissor) module->m_cache->m_state.m_scissor = *scissor;
}

void plugin_render_ogl_set_cull_face(plugin_render_ogl_module_t module, ui_runtime_render_cull_face_t face) {
    if (cpe_ba_get(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_cull_face) && module->m_cache->m_state.m_cull_face == face) return;

    if (face == ui_runtime_render_cull_face_none) {
        glDisable(GL_CULL_FACE);
    }
    else {
        if (module->m_cache->m_state.m_cull_face == ui_runtime_render_cull_face_none) {
            glEnable(GL_CULL_FACE);
        }
        
        switch(face) {
        case ui_runtime_render_cull_face_back:
            glCullFace((GLenum)GL_BACK);
            break;
        case ui_runtime_render_cull_face_front:
            glCullFace((GLenum)GL_FRONT);
            break;
        case ui_runtime_render_cull_face_front_and_back:
            glCullFace((GLenum)GL_FRONT_AND_BACK);
            break;
        default:
            break;
        }
    }

    cpe_ba_set(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_cull_face, cpe_ba_true);
    module->m_cache->m_state.m_cull_face = face;
}

void plugin_render_ogl_buff_copy_from_mem(
    plugin_render_ogl_module_t module,
    plugin_render_ogl_buffer_type_t type, uint32_t size, void const * data, GLenum usage)
{
    GLenum target;
#if defined DROW_RENDER_USE_MAP_BUFFER
    char * buf;
#endif

    switch(type) {
    case plugin_render_ogl_buffer_array:
        target = GL_ARRAY_BUFFER;
        break;
    case plugin_render_ogl_buffer_element:
        target = GL_ELEMENT_ARRAY_BUFFER;
        break;
    default:
        assert(0);
        return;
    }

#if defined DROW_RENDER_USE_MAP_BUFFER
    glBufferData(target, size, NULL, usage);
    if ((buf = glMapBufferOES(target, GL_WRITE_ONLY_OES))) {
        memcpy(buf, data, size);
    }
    glUnmapBufferOES(target);
#else
    glBufferData(target, size, data, usage);
#endif    

    DROW_RENDER_OGL_ERROR_DEBUG(module);
}
