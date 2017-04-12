#include <assert.h>
#include "cpe/pal/pal_strings.h" 
#include "cpe/cfg/cfg.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_mgr.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/model_ed/ui_ed_search.h"
#include "ops.h"

int do_manip_model(gd_app_context_t app, ui_data_mgr_t data_mgr, const char * op_script, error_monitor_t em) {
    ui_ed_mgr_t ed_mgr = NULL;
    ui_ed_search_t ed_search = NULL;
    ui_ed_obj_t obj = NULL;
    cfg_t op_cfg = NULL;
    int rv = -1;
    struct mem_buffer tmp_buff;

    mem_buffer_init(&tmp_buff, NULL);

    ed_mgr = ui_ed_mgr_find_nc(app, NULL);
    assert(ed_mgr);

    op_cfg = cfg_create(NULL);
    if (op_cfg == NULL) {
        CPE_ERROR(em, "create op_cfg fail!");
        goto COMPLETE;
    }

    if (cfg_yaml_read_file(op_cfg, gd_app_vfs_mgr(app), op_script, cfg_replace, NULL) != 0) {
        CPE_ERROR(em, "read op script from %s fail!", op_script);
        goto COMPLETE;
    }

    ed_search = ui_ed_search_create(ed_mgr);
    if (ed_search == NULL) {
        CPE_ERROR(em, "create ed_search fail!");
        goto COMPLETE;
    }

    /* if (ui_ed_search_add_root(ed_search, "Sprite/SamurailResource/Portrait/xiahoudun-4") != 0) { */
    /*     CPE_ERROR(em, "ed_search add root fail!"); */
    /*     goto COMPLETE; */
    /* } */

    if (ui_ed_search_add_obj_type(ed_search, ui_ed_obj_type_frame) != 0) {
        CPE_ERROR(em, "ed_search add type fail!");
        goto COMPLETE;
    }

    if (ui_ed_search_add_obj_type(ed_search, ui_ed_obj_type_frame_img) != 0) {
        CPE_ERROR(em, "ed_search add type fail!");
        goto COMPLETE;
    }

    while((obj = ui_ed_obj_search_next(ed_search))) {
        ui_ed_obj_t img_ref;

        printf("obj: %s\n", ui_ed_obj_dump_with_full_path(&tmp_buff, obj));
        img_ref = ui_ed_obj_using_find(obj, ui_ed_rel_type_use_img);
        if (img_ref) {
            printf("    ref-to: %s\n", ui_ed_obj_dump_with_full_path(&tmp_buff, img_ref));
        }
    }

    rv = 0;

COMPLETE:
    if (ed_search) ui_ed_search_free(ed_search);
    if (op_cfg) cfg_free(op_cfg);

    mem_buffer_clear(&tmp_buff);

    return rv;
}

