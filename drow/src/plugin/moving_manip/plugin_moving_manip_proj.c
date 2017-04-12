#include <assert.h>
#include "cpe/vfs/vfs_stream.h"
#include "cpe/cfg/cfg.h"
#include "cpe/dr/dr_cfg.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/moving/plugin_moving_module.h"
#include "plugin_moving_manip_i.h"

static int plugin_moving_data_info_do_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_moving_manip_t manip = ctx;
    cfg_t cfg = NULL;
    plugin_moving_plan_t plan = ui_data_src_product(src);
    struct vfs_write_stream fs;
    LPDRMETA plan_meta = plugin_moving_module_moving_plan_meta(manip->m_moving_module);
    LPDRMETA track_meta = plugin_moving_module_moving_plan_track_meta(manip->m_moving_module);
    LPDRMETA point_meta = plugin_moving_module_moving_plan_point_meta(manip->m_moving_module);
    LPDRMETA node_meta = plugin_moving_module_moving_plan_node_meta(manip->m_moving_module);
    LPDRMETA segment_meta = plugin_moving_module_moving_plan_segment_meta(manip->m_moving_module);
    struct plugin_moving_plan_track_it track_it;
    plugin_moving_plan_track_t track;
    cfg_t tracks_cfg;
    struct plugin_moving_plan_node_it node_it;
    plugin_moving_plan_node_t node;
    cfg_t nodes_cfg;

    cfg = cfg_create(manip->m_alloc);
    if (cfg == NULL) {
        CPE_ERROR(em, "create cfg fail!");
        return -1;
    }

    if (dr_cfg_write(cfg, plugin_moving_plan_data(plan), plan_meta, em) != 0) {
        CPE_ERROR(em, "write data fail!");
        cfg_free(cfg);
        return -1;
    }

    /*tracks*/
    tracks_cfg = cfg_add_seq(cfg, "tracks", em);
    if (tracks_cfg == NULL) {
        CPE_ERROR(em, "create tracks fail!");
        cfg_free(cfg);
        return -1;
    }
    
    plugin_moving_plan_tracks(&track_it, plan);
    while((track = plugin_moving_plan_track_it_next(&track_it))) {
        struct plugin_moving_plan_point_it point_it;
        plugin_moving_plan_point_t point;
        cfg_t points_cfg;
        
        cfg_t track_cfg = cfg_seq_add_struct(tracks_cfg);
        if (track_cfg == NULL) {
            CPE_ERROR(em, "write info trigger data fail!");
            cfg_free(cfg);
            return -1;
        }

        if (dr_cfg_write(track_cfg, plugin_moving_plan_track_data(track), track_meta, em) != 0) {
            CPE_ERROR(em, "write info track data fail!");
            cfg_free(cfg);
            return -1;
        }

        /*points*/
        points_cfg = cfg_add_seq(track_cfg, "points", em);
        if (points_cfg == NULL) {
            CPE_ERROR(em, "create points fail!");
            cfg_free(cfg);
            return -1;
        }

        plugin_moving_plan_track_points(&point_it, track);
        while((point = plugin_moving_plan_point_it_next(&point_it))) {
            cfg_t point_cfg = cfg_seq_add_struct(points_cfg);
            if (point_cfg == NULL) {
                CPE_ERROR(em, "write plan poins data fail!");
                cfg_free(cfg);
                return -1;
            }

            if (dr_cfg_write(point_cfg, plugin_moving_plan_point_data(point), point_meta, em) != 0) {
                CPE_ERROR(em, "write plain point data fail!");

                cfg_free(cfg);
                return -1;
            }
        }
    }

    /*nodes*/
    nodes_cfg = cfg_add_seq(cfg, "nodes", em);
    if (nodes_cfg == NULL) {
        CPE_ERROR(em, "create nodes fail!");
        cfg_free(cfg);
        return -1;
    }
    
    plugin_moving_plan_nodes(&node_it, plan);
    while((node = plugin_moving_plan_node_it_next(&node_it))) {
        struct plugin_moving_plan_segment_it segment_it;
        plugin_moving_plan_segment_t segment;
        cfg_t segments_cfg;
        
        cfg_t node_cfg = cfg_seq_add_struct(nodes_cfg);
        if (node_cfg == NULL) {
            CPE_ERROR(em, "write plan node data fail!");
            cfg_free(cfg);
            return -1;
        }

        if (dr_cfg_write(node_cfg, plugin_moving_plan_node_data(node), node_meta, em) != 0) {
            CPE_ERROR(em, "write plan node data fail!");
            cfg_free(cfg);
            return -1;
        }

        /*segments*/
        segments_cfg = cfg_add_seq(node_cfg, "segments", em);
        if (segments_cfg == NULL) {
            CPE_ERROR(em, "create segments fail!");
            cfg_free(cfg);
            return -1;
        }

        plugin_moving_plan_node_segments(&segment_it, node);
        while((segment = plugin_moving_plan_segment_it_next(&segment_it))) {
            cfg_t segment_cfg = cfg_seq_add_struct(segments_cfg);
            if (segment_cfg == NULL) {
                CPE_ERROR(em, "write plan poins data fail!");
                cfg_free(cfg);
                return -1;
            }

            if (dr_cfg_write(segment_cfg, plugin_moving_plan_segment_data(segment), segment_meta, em) != 0) {
                CPE_ERROR(em, "write plain segment data fail!");
                cfg_free(cfg);
                return -1;
            }
        }
    }

    /*write to file*/
    vfs_write_stream_init(&fs, fp);
    if (cfg_yaml_write((write_stream_t)&fs, cfg, em) != 0) {
        CPE_ERROR(em, "cfg write fail!");
        cfg_free(cfg);
        return -1;
    }

    cfg_free(cfg);
    return 0;
}

static int plugin_moving_data_info_do_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_moving_manip_t manip = ctx;
    plugin_moving_plan_t plan = NULL;
    struct vfs_read_stream fs;
    cfg_t cfg = NULL;
    LPDRMETA plan_meta = plugin_moving_module_moving_plan_meta(manip->m_moving_module);
    LPDRMETA track_meta = plugin_moving_module_moving_plan_track_meta(manip->m_moving_module);
    LPDRMETA point_meta = plugin_moving_module_moving_plan_point_meta(manip->m_moving_module);
    LPDRMETA node_meta = plugin_moving_module_moving_plan_node_meta(manip->m_moving_module);
    LPDRMETA segment_meta = plugin_moving_module_moving_plan_segment_meta(manip->m_moving_module);
    struct cfg_it track_it;
    cfg_t track_cfg;
    struct cfg_it node_it;
    cfg_t node_cfg;

    cfg = cfg_create(manip->m_alloc);
    if (cfg == NULL) {
        CPE_ERROR(em, "load moving plan: create cfg fail!");
        goto LOAD_ERROR;
    }

    vfs_read_stream_init(&fs, fp);
    if (cfg_yaml_read(cfg, (read_stream_t)&fs, cfg_replace, em) < 0) {
        CPE_ERROR(em, "load moving plan: read from file fail!");
        goto LOAD_ERROR;
    }

    plan = plugin_moving_plan_create(manip->m_moving_module, src);
    if (dr_cfg_read(plugin_moving_plan_data(plan), sizeof(*plugin_moving_plan_data(plan)), cfg, plan_meta, 0, NULL) < 0) {
        CPE_ERROR(em, "load plan: load plan data fail!");
        goto LOAD_ERROR;
    }

    cfg_it_init(&track_it, cfg_find_cfg(cfg, "tracks"));
    while((track_cfg = cfg_it_next(&track_it))) {
        plugin_moving_plan_track_t track;
        MOVING_PLAN_TRACK * track_data;
        struct cfg_it point_it;
        cfg_t point_cfg;

        track = plugin_moving_plan_track_create(plan);
        if (track == NULL) {
            CPE_ERROR(em, "load moving plan: create info track fail!");
            goto LOAD_ERROR;
        }

        track_data = plugin_moving_plan_track_data(track);
        assert(track_data);

        if (dr_cfg_read(track_data, sizeof(*track_data), track_cfg, track_meta, 0, NULL) < 0) {
            CPE_ERROR(em, "load moving plan: load moving plan track data fail!");
            goto LOAD_ERROR;
        }

        cfg_it_init(&point_it, cfg_find_cfg(track_cfg, "points"));
        while((point_cfg = cfg_it_next(&point_it))) {
            plugin_moving_plan_point_t point;
            MOVING_PLAN_POINT * point_data;

            point = plugin_moving_plan_point_create(track);
            if (point == NULL) {
                CPE_ERROR(em, "load moving plan: create point fail!");
                goto LOAD_ERROR;
            }

            point_data = plugin_moving_plan_point_data(point);
            assert(point_data);
            if (dr_cfg_read(point_data, sizeof(*point_data), point_cfg, point_meta, 0, NULL) < 0) {
                CPE_ERROR(em, "load moving plan: load moving plan point data fail!");
                goto LOAD_ERROR;
            }
        }
    }

    cfg_it_init(&node_it, cfg_find_cfg(cfg, "nodes"));
    while((node_cfg = cfg_it_next(&node_it))) {
        plugin_moving_plan_node_t node;
        MOVING_PLAN_NODE * node_data;
        struct cfg_it segment_it;
        cfg_t segment_cfg;

        node = plugin_moving_plan_node_create(plan);
        if (node == NULL) {
            CPE_ERROR(em, "load moving plan: create info node fail!");
            goto LOAD_ERROR;
        }

        node_data = plugin_moving_plan_node_data(node);
        assert(node_data);

        if (dr_cfg_read(node_data, sizeof(*node_data), node_cfg, node_meta, 0, NULL) < 0) {
            CPE_ERROR(em, "load moving plan: load moving plan node data fail!");
            goto LOAD_ERROR;
        }


        cfg_it_init(&segment_it, cfg_find_cfg(node_cfg, "segments"));
        while((segment_cfg = cfg_it_next(&segment_it))) {
            plugin_moving_plan_segment_t segment;
            MOVING_PLAN_SEGMENT * segment_data;

            segment = plugin_moving_plan_segment_create(node);
            if (segment == NULL) {
                CPE_ERROR(em, "load moving plan: create segment fail!");
                goto LOAD_ERROR;
            }

            segment_data = plugin_moving_plan_segment_data(segment);
            assert(segment_data);
            if (dr_cfg_read(segment_data, sizeof(*segment_data), segment_cfg, segment_meta, 0, NULL) < 0) {
                CPE_ERROR(em, "load moving plan: load moving plan segment data fail!");
                goto LOAD_ERROR;
            }
        }
    }
    
    cfg_free(cfg);
    return 0;

LOAD_ERROR:    
    if (cfg) cfg_free(cfg);
    if (plan) plugin_moving_plan_free(plan);
    return -1;
}

static int plugin_moving_plan_proj_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "moving", plugin_moving_data_info_do_save, ctx, em);
}

static int plugin_moving_plan_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "moving", plugin_moving_data_info_do_load, ctx, em);
}

static int plugin_moving_plan_proj_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "moving", em);
}

void plugin_moving_manip_install_proj_loader(plugin_moving_manip_t manip) {
    ui_data_mgr_set_loader(
        ui_ed_mgr_data_mgr(manip->m_ed_mgr),
        ui_data_src_type_moving_plan,
        plugin_moving_plan_proj_load,
        manip);
}

void plugin_moving_manip_install_proj_saver(plugin_moving_manip_t manip) {
    ui_data_mgr_set_saver(
        ui_ed_mgr_data_mgr(manip->m_ed_mgr),
        ui_data_src_type_moving_plan,
        plugin_moving_plan_proj_save,
        plugin_moving_plan_proj_rm,
        manip);
}
