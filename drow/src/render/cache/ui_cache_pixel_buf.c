#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "render/utils/ui_color.h"
#include "ui_cache_pixel_buf_i.h"
#include "ui_cache_manager_i.h"
#include "render/cache/ui_cache_pixel_format.h"

static uint32_t ui_cache_pixel_buf_calc_mipmap_levels(uint32_t width, uint32_t height);

ui_cache_pixel_buf_t ui_cache_pixel_buf_create(ui_cache_manager_t mgr) {
    ui_cache_pixel_buf_t buf;
    
    buf = mem_alloc(mgr->m_alloc, sizeof(struct ui_cache_pixel_buf));
    if (buf == NULL) {
        CPE_ERROR(mgr->m_em, "%s: create pixel buf fail!", ui_cache_manager_name(mgr));
        return NULL;
    }
    
    buf->m_mgr = mgr;
    buf->m_format = ui_cache_pf_unknown;
    buf->m_level_count = 0;
    buf->m_stride = 0;
    buf->m_pixel_buf = NULL;

    return buf;
}

void ui_cache_pixel_buf_free(ui_cache_pixel_buf_t buf) {
    ui_cache_manager_t mgr = buf->m_mgr;

    if (buf->m_pixel_buf) ui_cache_pixel_buf_pixel_buf_destory(buf);
    assert(buf->m_pixel_buf == NULL);

    mem_free(mgr->m_alloc, buf);
}

int ui_cache_pixel_buf_pixel_buf_create(
    ui_cache_pixel_buf_t buf,
    uint32_t width, uint32_t height, ui_cache_pixel_format_t format,
    uint32_t mipmap_levels)
{
    ui_cache_manager_t mgr = buf->m_mgr;

    if (buf->m_pixel_buf) ui_cache_pixel_buf_pixel_buf_destory(buf);
    assert(buf->m_pixel_buf == NULL);
    
    if (mipmap_levels) {
        assert(mipmap_levels <= ui_cache_pixel_buf_calc_mipmap_levels(width, height));
    }
    else {
        mipmap_levels = ui_cache_pixel_buf_calc_mipmap_levels(width, height);
    }

    assert(mipmap_levels < CPE_ARRAY_SIZE(buf->m_level_infos));

    buf->m_format = format;
    buf->m_stride = ui_cache_pixel_format_to_stride(format);
    buf->m_level_count = 0;
    buf->m_total_buf_size = 0;

	/*not support compress now*/
	if (!ui_cache_pixel_format_compressed(format)) {
        while(buf->m_level_count < mipmap_levels) {
            struct ui_cache_pixel_level_info * level_info = &buf->m_level_infos[buf->m_level_count++];
			level_info->m_width = width ? width : 1;
			level_info->m_height = height ? height : 1;
            level_info->m_buf_offset = buf->m_total_buf_size;
            level_info->m_buf_size = buf->m_stride * level_info->m_width * level_info->m_height;
            buf->m_total_buf_size += level_info->m_buf_size;

			width  >>= 1;
			height >>= 1;
		}
	}
	else {
        while(buf->m_level_count < mipmap_levels) {
            struct ui_cache_pixel_level_info * level_info = &buf->m_level_infos[buf->m_level_count++];
			level_info->m_width = width ? width : 1;
			level_info->m_height = height ? height : 1;
            level_info->m_buf_offset = buf->m_total_buf_size;
            level_info->m_buf_size = (buf->m_stride * level_info->m_width * level_info->m_height) >> 3;
            buf->m_total_buf_size += level_info->m_buf_size;

			width  >>= 1;
			height >>= 1;
		}
	}

    buf->m_pixel_buf = mem_alloc(mgr->m_alloc, buf->m_total_buf_size);
    if (buf->m_pixel_buf == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: alloc pixel buf: pixes buf alloc fail, size=%d",
            ui_cache_manager_name(mgr), (int)(buf->m_total_buf_size));
        buf->m_format = ui_cache_pf_unknown;
        buf->m_total_buf_size = 0;
        buf->m_level_count = 0;
        buf->m_stride = 0;
        buf->m_pixel_buf = NULL;
        return -1;
    }
    bzero(buf->m_pixel_buf, buf->m_total_buf_size);

    return 0;
}

void ui_cache_pixel_buf_pixel_buf_destory(ui_cache_pixel_buf_t buf) {
    ui_cache_manager_t mgr = buf->m_mgr;

    assert(buf->m_pixel_buf);
    mem_free(mgr->m_alloc, buf->m_pixel_buf);
    
    buf->m_format = ui_cache_pf_unknown;
    buf->m_total_buf_size = 0;
    buf->m_level_count = 0;
    buf->m_stride = 0;
    buf->m_pixel_buf = NULL;
}

ui_cache_pixel_format_t	ui_cache_pixel_buf_format(ui_cache_pixel_buf_t buf) {
    return buf->m_format;
}

uint32_t ui_cache_pixel_buf_stride(ui_cache_pixel_buf_t buf) {
    return buf->m_stride;
}

void * ui_cache_pixel_buf_pixel_buf(ui_cache_pixel_buf_t buf) {
    return buf->m_pixel_buf;
}

uint32_t ui_cache_pixel_buf_level_count(ui_cache_pixel_buf_t buf) {
    return buf->m_level_count;
}

ui_cache_pixel_level_info_t
ui_cache_pixel_buf_level_info_at(ui_cache_pixel_buf_t buf, uint8_t pos) {
    assert(pos < buf->m_level_count);
    return &buf->m_level_infos[pos];
}

uint32_t ui_cache_pixel_buf_level_width(ui_cache_pixel_level_info_t level_info) {
    return level_info->m_width;
}

uint32_t ui_cache_pixel_buf_level_height(ui_cache_pixel_level_info_t level_info) {
    return level_info->m_height;
}

uint32_t ui_cache_pixel_buf_level_buf_size(ui_cache_pixel_level_info_t level_info) {
    return level_info->m_buf_size;
}

uint32_t ui_cache_pixel_buf_total_buf_size(ui_cache_pixel_buf_t buf) {
    return buf->m_total_buf_size;
}

uint32_t ui_cache_pixel_buf_level_offset(ui_cache_pixel_buf_t buf, uint32_t level) {
    ui_cache_pixel_level_info_t level_info;
    
	assert(level < buf->m_level_count);

    level_info = &buf->m_level_infos[level];
    
	return level_info->m_buf_offset;
}

void * ui_cache_pixel_buf_level_buf(ui_cache_pixel_buf_t buf, uint32_t level) {
    return buf->m_pixel_buf
        ? ((char*)buf->m_pixel_buf) + ui_cache_pixel_buf_level_offset(buf, level)
        : NULL;
}

void * ui_cache_pixel_buf_pixel_at(ui_cache_pixel_buf_t buf, uint32_t level, uint32_t x, uint32_t y) {
    ui_cache_pixel_level_info_t level_info;
	if (ui_cache_pixel_format_compressed(buf->m_format)) return NULL;
    if (buf->m_pixel_buf == NULL) return NULL;

	assert(level < buf->m_level_count);

    level_info = &buf->m_level_infos[level];
    assert(x < level_info->m_width);
    assert(y < level_info->m_height);

    return ((char*)ui_cache_pixel_buf_level_buf(buf, level))
        + (y * level_info->m_width + x)  * buf->m_stride;
}

uint32_t ui_cache_pixel_buf_pixel_value_at(
    ui_cache_pixel_buf_t buf, uint32_t level, uint32_t x, uint32_t y, ui_cache_pixel_field_t field)
{
    unsigned char * r_p;
    uint32_t r = 0;
    
    r_p = (unsigned char*)ui_cache_pixel_buf_pixel_at(buf, level, x, y);
    if (r_p == NULL) return 0;

    switch(buf->m_format) {
    case ui_cache_pf_r8g8b8:
        switch(field) {
        case ui_cache_pixel_field_r:
            r = r_p[0];
            break;
        case ui_cache_pixel_field_g:
            r = r_p[1];
            break;
        case ui_cache_pixel_field_b:
            r = r_p[2];
            break;
        case ui_cache_pixel_field_a:
            r = 255;
            break;
        case ui_cache_pixel_field_argb:
            r = ui_color_make_argb_from_value(255u, r_p[0], r_p[1], r_p[2]);
            break;
        default:
            break;
        }
        break;
    case ui_cache_pf_r8g8b8a8:
        switch(field) {
        case ui_cache_pixel_field_r:
            r = r_p[0];
            break;
        case ui_cache_pixel_field_g:
            r = r_p[1];
            break;
        case ui_cache_pixel_field_b:
            r = r_p[2];
            break;
        case ui_cache_pixel_field_a:
            r = r_p[3];
            break;
        case ui_cache_pixel_field_argb:
            r = ui_color_make_argb_from_value(r_p[3], r_p[0], r_p[1], r_p[2]);
            break;
        default:
            break;
        }
        break;
    case ui_cache_pf_a8:
        switch(field) {
        case ui_cache_pixel_field_a:
            r = *r_p;
            break;
        case ui_cache_pixel_field_argb:
            r = ui_color_make_argb_from_value(*r_p, 0, 0, 0);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    
    return r;
}

static uint32_t ui_cache_pixel_buf_calc_mipmap_levels(uint32_t width, uint32_t height) {
	uint32_t mipmap_levels_w;
	uint32_t mipmap_levels_h;

	if (width == 0 || height == 0) return 0;

    mipmap_levels_w = 1;
	while (width >> 1) {
		width >>= 1;
		mipmap_levels_w++;
	}

	mipmap_levels_h = 1;
	while (height >> 1) {
		height >>= 1;
		mipmap_levels_h++;
	}

	return mipmap_levels_h < mipmap_levels_w ?  mipmap_levels_w :  mipmap_levels_h;
}

const char * ui_cache_pixel_field_to_str(ui_cache_pixel_field_t field) {
    switch(field) {
    case ui_cache_pixel_field_r:
        return "r";
    case ui_cache_pixel_field_g:
        return "g";
    case ui_cache_pixel_field_b:
        return "b";
    case ui_cache_pixel_field_a:
        return "a";
    case ui_cache_pixel_field_argb:
        return "argb";
    default:
        return "unknown";
    }
}

int ui_cache_pixel_field_from_str(const char * str, ui_cache_pixel_field_t * field) {
    if (strcmp(str, "argb") == 0) {
        *field = ui_cache_pixel_field_argb;
    }
    else if (strcmp(str, "r") == 0) {
        *field = ui_cache_pixel_field_r;
    }
    else if (strcmp(str, "g") == 0) {
        *field = ui_cache_pixel_field_g;
    }
    else if (strcmp(str, "b") == 0) {
        *field = ui_cache_pixel_field_b;
    }
    else if (strcmp(str, "a") == 0) {
        *field = ui_cache_pixel_field_a;
    }
    else {
        return -1;
    }
    
    return 0;
}
