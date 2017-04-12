#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/vfs/vfs_file.h"
#include "render/model/ui_data_action.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "ui_proj_save_utils.h"
#include "ui_proj_utils.h"

static int ui_data_proj_save_action_i(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    struct vfs_write_stream fs;
    ui_data_action_t action = ui_data_src_product(src);
    struct ui_data_src_it using_src_it;
    ui_data_src_t using_src;
    struct ui_data_actor_it actor_it;
    ui_data_actor_t actor;
    struct ui_data_actor_layer_it layer_it;
    ui_data_actor_layer_t layer;
    struct ui_data_actor_frame_it frame_it;
    ui_data_actor_frame_t frame;
    uint8_t have_ref_frame = 0;
    write_stream_t s = (write_stream_t)&fs;

    vfs_write_stream_init(&fs, fp);
    
    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    stream_printf(s, "<RAction>\n");

    ui_data_src_using_srcs(src, &using_src_it);
    while((using_src = ui_data_src_it_next(&using_src_it))) {
        if (ui_data_src_type(using_src) == ui_data_src_type_sprite) {
            if (have_ref_frame == 0) {
                stream_printf(s, "<FrameFile>\n");
                have_ref_frame = 1;
            }

            stream_printf(s, "    <File Resfile=\"%u\" />\n", ui_data_src_id(using_src));
        }
    }

    if (have_ref_frame) {
        stream_printf(s, "</FrameFile>\n");
    }
    else {
        stream_printf(s, "<FrameFile />\n");
    }

    ui_data_action_actors(&actor_it, action);
    actor = ui_data_actor_it_next(&actor_it);
    if (actor == NULL) {
        stream_printf(s, "<ActorList />\n");
    }
    else {
        stream_printf(s, "<ActorList>\n");
        for(; actor; actor = ui_data_actor_it_next(&actor_it)) {
            UI_ACTOR const * actor_data = ui_data_actor_data(actor);

            stream_printf(s, "    <Actor ID=\"%d\" Name=\"%s\">\n", actor_data->id, actor_data->name);
            ui_data_proj_save_bool(s, 2, "Loop", actor_data->is_loop);
            ui_data_proj_save_int(s, 2, "LoopStart", actor_data->loop_start);

            ui_data_actor_layers(&layer_it, actor);
            layer = ui_data_actor_layer_it_next(&layer_it);
            if (layer == NULL) {
                stream_printf(s, "        <LayerList />\n");
            }
            else {
                stream_printf(s, "        <LayerList>\n");

                for(; layer; layer = ui_data_actor_layer_it_next(&layer_it)) {
                    UI_ACTOR_LAYER const * layer_data = ui_data_actor_layer_data(layer);

                    ui_data_actor_layer_frames(&frame_it, layer);
                    frame = ui_data_actor_frame_it_next(&frame_it);
                    if (frame == NULL) {
                        stream_printf(s, "            <Layer Name=\"%s\" Lead=\"%s\" />\n", layer_data->name, layer_data->is_lead ? "True" : "False");
                    }
                    else {
                        stream_printf(s, "            <Layer Name=\"%s\" Lead=\"%s\">\n", layer_data->name, layer_data->is_lead ? "True" : "False");

                        for(; frame; frame = ui_data_actor_frame_it_next(&frame_it)) {
                            UI_ACTOR_FRAME const * frame_data = ui_data_actor_frame_data(frame);
                            const char * tag_name;

                            if (frame_data->texture.type == UI_TEXTURE_REF_FRAME) {
                                UI_FRAME_REF const * frame_ref_data = &frame_data->texture.data.frame;

                                stream_printf(s, "                <FrameRef Resfile=\"%u\" Frameid=\"%u\">\n",
                                              frame_ref_data->sprite_id, frame_ref_data->frame_id);
                                ui_data_proj_save_trans(s, 5, &frame_ref_data->trans);
                                tag_name = "FrameRef";
                            }
                            else if (frame_data->texture.type == UI_TEXTURE_REF_IMG) {
                                UI_IMG_REF const * img_ref_data = &frame_data->texture.data.img;

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
                                tag_name = "ImageRef";
                            }
                            else {
                                continue;
                            }

                            ui_data_proj_save_int(s, 5, "Time", frame_data->start_frame);
                            ui_data_proj_save_str(s, 5, "Event", "");
                            ui_data_proj_save_str(s, 5, "Sound", frame_data->sound);
                            ui_data_proj_save_bool(s, 5, "Smooth", frame_data->smooth);
                            ui_data_proj_save_str(s, 5, "Gfxfile", "");
                            ui_data_proj_save_bool(s, 5, "Ignored", 0);

                            stream_printf(s, "                </%s>\n", tag_name);
                        }

                        stream_printf(s, "            </Layer>\n");
                    }
                }

                stream_printf(s, "        </LayerList>\n");
            }

            stream_printf(s, "    </Actor>\n");
        }
        stream_printf(s, "</ActorList>\n");
        stream_printf(s, "</RAction>\n");
    }

    return 0;
}

int ui_data_proj_save_action(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    if (ui_data_proj_save_gen_meta_file(root, src, em) != 0) return -1;    
    return ui_data_src_save_to_file(src, root, ui_data_proj_postfix(ui_data_src_type(src)), ui_data_proj_save_action_i, ctx, em);
}
