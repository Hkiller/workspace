#include <assert.h>
#include "argtable2.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/memory_debug.h"
#include "cpe/utils/string_utils.h"
#include "gd/app/app_module.h"
#include "render/cache/ui_cache_manager.h"
#include "render/model_proj/ui_proj_loader.h"
#include "render/model_bin/ui_bin_loader.h"
#include "render/model_manip/model_manip_basic.h"
#include "plugin/spine/plugin_spine_module.h"
#include "plugin/spine_manip/plugin_spine_manip.h"
#include "plugin/spine_manip/plugin_spine_manip_import.h"
#include "plugin/particle/plugin_particle_module.h"
#include "plugin/particle_manip/plugin_particle_manip.h"
#include "plugin/barrage/plugin_barrage_module.h"
#include "plugin/barrage_manip/plugin_barrage_manip.h"
#include "plugin/barrage_manip/plugin_barrage_crazystorm.h"
#include "plugin/moving/plugin_moving_module.h"
#include "plugin/moving_manip/plugin_moving_manip.h"
#include "plugin/package/plugin_package_module.h"
#include "plugin/package_manip/plugin_package_manip.h"
#include "plugin/chipmunk/plugin_chipmunk_module.h"
#include "plugin/chipmunk_manip/plugin_chipmunk_manip.h"
#include "plugin/chipmunk_manip/plugin_chipmunk_basic.h"
#include "plugin/chipmunk_manip/plugin_chipmunk_rube.h"
#include "plugin/chipmunk_manip/plugin_chipmunk_chipmunk.h"
#include "plugin/tiledmap/plugin_tiledmap_module.h"
#include "plugin/tiledmap_manip/plugin_tiledmap_manip.h"
#include "plugin/tiledmap_manip/plugin_tiledmap_basic.h"
#include "plugin/scrollmap_manip/plugin_scrollmap_manip.h"
#include "plugin/swf/plugin_swf_module.h"
#include "plugin/mask/plugin_mask_module.h"
#include "plugin/mask_manip/plugin_mask_manip.h"
#include "ui/sprite_manip/ui_sprite_manip.h"
#include "ops.h"

static int load_model(gd_app_context_t app, ui_data_mgr_t data_mgr, const char * model, const char * model_format, int load_product, error_monitor_t em);
static int set_model_saver(gd_app_context_t app, ui_data_mgr_t data_mgr, const char * model_format, error_monitor_t em);

int main(int argc, char * argv[]) {
    /*convert*/
    struct arg_rex  * convert           =     arg_rex1(NULL, NULL, "convert", NULL, 0, NULL);
    struct arg_str  * convert_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * convert_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * convert_language  =     arg_strn(NULL, "language", "", 0, 100, "support language");
    struct arg_str  * convert_to        =     arg_str0(NULL, "to", NULL, "output file/dir");
    struct arg_str  * convert_to_format =     arg_str1(NULL, "to-format", "(yml|bin)", "output format");
    struct arg_int  * convert_texture_limit_width =     arg_int0(NULL, "texture-limit-width", NULL, "max pack texture width");
    struct arg_int  * convert_texture_limit_height =     arg_int0(NULL, "texture-limit-height", NULL, "max pack texture height");
    struct arg_str  * convert_texture_compress =     arg_str0(NULL, "texture-compress", "(tinypng)", "texture compress type");
    struct arg_str  * convert_accounts_tinypng =     arg_str0(NULL, "accounts-tinypng", NULL, "tinypng accounts");
    struct arg_str  * convert_pack           =     arg_str0(NULL, "pack-way", "(spack|native)", "pack way");    
    struct arg_lit  * convert_full           =     arg_lit0(NULL, "full", "export full package");
    struct arg_end  * convert_end       =     arg_end(20);
    void* convert_argtable[] = { convert, convert_model, convert_model_format, convert_language,
                                 convert_to, convert_to_format,
                                 convert_texture_limit_width, convert_texture_limit_height,
                                 convert_texture_compress,
                                 convert_accounts_tinypng,
                                 convert_pack,
                                 convert_full,
                                 convert_end };
    int convert_nerrors;

    /*manip*/
    struct arg_rex  * manip           =     arg_rex1(NULL, NULL, "manip", NULL, 0, NULL);
    struct arg_str  * manip_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * manip_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * manip_op      =     arg_str1(NULL, "op", NULL, "operation script");
    struct arg_end  * manip_end       =     arg_end(20);
    void* manip_argtable[] = { manip, manip_model, manip_model_format, manip_op, manip_end };
    int manip_nerrors;

    /*repaire */
    struct arg_rex  * repaire           =     arg_rex1(NULL, NULL, "repaire", NULL, 0, NULL);
    struct arg_str  * repaire_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * repaire_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_end  * repaire_end       =     arg_end(20);
    void* repaire_argtable[] = { repaire, repaire_model, repaire_model_format, repaire_end };
    int repaire_nerrors;
    
    /*cocos_module_import*/
    struct arg_rex  * cocos_module           =     arg_rex1(NULL, NULL, "cocos-module-import", NULL, 0, NULL);
    struct arg_str  * cocos_module_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * cocos_module_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * cocos_module_to_module      =     arg_str1(NULL, "to-module", NULL, "import to module path");
    struct arg_str  * cocos_module_import_plist      =     arg_str1(NULL, "plist", NULL, "plist path");
    struct arg_str  * cocos_module_import_pic      =     arg_str1(NULL, "pic", NULL, "pic path");
    struct arg_end  * cocos_module_end       =     arg_end(20);
    void* cocos_module_argtable[] = { cocos_module, cocos_module_model, cocos_module_model_format, cocos_module_to_module, cocos_module_import_plist, cocos_module_import_pic, cocos_module_end };
    int cocos_module_nerrors;

    /*cocos_effect_import*/
    struct arg_rex  * cocos_effect_import           =     arg_rex1(NULL, NULL, "cocos-effect-import", NULL, 0, NULL);
    struct arg_str  * cocos_effect_import_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * cocos_effect_import_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * cocos_effect_import_to_effect      =     arg_str1(NULL, "to-effect", NULL, "import to effect path");
    struct arg_str  * cocos_effect_import_to_module      =     arg_str1(NULL, "to-module", NULL, "import to module path");
    struct arg_str  * cocos_effect_import_plist      =     arg_str1(NULL, "plist", NULL, "plist path");
    struct arg_str  * cocos_effect_import_pic      =     arg_str1(NULL, "pic", NULL, "pic path");
    struct arg_int  * cocos_effect_import_frame_duration      =     arg_int1(NULL, "frame-duration", NULL, "frame duration");
    struct arg_str  * cocos_effect_import_frame_position      =     arg_str1(NULL, "frame-position", "[center|center-left|center-right|bottom-center|bottom-left|bottom-right|top-center|top-left|top-right]", "frame position");
    struct arg_str  * cocos_effect_import_frame_order      =     arg_str1(NULL, "frame-order", "[native|postfix]", "frame order");
    struct arg_end  * cocos_effect_import_end       =     arg_end(20);
    void* cocos_effect_import_argtable[] = { 
        cocos_effect_import, cocos_effect_import_model, cocos_effect_import_model_format,
        cocos_effect_import_to_effect, cocos_effect_import_to_module, 
        cocos_effect_import_plist, cocos_effect_import_pic,
        cocos_effect_import_frame_duration, cocos_effect_import_frame_position, cocos_effect_import_frame_order,
        cocos_effect_import_end
    };
    int cocos_effect_import_nerrors;

    /*cocos_particle_import*/
    struct arg_rex  * cocos_particle_import           =     arg_rex1(NULL, NULL, "cocos-particle-import", NULL, 0, NULL);
    struct arg_str  * cocos_particle_import_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * cocos_particle_import_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * cocos_particle_import_to_particle      =     arg_str1(NULL, "to-particle", NULL, "import to particle path");
    struct arg_str  * cocos_particle_import_plist      =     arg_str1(NULL, "plist", NULL, "plist path");
    struct arg_str  * cocos_particle_import_pic      =     arg_str1(NULL, "pic", NULL, "pic path");
    struct arg_end  * cocos_particle_import_end       =     arg_end(20);
    void* cocos_particle_import_argtable[] = { 
        cocos_particle_import, cocos_particle_import_model, cocos_particle_import_model_format,
        cocos_particle_import_to_particle,
        cocos_particle_import_plist, cocos_particle_import_pic, cocos_particle_import_end
    };
    int cocos_particle_import_nerrors;

    /*spine_anim_import*/
    struct arg_rex  * spine_anim_import           =     arg_rex1(NULL, NULL, "spine-anim-import", NULL, 0, NULL);
    struct arg_str  * spine_anim_import_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * spine_anim_import_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * spine_anim_import_to_module      =     arg_str1(NULL, "output-module", NULL, "converto to module base name");
    struct arg_str  * spine_anim_import_atlas      =     arg_str1(NULL, "input-atlas", NULL, "atlas file");
    struct arg_str  * spine_anim_import_pic      =     arg_str1(NULL, "pic", NULL, "pic path");
    struct arg_end  * spine_anim_import_end       =     arg_end(20);
    void* spine_anim_import_argtable[] = { 
        spine_anim_import, spine_anim_import_model, spine_anim_import_model_format,
        spine_anim_import_to_module,
        spine_anim_import_atlas, spine_anim_import_pic, spine_anim_import_end
    };
    int spine_anim_import_nerrors;

    /*crazystorm_emitter_import*/
    struct arg_rex  * crazystorm_emitter_import           =     arg_rex1(NULL, NULL, "crazystorm-emitter-import", NULL, 0, NULL);
    struct arg_str  * crazystorm_emitter_import_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * crazystorm_emitter_import_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * crazystorm_emitter_import_to_emitter      =     arg_str1(NULL, "output", NULL, "converto to emitter");
    struct arg_str  * crazystorm_emitter_import_from_emitter      =     arg_str1(NULL, "input", NULL, "emitter path");
    struct arg_str  * crazystorm_emitter_import_use_bullets      =     arg_str1(NULL, "bullets", NULL, "bullets path");
    struct arg_end  * crazystorm_emitter_import_end       =     arg_end(20);
    void* crazystorm_emitter_import_argtable[] = { 
        crazystorm_emitter_import, crazystorm_emitter_import_model, crazystorm_emitter_import_model_format,
        crazystorm_emitter_import_to_emitter, crazystorm_emitter_import_from_emitter, crazystorm_emitter_import_use_bullets,
        crazystorm_emitter_import_end
    };
    int crazystorm_emitter_import_nerrors;

    /*chipmunk_import_from_sprite*/
    struct arg_rex  * chipmunk_import_from_sprite           =     arg_rex1(NULL, NULL, "chipmunk-import-from-sprite", NULL, 0, NULL);
    struct arg_str  * chipmunk_import_from_sprite_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * chipmunk_import_from_sprite_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * chipmunk_import_from_sprite_to_chipmunk      =     arg_str1(NULL, "output", NULL, "output path");
    struct arg_str  * chipmunk_import_from_sprite_from_sprite      =     arg_str1(NULL, "input", NULL, "input path");
    struct arg_end  * chipmunk_import_from_sprite_end       =     arg_end(20);
    void* chipmunk_import_from_sprite_argtable[] = { 
        chipmunk_import_from_sprite, chipmunk_import_from_sprite_model, chipmunk_import_from_sprite_model_format,
        chipmunk_import_from_sprite_to_chipmunk, chipmunk_import_from_sprite_from_sprite,
        chipmunk_import_from_sprite_end
    };
    int chipmunk_import_from_sprite_nerrors;
    
    /*chipmunk_import_from_rube*/
    struct arg_rex  * chipmunk_import_from_rube           =     arg_rex1(NULL, NULL, "chipmunk-import-from-rube", NULL, 0, NULL);
    struct arg_str  * chipmunk_import_from_rube_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * chipmunk_import_from_rube_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * chipmunk_import_from_rube_to_chipmunk      =     arg_str1(NULL, "output", NULL, "output path");
    struct arg_str  * chipmunk_import_from_rube_from_rube      =     arg_str1(NULL, "input", NULL, "input path");
    struct arg_dbl  * chipmunk_import_from_rube_ptm      =     arg_dbl1(NULL, "ptm", NULL, "ptm");
    struct arg_end  * chipmunk_import_from_rube_end       =     arg_end(20);
    void* chipmunk_import_from_rube_argtable[] = { 
        chipmunk_import_from_rube, chipmunk_import_from_rube_model, chipmunk_import_from_rube_model_format,
        chipmunk_import_from_rube_to_chipmunk, chipmunk_import_from_rube_from_rube, chipmunk_import_from_rube_ptm,
        chipmunk_import_from_rube_end
    };
    int chipmunk_import_from_rube_nerrors;

    /*chipmunk_import_from_chipmunk*/
    struct arg_rex  * chipmunk_import_from_chipmunk           =     arg_rex1(NULL, NULL, "chipmunk-import-from-chipmunk", NULL, 0, NULL);
    struct arg_str  * chipmunk_import_from_chipmunk_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * chipmunk_import_from_chipmunk_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * chipmunk_import_from_chipmunk_to_chipmunk      =     arg_str1(NULL, "output", NULL, "output path");
    struct arg_str  * chipmunk_import_from_chipmunk_from_chipmunk      =     arg_str1(NULL, "input", NULL, "input path");
    struct arg_str  * chipmunk_import_from_chipmunk_adj_base      =     arg_str0(NULL, "adj-base", NULL, "input png fil");
    struct arg_dbl  * chipmunk_import_from_chipmunk_ptm      =     arg_dbl1(NULL, "ptm", NULL, "ptm");
    struct arg_end  * chipmunk_import_from_chipmunk_end       =     arg_end(20);
    void* chipmunk_import_from_chipmunk_argtable[] = { 
        chipmunk_import_from_chipmunk, chipmunk_import_from_chipmunk_model, chipmunk_import_from_chipmunk_model_format,
        chipmunk_import_from_chipmunk_to_chipmunk, chipmunk_import_from_chipmunk_from_chipmunk,
        chipmunk_import_from_chipmunk_adj_base, chipmunk_import_from_chipmunk_ptm,
        chipmunk_import_from_chipmunk_end
    };
    int chipmunk_import_from_chipmunk_nerrors;
    
    /*tiledmap_scene_import*/
    struct arg_rex  * tiledmap_scene_import           =     arg_rex1(NULL, NULL, "tiledmap-scene-import", NULL, 0, NULL);
    struct arg_str  * tiledmap_scene_import_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * tiledmap_scene_import_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * tiledmap_scene_import_project    =     arg_str1(NULL, "project", NULL, "project path");
    struct arg_end  * tiledmap_scene_import_end       =     arg_end(20);
    void* tiledmap_scene_import_argtable[] = { 
        tiledmap_scene_import, tiledmap_scene_import_model, tiledmap_scene_import_model_format,
        tiledmap_scene_import_project, tiledmap_scene_import_end
    };
    int tiledmap_scene_import_nerrors;

    /*tiledmap_scene_export*/
    struct arg_rex  * tiledmap_scene_export           =     arg_rex1(NULL, NULL, "tiledmap-scene-export", NULL, 0, NULL);
    struct arg_str  * tiledmap_scene_export_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * tiledmap_scene_export_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * tiledmap_scene_export_project    =     arg_str1(NULL, "project", NULL, "project path");
    struct arg_end  * tiledmap_scene_export_end       =     arg_end(20);
    void* tiledmap_scene_export_argtable[] = { 
        tiledmap_scene_export, tiledmap_scene_export_model, tiledmap_scene_export_model_format,
        tiledmap_scene_export_project, tiledmap_scene_export_end
    };
    int tiledmap_scene_export_nerrors;

    /*tiledmap_scene_to_pic*/
    struct arg_rex  * tiledmap_scene_to_pic           =     arg_rex1(NULL, NULL, "tiledmap-scene-to-pic", NULL, 0, NULL);
    struct arg_str  * tiledmap_scene_to_pic_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * tiledmap_scene_to_pic_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * tiledmap_scene_to_pic_input      =     arg_str1(NULL, "input", NULL, "input scene path");
    struct arg_str  * tiledmap_scene_to_pic_layer      =     arg_strn(NULL, "layer", NULL, 0, 200, "layer name");
    struct arg_str  * tiledmap_scene_to_pic_output      =     arg_str1(NULL, "output", NULL, "output file");
    struct arg_end  * tiledmap_scene_to_pic_end       =     arg_end(20);
    void* tiledmap_scene_to_pic_argtable[] = { 
        tiledmap_scene_to_pic, tiledmap_scene_to_pic_model, tiledmap_scene_to_pic_model_format,
        tiledmap_scene_to_pic_input, tiledmap_scene_to_pic_layer, tiledmap_scene_to_pic_output,
        tiledmap_scene_to_pic_end
    };
    int tiledmap_scene_to_pic_nerrors;

    /*module_import*/
    struct arg_rex  * module_import           =     arg_rex1(NULL, NULL, "module-import", NULL, 0, NULL);
    struct arg_str  * module_import_model_format =     arg_str1(NULL, "format", "(yml|bin)", "model format");
    struct arg_str  * module_import_model      =     arg_str1(NULL, "model", NULL, "model file/dir");
    struct arg_str  * module_import_to_module      =     arg_str1(NULL, "to-module", NULL, "import to module path");
    struct arg_str  * module_import_base      =     arg_str0(NULL, "import-base", NULL, "import path base");
    struct arg_str  * module_import_src      =     arg_strn(NULL, "import", NULL, 0, 200, "source pic name");
    struct arg_end  * module_import_end       =     arg_end(20);
    void* module_import_argtable[] = { module_import, module_import_model, module_import_model_format, module_import_to_module,
                                       module_import_base, module_import_src, module_import_end };
    int module_import_nerrors;
    
    /*common*/
    struct arg_lit * common_help = arg_lit0(NULL,"help",    "print this help and exit");
    struct arg_end * common_end     = arg_end(20);
    void * common_argtable[] = { common_help, common_end };
    int common_nerrors;

    struct error_monitor em_buf;
    error_monitor_t em;
    int rv = -1;

    mem_allocrator_t alloc = NULL;
    gd_app_context_t app = NULL;
    ui_data_mgr_t data_mgr = NULL;
    ui_cache_manager_t cache = NULL;
    ui_ed_mgr_t ed_mgr = NULL;
    uint8_t save_ed_mgr = 0;
    
    struct gd_app_module_def load_modules[] = {
        { "ui_cache_manager", NULL, NULL, NULL, NULL }
        , { "ui_data_mgr", NULL, NULL, NULL, NULL }
        , { "ui_ed_mgr", NULL, NULL, NULL, NULL }
        , { "plugin_package_module", NULL, NULL, NULL, NULL }
        , { "plugin_package_manip", NULL, NULL, NULL, NULL }
        , { "plugin_particle_module", NULL, NULL, NULL, NULL }
        , { "plugin_particle_manip", NULL, NULL, NULL, NULL }
        , { "plugin_spine_module", NULL, NULL, NULL, NULL }
        , { "plugin_spine_manip", NULL, NULL, NULL, NULL }
        , { "plugin_swf_module", NULL, NULL, NULL, NULL }
        , { "plugin_barrage_module", NULL, NULL, NULL, NULL }
        , { "plugin_barrage_manip", NULL, NULL, NULL, NULL }
        , { "plugin_moving_module", NULL, NULL, NULL, NULL }
        , { "plugin_moving_manip", NULL, NULL, NULL, NULL }
        , { "plugin_chipmunk_module", NULL, NULL, NULL, NULL }
        , { "plugin_chipmunk_manip", NULL, NULL, NULL, NULL }
        , { "plugin_tiledmap_module", NULL, NULL, NULL, NULL }
        , { "plugin_tiledmap_manip", NULL, NULL, NULL, NULL }
        , { "plugin_scrollmap_module", NULL, NULL, NULL, NULL }
        , { "plugin_scrollmap_manip", NULL, NULL, NULL, NULL }
        , { "plugin_mask_module", NULL, NULL, NULL, NULL }
        , { "plugin_mask_manip", NULL, NULL, NULL, NULL }
        , { "plugin_pack_module", NULL, NULL, NULL, NULL }
        , { "ui_sprite_manip", NULL, NULL, NULL, NULL }
    };

    cpe_error_monitor_init(&em_buf, cpe_error_log_to_consol, 0);
    em = &em_buf;

    convert_nerrors = arg_parse(argc, argv, convert_argtable);
    manip_nerrors = arg_parse(argc, argv, manip_argtable);
    repaire_nerrors = arg_parse(argc, argv, repaire_argtable);
    cocos_module_nerrors = arg_parse(argc, argv, cocos_module_argtable);
    cocos_effect_import_nerrors = arg_parse(argc, argv, cocos_effect_import_argtable);
    cocos_particle_import_nerrors = arg_parse(argc, argv, cocos_particle_import_argtable);
    spine_anim_import_nerrors = arg_parse(argc, argv, spine_anim_import_argtable);
    crazystorm_emitter_import_nerrors = arg_parse(argc, argv, crazystorm_emitter_import_argtable);
    chipmunk_import_from_sprite_nerrors = arg_parse(argc, argv, chipmunk_import_from_sprite_argtable);
    chipmunk_import_from_rube_nerrors = arg_parse(argc, argv, chipmunk_import_from_rube_argtable);
    chipmunk_import_from_chipmunk_nerrors = arg_parse(argc, argv, chipmunk_import_from_chipmunk_argtable);
    tiledmap_scene_import_nerrors = arg_parse(argc, argv, tiledmap_scene_import_argtable);
    tiledmap_scene_export_nerrors = arg_parse(argc, argv, tiledmap_scene_export_argtable);
    tiledmap_scene_to_pic_nerrors = arg_parse(argc, argv, tiledmap_scene_to_pic_argtable);
    module_import_nerrors = arg_parse(argc, argv, module_import_argtable);
    common_nerrors = arg_parse(argc, argv, common_argtable);

    alloc = mem_allocrator_debug_create(NULL, NULL, 10, em);
    assert(alloc);

    app = gd_app_context_create_main(alloc, 0, argc, argv);
    assert(app);
    gd_app_ins_set(app);

    if (gd_app_bulk_install_module(app, load_modules, CPE_ARRAY_SIZE(load_modules), NULL) != 0) {
        CPE_ERROR(em, "load modules fail!");
        return -1;
    }

    data_mgr = ui_data_mgr_find_nc(app, NULL);
    assert(data_mgr);

    cache = ui_cache_manager_find_nc(app, NULL);
    assert(cache);

    ed_mgr = ui_ed_mgr_find_nc(app, NULL);
    assert(ed_mgr);

    /******/
    if (convert_nerrors == 0) {
        ui_cache_manager_set_texture_data_buff_keep(cache, 1);
        
        if (load_model(
                app, data_mgr, convert_model->sval[0], convert_model_format->sval[0], 1,
                em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, convert_to_format->sval[0], em) != 0) goto EXIT;

        rv = do_convert_model(
            app, convert_model->sval[0],
            convert_to->count ? convert_to->sval[0] : NULL,
            convert_to_format->sval[0],
            convert_texture_limit_width->count ? (uint32_t)convert_texture_limit_width->ival[0] : (uint32_t)0,
            convert_texture_limit_height->count ? (uint32_t)convert_texture_limit_height->ival[0] : (uint32_t)0,
            convert_texture_compress->count ? convert_texture_compress->sval[0] : NULL,
            convert_accounts_tinypng->count ? convert_accounts_tinypng->sval[0] : NULL,
            convert_language->sval, convert_language->count,
            convert_pack->count > 0 ? convert_pack->sval[0] : NULL,
            convert_full->count > 0 ? 1 : 0,
            em);
    }
    else if (manip_nerrors == 0) {
        if (load_model(app, data_mgr, manip_model->sval[0], manip_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, manip_model_format->sval[0], em) != 0) goto EXIT;

        rv = do_manip_model(app, data_mgr, manip_op->sval[0], em);
    }
    else if (repaire_nerrors == 0) {
        if (load_model(app, data_mgr, repaire_model->sval[0], repaire_model_format->sval[0], 1, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, repaire_model_format->sval[0], em) != 0) goto EXIT;

        rv = do_repaire_model(app, data_mgr, ed_mgr, em);
    }
    else if (cocos_module_nerrors == 0) {
        if (load_model(app, data_mgr, cocos_module_model->sval[0], cocos_module_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, cocos_module_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = do_cocos_module_import(
            app, data_mgr,
            cocos_module_to_module->sval[0],
            cocos_module_import_plist->sval[0],
            cocos_module_import_pic->sval[0]);
    }
    else if (cocos_effect_import_nerrors == 0) {
        if (load_model(app, data_mgr, cocos_effect_import_model->sval[0], cocos_effect_import_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, cocos_effect_import_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = do_cocos_effect_import(
            app, data_mgr,
            cocos_effect_import_to_effect->sval[0], 
            cocos_effect_import_to_module->sval[0], 
            cocos_effect_import_plist->sval[0],
            cocos_effect_import_pic->sval[0],
            cocos_effect_import_frame_duration->ival[0],
            cocos_effect_import_frame_position->sval[0],
            cocos_effect_import_frame_order->sval[0]);
    }
    else if (cocos_particle_import_nerrors == 0) {
        if (load_model(app, data_mgr, cocos_particle_import_model->sval[0], cocos_particle_import_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, cocos_particle_import_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = do_cocos_particle_import(
            app, data_mgr,
            cocos_particle_import_to_particle->sval[0], 
            cocos_particle_import_plist->sval[0],
            cocos_particle_import_pic->sval[0]);
    }
    else if (spine_anim_import_nerrors == 0) {
        if (load_model(app, data_mgr, spine_anim_import_model->sval[0], spine_anim_import_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, spine_anim_import_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = plugin_spine_manip_import(
                ed_mgr,
                spine_anim_import_to_module->sval[0],
                spine_anim_import_atlas->sval[0],
                spine_anim_import_pic->sval[0],
                em);
    }
    else if (crazystorm_emitter_import_nerrors == 0) {
        if (load_model(app, data_mgr, crazystorm_emitter_import_model->sval[0], crazystorm_emitter_import_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, crazystorm_emitter_import_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = plugin_barrage_import_crazystorm_emitter(
            ed_mgr,
            crazystorm_emitter_import_from_emitter->sval[0],
            crazystorm_emitter_import_to_emitter->sval[0], 
            crazystorm_emitter_import_use_bullets->sval[0],
            em);
    }
    else if (chipmunk_import_from_sprite_nerrors == 0) {
        if (load_model(app, data_mgr, chipmunk_import_from_sprite_model->sval[0], chipmunk_import_from_sprite_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, chipmunk_import_from_sprite_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = plugin_chipmunk_import_from_sprite(
            data_mgr,
            ed_mgr,
            chipmunk_import_from_sprite_from_sprite->sval[0],
            chipmunk_import_from_sprite_to_chipmunk->sval[0], 
            em);
    }
    else if (chipmunk_import_from_rube_nerrors == 0) {
        if (load_model(app, data_mgr, chipmunk_import_from_rube_model->sval[0], chipmunk_import_from_rube_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, chipmunk_import_from_rube_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = plugin_chipmunk_import_from_rube(
            data_mgr,
            ed_mgr,
            chipmunk_import_from_rube_from_rube->sval[0],
            chipmunk_import_from_rube_to_chipmunk->sval[0],
            (float)chipmunk_import_from_rube_ptm->dval[0],
            em);
    }
    else if (chipmunk_import_from_chipmunk_nerrors == 0) {
        if (load_model(app, data_mgr, chipmunk_import_from_chipmunk_model->sval[0], chipmunk_import_from_chipmunk_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, chipmunk_import_from_chipmunk_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = plugin_chipmunk_import_from_chipmunk(
            data_mgr,
            ed_mgr,
            chipmunk_import_from_chipmunk_from_chipmunk->sval[0],
            chipmunk_import_from_chipmunk_to_chipmunk->sval[0],
            chipmunk_import_from_chipmunk_adj_base->count ? chipmunk_import_from_chipmunk_adj_base->sval[0] : NULL,
            (float)chipmunk_import_from_chipmunk_ptm->dval[0],
            em);
    }
    else if (tiledmap_scene_import_nerrors == 0) {
        if (load_model(app, data_mgr, tiledmap_scene_import_model->sval[0], tiledmap_scene_import_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, tiledmap_scene_import_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = plugin_tiledmap_scene_import(data_mgr, ed_mgr, cache, tiledmap_scene_import_project->sval[0], em);
    }
    else if (tiledmap_scene_export_nerrors == 0) {
        if (load_model(app, data_mgr, tiledmap_scene_export_model->sval[0], tiledmap_scene_export_model_format->sval[0], 0, em) != 0) goto EXIT;

        rv = plugin_tiledmap_scene_export(data_mgr, cache, tiledmap_scene_export_project->sval[0], em);
    }
    else if (tiledmap_scene_to_pic_nerrors == 0) {
        if (load_model(app, data_mgr, tiledmap_scene_to_pic_model->sval[0], tiledmap_scene_to_pic_model_format->sval[0], 0, em) != 0) goto EXIT;

        rv = plugin_tiledmap_layers_to_pic(
            data_mgr, cache,
            tiledmap_scene_to_pic_input->sval[0],
            tiledmap_scene_to_pic_layer->count,
            tiledmap_scene_to_pic_layer->sval,
            tiledmap_scene_to_pic_output->sval[0],
            em);
    }
    else if (module_import_nerrors == 0) {
        if (load_model(app, data_mgr, module_import_model->sval[0], module_import_model_format->sval[0], 0, em) != 0) goto EXIT;
        if (set_model_saver(app, data_mgr, module_import_model_format->sval[0], em) != 0) goto EXIT;

        save_ed_mgr = 1;
        rv = ui_model_import_module(
            ed_mgr, cache, 
            module_import_to_module->sval[0],
            module_import_base->count > 0 ? module_import_base->sval[0] : NULL,
            module_import_src->sval, module_import_src->count,
            em);
    }
    else if (common_nerrors == 0) {
        if (common_help->count) {
            goto PRINT_HELP;
        }
    }
    else {
        goto PRINT_HELP;
    }

    if (rv == 0 && save_ed_mgr) {
        if (ui_ed_mgr_save(ed_mgr, NULL, gd_app_em(app)) != 0) {
            APP_CTX_ERROR(app, "save changed data fail!");
            rv = -1;
        }
    }

    goto EXIT;

PRINT_HELP:
    printf("%s:\n", argv[0]);
    printf("usage 01: %s ", argv[0]); arg_print_syntax(stdout, convert_argtable, "\n");
    printf("usage 02: %s ", argv[0]); arg_print_syntax(stdout, cocos_module_argtable, "\n");
    printf("usage 03: %s ", argv[0]); arg_print_syntax(stdout, cocos_effect_import_argtable, "\n");
    printf("usage 04: %s ", argv[0]); arg_print_syntax(stdout, cocos_particle_import_argtable, "\n");
    printf("usage 05: %s ", argv[0]); arg_print_syntax(stdout, spine_anim_import_argtable, "\n");
    printf("usage 07: %s ", argv[0]); arg_print_syntax(stdout, crazystorm_emitter_import_argtable, "\n");
    printf("usage 08: %s ", argv[0]); arg_print_syntax(stdout, chipmunk_import_from_sprite_argtable, "\n");
    printf("usage 08: %s ", argv[0]); arg_print_syntax(stdout, chipmunk_import_from_rube_argtable, "\n");
    printf("usage 09: %s ", argv[0]); arg_print_syntax(stdout, tiledmap_scene_import_argtable, "\n");
    printf("usage 10: %s ", argv[0]); arg_print_syntax(stdout, tiledmap_scene_export_argtable, "\n");
    printf("usage 11: %s ", argv[0]); arg_print_syntax(stdout, tiledmap_scene_to_pic_argtable, "\n");
    printf("usage 14: %s ", argv[0]); arg_print_syntax(stdout, module_import_argtable, "\n");
    printf("usage 15: %s ", argv[0]); arg_print_syntax(stdout, common_argtable, "\n");

EXIT:
#define ARG_FREETABLE(__arg)     arg_freetable(__arg, sizeof(__arg) / sizeof(__arg[0]))

    ARG_FREETABLE(convert_argtable);
    ARG_FREETABLE(cocos_module_argtable);
    ARG_FREETABLE(cocos_effect_import_argtable);
    ARG_FREETABLE(cocos_particle_import_argtable);
    ARG_FREETABLE(spine_anim_import_argtable);
    ARG_FREETABLE(crazystorm_emitter_import_argtable);
    ARG_FREETABLE(chipmunk_import_from_sprite_argtable);
    ARG_FREETABLE(chipmunk_import_from_rube_argtable);
    ARG_FREETABLE(tiledmap_scene_import_argtable);
    ARG_FREETABLE(tiledmap_scene_export_argtable);
    ARG_FREETABLE(tiledmap_scene_to_pic_argtable);
    ARG_FREETABLE(module_import_argtable);
    ARG_FREETABLE(common_argtable);

    if (app) gd_app_context_free(app);

    return rv;
}

static int load_model(
    gd_app_context_t app, ui_data_mgr_t data_mgr, const char * model, const char * model_format, int load_product,
    error_monitor_t em)
{
    if (ui_data_mgr_set_root(data_mgr, model) != 0) {
        CPE_ERROR(em, "load_model: set root %s fail!", model);
        return -1;
    }

    if (strcmp(model_format, "proj") == 0) {
        ui_proj_loader_t loader = ui_proj_loader_find_nc(app, NULL);
        if (loader == NULL) {
            loader = ui_proj_loader_create(app, NULL, NULL, em);
            if (loader == NULL) {
                CPE_ERROR(em, "load_model: format %s: create loader fail!", model_format);
                return -1;
            }
        }        

        plugin_particle_manip_install_proj_loader(plugin_particle_manip_find_nc(app, NULL));
        plugin_spine_manip_install_proj_loader(plugin_spine_manip_find_nc(app, NULL));
        plugin_swf_module_install_bin_loader(plugin_swf_module_find_nc(app, NULL));
        plugin_barrage_manip_install_proj_loader(plugin_barrage_manip_find_nc(app, NULL));
        plugin_moving_manip_install_proj_loader(plugin_moving_manip_find_nc(app, NULL));
        plugin_chipmunk_manip_install_proj_loader(plugin_chipmunk_manip_find_nc(app, NULL));
        plugin_tiledmap_manip_install_proj_loader(plugin_tiledmap_manip_find_nc(app, NULL));

        if (ui_proj_loader_set_root(loader, model) != 0) {
            CPE_ERROR(em, "load_model: format %s: set root %s!", model_format, model);
            return -1;
        }

        if (ui_data_proj_loader_load(loader, data_mgr, load_product) != 0) {
            CPE_ERROR(em, "load_model: format %s: load from %s fail!", model_format, model);
            return -1;
        }
    }
    else {
        CPE_ERROR(em, "load_model: format %s: format unknown!", model_format);
        return -1;
    }

    return 0;
}

static int set_model_saver(gd_app_context_t app, ui_data_mgr_t data_mgr, const char * model_format, error_monitor_t em) {
    if (strcmp(model_format, "proj") == 0) {
        ui_proj_loader_t loader = ui_proj_loader_find_nc(app, NULL);
        if (loader == NULL) {
            loader = ui_proj_loader_create(app, NULL, NULL, em);
            if (loader == NULL) {
                CPE_ERROR(em, "set_model_saver: format %s: create loader fail!", model_format);
                return -1;
            }
        }        

        plugin_spine_manip_install_proj_saver(plugin_spine_manip_find_nc(app, NULL));
        plugin_barrage_manip_install_proj_saver(plugin_barrage_manip_find_nc(app, NULL));
        plugin_moving_manip_install_proj_saver(plugin_moving_manip_find_nc(app, NULL));
        plugin_chipmunk_manip_install_proj_saver(plugin_chipmunk_manip_find_nc(app, NULL));
        plugin_tiledmap_manip_install_proj_saver(plugin_tiledmap_manip_find_nc(app, NULL));

        if (ui_data_proj_loader_set_save_to_data_mgr(loader, data_mgr) != 0) {
            CPE_ERROR(em, "set_model_saver: format %s: set saver fail!", model_format);
            return -1;
        }
    }
    else if (strcmp(model_format, "bin") == 0) {
        ui_bin_loader_t loader = ui_bin_loader_find_nc(app, NULL);
        if (loader == NULL) {
            loader = ui_bin_loader_create(app, NULL, NULL, em);
            if (loader == NULL) {
                CPE_ERROR(em, "set_model_saver: format %s: create loader fail!", model_format);
                return -1;
            }
        }        

        plugin_spine_module_install_bin_saver(plugin_spine_module_find_nc(app, NULL));
        plugin_barrage_module_install_bin_saver(plugin_barrage_module_find_nc(app, NULL));
        plugin_moving_module_install_bin_saver(plugin_moving_module_find_nc(app, NULL));

        if (ui_data_bin_loader_set_save_to_data_mgr(loader, data_mgr) != 0) {
            CPE_ERROR(em, "set_model_saver: format %s: set saver fail!", model_format);
            return -1;
        }
    }
    else {
        CPE_ERROR(em, "set_model_saver: format %s: format unknown!", model_format);
        return -1;
    }

    return 0;
}
