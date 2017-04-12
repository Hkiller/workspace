#ifndef UI_RUNTIME_RENDER_UTILS_H
#define UI_RUNTIME_RENDER_UTILS_H
#include "render/utils/ui_utils_types.h"
#include "ui_runtime_types.h"

#ifdef __cplusplus
extern "C" {
#endif

int ui_runtime_render_blend_factor_from_str(ui_runtime_render_blend_factor_t * dft, const char * str_enum);
const char * ui_runtime_render_blend_factor_to_str(ui_runtime_render_blend_factor_t blend_factor);

ui_runtime_render_second_color_mix_t ui_runtime_render_second_color_mix_from_str(const char * str_enum, ui_runtime_render_second_color_mix_t dft);
const char * ui_runtime_render_second_color_mix_to_str(ui_runtime_render_second_color_mix_t second_color_mix);
    
const char * ui_runtime_render_program_unif_type_to_str(ui_runtime_render_program_unif_type_t type);
const char * ui_runtime_render_program_attr_id_to_str(ui_runtime_render_program_attr_id_t type);
const char * ui_runtime_render_buff_type_to_str(ui_runtime_render_buff_type_t etype);

ui_runtime_render_texture_filter_t ui_runtime_render_texture_filter_from_str(const char * str,  ui_runtime_render_texture_filter_t dft);
const char *  ui_runtime_render_texture_filter_to_str(ui_runtime_render_texture_filter_t filter);

#ifdef __cplusplus
}
#endif

#endif
