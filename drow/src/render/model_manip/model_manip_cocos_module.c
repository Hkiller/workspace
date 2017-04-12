#include <assert.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/plist/plist_cfg.h"
#include "render/model/ui_data_module.h"
#include "render/model/ui_data_src.h"
#include "render/model_manip/model_id_allocrator.h"
#include "model_manip_cocos_utils.h"

int ui_model_import_cocos_module(ui_ed_mgr_t ed_mgr, const char * module_path, const char * plist, const char * pic, error_monitor_t em) {
    cfg_t root_cfg;
    ui_data_module_t module;
    ui_ed_src_t module_src;
    struct cfg_it frame_it;
    cfg_t frame_cfg;
    ui_model_id_allocrator_t id_alloc;
    
    /*读取配置文件 */
    root_cfg = plist_cfg_load_dict_from_file(plist, em);
    if (root_cfg == NULL) {
        CPE_ERROR(em, "check create module at %s: read plist %s fail!", module_path, plist);
        return -1;
    }

    module_src = ui_ed_src_check_create(ed_mgr, module_path, ui_data_src_type_module);
    if (module_src == NULL) {
        CPE_ERROR(em, "check create module at %s fail!", module_path);
        cfg_free(root_cfg);
        return -1;
    }

    module = ui_data_src_product(ui_ed_src_data(module_src));
    assert(module);

    /*分配ID */
    id_alloc = ui_model_id_allocrator_create(NULL);
    if (id_alloc == NULL) {
        CPE_ERROR(em, "create model id alloc fail!");
        cfg_free(root_cfg);
        return -1;
    }

    /*    首先放置文件名分配的ID */
    cfg_it_init(&frame_it, cfg_find_cfg(root_cfg, "frames"));
    while((frame_cfg = cfg_it_next(&frame_it))) {
        const char * name = cfg_name(frame_cfg);
        char * end_p;
        int img_id;

        img_id = strtol(name, &end_p, 10);
        if (end_p && (*end_p == '_' || (*end_p == '.' && strchr(end_p + 1, '.') == NULL))) {
            if (ui_model_id_allocrator_remove(id_alloc, img_id) != 0) {
                CPE_ERROR(em, "id %u [file name id] (name=%s) duplicate!", img_id, name);
                continue;
            }

            cfg_struct_add_uint32(frame_cfg, "__id", img_id, cfg_replace);
            continue;
        }
    }

    /*   其次根据已经分配好的ID进行放置 */
    cfg_it_init(&frame_it, cfg_find_cfg(root_cfg, "frames"));
    while((frame_cfg = cfg_it_next(&frame_it))) {
        char name[64];
        char * p;
        ui_data_img_block_t img_block;
        UI_IMG_BLOCK * img_block_data;
        
        if (cfg_find_cfg(frame_cfg, "__id")) continue;

        cpe_str_dup(name, sizeof(name), cfg_name(frame_cfg));
        if ((p = strrchr(name, '.'))) *p = 0;
        
        img_block = ui_data_img_block_find_by_name(module, name);
        if (img_block == NULL) continue;
        img_block_data = ui_data_img_block_data(img_block);

        if (ui_model_id_allocrator_remove(id_alloc, img_block_data->id) != 0) {
            CPE_ERROR(em, "id %u [modlue old id] (name=%s) duplicate!", img_block_data->id, name);
            continue;
        }
        
        cfg_struct_add_uint32(frame_cfg, "__id", img_block_data->id, cfg_replace);
    }

    /*    最后给所以没有生成的ID进行分配 */
    cfg_it_init(&frame_it, cfg_find_cfg(root_cfg, "frames"));
    while((frame_cfg = cfg_it_next(&frame_it))) {
        uint32_t img_id;
        
        if (cfg_find_cfg(frame_cfg, "__id")) continue;

        if (ui_model_id_allocrator_alloc(id_alloc, &img_id) != 0) {
            CPE_ERROR(em, "allock (name=%s) fail!", cfg_name(frame_cfg));
            continue;
        }
        
        cfg_struct_add_uint32(frame_cfg, "__id", img_id, cfg_replace);
    }

    /*创建图片 */
    ui_ed_obj_remove_childs(ui_ed_src_root_obj(module_src));
    ui_ed_src_strings_clear(module_src);
    
    cfg_it_init(&frame_it, cfg_find_cfg(root_cfg, "frames"));
    while((frame_cfg = cfg_it_next(&frame_it))) {
        uint32_t img_id = cfg_get_uint32(frame_cfg, "__id", (uint32_t)-1);
        
        if (cocos_build_img_block(frame_cfg, ui_ed_src_root_obj(module_src), img_id, pic, em) == 0) {
            cfg_free(root_cfg);
            ui_model_id_allocrator_free(id_alloc);
            ui_ed_src_delete(module_src);
            return -1;
        }
    }

    cfg_free(root_cfg);
    ui_model_id_allocrator_free(id_alloc);

    return 0;
}
