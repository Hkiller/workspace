#include <assert.h>
#include "cpe/pal/pal_strings.h" 
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/model/ui_data_src.h"
#include "render/model/ui_data_sprite.h"
#include "render/model/ui_data_module.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "render/model_ed/ui_ed_obj.h"
#include "render/model_ed/ui_ed_src.h"
#include "render/model_ed/ui_ed_search.h"
#include "ops.h"

static int do_repaire_sprite(gd_app_context_t app, ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr, error_monitor_t em) {
    ui_ed_search_t ed_search = NULL;
    ui_ed_obj_t obj = NULL;
    int rv = 0;
    struct mem_buffer tmp_buff;
    struct mem_buffer sprite_path_buff;
    
    mem_buffer_init(&tmp_buff, NULL);
    mem_buffer_init(&sprite_path_buff, NULL);
    
    ed_search = ui_ed_search_create(ed_mgr);
    if (ed_search == NULL) {
        CPE_ERROR(em, "do_repaire_sprite: create ed_search fail!");
        rv = -1;
        goto COMPLETE;
    }

    if (ui_ed_search_add_obj_type(ed_search, ui_ed_obj_type_frame_img) != 0) {
        CPE_ERROR(em, "do_repaire_sprite: ed_search add type fail!");
        rv = -1;
        goto COMPLETE;
    }

    while((obj = ui_ed_obj_search_next(ed_search))) {
        UI_IMG_REF * img_ref = ui_ed_obj_data(obj);
        ui_data_src_t module_src;
        ui_data_module_t module;
        ui_data_img_block_t img_block;
        UI_IMG_BLOCK const * img_block_data;
        
        if (img_ref->name[0] == 0) continue;

        module_src = ui_data_src_find_by_id(data_mgr, img_ref->module_id);
        if (module_src == NULL) {
            CPE_ERROR(
                em, "do_repaire_sprite: %s: module %u not exist!",
                ui_data_src_path_dump(&sprite_path_buff, ui_ed_src_data(ui_ed_obj_src(obj))),
                img_ref->module_id);
            rv = -1;
            continue;
        }

        module = ui_data_src_product(module_src);
        if (module == NULL) {
            CPE_ERROR(
                em, "do_repaire_sprite: %s: module %s not load!",
                ui_data_src_path_dump(&sprite_path_buff, ui_ed_src_data(ui_ed_obj_src(obj))),
                ui_data_src_path_dump(&tmp_buff, module_src));
            rv = -1;
            continue;
        }

        img_block = ui_data_img_block_find_by_name(module, img_ref->name);
        if (img_block == NULL) {
            CPE_ERROR(
                em, "do_repaire_sprite: %s: module %s img %s not exist!",
                ui_data_src_path_dump(&sprite_path_buff, ui_ed_src_data(ui_ed_obj_src(obj))),
                ui_data_src_path_dump(&tmp_buff, module_src), img_ref->name);
            rv = -1;
            continue;
        }

        img_block_data = ui_data_img_block_data(img_block);
        assert(img_block_data);
        
        if (img_block_data->id != img_ref->img_block_id) {
            img_ref->img_block_id = img_block_data->id;
            ui_ed_src_touch(ui_ed_obj_src(obj));
        }
        
        //printf("obj: %s\n", ui_ed_obj_dump_with_full_path(&tmp_buff, obj));
        /* img_ref = ui_ed_obj_using_find(obj, ui_ed_rel_type_use_img); */
        /* if (img_ref) { */
        /*     printf("    ref-to: %s\n", ui_ed_obj_dump_with_full_path(&tmp_buff, img_ref)); */
        /* } */
    }
    
COMPLETE:
    if (ed_search) ui_ed_search_free(ed_search);

    mem_buffer_clear(&tmp_buff);
    mem_buffer_clear(&sprite_path_buff);

    return rv;
}

int do_repaire_model(gd_app_context_t app, ui_data_mgr_t data_mgr, ui_ed_mgr_t ed_mgr, error_monitor_t em) {
    int rv = 0;

    if (do_repaire_sprite(app, data_mgr, ed_mgr, em) != 0) rv = -1;
    
    return rv;
}


