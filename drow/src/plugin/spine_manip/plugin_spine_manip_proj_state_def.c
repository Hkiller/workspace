#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/vfs/vfs_stream.h"
#include "cpe/cfg/cfg.h"
#include "cpe/dr/dr_cfg.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model/ui_data_mgr.h"
#include "plugin/spine/plugin_spine_module.h"
#include "plugin/spine/plugin_spine_data_state_def.h"
#include "plugin_spine_manip_i.h"

static int plugin_spine_manip_state_def_do_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_spine_manip_t manip = ctx;
    plugin_spine_data_state_def_t state_def = NULL;
    struct vfs_read_stream fs;
    cfg_t cfg;
    struct cfg_it part_it;
    cfg_t part_cfg;
    LPDRMETA meta_part = plugin_spine_module_meta_data_part(manip->m_spine_module);
    LPDRMETA meta_part_state = plugin_spine_module_meta_data_part_state(manip->m_spine_module);
    LPDRMETA meta_part_transition = plugin_spine_module_meta_data_part_transition(manip->m_spine_module);
    ui_string_table_builder_t string_table = ui_data_src_strings_build_begin(src);

    if (string_table == NULL) {
        CPE_ERROR(em, "load state_def: create string table builder fail");
        return -1;
    }

    cfg = cfg_create(manip->m_alloc);
    if (cfg == NULL) {
        CPE_ERROR(em, "load state_def: create cfg fail!");
        return -1;
    }

    vfs_read_stream_init(&fs, fp);
    if (cfg_yaml_read_with_name(cfg, "root", (read_stream_t)&fs, cfg_replace, em) < 0) {
        CPE_ERROR(em, "load state_def: read from file fail!");
        cfg_free(cfg);
        return -1;
    }

    state_def = plugin_spine_data_state_def_create(manip->m_spine_module, src);

    cfg_it_init(&part_it, cfg_find_cfg(cfg, "root"));
    while((part_cfg = cfg_it_next(&part_it))) {
        plugin_spine_data_part_t part;
        SPINE_PART * part_data;
        struct cfg_it part_state_it;
        cfg_t part_state_cfg;
        struct cfg_it part_transition_it;
        cfg_t part_transition_cfg;
        const char * str_value;

        part = plugin_spine_data_part_create(state_def);
        if (part == NULL) {
            CPE_ERROR(em, "load state_def: create part fail!");
            cfg_free(cfg);
            plugin_spine_data_state_def_free(state_def);
            return -1;
        }

        part_data = plugin_spine_data_part_data(part);
        assert(part_data);

        part_data->name = ui_string_table_builder_msg_alloc(string_table, cfg_name(part_cfg));
        part_data->init_state = ui_string_table_builder_msg_alloc(string_table, cfg_get_string(part_cfg, "init_state", NULL));
        part_data->init_anim = ui_string_table_builder_msg_alloc(string_table, cfg_get_string(part_cfg, "init_anim", NULL));

        cfg_it_init(&part_state_it, cfg_find_cfg(part_cfg, "states"));
        while((part_state_cfg = cfg_it_next(&part_state_it))) {
            plugin_spine_data_part_state_t part_state;
            SPINE_PART_STATE * part_state_data;

            part_state = plugin_spine_data_part_state_create(part);
            if (part_state == NULL) {
                CPE_ERROR(em, "load state_def: create part_state fail!");
                cfg_free(cfg);
                plugin_spine_data_state_def_free(state_def);
                return -1;
            }

            part_state_data = plugin_spine_data_part_state_data(part_state);
            assert(part_state_data);

            part_state_data->name = ui_string_table_builder_msg_alloc(string_table, cfg_get_string(part_state_cfg, "name", NULL));
            part_state_data->anim = ui_string_table_builder_msg_alloc(string_table, cfg_get_string(part_state_cfg, "anim", NULL));
        }

        cfg_it_init(&part_transition_it, cfg_find_cfg(part_cfg, "transitions"));
        while((part_transition_cfg = cfg_it_next(&part_transition_it))) {
            plugin_spine_data_part_transition_t part_transition;
            SPINE_PART_TRANSITION * part_transition_data;

            part_transition = plugin_spine_data_part_transition_create(part);
            if (part_transition == NULL) {
                CPE_ERROR(em, "load state_def: create part_transition fail!");
                cfg_free(cfg);
                plugin_spine_data_state_def_free(state_def);
                return -1;
            }

            part_transition_data = plugin_spine_data_part_transition_data(part_transition);
            assert(part_transition_data);

            part_transition_data->from = ui_string_table_builder_msg_alloc(string_table, cfg_get_string(part_transition_cfg, "from", NULL));
            part_transition_data->to = ui_string_table_builder_msg_alloc(string_table, cfg_get_string(part_transition_cfg, "to", NULL));
            part_transition_data->name = ui_string_table_builder_msg_alloc(string_table, cfg_get_string(part_transition_cfg, "name", NULL));
            part_transition_data->anim = ui_string_table_builder_msg_alloc(string_table, cfg_get_string(part_transition_cfg, "anim", NULL));
        }
    }

    cfg_free(cfg);
    return 0;
}

int plugin_spine_manip_state_def_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "spine-state", plugin_spine_manip_state_def_do_load, ctx, em);
}
