#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_stream.h"
#include "gd/app/app_context.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"
#include "plugin/particle/plugin_particle_data.h"
#include "plugin/barrage_manip/plugin_barrage_crazystorm.h"

int plugin_barrage_export_crazystorm_bullets(
    ui_data_mgr_t data_mgr, const char * bullet_path,
    const char * to_crazystorm_file, error_monitor_t em)
{
    vfs_file_t fp = NULL;
    struct vfs_write_stream fs;
    struct mem_buffer buf;
    const char * path;
    int rv = -1;
    ui_data_src_t particle_src;
    plugin_particle_data_t particle;
    struct plugin_particle_data_emitter_it emitter_it;
    plugin_particle_data_emitter_t emitter;

    mem_buffer_init(&buf, NULL);

    particle_src = 
        ui_data_src_child_find_by_path(ui_data_mgr_src_root(data_mgr), bullet_path, ui_data_src_type_particle);
    if (particle_src == NULL) {
        CPE_ERROR(em, "particle %s not exist!", bullet_path);
        goto COMPLETE;
    }

    particle = ui_data_src_product(particle_src);
    assert(particle);

    path = dir_name(to_crazystorm_file, &buf);
    if (dir_mk_recursion(path, DIR_DEFAULT_MODE, NULL, NULL) != 0) {
        CPE_ERROR(em, "make dir '%s' fail, errno=%d (%s)!", path, errno, (const char*)strerror(errno));
        goto COMPLETE;
    }

    fp = vfs_file_open(gd_app_vfs_mgr(ui_data_mgr_app(data_mgr)), to_crazystorm_file, "w");
    if (fp == NULL) {
        CPE_ERROR(em, "open file '%s' fail!", to_crazystorm_file);
        goto COMPLETE;
    }

    vfs_write_stream_init(&fs, fp);

    plugin_particle_data_emitters(&emitter_it, particle);
    while((emitter = plugin_particle_data_emitter_it_next(&emitter_it))) {
        UI_PARTICLE_EMITTER const * emitter_data;
        uint32_t collision_w;
        uint32_t collision_h;
        uint32_t id = (uint32_t)-1;
        const char * name;
        
        emitter_data = plugin_particle_data_emitter_data(emitter);
        assert(emitter_data);

        name = plugin_particle_data_emitter_msg(emitter, emitter_data->name_id);
        sscanf(name, FMT_UINT32_T, &id);
        /* printf("xxxxxx: collision: (%f,%f)-(%f,%f)\n", */
        /*        emitter_data->collision.lt.x, emitter_data->collision.lt.y, */
        /*        emitter_data->collision.rb.x, emitter_data->collision.rb.y); */

        collision_w = emitter_data->collision_atlas_w;
        collision_h = emitter_data->collision_atlas_h;

        /* printf("xxxxxx: collision_w=%d, collision_h=%d\n", collision_w, collision_h); */

        stream_printf(
            (write_stream_t)&fs, "%d_%d_%d_%d_%d_%d_%d_%d_0\n",
            id,
            (int)emitter_data->collision_atlas_x, (int)emitter_data->collision_atlas_y,
            (int)emitter_data->collision_atlas_w, (int)emitter_data->collision_atlas_h,
            collision_w / 2,
            collision_h / 2,
            collision_w > collision_h ? collision_h / 2 : collision_w / 2);
    }

    rv = 0;

COMPLETE:
    if (fp) vfs_file_close(fp);
    mem_buffer_clear(&buf);

    return rv;
}
