#include <assert.h>
#include <setjmp.h>
#include "png.h"
#include "pngconf.h"
#include "cpe/utils/stream.h"
#include "ui_cache_pixel_buf_i.h"

struct ui_cache_pixel_load_png_ctx {
    read_stream_t m_rs;
    jmp_buf m_jump_buf;
};

static void ui_cache_pixel_load_png_error(png_structp png_reader, png_const_charp error) {
    CPE_ERROR((error_monitor_t)png_get_error_ptr(png_reader), "%s", error);
}

static void ui_cache_pixel_load_png_warning(png_structp png_reader, png_const_charp warning) {
    CPE_INFO((error_monitor_t)png_get_error_ptr(png_reader), "%s", warning);
}

static void ui_cache_pixel_load_png_read(png_structp png_reader, png_bytep buffer, png_size_t size) {
    struct ui_cache_pixel_load_png_ctx * ctx = png_get_io_ptr(png_reader);
    int r;
    png_size_t readed_size = 0;
    
    while(readed_size < size) {
        r = stream_read(ctx->m_rs, buffer + readed_size, size - readed_size);

        if (r <= 0) {
            longjmp(ctx->m_jump_buf, -1);
        }
        
        readed_size += r;
    }
}

int ui_cache_pixel_load_png(ui_cache_pixel_buf_t buf, read_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc) {
	png_structp png_reader;
    png_infop png_info;
	png_byte depth;
	png_byte color_type;
	png_uint_32 width;
	png_uint_32 height;
    png_bytep trans_alpha;
    int num_trans;
    png_color_16p trans_color;
    ui_cache_pixel_format_t format = ui_cache_pf_unknown;
    struct ui_cache_pixel_load_png_ctx read_ctx;

    read_ctx.m_rs = rs;
    if (setjmp(read_ctx.m_jump_buf) != 0) {
        CPE_ERROR(em, "load png: read data fail!");
		return -1;
    }
    
    png_reader = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, em,
        ui_cache_pixel_load_png_error, ui_cache_pixel_load_png_warning);
	if (png_reader == NULL) {
        CPE_ERROR(em, "load png: create png reader fail!");
		return -1;
    }
	png_set_read_fn(png_reader, &read_ctx, ui_cache_pixel_load_png_read);

    png_info = png_create_info_struct(png_reader);
	if (png_info == NULL) {
        CPE_ERROR(em, "load png: create png info fail!");
		png_destroy_read_struct(&png_reader, NULL, NULL);
		return -1;
	}
	png_read_info(png_reader, png_info);

    depth = png_get_bit_depth(png_reader, png_info);
    color_type = png_get_color_type(png_reader, png_info);
    width = png_get_image_width(png_reader, png_info);
    height = png_get_image_height(png_reader, png_info);
    png_get_tRNS(png_reader, png_info, &trans_alpha, &num_trans, &trans_color);

	if (depth == 1 || depth == 2 || depth == 4) {
		png_set_expand_gray_1_2_4_to_8(png_reader);
    }
    
	if (depth == 1 || depth == 2 || depth == 4 || depth == 8) {
		switch (color_type) {
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			format = ui_cache_pf_pala8;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			format = ui_cache_pf_r8g8b8a8;
			break;
        case PNG_COLOR_TYPE_GRAY:
            format = num_trans ? ui_cache_pf_pala8 : ui_cache_pf_pal8;
            break;
        case PNG_COLOR_TYPE_RGB:
            format = num_trans ? ui_cache_pf_r8g8b8a8 : ui_cache_pf_r8g8b8;
            break;
        case PNG_COLOR_TYPE_PALETTE:
			png_set_palette_to_rgb(png_reader);
            format = num_trans ? ui_cache_pf_r8g8b8a8 :ui_cache_pf_r8g8b8;
            break;
        default:
            CPE_ERROR(em, "load png: not support color type %d!", color_type);
            png_destroy_read_struct(&png_reader, &png_info, NULL);
            return -1;
		}
	}
    else {
        CPE_ERROR(em, "load png: not support depth %d!", depth);
        png_destroy_read_struct(&png_reader, &png_info, NULL);
        return -1;
    }

    assert(format != ui_cache_pf_unknown);
    
	/* convert transparency info into RGBA, but only if we have RGB/RGBA, not indexed colors */
	png_set_tRNS_to_alpha(png_reader);

	/* create image data */
	if (ui_cache_pixel_buf_pixel_buf_create(buf, width, height, format, 1) != 0) {
        CPE_ERROR(
            em, "load png: create pixel buf fail, width=%d, height=%d, format=%d!", width, height, format);
        png_destroy_read_struct(&png_reader, &png_info, NULL);
        return -1;
    }

	/* Make array with pointers to each individual row inside the image buffer */
    do {
        char * pixels = ui_cache_pixel_buf_pixel_buf(buf);
        uint32_t role_size = ui_cache_pixel_buf_stride(buf) * width;
        png_bytepp role_pointers = mem_alloc(tmp_alloc, sizeof(png_bytep) * height);
        uint32_t i;

        if (role_pointers == NULL) {
            CPE_ERROR(em, "load png: alloc role pointers fail!");
            png_destroy_read_struct(&png_reader, &png_info, NULL);
            return -1;
        }
        
        for (i = 0; i < height; ++i) {
            role_pointers[i] = (png_bytep)pixels;
            pixels += role_size;
        }

        /* read image */
        png_read_image(png_reader, role_pointers);

        mem_free(tmp_alloc, role_pointers);
    } while(0);
    
    png_read_end(png_reader, NULL);
	png_destroy_read_struct(&png_reader, &png_info, NULL);

	return 0;
}

int ui_cache_pixel_load_pzd(ui_cache_pixel_buf_t buf, read_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc) {
    char ignore[32];
    int r;
    png_size_t readed_size = 0;
    
    while(readed_size < CPE_ARRAY_SIZE(ignore)) {
        r = stream_read(rs, ignore + readed_size, CPE_ARRAY_SIZE(ignore) - readed_size);
        if (r == 0) {
            CPE_ERROR(em, "read ignore fail!");
            return -1;
        }

        assert(r > 0);
        readed_size += r;
    }

    return ui_cache_pixel_load_png(buf, rs, em, tmp_alloc);
}
