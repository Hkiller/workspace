#include <assert.h>
#include "cpe/utils/string_utils.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_sprite.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "plugin/chipmunk_manip/plugin_chipmunk_basic.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"

static int plugin_chipmunk_import_from_sprite_add_polygon_node(ui_ed_obj_t fixture_obj, float x, float y, error_monitor_t em) {
    ui_ed_obj_t polygon_node_obj;
    CHIPMUNK_POLYGON_NODE * polygon_node_data;

    polygon_node_obj = ui_ed_obj_new(fixture_obj, ui_ed_obj_type_chipmunk_polygon_node);
    if (polygon_node_obj == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_sprite: create polygon_node obj faile!");
        return -1;
    }
    polygon_node_data = ui_ed_obj_data(polygon_node_obj);

    polygon_node_data->group_id = 0;
    polygon_node_data->pos.x = x;
    polygon_node_data->pos.y = y;
    return 0;
}

int plugin_chipmunk_import_from_sprite(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr,
    const char * sprite_path, const char * to_chipmunk_path, error_monitor_t em)
{
    ui_data_src_t sprite_src;
    ui_data_sprite_t sprite;
    ui_data_frame_t frame;
    struct ui_data_frame_it frame_it;
    ui_ed_src_t chipmunk_ed_src;
    ui_ed_obj_t scene_obj;

    sprite_src = ui_data_src_find_by_path(data_mgr, sprite_path, ui_data_src_type_sprite);
    if (sprite_src == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_sprite: sprite %s not exist!", sprite_path);
        return -1;
    }

    if (ui_data_src_check_load_with_usings(sprite_src, em) != 0) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_sprite: sprite %s check load fail!", sprite_path);
        return -1;
    }

    sprite = ui_data_src_product(sprite_src);
    assert(sprite);

    chipmunk_ed_src =  ui_ed_src_check_create(ed_mgr, to_chipmunk_path, ui_data_src_type_chipmunk_scene);
    if (chipmunk_ed_src == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_sprite: create output chipmunk src %s faile!", to_chipmunk_path);
        return -1;
    }

    scene_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(chipmunk_ed_src));
    assert(scene_obj);
    ui_ed_obj_remove_childs(scene_obj);

    ui_data_sprite_frames(&frame_it, sprite);
    while((frame = ui_data_frame_it_next(&frame_it))) {
        UI_FRAME const * frame_data = ui_data_frame_data(frame);
        ui_ed_obj_t body_obj;
        CHIPMUNK_BODY * body_data;
        ui_data_frame_collision_t collision;
        struct ui_data_frame_collision_it collision_it;

        body_obj = ui_ed_obj_new(scene_obj, ui_ed_obj_type_chipmunk_body);
        if (body_obj == NULL) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_sprite: create body obj faile!");
            return -1;
        }
        body_data = ui_ed_obj_data(body_obj);

        body_data->id = (frame_data->id == (uint32_t)-1) ? 0 : frame_data->id;
        cpe_str_dup(body_data->name, sizeof(body_data->name), frame_data->name);
        body_data->type = CHIPMUNK_OBJ_TYPE_DYNAMIC;

        ui_data_frame_collisions(&collision_it, frame);
        while((collision = ui_data_frame_collision_it_next(&collision_it))) {
            UI_COLLISION const * collision_data = ui_data_frame_collision_data(collision);
            ui_ed_obj_t fixture_obj;
            CHIPMUNK_FIXTURE * fixture_data;

            fixture_obj = ui_ed_obj_new(body_obj, ui_ed_obj_type_chipmunk_fixture);
            if (fixture_obj == NULL) {
                CPE_ERROR(em, "plugin_chipmunk_import_from_sprite: create fixture obj faile!");
                return -1;
            }
            fixture_data = ui_ed_obj_data(fixture_obj);

            cpe_str_dup(fixture_data->name, sizeof(fixture_data->name), collision_data->name);
            fixture_data->fixture_type = chipmunk_fixture_type_polygon;

            if (plugin_chipmunk_import_from_sprite_add_polygon_node(
                    fixture_obj, collision_data->bounding.lt, collision_data->bounding.tp, em) != 0
                || plugin_chipmunk_import_from_sprite_add_polygon_node(
                    fixture_obj, collision_data->bounding.rt, collision_data->bounding.tp, em) != 0
                || plugin_chipmunk_import_from_sprite_add_polygon_node(
                    fixture_obj, collision_data->bounding.rt, collision_data->bounding.bm, em) != 0
                || plugin_chipmunk_import_from_sprite_add_polygon_node(
                    fixture_obj, collision_data->bounding.lt, collision_data->bounding.bm, em) != 0
                )
            {
                return -1;
            }
        }
    }

    return 0;
}
