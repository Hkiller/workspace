#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/cfg/cfg.h"
#include "cpe/plist/plist_cfg.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_pixel_buf.h"
#include "plugin/chipmunk_manip/plugin_chipmunk_chipmunk.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"

static int plugin_chipmunk_import_from_chipmunk_create_bodies(
    ui_ed_obj_t scene_obj, cfg_t chipmunk_root, float adj_pic_height, float ptm, error_monitor_t em);

int plugin_chipmunk_import_from_chipmunk(
    ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr,
    const char * chipmunk_path, const char * to_chipmunk_path,
    const char * adj_picture, float ptm, error_monitor_t em)
{
    ui_ed_src_t chipmunk_ed_src;
    ui_ed_obj_t scene_obj;
    cfg_t chipmunk_root = NULL;
    int rv = -1;
    uint32_t adj_pic_height = 0;

    chipmunk_root = plist_cfg_load_dict_from_file(chipmunk_path, em);
    if (chipmunk_root == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: load from file %s fail!", chipmunk_path);
        goto COMPLETE;
    }

    if (adj_picture) {
        ui_cache_manager_t cache_mgr;
        ui_cache_pixel_buf_t pixel_buf;

        cache_mgr = ui_cache_manager_find_nc(ui_data_mgr_app(data_mgr), NULL);
        if (cache_mgr == NULL) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: load adj picture %s: no cache mgr!", adj_picture);
            goto COMPLETE;
        };
        
        pixel_buf = ui_cache_pixel_buf_create(cache_mgr);
        if (pixel_buf == NULL) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: load adj picture %s: create pixel buff fail!", adj_picture);
            goto COMPLETE;
        }

        if (ui_cache_pixel_buf_load_from_file(pixel_buf, adj_picture, em, NULL) != 0) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: load adj picture %s: load fail!", adj_picture);
            ui_cache_pixel_buf_free(pixel_buf);
            goto COMPLETE;
        }

        adj_pic_height = ui_cache_pixel_buf_level_height(ui_cache_pixel_buf_level_info_at(pixel_buf, 0));
        
        ui_cache_pixel_buf_free(pixel_buf);
    }
    
    /* do { */
    /*     struct mem_buffer buffer; */
    /*     mem_buffer_init(&buffer, NULL); */
    /*     printf("dump data\n%s", cfg_dump(chipmunk_root, &buffer, 0, 4)); */
    /*     mem_buffer_clear(&buffer); */
    /* } while(0); */
    
    chipmunk_ed_src =  ui_ed_src_check_create(ed_mgr, to_chipmunk_path, ui_data_src_type_chipmunk_scene);
    if (chipmunk_ed_src == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: create output chipmunk src %s faile!", to_chipmunk_path);
        goto COMPLETE;
    }

    scene_obj = ui_ed_obj_only_child(ui_ed_src_root_obj(chipmunk_ed_src));
    assert(scene_obj);
    ui_ed_obj_remove_childs(scene_obj);

    if (plugin_chipmunk_import_from_chipmunk_create_bodies(scene_obj, chipmunk_root, adj_pic_height, ptm, em) != 0) {
        goto COMPLETE;
    }

    rv = 0;
    
COMPLETE:
    if (chipmunk_root) cfg_free(chipmunk_root);

    return rv;
}

static ui_ed_obj_t plugin_chipmunk_import_from_chipmunk_create_fixture(
    ui_ed_obj_t body_obj, cfg_t chipmunk_fixture, float ptm, error_monitor_t em)
{
    ui_ed_obj_t fixture_obj;
    CHIPMUNK_FIXTURE * fixture_data;
            
    fixture_obj = ui_ed_obj_new(body_obj, ui_ed_obj_type_chipmunk_fixture);
    if (fixture_obj == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: create fixture obj faile!");
        return NULL;
    }
    fixture_data = ui_ed_obj_data(fixture_obj);

    cpe_str_dup(fixture_data->name, sizeof(fixture_data->name), cfg_get_string(chipmunk_fixture, "name", ""));
    fixture_data->mass = cfg_get_float(chipmunk_fixture, "mass", 0.0f);
    fixture_data->elasticity = cfg_get_float(chipmunk_fixture, "elasticity", 0.0f);
    fixture_data->density = cfg_get_float(chipmunk_fixture, "density", 0.0f);
    fixture_data->friction = cfg_get_float(chipmunk_fixture, "friction", 0.0f);
    fixture_data->surface_velocity.x = cfg_get_float(chipmunk_fixture, "", 0.0f) * ptm;
    fixture_data->surface_velocity.y = cfg_get_float(chipmunk_fixture, "", 0.0f) * ptm;
    fixture_data->collision_mask = 0;
    fixture_data->collision_group = 0;
    fixture_data->collision_category = 0;
    fixture_data->is_sensor = cfg_get_uint8(chipmunk_fixture, "isSensor", 0);;

    return fixture_obj;
}

static int plugin_chipmunk_import_from_chipmunk_read_pair(CHIPMUNK_PAIR * pos, const char * str, error_monitor_t em) {
    char point_pair_buf[128];
    char * begin;
    char * end;
    char * sep;
            
    cpe_str_dup(point_pair_buf, sizeof(point_pair_buf), str);
    begin = strchr(point_pair_buf, '{');
    sep = strchr(begin + 1, ',');
    end = strchr(sep + 1, '}');

    if (begin == NULL || sep == NULL || end == NULL) {
        CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: read point data %s format error!", str);
        return -1;
    }

    *sep = 0;
    *end = 0;
            
    pos->x = atof(begin + 1);
    pos->y = atof(sep + 1);

    return 0;
}

static int plugin_chipmunk_import_from_chipmunk_create_fixture_polygon(
    ui_ed_obj_t body_obj, cfg_t fixture_cfg, cfg_t polygon_cfg, float adj_pic_height, float ptm, error_monitor_t em)
{
    CHIPMUNK_BODY * body_data = ui_ed_obj_data(body_obj);
    ui_ed_obj_t fixture_obj;
    CHIPMUNK_FIXTURE * fixture_data;
    uint32_t group_id = 0;
    struct cfg_it group_it;
    cfg_t group_cfg;
    int rv = 0;

    fixture_obj = plugin_chipmunk_import_from_chipmunk_create_fixture(body_obj, fixture_cfg, ptm, em);
    if (fixture_obj == NULL) return -1;

    fixture_data = ui_ed_obj_data(fixture_obj);
    fixture_data->fixture_type = chipmunk_fixture_type_polygon;

    cfg_it_init(&group_it, polygon_cfg);
    for(group_cfg = cfg_it_next(&group_it); group_cfg; group_cfg = cfg_it_next(&group_it), group_id++) {
        struct cfg_it point_it;
        cfg_t point_cfg;
        ui_ed_obj_t polygon_node_obj;
        CHIPMUNK_POLYGON_NODE * polygon_node_data;
        uint32_t node_count;
        CHIPMUNK_PAIR * pos_buf = NULL;
        uint32_t i;

        /*读取节点数据 */
        node_count = cfg_child_count(group_cfg);
        if (node_count < 3) {
            CPE_ERROR(
                em, "plugin_chipmunk_import_from_chipmunk: fixture %s.%s: group %d: node count %d too small!",
                body_data->name, fixture_data->name, group_id, node_count);
            rv = -1;
            continue;
        }

        /*构造节点数组的空间 */
        pos_buf = mem_alloc(NULL, sizeof(pos_buf[0]) * node_count);
        if (pos_buf == NULL) {
            CPE_ERROR(
                em, "plugin_chipmunk_import_from_chipmunk: fixture %s.%s: group %d: alloc pos buf(count=%d) fail!",
                body_data->name, fixture_data->name, group_id, node_count);
            rv = -1;
            continue;
        }

        /*读取节点列表 */
        cfg_it_init(&point_it, group_cfg);
        i = 0;
        while((point_cfg = cfg_it_next(&point_it))) {
            if (plugin_chipmunk_import_from_chipmunk_read_pair(&pos_buf[i], cfg_as_string(point_cfg, ""), em) != 0) break;
            ++i;
        }

        if (i != node_count) { mem_free(NULL, pos_buf); rv = -1; continue; }

        /*构建节点列表 */
        if (adj_pic_height) {
            /*翻转轴坐标，同时调整节点顺序，保持顺时针顺序 */
            for(i = node_count; i > 0; --i) {
                polygon_node_obj = ui_ed_obj_new(fixture_obj, ui_ed_obj_type_chipmunk_polygon_node);
                if (polygon_node_obj == NULL) {
                    CPE_ERROR(
                        em, "plugin_chipmunk_import_from_chipmunk: fixture %s.%s: create polygon_node obj faile!",
                        body_data->name, fixture_data->name);
                    mem_free(NULL, pos_buf);
                    return -1;
                }
                polygon_node_data = ui_ed_obj_data(polygon_node_obj);
                polygon_node_data->group_id = group_id;
                polygon_node_data->pos = pos_buf[i - 1];
                polygon_node_data->pos.y = adj_pic_height - polygon_node_data->pos.y;
            }
        }
        else {
            /*不翻转轴坐标 */
            for(i = 0; i < node_count; ++i) {
                polygon_node_obj = ui_ed_obj_new(fixture_obj, ui_ed_obj_type_chipmunk_polygon_node);
                if (polygon_node_obj == NULL) {
                    CPE_ERROR(
                        em, "plugin_chipmunk_import_from_chipmunk: fixture %s.%s: create polygon_node obj faile!",
                        body_data->name, fixture_data->name);
                    mem_free(NULL, pos_buf);
                    return -1;
                }
                polygon_node_data = ui_ed_obj_data(polygon_node_obj);
                polygon_node_data->group_id = group_id;
                polygon_node_data->pos = pos_buf[i];
            }
        }

        mem_free(NULL, pos_buf);
    }
    
    return rv;
}

static int plugin_chipmunk_import_from_chipmunk_create_bodies(
    ui_ed_obj_t scene_obj, cfg_t chipmunk_root, float adj_pic_height, float ptm, error_monitor_t em)
{
    struct cfg_it bodies_it;
    cfg_t chipmunk_body;
    int rv = 0;
    
    cfg_it_init(&bodies_it, cfg_find_cfg(chipmunk_root, "bodies"));
    while((chipmunk_body = cfg_it_next(&bodies_it))) {
        ui_ed_obj_t body_obj;
        CHIPMUNK_BODY * body_data;
        struct cfg_it fixtures_it;
        cfg_t chipmunk_fixture;

        body_obj = ui_ed_obj_new(scene_obj, ui_ed_obj_type_chipmunk_body);
        if (body_obj == NULL) {
            CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: create body obj faile!");
            return -1;
        }
        body_data = ui_ed_obj_data(body_obj);

        cpe_str_dup(body_data->name, sizeof(body_data->name), cfg_name(chipmunk_body));
        body_data->anchorpoint.x = cfg_get_float(chipmunk_body, "anchorpoint.x", 0.0f) * ptm;
        body_data->anchorpoint.y = - cfg_get_float(chipmunk_body, "anchorpoint.y", 0.0f) * ptm;

        cfg_it_init(&fixtures_it, cfg_find_cfg(chipmunk_body, "fixtures"));
        while((chipmunk_fixture = cfg_it_next(&fixtures_it))) {
            cfg_t entry_cfg;

            if ((entry_cfg = cfg_find_cfg(chipmunk_fixture, "polygons"))) {
                if (plugin_chipmunk_import_from_chipmunk_create_fixture_polygon(body_obj, chipmunk_fixture, entry_cfg, adj_pic_height, ptm, em) != 0) {
                    rv = -1;
                }
            }
            else {
                CPE_ERROR(em, "plugin_chipmunk_import_from_chipmunk: unknown fixture type!");
                rv = -1;
            }
        }
    }

    return rv;
}
