#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "render/model_ed/ui_ed_mgr.h"
#include "plugin/spine/plugin_spine_data_skeleton.h"
#include "plugin/spine/plugin_spine_atlas.h"
#include "plugin_spine_manip_i.h"

void plugin_spine_manip_load_skeleton_i(void * p, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    plugin_spine_manip_t module = (plugin_spine_manip_t)p;
    plugin_spine_data_skeleton_t spine = NULL;
    spAtlas * atlas = NULL;
    spSkeletonJson* json = NULL;
    spSkeletonData* skeleton_data = NULL;
    struct mem_buffer path_buff;
    struct mem_buffer data_buff;
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&path_buff);
    const char * path;

    mem_buffer_init(&path_buff, NULL);
    mem_buffer_init(&data_buff, NULL);

    /*load atlas */
    path = ui_data_src_path_dump(&path_buff, src);
    atlas = plugin_spine_atlas_load_from_model(module->m_spine_module, path);
    if (atlas == NULL) {
        CPE_ERROR(em, "atlas %s not loaded", path);
        goto LOAD_ERROR;
    }

    /*load skeleton*/
    mem_buffer_clear_data(&path_buff);
    ui_data_src_path_print_to((write_stream_t)&stream, src, NULL);
    stream_printf((write_stream_t)&stream, ".spine");
    stream_putc((write_stream_t)&stream, 0);
    path = mem_buffer_make_continuous(&path_buff, 0);

	json = spSkeletonJson_create(atlas);
	/* json->scale = scale; */
	skeleton_data = spSkeletonJson_readSkeletonDataFile(json, path);
    if (skeleton_data == NULL || json->error) {
        CPE_ERROR(em, "read skeleton fail, error=(%s), from(%s)", json->error, path);
        goto LOAD_ERROR;
    }

    spine = plugin_spine_data_skeleton_create(module->m_spine_module, src);
    if (spine == NULL) {
        CPE_ERROR(em, "create spine fail");
        goto LOAD_ERROR;
    }

    plugin_spine_data_skeleton_set_data(spine, skeleton_data, atlas);
    skeleton_data = NULL;
    
    mem_buffer_clear(&path_buff);
    mem_buffer_clear(&data_buff);
	spSkeletonJson_dispose(json);

    return;

LOAD_ERROR:
    mem_buffer_clear(&path_buff);
    mem_buffer_clear(&data_buff);
    if(json) spSkeletonJson_dispose(json);
    if (skeleton_data) spSkeletonData_dispose(skeleton_data);
    if (atlas) plugin_spine_atlas_free(atlas);
}

int plugin_spine_manip_skeleton_proj_load(void * ctx, ui_data_mgr_t mgr, ui_data_src_t src, error_monitor_t em) {
    int ret = 0;

    if (em) {
        CPE_DEF_ERROR_MONITOR_ADD(logError, em, cpe_error_save_last_errno, &ret);
        plugin_spine_manip_load_skeleton_i(ctx, mgr, src, em);
        CPE_DEF_ERROR_MONITOR_REMOVE(logError, em);
    }
    else {
        CPE_DEF_ERROR_MONITOR(logError, cpe_error_save_last_errno, &ret);
        plugin_spine_manip_load_skeleton_i(ctx, mgr, src, &logError);
    }

    return ret;
}
