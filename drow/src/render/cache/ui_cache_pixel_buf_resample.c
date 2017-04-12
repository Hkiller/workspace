#include "cpe/utils/math_ex.h"
#include "cpe/utils/string_utils.h"
#include "ui_cache_pixel_buf_i.h"

ui_cache_pixel_buf_t
ui_cache_pixel_buf_resample(ui_cache_pixel_buf_t pixel_buf, uint8_t scale_level) {
    ui_cache_manager_t mgr = pixel_buf->m_mgr;
    ui_cache_pixel_buf_t output_buf;
    int bytes_per_pixel;
    uint32_t new_width;
    uint32_t new_height;
    ui_cache_pixel_level_info_t level_info;
    int src_pitch;
    
    if (scale_level == 0) {
        CPE_ERROR(mgr->m_em, "ui_cache_pixel_buf_resize: not support scale level %d", scale_level);
        return NULL;
    }

    if (ui_cache_pixel_buf_level_count(pixel_buf) == 0) {
        CPE_ERROR(mgr->m_em, "ui_cache_pixel_buf_resize: input buff no data");
        return NULL;
    }

    switch(pixel_buf->m_format) {
    case ui_cache_pf_r8g8b8a8:
        bytes_per_pixel = 4;
        break;
    default:
        CPE_ERROR(mgr->m_em, "ui_cache_pixel_buf_resize: not support format %d", pixel_buf->m_format);
        return NULL;
    }

    level_info = ui_cache_pixel_buf_level_info_at(pixel_buf, 0);
    assert(level_info);

    //scale_level
    new_width = level_info->m_width;
    new_height = level_info->m_height;
    
    while(scale_level > 0) {
        new_width /=  2.0f;
        new_height /= 2.0f;
        scale_level--;
    }

    output_buf = ui_cache_pixel_buf_create(mgr);
    if (output_buf == NULL) {
        CPE_ERROR(mgr->m_em, "ui_cache_pixel_buf_resize: create pixel buf fail");
        return NULL;
    }

    if (ui_cache_pixel_buf_pixel_buf_create(output_buf, new_width, new_height, pixel_buf->m_format, 1) != 0) {
        CPE_ERROR(mgr->m_em, "ui_cache_pixel_buf_resize: create pixel buf fail");
        ui_cache_pixel_buf_free(output_buf);
        return NULL;
    }

    src_pitch = level_info->m_width * bytes_per_pixel;
    
    ui_cache_pixel_software_resample(
        ui_cache_pixel_buf_level_buf(output_buf, 0), new_width, new_height,
        ui_cache_pixel_buf_level_buf(pixel_buf, 0), level_info->m_width, level_info->m_height, src_pitch, 
        bytes_per_pixel);
    
    return output_buf;
}

void ui_cache_pixel_software_resample(
    uint8_t * dst_data, int dst_width, int dst_height,
    uint8_t * src_data, int src_width, int src_height, int src_pitch, 
    int bytes_per_pixel)
{
	// FAST bi-linear filtering
	// the code here is designed to be fast, not readable
	float Uf, Vf;		// fractional parts
	float Ui, Vi;		// integral parts
	float w1, w2, w3, w4;	// weighting
	uint8_t* psrc;
	uint8_t* pdst = dst_data;
    
	// i1,i2,i3,i4 are the offsets of the surrounding 4 pixels
	const int i1 = 0;
	const int i2 = bytes_per_pixel;
	int i3 = src_pitch;
	int i4 = src_pitch + bytes_per_pixel;
    
	// change in source u and v
	float dv = (float)(src_height - 2) / dst_height;
	float du = (float)(src_width - 2) / dst_width;
    
	// source u and source v
	float U;
	float V = 0;
    int v;
    int u;
    
#define BYTE_SAMPLE(offset)	\
(uint8_t) (w1 * psrc[i1 + (offset)] + w2 * psrc[i2 + (offset)] + w3 * psrc[i3 + (offset)] + w4 * psrc[i4 + (offset)])

    assert(bytes_per_pixel == 3 || bytes_per_pixel == 4);


	if (bytes_per_pixel == 3) {
		for (v = 0; v < dst_height; ++v) {
			Vf = modff(V, &Vi);
			V += dv;
			U = 0;
            
			for (u = 0; u < dst_width; ++u) {
				Uf = modff(U, &Ui);
				U += du;
                
				w1 = (1 - Uf) * (1 - Vf);
				w2 = Uf * (1 - Vf);
				w3 = (1 - Uf) * Vf;
				w4 = Uf * Vf;
				psrc = &src_data[(int) (Vi * src_pitch) + (int) (Ui * bytes_per_pixel)];
                
				*pdst++ = BYTE_SAMPLE(0);	// red
				*pdst++ = BYTE_SAMPLE(1);	// green
				*pdst++ = BYTE_SAMPLE(2);	// blue
                
				psrc += 3;
			}
		}
	}
	else {
		assert(bytes_per_pixel == 4);
        
		for (v = 0; v < dst_height; ++v) {
			Vf = modff(V, &Vi);
			V += dv;
			U = 0;
            
			for (u = 0; u < dst_width; ++u) {
				Uf = modff(U, &Ui);
				U += du;
                
				w1 = (1 - Uf) * (1 - Vf);
				w2 = Uf * (1 - Vf);
				w3 = (1 - Uf) * Vf;
				w4 = Uf * Vf;
				psrc = &src_data[(int) (Vi * src_pitch) + (int) (Ui * bytes_per_pixel)];
                
				*pdst++ = BYTE_SAMPLE(0);	// red
				*pdst++ = BYTE_SAMPLE(1);	// green
				*pdst++ = BYTE_SAMPLE(2);	// blue
				*pdst++ = BYTE_SAMPLE(3);	// alpha
                
				psrc += 4;
			}
		}
	}
}

