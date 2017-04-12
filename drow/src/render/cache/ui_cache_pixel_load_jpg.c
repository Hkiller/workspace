#include <assert.h>
#include <setjmp.h>
#include "jinclude.h"
#include "jerror.h"
#include "jpeglib.h"
#include "cpe/utils/stream.h"
#include "ui_cache_pixel_buf_i.h"

struct ui_cache_pixel_jpg_error_mgr {
    struct jpeg_error_mgr pub;	/* "public" fields */
    jmp_buf setjmp_buffer;
};

typedef struct ui_cache_pixel_jpg_error_mgr * ui_cache_pixel_jpg_error_mgr_t;

static void ui_cache_pixel_jpg_error_exit(j_common_ptr cinfo) {
    ui_cache_pixel_jpg_error_mgr_t myerr = (ui_cache_pixel_jpg_error_mgr_t) cinfo->err;
    (*cinfo->err->output_message) (cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

struct ui_cache_pixel_jpg_read_source {
    struct jpeg_source_mgr pub;	/* public fields */
    read_stream_t stream;
    unsigned char * buffer;		/* start of buffer */
    size_t buffer_capacity;
    uint8_t start_of_file;	/* have we gotten any data yet? */
} my_source_mgr;

typedef struct ui_cache_pixel_jpg_read_source * ui_cache_pixel_jpg_read_source_t;

static void ui_cache_pixel_jpg_read_init_source(j_decompress_ptr cinfo) {
    ui_cache_pixel_jpg_read_source_t src = (ui_cache_pixel_jpg_read_source_t)cinfo->src;
    src->start_of_file = 1;
}

static boolean ui_cache_pixel_jpg_read_to_buff(j_decompress_ptr cinfo) {
    ui_cache_pixel_jpg_read_source_t src = (ui_cache_pixel_jpg_read_source_t)cinfo->src;
    int nbytes;

    nbytes = stream_read(src->stream, src->buffer, src->buffer_capacity);
    if (nbytes <= 0) {
        if (src->start_of_file)	/* Treat empty input file as fatal error */
            ERREXIT(cinfo, JERR_INPUT_EMPTY);
        WARNMS(cinfo, JWRN_JPEG_EOF);
        /* Insert a fake EOI marker */
        src->buffer[0] = (JOCTET) 0xFF;
        src->buffer[1] = (JOCTET) JPEG_EOI;
        nbytes = 2;
    }

    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
    src->start_of_file = 0;

    return 1;
}

static void ui_cache_pixel_jpg_read_skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
    ui_cache_pixel_jpg_read_source_t src = (ui_cache_pixel_jpg_read_source_t)cinfo->src;
    
    if (num_bytes > 0) {
        while (num_bytes > (long) src->pub.bytes_in_buffer) {
            num_bytes -= (long) src->pub.bytes_in_buffer;
            ui_cache_pixel_jpg_read_to_buff(cinfo);
            /* note we assume that fill_input_buffer will never return FALSE,
             * so suspension need not be handled.
             */
        }
        src->pub.next_input_byte += (size_t) num_bytes;
        src->pub.bytes_in_buffer -= (size_t) num_bytes;
    }
}

static void ui_cache_pixel_jpg_read_term_source(j_decompress_ptr cinfo) {
    ui_cache_pixel_jpg_read_source_t src = (ui_cache_pixel_jpg_read_source_t)cinfo->src;
    mem_free(NULL, src->buffer);
}

int ui_cache_pixel_load_jpg(ui_cache_pixel_buf_t buf, read_stream_t rs, error_monitor_t em, mem_allocrator_t tmp_alloc) {
    struct jpeg_decompress_struct cinfo;
    struct ui_cache_pixel_jpg_error_mgr jerr;
    struct ui_cache_pixel_jpg_read_source source;
    ui_cache_pixel_format_t format = ui_cache_pf_unknown;
    int row_stride;
    unsigned char * output_buf;
    
    /* Step 1: allocate and initialize JPEG decompression object */
    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = ui_cache_pixel_jpg_error_exit;
    
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        return -1;
    }
    
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */
    cinfo.src = &source.pub;
    source.pub.init_source = ui_cache_pixel_jpg_read_init_source;
    source.pub.fill_input_buffer = ui_cache_pixel_jpg_read_to_buff;
    source.pub.skip_input_data = ui_cache_pixel_jpg_read_skip_input_data;
    source.pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
    source.pub.term_source = ui_cache_pixel_jpg_read_term_source;
    source.pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
    source.pub.next_input_byte = NULL; /* until buffer loaded */
    source.stream = rs;
    source.buffer = mem_alloc(NULL, 1024);
    source.buffer_capacity = 1024;

    /* Step 3: read file parameters with jpeg_read_header() */
    jpeg_read_header(&cinfo, TRUE);

    /* Step 4: set parameters for decompression */

    /* Step 5: Start decompressor */
    jpeg_start_decompress(&cinfo);
    
    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */

    switch(cinfo.out_color_space) {
    case JCS_RGB:
        format = ui_cache_pf_r8g8b8;
        break;
    default:
        CPE_ERROR(em, "load jpg: not support color space %d", cinfo.jpeg_color_space);
        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    /* JSAMPLEs per row in output buffer */
    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

	/* create image data */
	if (ui_cache_pixel_buf_pixel_buf_create(buf, cinfo.output_width, cinfo.output_height, format, 1) != 0) {
        CPE_ERROR(
            em, "load jpg: create pixel buf fail, width=%d, height=%d, format=%d!",
            cinfo.output_width, cinfo.output_height, format);
        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    if (cinfo.output_components != ui_cache_pixel_buf_stride(buf)) {
        CPE_ERROR(
            em, "load jpg: create pixel buf fail, format %d stride=%d, but output_components=%d",
            format, ui_cache_pixel_buf_stride(buf), cinfo.output_components);
        jpeg_destroy_decompress(&cinfo);
        return -1;
    }

    output_buf = ui_cache_pixel_buf_level_buf(buf, 0);
    row_stride = cinfo.output_width * cinfo.output_components;

    while (cinfo.output_scanline < cinfo.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        unsigned char * p = output_buf;
        jpeg_read_scanlines(&cinfo, &p, 1);
        output_buf += row_stride;
    }

    /* Step 7: Finish decompression */
    jpeg_finish_decompress(&cinfo);

    /* Step 8: Release JPEG decompression object */
    jpeg_destroy_decompress(&cinfo);

	return 0;
}
