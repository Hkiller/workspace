#include <assert.h>
#include "cpe/pal/pal_math.h"
#include "cpe/utils/math_ex.h"
#include "render/utils/ui_transform.h"
#include "render/utils/ui_vector_2.h"
#include "render/utils/ui_rect.h"
#include "render/utils/ui_color.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_sprite.h"
#include "render/cache/ui_cache_res.h"
#include "render/cache/ui_cache_texture.h"
#include "render/runtime/ui_runtime_render_buff_use.h"
#include "render/runtime/ui_runtime_render_second_color.h"
#include "render/runtime/ui_runtime_render_utils.h"
#include "ui_runtime_render_i.h"

int ui_runtime_render_blend_factor_from_str(ui_runtime_render_blend_factor_t * r, const char * str_enum) {
    if (strcmp(str_enum, "des-alpha") == 0) {
        *r = ui_runtime_render_dst_alpha;
    }
    else if (strcmp(str_enum, "des-color") == 0) {
        *r = ui_runtime_render_dst_color;
    }
    else if (strcmp(str_enum, "one") == 0) {
        *r = ui_runtime_render_one;
    }
    else if (strcmp(str_enum, "des-alpha") == 0) {
        *r = ui_runtime_render_one_minus_dst_alpha;
    }
    else if (strcmp(str_enum, "des-color") == 0) {
        *r = ui_runtime_render_one_minus_dst_color;
    }
    else if (strcmp(str_enum, "one-minus-src-alpha") == 0) {
        *r = ui_runtime_render_one_minus_src_alpha;
    }
    else if (strcmp(str_enum, "one-minus-src-color") == 0) {
        *r = ui_runtime_render_one_minus_src_color;
    }
    else if (strcmp(str_enum, "src-alpha") == 0) {
        *r = ui_runtime_render_src_alpha;
    }
    else if (strcmp(str_enum, "src-color") == 0) {
        *r = ui_runtime_render_src_color;
    }
    else if (strcmp(str_enum, "zero") == 0) {
        *r = ui_runtime_render_zero;
    }
    else {
		return -1;
	}

    return 0;
}

const char * ui_runtime_render_blend_factor_to_str(ui_runtime_render_blend_factor_t factor) {
    switch(factor) {
    case ui_runtime_render_dst_alpha:
        return "des-alpha";
    case ui_runtime_render_dst_color:
        return "des-color";
    case ui_runtime_render_one:
        return "one";
    case ui_runtime_render_one_minus_dst_alpha:
        return "des-alpha";
    case ui_runtime_render_one_minus_dst_color:
        return "des-color";
    case ui_runtime_render_one_minus_src_alpha:
        return "src-alpha";
    case ui_runtime_render_one_minus_src_color:
        return "src-color";
    case ui_runtime_render_src_alpha:
        return "src-alpha";
    case ui_runtime_render_src_color:
        return "src-color";
    case ui_runtime_render_zero:
        return "zero";
    default:
        return "unknown";
    }
}

ui_runtime_render_second_color_mix_t
ui_runtime_render_second_color_mix_from_str(const char * str_enum, ui_runtime_render_second_color_mix_t dft) {
    if (strcmp(str_enum, "none") == 0) {
        return ui_runtime_render_second_color_none;
    }
    else if (strcmp(str_enum, "add") == 0) {
        return ui_runtime_render_second_color_add;
    }
    else if (strcmp(str_enum, "multiply") == 0) {
        return ui_runtime_render_second_color_multiply;
    }
    else if (strcmp(str_enum, "color") == 0) {
        return ui_runtime_render_second_color_color;
    }
    else {
        return dft;
    }
}

const char * ui_runtime_render_second_color_mix_to_str(ui_runtime_render_second_color_mix_t second_color_mix) {
    switch(second_color_mix) {
    case ui_runtime_render_second_color_none:
        return "none";
    case ui_runtime_render_second_color_add:
        return "add";
    case ui_runtime_render_second_color_multiply:
        return "multiply";
    case ui_runtime_render_second_color_color:
        return "color";
    default:
        return "unknown-second-color-mix";
    }
}

const char * ui_runtime_render_program_unif_type_to_str(ui_runtime_render_program_unif_type_t type) {
    switch(type) {
    case ui_runtime_render_program_unif_f:
        return "float";
    case ui_runtime_render_program_unif_i:
        return "integer";
    case ui_runtime_render_program_unif_v2:
        return "vector2";
    case ui_runtime_render_program_unif_v3:
        return "vector3";
    case ui_runtime_render_program_unif_v4:
        return "vector4";
    case ui_runtime_render_program_unif_m16:
        return "matrix16";
    default:
        return "unknown-unif-type";
    }
}

const char * ui_runtime_render_program_attr_id_to_str(ui_runtime_render_program_attr_id_t type) {
    switch(type) {
    case ui_runtime_render_program_attr_position:
        return "position";
    case ui_runtime_render_program_attr_normal:
        return "normal";
    case ui_runtime_render_program_attr_binormal:
        return "binormal";
    case ui_runtime_render_program_attr_tangent:
        return "tangent";
    case ui_runtime_render_program_attr_texcoord0:
        return "texcoord0";
    case ui_runtime_render_program_attr_texcoord1:
        return "texcoord1";
    case ui_runtime_render_program_attr_texcoord2:
        return "texcoord2";
    case ui_runtime_render_program_attr_texcoord3:
        return "texcoord3";
    case ui_runtime_render_program_attr_texcoord4:
        return "texcoord4";
    case ui_runtime_render_program_attr_texcoord5:
        return "texcoord5";
    case ui_runtime_render_program_attr_texcoord6:
        return "texcoord6";
    case ui_runtime_render_program_attr_texcoord7:
        return "texcoord7";
    case ui_runtime_render_program_attr_texcoord8:
        return "texcoord8";
    case ui_runtime_render_program_attr_texcoord9:
        return "texcoord9";
    case ui_runtime_render_program_attr_blendindices:
        return "blendindices";
    case ui_runtime_render_program_attr_blendweight:
        return "blendweight";
    case ui_runtime_render_program_attr_color:
        return "color";
    case ui_runtime_render_program_attr_index:
        return "index";
    case ui_runtime_render_program_attr_bonematrices:
        return "bonematrices";
    case ui_runtime_render_program_attr_bonepalette:
        return "bonepalette";
    case ui_runtime_render_program_attr_transforms:
        return "transforms";
    case ui_runtime_render_program_attr_instancetransforms:
        return "instancetransforms";
    case ui_runtime_render_program_attr_user:
        return "user";
    default:
        return "unknown";
    }
}

const char * ui_runtime_render_buff_type_to_str(ui_runtime_render_buff_type_t etype) {
    switch(etype) {
    case ui_runtime_render_buff_index_uint16:
        return "index-uint16";
    case ui_runtime_render_buff_vertex_v3f_t2f_c4b:
        return "vertex-v3f-t2f-c4b";
    default:
        return "unknown-buff-type";
    }
}

void ui_runtime_render_second_color_mix(ui_runtime_render_second_color_t second_color, ui_color_t to_color) {
    switch(second_color->m_mix) {
    case ui_runtime_render_second_color_none:
        break;
    case ui_runtime_render_second_color_add:
        to_color->a += second_color->m_color.a;
        to_color->r += second_color->m_color.r;
        to_color->g += second_color->m_color.g;
        to_color->b += second_color->m_color.b;
        break;
    case ui_runtime_render_second_color_multiply:
        to_color->a *= second_color->m_color.a;
        to_color->r *= second_color->m_color.r;
        to_color->g *= second_color->m_color.g;
        to_color->b *= second_color->m_color.b;
        break;
    case ui_runtime_render_second_color_color:
        *to_color = second_color->m_color;
        break;
    }
}

uint32_t ui_runtime_render_buff_use_count(ui_runtime_render_buff_use_t buff_use) {
    switch(buff_use->m_data_source) {
    case ui_runtime_render_buff_source_device:
        assert(0);
        return 0;
    case ui_runtime_render_buff_source_inline:
        return buff_use->m_inline.m_count;
    default:
        assert(0);
        return 0;
    }
}

ui_runtime_render_buff_type_t ui_runtime_render_buff_use_type(ui_runtime_render_buff_use_t buff_use) {
    switch(buff_use->m_data_source) {
    case ui_runtime_render_buff_source_device:
        assert(0);
        return 0;
    case ui_runtime_render_buff_source_inline:
        return buff_use->m_inline.m_e_type;
    default:
        assert(0);
        return 0;
    }
}

uint32_t ui_runtime_render_buff_type_stride(ui_runtime_render_buff_type_t e_type) {
    switch(e_type) {
    case ui_runtime_render_buff_index_uint16:
        return 2;
    case ui_runtime_render_buff_vertex_v3f_t2f_c4b:
        return sizeof(struct ui_runtime_vertex_v3f_t2f_c4ub);
    default:
        assert(0);
        return 0;
    }
}

struct ui_runtime_render_buff_use ui_runtime_render_buff_inline(void const * buf, ui_runtime_render_buff_type_t e_type, uint32_t count) {
    struct ui_runtime_render_buff_use use;
    use.m_data_source = ui_runtime_render_buff_source_inline;
    use.m_inline.m_buf = buf;
    use.m_inline.m_e_type = e_type;
    use.m_inline.m_count = count;
    return use;
}

struct ui_runtime_render_buff_use ui_runtime_render_buff_device(ui_runtime_render_vertex_buff_t vertex_buf) {
    struct ui_runtime_render_buff_use use;
    use.m_data_source = ui_runtime_render_buff_source_device;
    use.m_vertex_buf = vertex_buf;
    return use;
}

ui_runtime_render_texture_filter_t
ui_runtime_render_texture_filter_from_str(const char * str,  ui_runtime_render_texture_filter_t dft) {
	if (strcmp(str, "linear") == 0) {
		return ui_runtime_render_filter_linear;
	}
	else if (strcmp(str, "nearest") == 0) {
		return ui_runtime_render_filter_nearest;
	}
	else {
		return dft;
	}
}

const char *  ui_runtime_render_texture_filter_to_str(ui_runtime_render_texture_filter_t filter) {
	switch(filter) {
	case ui_runtime_render_filter_linear:
		return "linear";
	case ui_runtime_render_filter_nearest:
		return "nearest";
	default:
		return "unknown";
	}
}
