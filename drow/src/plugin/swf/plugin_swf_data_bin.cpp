#include <assert.h>
#include "gameswf/gameswf_movie_def.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "plugin/swf/plugin_swf_data.h"
#include "render/model/ui_data_src.h"
#include "plugin_swf_data_i.hpp"
#include "plugin_swf_file_i.hpp"

void ui_data_bin_load_swf_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    plugin_swf_module_t module = (plugin_swf_module_t)p;
    plugin_swf_data_t swf_data = NULL;
    struct mem_buffer path_buff;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&path_buff);
    const char * path;
    
    /*build path*/
    mem_buffer_init(&path_buff, NULL);
    ui_data_src_path_print_to((write_stream_t)&stream, src, NULL);
    stream_printf((write_stream_t)&stream, ".swf");
    stream_putc((write_stream_t)&stream, 0);
    path = (const char *)mem_buffer_make_continuous(&path_buff, 0);

    swf_data = plugin_swf_data_create(module, src);
    if (swf_data == NULL) {
        CPE_ERROR(em, "create swf fail");
        goto LOAD_ERROR;
    }

    try {
        swf_data->m_movie_def = new movie_def_impl(module->m_player, DO_LOAD_BITMAPS, DO_LOAD_FONT_SHAPES);

        tu_file * f = plugin_swf_file_open(module->m_app, path);
        swf_data->m_movie_def->read(f);

        if (module->m_debug) {
            CPE_INFO(
                em, "load swf from %s success: version=%d, size=(%fx%f), frame-count=%d, frame-rate=%f", path,
                swf_data->m_movie_def->get_version(),
                swf_data->m_movie_def->get_width_pixels(), swf_data->m_movie_def->get_height_pixels(),
                swf_data->m_movie_def->get_frame_count() , swf_data->m_movie_def->get_frame_rate() );
        }
        
        mem_buffer_clear(&path_buff);
        return;
    }
    catch(::std::exception const & e) {
        CPE_ERROR(em, "load swf from %s catch exception %s", path, e.what());
    }
    catch(...) {
        CPE_ERROR(em, "load swf from %s catch unknown exception", path);
    }
    
LOAD_ERROR:
    if (swf_data) plugin_swf_data_free(swf_data);
    mem_buffer_clear(&path_buff);
}

int plugin_swf_data_bin_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        ui_data_bin_load_swf_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        ui_data_bin_load_swf_i(ctx, mgr, src, &logError);
    }

    return ret;
}
