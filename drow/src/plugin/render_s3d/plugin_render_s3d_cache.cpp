#include <assert.h>
#include "cpe/utils/bitarry.h"
#include "cpe/pal/pal_strings.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_texture.h"
#include "render/cache/ui_cache_res_plugin.h"
#include "plugin_render_s3d_cache_i.hpp"
#include "plugin_render_s3d_texture_i.hpp"
#include "plugin_render_s3d_program_i.hpp"
#include "plugin_render_s3d_utils.hpp"

int plugin_render_s3d_cache_init(plugin_render_s3d_module_t module) {
    plugin_render_s3d_cache_t  cache =
        (plugin_render_s3d_cache_t )mem_alloc(module->m_alloc, sizeof(struct plugin_render_s3d_cache));
    if (cache == NULL) {
        CPE_ERROR(module->m_em, "plugin_render_s3d_cache_init: alloc fail!");
        return -1;
    }
    bzero(cache, sizeof(*cache));

    cache->m_view_point.x = cache->m_view_point.y = -1.0f;

    module->m_cache = cache;

    plugin_render_s3d_set_blend(module, NULL);

    return 0;
}

void plugin_render_s3d_cache_fini(plugin_render_s3d_module_t module) {
    assert(module->m_cache->m_program == NULL);
    
    mem_free(module->m_alloc, module->m_cache);
    module->m_cache = NULL;
}

void plugin_render_s3d_cache_clear(plugin_render_s3d_module_t module) {
    plugin_render_s3d_cache_t cache = module->m_cache;

    for(uint8_t i = 0; i < CPE_ARRAY_SIZE(module->m_cache->m_textures); i++) {
        if (module->m_cache->m_textures[i]) {
            module->m_cache->m_textures[i]->m_active_at = -1;
            try {
                module->m_ctx3d->setTextureAt(i, internal::_null);
            }
            S3D_REPORT_EXCEPTION("cache_clear", );
        }
    }
    plugin_render_s3d_set_blend(module, NULL);
    
    bzero(cache, sizeof(*cache));
    cache->m_view_point.x = cache->m_view_point.y = -1.0f;
}


void plugin_render_s3d_state_save(void * ctx, ui_runtime_render_state_data_t state_data) {
    plugin_render_s3d_module_t module = (plugin_render_s3d_module_t)ctx;
    *state_data = module->m_cache->m_state;
}

void plugin_render_s3d_state_restore(void * ctx, ui_runtime_render_state_data_t queue) {
    /* if (s3d_queue->m_is_cull_enabled) { */
    /*     //RenderState::StateBlock::_defaultState->setCullFace(true); */
    /* } */
    /* else { */
    /*     //RenderState::StateBlock::_defaultState->setCullFace(false); */
    /* } */

    /* if (s3d_queue->m_is_depth_enabled) { */
    /*     //RenderState::StateBlock::_defaultState->setDepthTest(true); */
    /* } */
    /* else { */
    /*     //RenderState::StateBlock::_defaultState->setDepthTest(false); */
    /* } */
    
    /* //RenderState::StateBlock::_defaultState->setDepthWrite(_isDepthEnabled); */
}

void plugin_render_s3d_set_view_point(plugin_render_s3d_module_t module, ui_vector_2_t sz) {
    if (module->m_cache->m_view_point.x != sz->x || module->m_cache->m_view_point.y != sz->y) {
        try {
            module->m_ctx3d->configureBackBuffer(sz->x, sz->y, 2, true, false);
            module->m_ctx3d->setDepthTest(false, flash::display3D::Context3DCompareMode::ALWAYS);
        }
        S3D_REPORT_EXCEPTION_2("set view point: size (%f,%f)", sz->x, sz->y, return);
        
        module->m_cache->m_view_point = *sz;

        if (module->m_debug) {
            CPE_INFO(module->m_em, "s3d: configureBackBuffer: size=(%f,%f)", sz->x, sz->y);
        }
    }
}

void plugin_render_s3d_clear(plugin_render_s3d_module_t module, ui_color_t color) {
    try {
        module->m_ctx3d->clear(color->r, color->g, color->b, color->a, 1.0f, 0.0f, 0xffffffff);
    }
    S3D_REPORT_EXCEPTION("clear", );
}

void plugin_render_s3d_bind_texture(
    plugin_render_s3d_module_t module,
    ui_cache_res_t res,
    ui_runtime_render_texture_filter_t min_filter,
    ui_runtime_render_texture_filter_t mag_filter,
    ui_runtime_render_texture_wrapping_t wrap_s,
    ui_runtime_render_texture_wrapping_t wrap_t,
    uint8_t index)
{
    plugin_render_s3d_texture_t s3d_texture = (plugin_render_s3d_texture_t)ui_cache_res_plugin_data(res);
    
    assert(index < CPE_ARRAY_SIZE(module->m_cache->m_textures));

    if (s3d_texture->m_active_at != index) {
        if (s3d_texture->m_active_at >= 0) {
            try {
                module->m_ctx3d->setTextureAt(s3d_texture->m_active_at, internal::_null);
            }
            S3D_REPORT_EXCEPTION_1("bind_texture: unbind at %d", s3d_texture->m_active_at, );
            module->m_cache->m_textures[s3d_texture->m_active_at] = NULL;
        }

        if (module->m_cache->m_textures[index]) {
            module->m_cache->m_textures[index]->m_active_at = -1;
        }
        
        module->m_cache->m_textures[index] = s3d_texture;
        s3d_texture->m_active_at = index;
        module->m_ctx3d->setTextureAt(index, s3d_texture->m_texture);
    }

    if (s3d_texture->m_min_filter != min_filter) {
        //plugin_render_s3d_set_texture_filter(module, GL_TEXTURE_MIN_FILTER, min_filter);
        s3d_texture->m_min_filter = min_filter;
    }

    if (s3d_texture->m_mag_filter != mag_filter) {
        //plugin_render_s3d_set_texture_filter(module, GL_TEXTURE_MAG_FILTER, mag_filter);
        s3d_texture->m_mag_filter = mag_filter;
    }

    if (s3d_texture->m_wrap_s != wrap_s) {
        //plugin_render_s3d_set_texture_wrapping(module, GL_TEXTURE_WRAP_S, wrap_s);
        s3d_texture->m_wrap_s = wrap_s;
    }

    if (s3d_texture->m_wrap_t != wrap_t) {
        //plugin_render_s3d_set_texture_wrapping(module, GL_TEXTURE_WRAP_T, wrap_t);
        s3d_texture->m_wrap_t = wrap_t;
    }

    if (ui_cache_texture_is_dirty(res)) {
        plugin_render_s3d_texture_upload(res);
    }
}

static String plugin_render_s3d_blend_factor_to_s3d(ui_runtime_render_blend_factor_t factor) {
    switch(factor) {
    case ui_runtime_render_dst_alpha:
        return flash::display3D::Context3DBlendFactor::DESTINATION_ALPHA;
    case ui_runtime_render_dst_color:
        return flash::display3D::Context3DBlendFactor::DESTINATION_COLOR;
    case ui_runtime_render_one:
        return flash::display3D::Context3DBlendFactor::ONE;
    case ui_runtime_render_one_minus_dst_alpha:
        return flash::display3D::Context3DBlendFactor::ONE_MINUS_DESTINATION_ALPHA;
    case ui_runtime_render_one_minus_dst_color:
        return flash::display3D::Context3DBlendFactor::ONE_MINUS_DESTINATION_COLOR;
    case ui_runtime_render_one_minus_src_alpha:
        return flash::display3D::Context3DBlendFactor::ONE_MINUS_SOURCE_ALPHA;
    case ui_runtime_render_one_minus_src_color:
        return flash::display3D::Context3DBlendFactor::ONE_MINUS_SOURCE_COLOR;
    case ui_runtime_render_src_alpha:
        return flash::display3D::Context3DBlendFactor::SOURCE_ALPHA;
    case ui_runtime_render_src_color:
        return flash::display3D::Context3DBlendFactor::SOURCE_COLOR;
    case ui_runtime_render_zero:
        return flash::display3D::Context3DBlendFactor::ZERO;
    default:
        assert(0);
        return flash::display3D::Context3DBlendFactor::ZERO;
    }
}

void plugin_render_s3d_set_blend(plugin_render_s3d_module_t module, ui_runtime_render_blend_t blend) {
    if (cpe_ba_get(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_blend)
        && ((blend == NULL && module->m_cache->m_state.m_blend_on == 0)
            || (blend && module->m_cache->m_state.m_blend_on
                && (module->m_cache->m_state.m_blend.m_src_factor == blend->m_src_factor
                    && module->m_cache->m_state.m_blend.m_dst_factor == blend->m_dst_factor)
                ))) return;

    try {
        if (blend == NULL) {
            module->m_ctx3d->setBlendFactors(
                flash::display3D::Context3DBlendFactor::ONE,
                flash::display3D::Context3DBlendFactor::ZERO);
        }
        else {
            module->m_ctx3d->setBlendFactors(
                plugin_render_s3d_blend_factor_to_s3d(blend->m_src_factor),
                plugin_render_s3d_blend_factor_to_s3d(blend->m_dst_factor));
        }
    }
    S3D_REPORT_EXCEPTION("set_blend", );
    
    cpe_ba_set(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_blend, cpe_ba_true);
    if (blend) {
        module->m_cache->m_state.m_blend_on = 1;
        module->m_cache->m_state.m_blend = *blend;
    }
    else {
        module->m_cache->m_state.m_blend_on = 0;
    }
}

void plugin_render_s3d_set_scissor(plugin_render_s3d_module_t module, ui_rect_t scissor) {
    uint8_t scissor_on = scissor ? 1 : 0;
    if (cpe_ba_get(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_scissor)
        && module->m_cache->m_state.m_scissor_on == scissor_on
        && (scissor_on == 0
            || ui_rect_cmp(&module->m_cache->m_state.m_scissor, scissor) == 0)) return;

    if (scissor == NULL) {
        //glDisable(GL_SCISSOR_TEST);
    }
    else {
        if (module->m_cache->m_state.m_scissor_on == 0) {
            //glEnable(GL_SCISSOR_TEST);
        }

        if (ui_rect_cmp(&module->m_cache->m_state.m_scissor, scissor) != 0) {
            // glScissor(
            //     scissor->lt.x, scissor->lt.y,
            //     ui_rect_width(scissor), ui_rect_height(scissor));
        }
    }

    cpe_ba_set(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_scissor, cpe_ba_true);
    module->m_cache->m_state.m_scissor_on = scissor_on;
    if (scissor) module->m_cache->m_state.m_scissor = *scissor;
}

void plugin_render_s3d_set_cull_face(plugin_render_s3d_module_t module, ui_runtime_render_cull_face_t face) {
    if (cpe_ba_get(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_cull_face) && module->m_cache->m_state.m_cull_face == face) return;

    try {
        if (face == ui_runtime_render_cull_face_none) {
            module->m_ctx3d->setCulling(flash::display3D::Context3DTriangleFace::NONE);
        }
        else {
            switch(face) {
            case ui_runtime_render_cull_face_back:
                module->m_ctx3d->setCulling(flash::display3D::Context3DTriangleFace::BACK);
                break;
            case ui_runtime_render_cull_face_front:
                module->m_ctx3d->setCulling(flash::display3D::Context3DTriangleFace::FRONT);
                break;
            case ui_runtime_render_cull_face_front_and_back:
                module->m_ctx3d->setCulling(flash::display3D::Context3DTriangleFace::FRONT_AND_BACK);
                break;
            default:
                break;
            }
        }
    }
    S3D_REPORT_EXCEPTION("set_cull_face", );
    
    cpe_ba_set(&module->m_cache->m_state.m_bits, ui_runtime_render_state_tag_cull_face, cpe_ba_true);
    module->m_cache->m_state.m_cull_face = face;
}

void plugin_render_s3d_use_program(plugin_render_s3d_module_t module, plugin_render_s3d_program_t program) {
    if (module->m_cache->m_program == program) return;

    module->m_cache->m_program = program;
    if (module->m_cache->m_program) {
        module->m_ctx3d->setProgram(module->m_cache->m_program->m_program);
    }
    else {
        module->m_ctx3d->setProgram(internal::_null);
    }
}

void plugin_render_s3d_unbind_other_textures(plugin_render_s3d_module_t module, uint32_t using_textures) {
    for(uint8_t i = 0; i < CPE_ARRAY_SIZE(module->m_cache->m_textures); i++) {
        if (module->m_cache->m_textures[i]) {
            if (!cpe_ba_get(&using_textures, i)) {
                module->m_cache->m_textures[i]->m_active_at = -1;
                try {
                    module->m_ctx3d->setTextureAt(i, internal::_null);
                }
                S3D_REPORT_EXCEPTION("unbind_other_textures", );
            }
        }
    }
}

void plugin_render_s3d_unbind_other_vertexes(plugin_render_s3d_module_t module, uint32_t using_vertexes) {
    for(uint8_t i = 0; i < sizeof(module->m_cache->m_using_vertexes); i++) {
        if (cpe_ba_get(&module->m_cache->m_using_vertexes, i)
            && !cpe_ba_get(&using_vertexes, i))
        {
            try {
                module->m_ctx3d->setVertexBufferAt(i, internal::_null);
            }
            S3D_REPORT_EXCEPTION("unbind_other_vertexes", );
        }
    }
    module->m_cache->m_using_vertexes = using_vertexes;
}

