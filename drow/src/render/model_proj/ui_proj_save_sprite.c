#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "render/cache/ui_cache_res.h"
#include "render/model/ui_data_sprite.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_proj_save_utils.h"
#include "ui_proj_utils.h"

static int ui_data_proj_save_sprite_i(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    struct vfs_write_stream fs;
    ui_data_sprite_t sprite = ui_data_src_product(src);
    struct ui_data_src_it using_src_it;
    ui_data_src_t using_src;
    struct ui_data_frame_it frame_it;
    ui_data_frame_t frame;
    struct ui_data_frame_img_it img_ref_it;
    ui_data_frame_img_t img_ref;
    struct ui_data_frame_collision_it collision_it;
    ui_data_frame_collision_t collision;
    write_stream_t s = (write_stream_t)&fs;

    vfs_write_stream_init(&fs, fp);

    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    stream_printf(s, "<RSprite>\n");

    ui_data_src_user_srcs(src, &using_src_it);
    using_src = ui_cache_res_it_next(&using_src_it);
    if (using_src == NULL) {
        stream_printf(s, "    <ImageFile />\n");
    }
    else {
        stream_printf(s, "    <ImageFile>\n");
        for(; using_src; using_src = ui_data_src_it_next(&using_src_it)) {
            stream_printf(s, "        <File Resfile=\"%u\" />\n", ui_data_src_id(using_src));
        }
        stream_printf(s, "    </ImageFile>\n");
    }

    ui_data_sprite_frames(&frame_it, sprite);
    frame = ui_data_frame_it_next(&frame_it);
    if (frame == NULL) {
        stream_printf(s, "    <FrameList />\n");
    }
    else {
        stream_printf(s, "    <FrameList>\n");
        for(; frame; frame = ui_data_frame_it_next(&frame_it)) {
            UI_FRAME const * frame_data = ui_data_frame_data(frame);

            stream_printf(s, "        <Frame ID=\"%u\" Name=\"%s\">\n", frame_data->id, frame_data->name);
            ui_data_proj_save_rect(s, 3, "Bounding", &frame_data->bounding);
            ui_data_proj_save_bool(s, 3, "BoundCustom", frame_data->bound_custom);
            ui_data_proj_save_bool(s, 3, "AcceptScale", frame_data->accept_scale);
            ui_data_proj_save_bool(s, 3, "MulLanguage", 0);

            ui_data_frame_imgs(&img_ref_it, frame);
            img_ref = ui_data_frame_img_it_next(&img_ref_it);
            if (img_ref == NULL) {
                stream_printf(s, "            <ImageRefList />\n");
            }
            else {
                stream_printf(s, "            <ImageRefList>\n");
                for(; img_ref; img_ref = ui_data_frame_img_it_next(&img_ref_it)) {
                    UI_IMG_REF const * img_ref_data = ui_data_frame_img_data(img_ref);

                    stream_printf(s, "                <ImageRef Resfile=\"%u\" Imageid=\"%u\" Resname=\"%s\">\n",
                                  img_ref_data->module_id, img_ref_data->img_block_id, img_ref_data->name);
                    ui_data_proj_save_trans(s, 5, &img_ref_data->trans);
                    ui_data_proj_save_bool(s, 5, "Freedom", img_ref_data->freedom);
                    if (img_ref_data->freedom) {
                        ui_data_proj_save_vector_2(s, 5, "PolyVtx0", img_ref_data->polys + 0);
                        ui_data_proj_save_vector_2(s, 5, "PolyVtx1", img_ref_data->polys + 1);
                        ui_data_proj_save_vector_2(s, 5, "PolyVtx2", img_ref_data->polys + 2);
                        ui_data_proj_save_vector_2(s, 5, "PolyVtx3", img_ref_data->polys + 3);
                    }
                    stream_printf(s, "                </ImageRef>\n");
                }
                stream_printf(s, "            </ImageRefList>\n");
            }

            ui_data_frame_collisions(&collision_it, frame);
            collision = ui_data_frame_img_it_next(&collision_it);
            if (collision == NULL) {
                stream_printf(s, "            <ColliderList />\n");
            }
            else {
                stream_printf(s, "            <ColliderList>\n");
                for(; collision; collision = ui_data_frame_collision_it_next(&collision_it)) {
                    UI_COLLISION const * collision_data = ui_data_frame_collision_data(collision);

                    stream_printf(
                        s, "                <Collider Name=\"%s\" Type=\"0\" LT=\"%d\" TP=\"%d\" RT=\"%d\" BM=\"%d\"/>\n",
                        collision_data->name, collision_data->bounding.tp, collision_data->bounding.tp,
                        collision_data->bounding.rt, collision_data->bounding.bm);
                }
                stream_printf(s, "            </ColliderList>\n");
            }

            stream_printf(s, "        </Frame>\n");
        }
        stream_printf(s, "    </FrameList>\n");
    }
    stream_printf(s, "</RSprite>\n");

    return 0;
}

int ui_data_proj_save_sprite(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    if (ui_data_proj_save_gen_meta_file(root, src, em) != 0) return -1;    
    return ui_data_src_save_to_file(src, root, ui_data_proj_postfix(ui_data_src_type(src)), ui_data_proj_save_sprite_i, ctx, em);
}
