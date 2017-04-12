#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_proj_save_utils.h"
#include "ui_proj_utils.h"

static int ui_data_proj_save_module_i(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    struct vfs_write_stream fs;
    ui_data_module_t module = ui_data_src_product(src);
    struct ui_data_img_block_it img_block_it;
    ui_data_img_block_t img_block;
    char * sep;
    write_stream_t s = (write_stream_t)&fs;

    vfs_write_stream_init(&fs, fp);
    
    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    stream_printf(s, "<RModule>\n");

    /*资源列表 */
    stream_printf(s, "    <SCTexList SCTexAuto=\"True\" SCTexAsyn=\"False\" SCTexManaged=\"True\" SCTexIndex=\"0\">\n");

    ui_data_img_block_in_module(&img_block_it, module);
    while((img_block = ui_data_img_block_it_next(&img_block_it))) {
        const char * using_img = ui_data_img_block_using_texture_path(img_block);

        if ((sep = strrchr(using_img, '/'))) {
            stream_printf(s, "        <SCTex Path=\"");
            stream_write(s, using_img, sep - using_img);
            stream_printf(s, "/\" File=\"%s\" />\n", sep + 1);
        }
        else {
            stream_printf(s, "        <SCTex Path=\"/\" File=\"%s\" />\n", using_img);
        }
        break;
    }
    stream_printf(s, "    </SCTexList>\n");

    stream_printf(s, "    <ImageList>\n");
    ui_data_img_block_in_module(&img_block_it, module);
    while((img_block = ui_data_img_block_it_next(&img_block_it))) {
        UI_IMG_BLOCK const * block_data = ui_data_img_block_data(img_block);
        
        stream_printf(s, "        <Image ID=\"%u\" Name=\"%s\" SrcX=\"%d\" SrcY=\"%d\" SrcW=\"%d\" SrcH=\"%d\" Flag=\"%d\">\n",
                      block_data->id, ui_data_img_block_name(img_block),
                      block_data->src_x, block_data->src_y, block_data->src_w, block_data->src_h, block_data->flag);
        ui_data_proj_save_rect(s, 3, "Rect", &block_data->rect);
        stream_printf(s, "        </Image>\n");
    }
    stream_printf(s, "    </ImageList>\n");
    stream_printf(s, "</RModule>\n");

    return 0;
}

int ui_data_proj_save_module(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    if (ui_data_proj_save_gen_meta_file(root, src, em) != 0) return -1;    
    return ui_data_src_save_to_file(src, root, ui_data_proj_postfix(ui_data_src_type(src)), ui_data_proj_save_module_i, ctx, em);
}
