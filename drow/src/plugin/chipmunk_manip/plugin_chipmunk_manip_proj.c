#include <assert.h>
#include "cpe/vfs/vfs_stream.h"
#include "cpe/cfg/cfg.h"
#include "cpe/dr/dr_cfg.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/chipmunk/plugin_chipmunk_data_scene.h"
#include "plugin/chipmunk/plugin_chipmunk_data_body.h"
#include "plugin/chipmunk/plugin_chipmunk_data_fixture.h"
#include "plugin/chipmunk/plugin_chipmunk_data_polygon_node.h"
#include "plugin/chipmunk/plugin_chipmunk_data_constraint.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin_chipmunk_manip_i.h"

static int plugin_chipmunk_data_scene_do_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_chipmunk_manip_t manip = ctx;
    cfg_t cfg = NULL;
    plugin_chipmunk_data_scene_t scene = ui_data_src_product(src);
    struct plugin_chipmunk_data_body_it bodies_it;
    plugin_chipmunk_data_body_t body;
    cfg_t bodies_cfg = NULL;
    struct plugin_chipmunk_data_fixture_it fixtures_it;
    plugin_chipmunk_data_fixture_t fixture;
    cfg_t fixtures_cfg = NULL;
    plugin_chipmunk_data_polygon_node_t polygon_node;
    cfg_t polygon_nodes_cfg = NULL;
    struct plugin_chipmunk_data_constraint_it constraints_it;
    plugin_chipmunk_data_constraint_t constraint;
    cfg_t constraints_cfg = NULL;
    struct vfs_write_stream fs;
    LPDRMETA scene_meta = plugin_chipmunk_module_data_scene_meta(manip->m_chipmunk_module);
    LPDRMETA body_meta = plugin_chipmunk_module_data_body_meta(manip->m_chipmunk_module);
    LPDRMETA fixture_meta = plugin_chipmunk_module_data_fixture_meta(manip->m_chipmunk_module);
    LPDRMETA polygon_node_meta = plugin_chipmunk_module_data_polygon_node_meta(manip->m_chipmunk_module);
    LPDRMETA constraint_meta = plugin_chipmunk_module_data_constraint_meta(manip->m_chipmunk_module);

    cfg = cfg_create(manip->m_alloc);
    if (cfg == NULL) {
        CPE_ERROR(em, "create cfg fail!");
        return -1;
    }

    if (dr_cfg_write(cfg, plugin_chipmunk_data_scene_data(scene), scene_meta, em) != 0) {
        CPE_ERROR(em, "write scene data fail!");
        return -1;
    }

    bodies_cfg = cfg_struct_add_seq(cfg, "bodies", cfg_replace);
    if (bodies_cfg == NULL) {
        CPE_ERROR(em, "create bodies_cfg fail!");
        cfg_free(cfg);
        return -1;
    }
    
    plugin_chipmunk_data_scene_bodies(&bodies_it, scene);
    while((body = plugin_chipmunk_data_body_it_next(&bodies_it))) {
        cfg_t body_cfg = cfg_seq_add_struct(bodies_cfg);
        if (dr_cfg_write(body_cfg, plugin_chipmunk_data_body_data(body), body_meta, em) != 0) {
            CPE_ERROR(em, "write body data fail!");
            cfg_free(cfg);
            return -1;
        }

        fixtures_cfg = cfg_struct_add_seq(body_cfg, "fixtures", cfg_replace);
        if (fixtures_cfg == NULL) {
            CPE_ERROR(em, "create fixtures_cfg fail!");
            cfg_free(cfg);
            return -1;
        }
    
        plugin_chipmunk_data_body_fixtures(&fixtures_it, body);
        while((fixture = plugin_chipmunk_data_fixture_it_next(&fixtures_it))) {
            cfg_t fixture_cfg = cfg_seq_add_struct(fixtures_cfg);
            if (dr_cfg_write(fixture_cfg, plugin_chipmunk_data_fixture_data(fixture), fixture_meta, em) != 0) {
                CPE_ERROR(em, "write fixture data fail!");
                cfg_free(cfg);
                return -1;
            }

            polygon_nodes_cfg = cfg_struct_add_seq(fixture_cfg, "polygon-nodes", cfg_replace);
            if (polygon_nodes_cfg == NULL) {
                CPE_ERROR(em, "create polygons_nodes_cfg fail!");
                cfg_free(cfg);
                return -1;
            }

            for(polygon_node = plugin_chipmunk_data_polygon_node_head(fixture);
                polygon_node;
                polygon_node = plugin_chipmunk_data_polygon_node_next(polygon_node))
            {
                if (dr_cfg_write(
                        cfg_seq_add_struct(polygon_nodes_cfg),
                        plugin_chipmunk_data_polygon_node_data(polygon_node), polygon_node_meta, em)
                    != 0)
                {
                    CPE_ERROR(em, "write polygon node data fail!");
                    cfg_free(cfg);
                    return -1;
                }
            }
        }
    }

    constraints_cfg = cfg_struct_add_seq(cfg, "constraints", cfg_replace);
    if (constraints_cfg == NULL) {
        CPE_ERROR(em, "create constraints_cfg fail!");
        cfg_free(cfg);
        return -1;
    }
    
    plugin_chipmunk_data_scene_constraints(&constraints_it, scene);
    while((constraint = plugin_chipmunk_data_constraint_it_next(&constraints_it))) {
        cfg_t constraint_cfg = cfg_seq_add_struct(constraints_cfg);
        if (dr_cfg_write(constraint_cfg, plugin_chipmunk_data_constraint_data(constraint), constraint_meta, em) != 0) {
            CPE_ERROR(em, "write constraint data fail!");
            cfg_free(cfg);
            return -1;
        }
    }
    
    vfs_write_stream_init(&fs, fp);
    if (cfg_yaml_write((write_stream_t)&fs, cfg, em) != 0) {
        CPE_ERROR(em, "cfg write to file fail!");
        cfg_free(cfg);
        return -1;
    }

    cfg_free(cfg);
    return 0;
}

static int plugin_chipmunk_data_scene_do_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_chipmunk_manip_t manip = ctx;
    plugin_chipmunk_data_scene_t scene = NULL;
    plugin_chipmunk_data_body_t body = NULL;
    plugin_chipmunk_data_fixture_t fixture = NULL;
    plugin_chipmunk_data_polygon_node_t polygon_node = NULL;
    plugin_chipmunk_data_constraint_t constraint = NULL;
    struct vfs_read_stream fs;
    cfg_t cfg;
    struct cfg_it bodies_it;
    cfg_t body_cfg;
    struct cfg_it fixtures_it;
    cfg_t fixture_cfg;
    struct cfg_it polygon_nodes_it;
    cfg_t polygon_node_cfg;
    struct cfg_it constraints_it;
    cfg_t constraint_cfg;
    LPDRMETA scene_meta = plugin_chipmunk_module_data_scene_meta(manip->m_chipmunk_module);
    LPDRMETA body_meta = plugin_chipmunk_module_data_body_meta(manip->m_chipmunk_module);
    LPDRMETA fixture_meta = plugin_chipmunk_module_data_fixture_meta(manip->m_chipmunk_module);
    LPDRMETA polygon_node_meta = plugin_chipmunk_module_data_polygon_node_meta(manip->m_chipmunk_module);
    LPDRMETA constraint_meta = plugin_chipmunk_module_data_constraint_meta(manip->m_chipmunk_module);
    int rv = -1;

    cfg = cfg_create(manip->m_alloc);
    if (cfg == NULL) {
        CPE_ERROR(em, "load scene: create cfg fail!");
        goto COMPLETE;
    }

    vfs_read_stream_init(&fs, fp);
    if (cfg_yaml_read(cfg, (read_stream_t)&fs, cfg_replace, em) < 0) {
        CPE_ERROR(em, "load scene: read from file fail!");
        goto COMPLETE;
    }

    scene = plugin_chipmunk_data_scene_create(manip->m_chipmunk_module, src);
    if (scene == NULL) {
        CPE_ERROR(em, "load scene: create scene fail!");
        goto COMPLETE;
    }

    if (dr_cfg_read(
            plugin_chipmunk_data_scene_data(scene),
            sizeof(*plugin_chipmunk_data_scene_data(scene)),
            cfg, scene_meta, 0, NULL) < 0)
    {
        CPE_ERROR(em, "load scene: load bullet data fail!");
        goto COMPLETE;
    }
    
    cfg_it_init(&bodies_it, cfg_find_cfg(cfg, "bodies"));
    while((body_cfg = cfg_it_next(&bodies_it))) {
        body = plugin_chipmunk_data_body_create(scene);
        if (body == NULL) {
            CPE_ERROR(em, "load scene: create body fail!");
            goto COMPLETE;
        }
        
        if (dr_cfg_read(
                plugin_chipmunk_data_body_data(body),
                sizeof(*plugin_chipmunk_data_body_data(body)),
                body_cfg, body_meta, 0, NULL) < 0)
        {
            CPE_ERROR(em, "load scene: load body data fail!");
            goto COMPLETE;
        }

        cfg_it_init(&fixtures_it, cfg_find_cfg(body_cfg, "fixtures"));
        while((fixture_cfg = cfg_it_next(&fixtures_it))) {
            fixture = plugin_chipmunk_data_fixture_create(body);
            if (fixture == NULL) {
                CPE_ERROR(em, "load body: create fixture fail!");
                goto COMPLETE;
            }
        
            if (dr_cfg_read(
                    plugin_chipmunk_data_fixture_data(fixture),
                    sizeof(*plugin_chipmunk_data_fixture_data(fixture)),
                    fixture_cfg, fixture_meta, 0, NULL) < 0)
            {
                CPE_ERROR(em, "load body: load fixture data fail!");
                goto COMPLETE;
            }

            cfg_it_init(&polygon_nodes_it, cfg_find_cfg(fixture_cfg, "polygon-nodes"));
            while((polygon_node_cfg = cfg_it_next(&polygon_nodes_it))) {
                polygon_node = plugin_chipmunk_data_polygon_node_create(fixture);
                if (polygon_node == NULL) {
                    CPE_ERROR(em, "load body: create polygon_node fail!");
                    goto COMPLETE;
                }
        
                if (dr_cfg_read(
                        plugin_chipmunk_data_polygon_node_data(polygon_node),
                        sizeof(*plugin_chipmunk_data_polygon_node_data(polygon_node)),
                        polygon_node_cfg, polygon_node_meta, 0, NULL) < 0)
                {
                    CPE_ERROR(em, "load body: load polygon_node data fail!");
                    goto COMPLETE;
                }
            }
        }
    }

    cfg_it_init(&constraints_it, cfg_find_cfg(cfg, "constraints"));
    while((constraint_cfg = cfg_it_next(&constraints_it))) {
        constraint = plugin_chipmunk_data_constraint_create(scene);
        if (constraint == NULL) {
            CPE_ERROR(em, "load scene: create constraint fail!");
            goto COMPLETE;
        }
        
        if (dr_cfg_read(
                plugin_chipmunk_data_constraint_data(constraint),
                sizeof(*plugin_chipmunk_data_constraint_data(constraint)),
                constraint_cfg, constraint_meta, 0, NULL) < 0)
        {
            CPE_ERROR(em, "load scene: load constraint data fail!");
            goto COMPLETE;
        }
    }
    
    rv = 0;

COMPLETE:
    if (cfg) cfg_free(cfg);

    if (rv != 0 && scene) {
        plugin_chipmunk_data_scene_free(scene);        
    }
    
    return rv;
}

int plugin_chipmunk_scene_proj_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "chipmunk", plugin_chipmunk_data_scene_do_save, ctx, em);
}

int plugin_chipmunk_scene_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "chipmunk", plugin_chipmunk_data_scene_do_load, ctx, em);
}

int plugin_chipmunk_scene_proj_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "chipmunk", em);
}
