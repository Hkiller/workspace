#include "cpe/utils/string_utils.h"
#include "ui_app_manip_src_i.h"

static int ui_app_manip_collect_src_from_render_env(
    ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t render_env_cfg, error_monitor_t em)
{
    struct cfg_it childs;
    cfg_t child_cfg;
    int rv = 0;
    
    cfg_it_init(&childs, cfg_find_cfg(render_env_cfg, "layers"));
    while((child_cfg = cfg_it_next(&childs))) {
        struct cfg_it anims;
        cfg_t anim_cfg;

        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) continue;
        
        cfg_it_init(&anims, cfg_find_cfg(child_cfg, "init"));
        while((anim_cfg = cfg_it_next(&anims))) {
            const char * res = cfg_as_string(anim_cfg, NULL);
            if (res == NULL) continue;
            
            if (res[0] == '[') {
                const char * args_end = cpe_str_char_not_in_pair(res + 1, ']', "{[(", ")]}");
                if (args_end == NULL) {
                    rv = -1;
                    continue;
                }

                res = args_end + 1;
            }

            if (ui_app_manip_collect_src_by_res(src_group, cache_group, res, em) != 0) {
                rv = -1;
            }
        }
    }
    
    return rv;
}

int ui_app_manip_collect_src_from_scene(ui_data_src_group_t src_group, ui_cache_group_t cache_group, cfg_t scene_cfg, error_monitor_t em) {
    struct cfg_it childs;
    cfg_t child_cfg;
    const char * res_type;
    int rv = 0;
    
    /*world resource*/
    cfg_it_init(&childs, cfg_find_cfg(scene_cfg, "resources"));
    while((child_cfg = cfg_it_next(&childs))) {
        
        child_cfg = cfg_child_only(child_cfg);
        if (child_cfg == NULL) continue;

        res_type = cfg_name(child_cfg);

        if (strcmp(res_type, "ChipmunkEnv") == 0) {
        }
        else if (strcmp(res_type, "RenderEnv") == 0) {
            if (ui_app_manip_collect_src_from_render_env(src_group, cache_group, child_cfg, em) != 0) {
                rv = -1;
            }
        }
    }

    /*protos */
    cfg_it_init(&childs, cfg_find_cfg(scene_cfg, "protos"));
    while((child_cfg = cfg_it_next(&childs))) {
        if (ui_app_manip_collect_src_from_entity(src_group, cache_group, child_cfg, em) != 0) {
            rv = -1;
        }
    }

    /*entities */
    cfg_it_init(&childs, cfg_find_cfg(scene_cfg, "entities"));
    while((child_cfg = cfg_it_next(&childs))) {
        if (ui_app_manip_collect_src_from_entity(src_group, cache_group, child_cfg, em) != 0) {
            rv = -1;
        }
    }
    
    return rv;
}
