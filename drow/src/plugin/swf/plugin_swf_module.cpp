#include <assert.h>
#include "gameswf/gameswf_player.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_read.h"
#include "gd/app/app_log.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/cache/ui_cache_manager.h"
#include "render/runtime/ui_runtime_module.h"
#include "render/runtime/ui_runtime_render_obj_meta.h"
#include "plugin_swf_module_i.hpp"
#include "plugin_swf_data_i.hpp"
#include "plugin_swf_obj_i.hpp"
#include "plugin_swf_render_i.hpp"

namespace gameswf {
void ensure_loaders_registered();
void clears_tag_loaders();
}

static void plugin_swf_module_do_clear(nm_node_t node);

static void plugin_swf_fscommand_handler(gameswf::character* movie, const char* command, const char* arg);
static void plugin_swf_log_handler(bool error, const char* message);

struct nm_node_type s_nm_node_type_plugin_swf_module = {
    "plugin_swf_module",
    plugin_swf_module_do_clear
};

extern "C"
plugin_swf_module_t
plugin_swf_module_create(
    gd_app_context_t app, mem_allocrator_t alloc,
    ui_data_mgr_t data_mgr, ui_cache_manager_t cache_mgr, ui_runtime_module_t runtime,
    uint8_t debug, const char * name, error_monitor_t em)
{
    plugin_swf_module_t module;
    nm_node_t module_node;

    assert(app);

    if (name == NULL) name = "plugin_swf_module";

    module_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct plugin_swf_module));
    if (module_node == NULL) return NULL;

    module = (plugin_swf_module_t)nm_node_data(module_node);

    module->m_app = app;
    module->m_alloc = alloc;
    module->m_em = em;
    module->m_debug = debug;

    module->m_data_mgr = data_mgr;
    module->m_cache_mgr = cache_mgr;
    module->m_runtime = runtime;
    module->m_render = NULL;

    if (ui_data_mgr_register_type(
            data_mgr, ui_data_src_type_swf,
            (ui_data_product_create_fun_t)plugin_swf_data_create, module,
            (ui_data_product_free_fun_t)plugin_swf_data_free, module,
            plugin_swf_data_update_usings)
        != 0)
    {
        CPE_ERROR(em, "%s: create: register type swf skeleton fail!", name);
        nm_node_free(module_node);
        return NULL;
    }

    if (runtime) {
        ui_runtime_render_obj_meta_t obj_meta;

        obj_meta =
            ui_runtime_render_obj_meta_create(
                runtime, "swf", UI_OBJECT_TYPE_SWF, sizeof(struct plugin_swf_obj), module,
                plugin_swf_obj_do_init,
                plugin_swf_obj_do_set,
                plugin_swf_obj_do_setup,
                plugin_swf_obj_do_update,
                plugin_swf_obj_do_free,
                plugin_swf_obj_do_render,
                plugin_swf_obj_do_is_playing,
                NULL,
                NULL);
        if (obj_meta == NULL) {
            CPE_ERROR(em, "%s: create: register render obj fail", name);
            ui_data_mgr_unregister_type(data_mgr, ui_data_src_type_swf);
            nm_node_free(module_node);
            return NULL;
        }

    }

    /*初始化swf环境 */
    module->m_render = new plugin_swf_render_handler(module);
    //gameswf::register_file_opener_callback(&plugin_swf_file_opener);
    gameswf::register_fscommand_callback(plugin_swf_fscommand_handler);
    gameswf::register_log_callback(plugin_swf_log_handler);
    gameswf::set_sound_handler(0);
    gameswf::set_render_handler(module->m_render);
    gameswf::set_glyph_provider(gameswf::create_glyph_provider_tu());
    gameswf::ensure_loaders_registered();
    
    new ((void*)&module->m_player) gc_ptr<player>();
    module->m_player = new player();
	module->m_player->verbose_action(false);
	module->m_player->verbose_parse(false);
	module->m_player->set_separate_thread(false);
	module->m_player->set_log_bitmap_info(false);

    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_swf, plugin_swf_data_bin_load, module);

    mem_buffer_init(&module->m_dump_buffer, alloc);

    nm_node_set_type(module_node, &s_nm_node_type_plugin_swf_module);

    return module;
}

static void plugin_swf_module_do_clear(nm_node_t node) {
    plugin_swf_module_t module;

    module = (plugin_swf_module_t)nm_node_data(node);

    if (module->m_runtime) {
        ui_runtime_render_obj_meta_t obj_meta = ui_runtime_render_obj_meta_find_by_id(module->m_runtime, UI_OBJECT_TYPE_SKELETON);
        if (obj_meta) {
            ui_runtime_render_obj_meta_free(obj_meta);
        }
    }

    ui_data_mgr_unregister_type(module->m_data_mgr, ui_data_src_type_swf);

    module->m_player.~gc_ptr<player>();

    /*清理swf环境 */
    clears_tag_loaders();
    register_file_opener_callback(NULL);
    register_fscommand_callback(NULL);
    register_log_callback(NULL);
    set_sound_handler(NULL);
    set_render_handler(NULL);
    set_glyph_provider(NULL);
    
    delete module->m_render;
    module->m_render = NULL;
    
    mem_buffer_clear(&module->m_dump_buffer);
}

extern "C"
gd_app_context_t plugin_swf_module_app(plugin_swf_module_t module) {
    return module->m_app;
}

extern "C"
void plugin_swf_module_free(plugin_swf_module_t module) {
    nm_node_t module_node;
    assert(module);

    module_node = nm_node_from_data(module);
    if (nm_node_type(module_node) != &s_nm_node_type_plugin_swf_module) return;
    nm_node_free(module_node);
}

extern "C"
plugin_swf_module_t
plugin_swf_module_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_swf_module) return NULL;
    return (plugin_swf_module_t)nm_node_data(node);
}

extern "C"
plugin_swf_module_t
plugin_swf_module_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if(name == NULL) name = "plugin_swf_module";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_plugin_swf_module) return NULL;
    return (plugin_swf_module_t)nm_node_data(node);
}

extern "C"
const char * plugin_swf_module_name(plugin_swf_module_t module) {
    return nm_node_name(nm_node_from_data(module));
}

extern "C"
ui_data_mgr_t plugin_swf_module_data_mgr(plugin_swf_module_t module) {
    return module->m_data_mgr;
}

extern "C"
ui_runtime_module_t plugin_swf_module_runtime(plugin_swf_module_t module) {
    return module->m_runtime;
}

extern "C"
void plugin_swf_module_install_bin_loader(plugin_swf_module_t module) {
    ui_data_mgr_set_loader(
        module->m_data_mgr, ui_data_src_type_swf, plugin_swf_data_bin_load, module);
}

extern "C"
void plugin_swf_module_install_bin_saver(plugin_swf_module_t module) {
}

static void plugin_swf_fscommand_handler(gameswf::character* movie, const char* command, const char* arg) {
//    [[CCGameSWF sharedInstance] movieNamed:[NSString stringWithUTF8String:movie->m_name.c_str()] sentCommand:[NSString stringWithUTF8String:command] withArguments:[NSString stringWithUTF8String:arg]];
}

// tu_file* plugin_swf_file_opener(const char* url_or_path) {
//     assert(url_or_path);
// 	return new tu_file(url_or_path, "rb");
// }

static void plugin_swf_log_handler(bool error, const char* message) {
    if (error) {
        APP_ERROR("SWF: error: %s", message);
    }
    else {
        APP_INFO("SWF: info: %s", message);
    }
}
