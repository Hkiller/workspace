#include <assert.h>
#include "png.h"
#include "pngconf.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/stream.h"
#include "ui_cache_pixel_buf_i.h"

static void ui_cache_pixel_save_png_error(png_structp png_writer, png_const_charp error) {
    CPE_ERROR((error_monitor_t)png_get_error_ptr(png_writer), "%s", error);
}

static void ui_cache_pixel_save_png_warning(png_structp png_writer, png_const_charp warning) {
    CPE_INFO((error_monitor_t)png_get_error_ptr(png_writer), "%s", warning);
}

static void ui_cache_pixel_save_png_write(png_structp png_writer, png_bytep data, png_size_t size) {
    write_stream_t ws = (write_stream_t)png_get_io_ptr(png_writer);
    stream_write(ws, data, size);
}

static void ui_cache_pixel_save_png_flush(png_structp png_writer) {
    write_stream_t ws = (write_stream_t)png_get_io_ptr(png_writer);
    stream_flush(ws);
}

int ui_cache_pixel_save_png(ui_cache_pixel_buf_t buf, write_stream_t ws, error_monitor_t em, mem_allocrator_t tmp_alloc) {
    png_byte color_type;   
    png_structp png_ptr = NULL;  
    png_infop info_ptr = NULL;   
    png_bytep * row_pointers = NULL;
    ui_cache_pixel_level_info_t pixel_level_info;
    int rv = -1;
    uint8_t bit_depth;

    if (ui_cache_pixel_buf_level_count(buf) != 1) {
        CPE_ERROR(em, "write_png: input pixel buf level count %d error", ui_cache_pixel_buf_level_count(buf));  
        goto COMPLETE;  
    }

    pixel_level_info = ui_cache_pixel_buf_level_info_at(buf, 0);
    assert(pixel_level_info);
    
    /* initialize stuff */  
    png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, em,
        ui_cache_pixel_save_png_error, ui_cache_pixel_save_png_warning);
    if (png_ptr == NULL) {  
        CPE_ERROR(em, "write_png: png_create_write_struct failed");  
        goto COMPLETE;  
    }

    info_ptr = png_create_info_struct(png_ptr);  
    if (info_ptr == NULL) {  
        CPE_ERROR(em, "write_png: png_create_info_struct failed");  
        goto COMPLETE;  
    }

    png_set_write_fn(png_ptr, (void *)ws, ui_cache_pixel_save_png_write, ui_cache_pixel_save_png_flush);

    if (setjmp(png_jmpbuf(png_ptr))) {
        goto COMPLETE;  
    }

    /* write header */  
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    bit_depth = 8;

    png_set_IHDR(
        png_ptr, info_ptr, pixel_level_info->m_width, pixel_level_info->m_height,  
        bit_depth, color_type, PNG_INTERLACE_NONE,  
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);  
    png_write_info(png_ptr, info_ptr);

    do {
		uint32_t i;
        char * pixels = (char *)ui_cache_pixel_buf_pixel_buf(buf);
        uint32_t row_size = ui_cache_pixel_buf_stride(buf) * pixel_level_info->m_width;
        row_pointers = (png_bytep *)mem_alloc(tmp_alloc, sizeof(png_bytep) * pixel_level_info->m_height);  

        if (row_pointers == NULL) {
            CPE_ERROR(em, "write_png: alloc row pointers fail!");
            goto COMPLETE;
        }
        
        for (i = 0; i < pixel_level_info->m_height; ++i) {
            row_pointers[i] = (png_bytep)pixels;
            pixels += row_size;
        }

        /* read image */
        png_write_image(png_ptr, row_pointers);
    } while(0);

    png_write_end(png_ptr, NULL);  
  
    rv =  0;

COMPLETE:
    if (png_ptr) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
    }

    if (row_pointers) {
        mem_free(tmp_alloc, row_pointers);
        row_pointers = NULL;
    }

    return rv;
}

int ui_cache_pixel_save_pzd(ui_cache_pixel_buf_t buf, write_stream_t ws, error_monitor_t em, mem_allocrator_t tmp_alloc) {
    char head_buf[32];
    size_t i;

    for(i = 0; i < CPE_ARRAY_SIZE(head_buf); ++i) {
        head_buf[i] = (char)rand();
    }

    if (stream_write(ws, head_buf, sizeof(head_buf)) != sizeof(head_buf)) return -1;

    return ui_cache_pixel_save_png(buf, ws, em, tmp_alloc);
}
