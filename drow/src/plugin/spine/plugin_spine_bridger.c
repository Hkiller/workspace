#include <assert.h>
#include <errno.h>
#include "spine/extension.h"
#include "cpe/pal/pal_string.h"
#include "cpe/vfs/vfs_file.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "render/cache/ui_cache_manager.h"
#include "render/cache/ui_cache_texture.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_src.h"

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path) {
    gd_app_context_t app = gd_app_ins();
    ui_data_mgr_t data_mgr;
    ui_cache_manager_t cache_mgr;
    ui_cache_res_t texture;
    const char * root;

    assert(app);

    cache_mgr = ui_cache_manager_find_nc(app, NULL);
    assert(cache_mgr);

    data_mgr = ui_data_mgr_find_nc(app, NULL);
    assert(data_mgr);

    root = ui_data_src_data(ui_data_mgr_src_root(data_mgr));
    if (strstr(path, root) != path) {
        self->rendererObject = NULL;
        self->width = 0;
        self->height = 0;
        return;
    }

    path += strlen(root);
	if (path[0] == '/') path += 1;

    texture = ui_cache_res_find_by_path(cache_mgr, path);
    if (texture == NULL) {
        texture = ui_cache_res_create(cache_mgr, ui_cache_res_type_texture);
        if (texture == NULL) {
            APP_CTX_ERROR(app, "spine: load texture %s: create texture fail!", path);
            return;
        }

        if (ui_cache_res_set_path(texture, path) != 0) {
            APP_CTX_ERROR(app, "spine: load texture %s: set texture path!", path);
            return;
        }
    }

    if (ui_cache_texture_width(texture) <= 0 || ui_cache_texture_height(texture) <= 0) {
        if (ui_cache_res_load_state(texture) == ui_cache_res_not_load
            || ui_cache_res_load_state(texture) == ui_cache_res_loading)
        {
            ui_cache_res_load_sync(texture, ui_data_src_data(ui_data_mgr_src_root(data_mgr)));
        }

        if (ui_cache_res_load_state(texture) != ui_cache_res_loaded) {
            APP_CTX_ERROR(
                app, "spine: load texture %s: load fail, reason=%d!",
                path, (int)ui_cache_res_load_result(texture));
            return;
        }
    }
    
	self->rendererObject = texture;
	self->width = ui_cache_texture_width(texture);
	self->height = ui_cache_texture_height(texture);
}

void _spAtlasPage_disposeTexture (spAtlasPage* self) {
	//ui_cache_res_unload((ui_cache_res_t)self->rendererObject);
}

char* _spUtil_readFile (const char* path, int* length) {
    gd_app_context_t app = gd_app_ins();
    ssize_t sz;
    char * data;
    vfs_file_t fp;
    ssize_t load_size;

    assert(app);

    fp = vfs_file_open(gd_app_vfs_mgr(app), path, "rb");
    if (fp == NULL) {
        APP_CTX_ERROR(
            app, "spine: load file %s: open file error, errno=%d (%s)",
            path, errno, strerror(errno));
        return NULL;
    }

    sz = vfs_file_size(fp);
    if (sz < 0) {
        APP_CTX_ERROR(
            app, "spine: load file %s: load fail, errno=%d (%s)!",
            path, errno, strerror(errno));
        vfs_file_close(fp);
        return NULL;
    }

    data = MALLOC(char, sz);
    if (data == NULL) {
        APP_CTX_ERROR(app, "spine: load file %s: alloc buf fail, size=%d", path, (int)sz);
        vfs_file_close(fp);
        return NULL;
    }
    
    load_size = vfs_file_read(fp, data, sz);
    if (load_size < sz) {
        APP_CTX_ERROR(
            app, "spine: load file %s: read file error, load size=%d, size=%d, errno=%d (%s)",
            path, (int)load_size, (int)sz, errno, strerror(errno));
        vfs_file_close(fp);
        _free(data);
        return NULL;
    }

    vfs_file_close(fp);

    *length = (int)sz;
    return data;
}
