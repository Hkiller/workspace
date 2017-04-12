#include <assert.h>
#include "cpe/vfs/vfs_stream.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/tiledmap/plugin_tiledmap_data_scene.h"
#include "plugin/tiledmap/plugin_tiledmap_data_layer.h"
#include "plugin/tiledmap/plugin_tiledmap_data_tile.h"
#include "plugin/tiledmap/plugin_tiledmap_module.h"
#include "plugin_tiledmap_manip_i.h"

static int plugin_tiledmap_data_scene_do_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    //plugin_tiledmap_manip_t manip = ctx;
    plugin_tiledmap_data_scene_t scene = ui_data_src_product(src);
    struct plugin_tiledmap_data_layer_it layers_it;
    plugin_tiledmap_data_layer_t layer;
    struct plugin_tiledmap_data_tile_it tiles_it;
    plugin_tiledmap_data_tile_t tile;
    struct vfs_write_stream fs = VFS_WRITE_STREAM_INITIALIZER(fp);
    write_stream_t s = (write_stream_t)&fs;

    stream_printf(s, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    stream_printf(s, "<SceneLayerList>\n");
    
    plugin_tiledmap_data_scene_layers(&layers_it, scene);
    while((layer = plugin_tiledmap_data_layer_it_next(&layers_it))) {
        TILEDMAP_LAYER const * layer_data = plugin_tiledmap_data_layer_data(layer);
        uint32_t layer_row = layer_data->cell_row_begin < 0 ? layer_data->cell_row_begin : layer_data->cell_row_end;
        uint32_t layer_col = layer_data->cell_col_begin < 0 ? layer_data->cell_col_begin : layer_data->cell_col_end;

        stream_printf(
            s, "    <SceneLayer Name='%s' Type='0' CellR='%d' CellC='%d' CellW='%d' CellH='%d' X='0.000000' Y='0.000000'>\n",
            layer_data->name, layer_row, layer_col, layer_data->cell_w, layer_data->cell_h);

        plugin_tiledmap_data_layer_tiles(&tiles_it, layer);
        while((tile = plugin_tiledmap_data_tile_it_next(&tiles_it))) {
            TILEDMAP_TILE const * tile_data = plugin_tiledmap_data_tile_data(tile);

            if (tile_data->ref_type == tiledmap_tile_ref_type_img) {
                stream_printf(
                    s, "        <R2DSImageRef Row='0' Col='0' Name='%s' Resfile='%u' Imageid='%u' Resname=''>\n",
                    tile_data->name,
                    tile_data->ref_data.img.module_id,
                    tile_data->ref_data.img.img_block_id);

                stream_printf(s, "          <WorldAngle>%f</WorldAngle>\n", tile_data->angle_type * 90.0f);
                stream_printf(s, "          <WorldFlips>%d</WorldFlips>\n", tile_data->flip_type);
                stream_printf(s, "          <WorldScale x='%f' y='%f' z='1.000000'/>\n", tile_data->scale.x, tile_data->scale.y);
                stream_printf(s, "          <WorldTrans x='%f' y='%f' z='0.000000'/>\n", tile_data->pos.x, tile_data->pos.y);
                stream_printf(s, "          <BlendColor A='1.000000' R='1.000000' G='1.000000' B='1.000000'/>\n");
                stream_printf(s, "          <Filter>0</Filter>\n");
                stream_printf(s, "          <TexEnv>0</TexEnv>\n");
                stream_printf(s, "          <OLInfo>\n");
                stream_printf(s, "            <Outline>False</Outline>\n");
                stream_printf(s, "            <OutlineWidth>1</OutlineWidth>\n");
                stream_printf(s, "            <OutlineColor A='1.000000' R='0.000000' G='0.000000' B='0.000000'/>\n");
                stream_printf(s, "          </OLInfo>\n");
                
                stream_printf(
                    s, "        </R2DSImageRef>\n");
            }
            else if (tile_data->ref_type == tiledmap_tile_ref_type_frame) {
                stream_printf(
                    s, "        <R2DSFrameRef Row='0' Col='0' Name='%s' Resfile='%u' Frameid='%u'>\n",
                    tile_data->name,
                    tile_data->ref_data.img.module_id,
                    tile_data->ref_data.img.img_block_id);

                stream_printf(s, "          <WorldAngle>%f</WorldAngle>\n", tile_data->angle_type * 90.0f);
                stream_printf(s, "          <WorldFlips>%d</WorldFlips>\n", tile_data->flip_type);
                stream_printf(s, "          <WorldScale x='%f' y='%f' z='1.000000'/>\n", tile_data->scale.x, tile_data->scale.y);
                stream_printf(s, "          <WorldTrans x='%f' y='%f' z='0.000000'/>\n", tile_data->pos.x, tile_data->pos.y);
                stream_printf(s, "          <BlendColor A='1.000000' R='1.000000' G='1.000000' B='1.000000'/>\n");
                stream_printf(s, "          <Filter>0</Filter>\n");
                stream_printf(s, "          <TexEnv>0</TexEnv>\n");
                stream_printf(s, "          <OLInfo>\n");
                stream_printf(s, "            <Outline>False</Outline>\n");
                stream_printf(s, "            <OutlineWidth>1</OutlineWidth>\n");
                stream_printf(s, "            <OutlineColor A='1.000000' R='0.000000' G='0.000000' B='0.000000'/>\n");
                stream_printf(s, "          </OLInfo>\n");
                
                stream_printf(
                    s, "        </R2DSFrameRef>\n");
            }
        }

        stream_printf(s, "    </SceneLayer>\n");
    }

    stream_printf(s, "</SceneLayerList>\n");

    return 0;
}

int plugin_tiledmap_scene_proj_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "map", plugin_tiledmap_data_scene_do_save, ctx, em);
}
