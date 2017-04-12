#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdio.h"
#include "plugin_render_ogl_utils.h"

const char * gl_utils_strerror(GLenum err) {
    static char gl_msg_error[64];

    switch (err) {  
    case GL_INVALID_ENUM:  
        return "gl invalid enum";
    case GL_INVALID_VALUE:  
        return "gl invalid value";
    case GL_INVALID_OPERATION:  
        return "gl invalid operation";
    case GL_OUT_OF_MEMORY:  
        return "gl out of memory";
    case GL_INVALID_FRAMEBUFFER_OPERATION:  
        return "gl invalid framebuffer operation";
    /* case GL_STACK_OVERFLOW: */
    /*     return "gl stack overflow"; */
    /* case GL_STACK_UNDERFLOW: */
    /*     return "gl stack underflow"; */
    /* case GL_TABLE_TOO_LARGE:   */
    /*     return "gl table too large";   */
    default:
        snprintf(gl_msg_error, sizeof(gl_msg_error), "GL Undefined Error(%d)", err);
        return gl_msg_error;
    };  
}  

#if ! defined(GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG)
#    define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0
#endif

#if ! defined(GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG)
#    define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0
#endif

#if ! defined(GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG)
#    define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0
#endif

#if ! defined(GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG)
#    define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0
#endif

static
struct plugin_render_ogl_pixel_format_info {
    GLint api_format;
    GLenum api_type;
} g_ogl_pixel_format_infos[ui_cache_pf_format_count] = {
    /*                              api_format                           api_type                 ha_mask */
    /* ui_cache_pf_unknown */     { 0                                  , 0                         }, 
    /* ui_cache_pf_pal8 */        { GL_LUMINANCE                       , GL_UNSIGNED_BYTE          }, 
    /* ui_cache_pf_pala8 */       { GL_LUMINANCE_ALPHA                 , GL_UNSIGNED_BYTE          }, 
    /* ui_cache_pf_r5g6b5 */      { GL_RGB                             , GL_UNSIGNED_SHORT_5_6_5   }, 
    /* ui_cache_pf_r4g4b4a4 */    { GL_RGBA                            , GL_UNSIGNED_SHORT_4_4_4_4 }, 
    /* ui_cache_pf_r5g5b5a1 */    { GL_RGBA                            , GL_UNSIGNED_SHORT_5_5_5_1 }, 
    /* ui_cache_pf_r8g8b8 */      { GL_RGB                             , GL_UNSIGNED_BYTE          }, 
    /* ui_cache_pf_r8g8b8a8 */    { GL_RGBA                            , GL_UNSIGNED_BYTE          }, 
    /* ui_cache_pf_a8 */          { GL_ALPHA                           , GL_UNSIGNED_BYTE          }, 
    /* ui_cache_pf_s8 */          { GL_STENCIL_INDEX8                  , GL_UNSIGNED_BYTE          }, 
    /* ui_cache_pf_d16 */         { GL_DEPTH_COMPONENT16               , GL_UNSIGNED_SHORT         }, 
    /* ui_cache_pf_dxt1 */        { 0                                  , 0                         }, 
    /* ui_cache_pf_dxt3 */        { 0                                  , 0                         }, 
    /* ui_cache_pf_dxt5 */        { 0                                  , 0                         }, 
    /* ui_cache_pf_rgbpvrtc2 */   { GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG , 0                         }, 
    /* ui_cache_pf_rgbapvrtc2 */  { GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0                         }, 
    /* ui_cache_pf_rgbpvrtc4 */   { GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG , 0                         }, 
    /* ui_cache_pf_rgbapvrtc4 */  { GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0                         }, 
};
    

GLint plugin_render_ogl_format_api_format(ui_cache_pixel_format_t format) {
    return g_ogl_pixel_format_infos[format].api_format;
}

GLenum plugin_render_ogl_format_api_type(ui_cache_pixel_format_t format) {
    return g_ogl_pixel_format_infos[format].api_type;
}

const char * s_attr_names[ui_runtime_render_program_attr_max] = {
    "POSITION",
    "NORMAL",
    "BINORMAL",
    "TANGENT",
    "TEXCOORD0",
    "TEXCOORD1",
    "TEXCOORD2",
    "TEXCOORD3",
    "TEXCOORD4",
    "TEXCOORD5",
    "TEXCOORD6",
    "TEXCOORD7",
    "TEXCOORD8",
    "TEXCOORD9",
    "BLENDINDICES",
    "BLENDWEIGHT",
    "COLOR",
    "INDEX",
    "BONEMATRICES",
    "BONEPALETTE",
    "TRANSFORMS",
    "INSTANCETRANSFORMS",
    "USER",
};

ui_runtime_render_program_attr_id_t plugin_render_ogl_program_attr_id_from_str(const char * str) {
    uint8_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(s_attr_names); ++i) {
        if (strcmp(s_attr_names[i], str) == 0) return (ui_runtime_render_program_attr_id_t)i;
    }

	return ui_runtime_render_program_attr_max;
}


    /* switch(emitter_data->blend_mode) { */
    /* case UI_PARTICLE_BLEND_ADDITIVE: */
    /*     blend->src_factor = GL_SRC_ALPHA; */
    /*     blend->des_factor = GL_ONE; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_ALPHABASE: */
    /*     blend->src_factor = GL_SRC_ALPHA; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_ALPHA; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_COLORBASE: */
    /*     blend->src_factor = GL_ONE; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_NONE: */
    /*     blend->src_factor = GL_ONE; */
    /*     blend->des_factor = GL_ZERO; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_BLACK: */
    /*     blend->src_factor = GL_ONE_MINUS_DST_COLOR; */
    /*     blend->des_factor = GL_ONE; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_MULTIPLY: */
    /*     blend->src_factor = GL_ZERO; */
    /*     blend->des_factor = GL_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_MULTIPLY_FILTER: */
    /*     blend->src_factor = GL_ZERO; */
    /*     blend->des_factor = GL_DST_COLOR; */
    /*     break;		 */
    /* case UI_PARTICLE_BLEND_DARKROOM: */
    /*     blend->src_factor = GL_ONE_MINUS_DST_COLOR; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_ALPHA; */
    /*     break;	 */
    /* case UI_PARTICLE_BLEND_DODGE: */
    /*     blend->src_factor = GL_DST_COLOR; */
    /*     blend->des_factor = GL_DST_ALPHA; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_DODGE_FILTER: */
    /*     blend->src_factor = GL_DST_COLOR; */
    /*     blend->des_factor = GL_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_FILTER_COLOR: */
    /*     blend->src_factor = GL_ONE; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_ADDITIVE_1: */
    /*     blend->src_factor = GL_DST_COLOR; */
    /*     blend->des_factor = GL_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_ADDITIVE_2: */
    /*     blend->src_factor = GL_SRC_COLOR; */
    /*     blend->des_factor = GL_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_HIGHLIGHT_1: */
    /*     blend->src_factor = GL_SRC_COLOR; */
    /*     blend->des_factor = GL_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_HIGHLIGHT_2: */
    /*     blend->src_factor = GL_DST_COLOR; */
    /*     blend->des_factor = GL_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_SUBDUEDLIGHT: */
    /*     blend->src_factor = GL_ONE; */
    /*     blend->des_factor = GL_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_ADD_PIC_LEVEL_1: */
    /*     blend->src_factor = GL_ONE_MINUS_SRC_COLOR; */
    /*     blend->des_factor = GL_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_ADD_PIC_LEVEL_2: */
    /*     blend->src_factor = GL_ONE_MINUS_DST_COLOR; */
    /*     blend->des_factor = GL_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_CG_EFFECTS: */
    /*     blend->src_factor = GL_ZERO; */
    /*     blend->des_factor = GL_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_MASK: */
    /*     blend->src_factor = GL_ZERO; */
    /*     blend->des_factor = GL_SRC_ALPHA; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_RE_MASK: */
    /*     blend->src_factor = GL_ZERO; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_ALPHA; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_RE_ALPHA_FILL: */
    /*     blend->src_factor = GL_ONE_MINUS_SRC_ALPHA; */
    /*     blend->des_factor = GL_SRC_ALPHA; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_HIGHLIGHT_PROTECT: */
    /*     blend->src_factor = GL_SRC_COLOR; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_HIGHLIGHT_COVER: */
    /*     blend->src_factor = GL_SRC_COLOR; */
    /*     blend->des_factor = GL_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_DARK_FLIP: */
    /*     blend->src_factor = GL_SRC_COLOR; */
    /*     blend->des_factor = GL_ONE_MINUS_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_MIRROR_ADD: */
    /*     blend->src_factor = GL_ONE_MINUS_SRC_COLOR; */
    /*     blend->des_factor = GL_ONE; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_HIGHLIGHT_ADD: */
    /*     blend->src_factor = GL_SRC_COLOR; */
    /*     blend->des_factor = GL_ONE; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_MINUS: */
    /*     blend->src_factor = GL_DST_ALPHA; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_LINE_RE_ADD: */
    /*     blend->src_factor = GL_ONE_MINUS_DST_COLOR; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_RE_SUBDUEDLIGHT: */
    /*     blend->src_factor = GL_ONE_MINUS_DST_COLOR; */
    /*     blend->des_factor = GL_ONE_MINUS_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_RE_ADD: */
    /*     blend->src_factor = GL_DST_COLOR; */
    /*     blend->des_factor = GL_ONE_MINUS_DST_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_RE_DEEPED: */
    /*     blend->src_factor = GL_ZERO; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_COLOR; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_RE_FILTER_COLOR: */
    /*     blend->src_factor = GL_DST_COLOR; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_ALPHA; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_RE_LINE: */
    /*     blend->src_factor = GL_ONE_MINUS_SRC_ALPHA; */
    /*     blend->des_factor = GL_ONE_MINUS_SRC_ALPHA; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_EDGE_MAP: */
    /*     blend->src_factor = GL_ONE_MINUS_DST_ALPHA; */
    /*     blend->des_factor = GL_DST_ALPHA; */
    /*     break; */
    /* case UI_PARTICLE_BLEND_CUSTOM: */
    /*     blend->src_factor = emitter_data->custom_src_factor; */
    /*     blend->des_factor = emitter_data->custom_des_factor; */
    /*     break; */
    /* default: */
    /*     CPE_ERROR(module->m_em, "plugin_particle_obj_render: emitter blend mode %d unknown!", emitter_data->blend_mode); */
    /* } */
