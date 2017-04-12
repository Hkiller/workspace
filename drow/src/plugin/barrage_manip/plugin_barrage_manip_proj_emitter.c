#include <assert.h>
#include "cpe/vfs/vfs_stream.h"
#include "cpe/cfg/cfg.h"
#include "cpe/dr/dr_cfg.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_src_rw.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/barrage/plugin_barrage_data_emitter.h"
#include "plugin/barrage/plugin_barrage_module.h"
#include "plugin_barrage_manip_i.h"

static int plugin_barrage_data_barrage_do_save(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_barrage_manip_t manip = ctx;
    cfg_t cfg = NULL;
    cfg_t seq_cfg;
    cfg_t emitter_triggers_cfg;
    cfg_t bullet_triggers_cfg;
    plugin_barrage_data_barrage_t barrage = ui_data_src_product(src);
    struct vfs_write_stream fs;
    LPDRMETA barrage_meta = plugin_barrage_module_data_barrage_meta(manip->m_barrage_module);
    LPDRMETA emitter_meta = plugin_barrage_module_data_emitter_meta(manip->m_barrage_module);
    LPDRMETA emitter_trigger_meta = plugin_barrage_module_data_emitter_trigger_meta(manip->m_barrage_module);
    LPDRMETA bullet_trigger_meta = plugin_barrage_module_data_bullet_trigger_meta(manip->m_barrage_module);
    struct plugin_barrage_data_emitter_it emitter_it;
    plugin_barrage_data_emitter_t emitter;
    struct plugin_barrage_data_emitter_trigger_it emitter_trigger_it;
    plugin_barrage_data_emitter_trigger_t emitter_trigger;
    struct plugin_barrage_data_bullet_trigger_it bullet_trigger_it;
    plugin_barrage_data_bullet_trigger_t bullet_trigger;

    cfg = cfg_create(manip->m_alloc);
    if (cfg == NULL) {
        CPE_ERROR(em, "create cfg fail!");
        return -1;
    }

    if (dr_cfg_write(cfg, plugin_barrage_data_barrage_data(barrage), barrage_meta, em) != 0) {
        CPE_ERROR(em, "write barrage data fail!");
        cfg_free(cfg);
        return -1;
    }
    
    seq_cfg = cfg_add_seq(cfg, "emitters", em);
    if (seq_cfg == NULL) {
        CPE_ERROR(em, "create seq fail!");
        cfg_free(cfg);
        return -1;
    }

    plugin_barrage_data_barrage_emitters(&emitter_it, barrage);
    while((emitter = plugin_barrage_data_emitter_it_next(&emitter_it))) {
        cfg_t emitter_cfg = cfg_seq_add_struct(seq_cfg);
        if (emitter_cfg == NULL) {
            CPE_ERROR(em, "add struct to seq fail!");
            cfg_free(cfg);
            return -1;
        }
        
        if (dr_cfg_write(emitter_cfg, plugin_barrage_data_emitter_data(emitter), emitter_meta, em) != 0) {
            CPE_ERROR(em, "write data fail!");
            cfg_free(cfg);
            return -1;
        }

        emitter_triggers_cfg = cfg_add_seq(emitter_cfg, "emitter.triggers", em);
        if (emitter_triggers_cfg == NULL) {
            CPE_ERROR(em, "create emitter.triggers fail!");
            cfg_free(cfg);
            return -1;
        }
        plugin_barrage_data_emitter_triggers(&emitter_trigger_it, emitter);
        while((emitter_trigger = plugin_barrage_data_emitter_trigger_it_next(&emitter_trigger_it))) {
            cfg_t trigger_cfg = cfg_seq_add_struct(emitter_triggers_cfg);
            if (trigger_cfg == NULL) {
                CPE_ERROR(em, "write emitter trigger data fail!");
                cfg_free(cfg);
                return -1;
            }

            if (dr_cfg_write(trigger_cfg, plugin_barrage_data_emitter_trigger_data(emitter_trigger), emitter_trigger_meta, em) != 0) {
                CPE_ERROR(em, "write emitter trigger data fail!");
                cfg_free(cfg);
                return -1;
            }
        }

        bullet_triggers_cfg = cfg_add_seq(emitter_cfg, "bullet.triggers", em);
        if (bullet_triggers_cfg == NULL) {
            CPE_ERROR(em, "create bullet.triggers fail!");
            cfg_free(cfg);
            return -1;
        }
        plugin_barrage_data_bullet_triggers(&bullet_trigger_it, emitter);
        while((bullet_trigger = plugin_barrage_data_bullet_trigger_it_next(&bullet_trigger_it))) {
            cfg_t trigger_cfg = cfg_seq_add_struct(bullet_triggers_cfg);
            if (trigger_cfg == NULL) {
                CPE_ERROR(em, "write bullet trigger data fail!");
                cfg_free(cfg);
                return -1;
            }

            if (dr_cfg_write(trigger_cfg, plugin_barrage_data_bullet_trigger_data(bullet_trigger), bullet_trigger_meta, em) != 0) {
                CPE_ERROR(em, "write bullet trigger data fail!");
                cfg_free(cfg);
                return -1;
            }
        }
    }
    
    vfs_write_stream_init(&fs, fp);
    if (cfg_yaml_write((write_stream_t)&fs, cfg, em) != 0) {
        CPE_ERROR(em, "cfg write fail!");
        cfg_free(cfg);
        return -1;
    }

    cfg_free(cfg);
    return 0;
}

static int plugin_barrage_data_barrage_do_load(void * ctx, ui_data_src_t src, vfs_file_t fp, error_monitor_t em) {
    plugin_barrage_manip_t manip = ctx;
    plugin_barrage_data_barrage_t barrage = NULL;
    struct vfs_read_stream fs;
    cfg_t cfg;
    struct cfg_it emitter_it;
    cfg_t emitter_cfg;
    LPDRMETA barrage_meta = plugin_barrage_module_data_barrage_meta(manip->m_barrage_module);
    LPDRMETA emitter_meta = plugin_barrage_module_data_emitter_meta(manip->m_barrage_module);
    LPDRMETA emitter_trigger_meta = plugin_barrage_module_data_emitter_trigger_meta(manip->m_barrage_module);
    LPDRMETA bullet_trigger_meta = plugin_barrage_module_data_bullet_trigger_meta(manip->m_barrage_module);

    cfg = cfg_create(manip->m_alloc);
    if (cfg == NULL) {
        CPE_ERROR(em, "load barrage: create cfg fail!");
        return -1;
    }

    vfs_read_stream_init(&fs, fp);
    if (cfg_yaml_read(cfg, (read_stream_t)&fs, cfg_replace, em) < 0) {
        CPE_ERROR(em, "load barrage: read from file fail!");
        cfg_free(cfg);
        return -1;
    }

    barrage = plugin_barrage_data_barrage_create(manip->m_barrage_module, src);

    if (dr_cfg_read(
            plugin_barrage_data_barrage_data(barrage),
            sizeof(*plugin_barrage_data_barrage_data(barrage)),
            cfg, barrage_meta, 0, NULL) < 0)
    {
        CPE_ERROR(em, "load barrage: load barrage data fail!");
        cfg_free(cfg);
        plugin_barrage_data_barrage_free(barrage);
        return -1;
    }

    cfg_it_init(&emitter_it, cfg_find_cfg(cfg, "emitters"));
    while((emitter_cfg = cfg_it_next(&emitter_it))) {
        plugin_barrage_data_emitter_t emitter = plugin_barrage_data_emitter_create(barrage);
        struct cfg_it trigger_it;
        cfg_t trigger_cfg;

        if (dr_cfg_read(
                plugin_barrage_data_emitter_data(emitter),
                sizeof(*plugin_barrage_data_emitter_data(emitter)),
                emitter_cfg, emitter_meta, 0, NULL) < 0)
        {
            CPE_ERROR(em, "load barrage: load emitter data fail!");
            cfg_free(cfg);
            plugin_barrage_data_barrage_free(barrage);
            return -1;
        }
        
        cfg_it_init(&trigger_it, cfg_find_cfg(emitter_cfg, "emitter.triggers"));
        while((trigger_cfg = cfg_it_next(&trigger_it))) {
            plugin_barrage_data_emitter_trigger_t trigger;
            BARRAGE_EMITTER_EMITTER_TRIGGER_INFO * trigger_data;

            trigger = plugin_barrage_data_emitter_trigger_create(emitter);
            if (trigger == NULL) {
                CPE_ERROR(em, "load barrage: create emitter trigger fail!");
                cfg_free(cfg);
                plugin_barrage_data_barrage_free(barrage);
                return -1;
            }

            trigger_data = plugin_barrage_data_emitter_trigger_data(trigger);
            assert(trigger_data);

            if (dr_cfg_read(trigger_data, sizeof(*trigger_data), trigger_cfg, emitter_trigger_meta, 0, em) < 0) {
                CPE_ERROR(em, "load barrage: load barrage trigger data fail!");
                cfg_free(cfg);
                plugin_barrage_data_barrage_free(barrage);
                return -1;
            }

            if (plugin_barrage_data_emitter_trigger_update(trigger) != 0) {
                CPE_ERROR(em, "load barrage: update emitter trigger fail!");
                cfg_free(cfg);
                plugin_barrage_data_barrage_free(barrage);
                return -1;
            }
        }

        cfg_it_init(&trigger_it, cfg_find_cfg(emitter_cfg, "bullet.triggers"));
        while((trigger_cfg = cfg_it_next(&trigger_it))) {
            plugin_barrage_data_bullet_trigger_t trigger;
            BARRAGE_EMITTER_BULLET_TRIGGER_INFO * trigger_data;

            trigger = plugin_barrage_data_bullet_trigger_create(emitter);
            if (trigger == NULL) {
                CPE_ERROR(em, "load barrage: create bullet trigger fail!");
                cfg_free(cfg);
                plugin_barrage_data_barrage_free(barrage);
                return -1;
            }

            trigger_data = plugin_barrage_data_bullet_trigger_data(trigger);
            assert(trigger_data);

            if (dr_cfg_read(trigger_data, sizeof(*trigger_data), trigger_cfg, bullet_trigger_meta, 0, em) < 0) {
                CPE_ERROR(em, "load barrage: load barrage trigger data fail!");
                cfg_free(cfg);
                plugin_barrage_data_barrage_free(barrage);
                return -1;
            }

            if (plugin_barrage_data_bullet_trigger_update(trigger) != 0) {
                CPE_ERROR(em, "load barrage: update emitter trigger fail!");
                cfg_free(cfg);
                plugin_barrage_data_barrage_free(barrage);
                return -1;
            }
        }
    }
    
    cfg_free(cfg);
    return 0;
}

int plugin_barrage_barrage_proj_save(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_save_to_file(src, root, "barrage", plugin_barrage_data_barrage_do_save, ctx, em);
}

int plugin_barrage_barrage_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    return ui_data_src_load_from_file(src, "barrage", plugin_barrage_data_barrage_do_load, ctx, em);
}

int plugin_barrage_barrage_proj_rm(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, const char * root, error_monitor_t em) {
    return ui_data_src_remove_file(src, root, "barrage", em);
}
