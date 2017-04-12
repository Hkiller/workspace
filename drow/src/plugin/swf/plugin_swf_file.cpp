#include "base/tu_file.h"
#include "cpe/vfs/vfs_file.h"
#include "gd/app/app_context.h"
#include "gd/app/app_log.h"
#include "plugin_swf_file_i.hpp"

static int plugin_swf_file_read(void* dst, int bytes, void* appdata) {
    return vfs_file_read((vfs_file_t)appdata, dst, bytes);
}

static int plugin_swf_file_write(const void* src, int bytes, void* appdata) {
    return vfs_file_write((vfs_file_t)appdata, src, bytes);
}

static int plugin_swf_file_seek(int pos, void* appdata) {
    return vfs_file_seek((vfs_file_t)appdata, pos, vfs_file_seek_set);
}

static int plugin_swf_file_seek_to_end(void* appdata) {
    return vfs_file_seek((vfs_file_t)appdata, 0, vfs_file_seek_end);
}

static int plugin_swf_file_tell(const void* appdata) {
    return vfs_file_tell((vfs_file_t)appdata);
}

static bool plugin_swf_file_eof(void* appdata) {
    return vfs_file_eof((vfs_file_t)appdata) ? true : false;
}

static int plugin_swf_file_close(void* appdata) {
    vfs_file_close((vfs_file_t)appdata);
    return 0;
}

tu_file * plugin_swf_file_open(gd_app_context_t app, const char * path) {
    vfs_file_t vf = vfs_file_open(gd_app_vfs_mgr(app), path, "rb");
    if (vf == NULL) {
        APP_CTX_ERROR(app, "plugin_swf_file_open: open file %s fail!", path);
        return NULL;
    }

    try {
        return new tu_file((void *)vf,
                           plugin_swf_file_read,
                           plugin_swf_file_write,
                           plugin_swf_file_seek,
                           plugin_swf_file_seek_to_end,
                           plugin_swf_file_tell,
                           plugin_swf_file_eof,
                           plugin_swf_file_close);
    }
    catch(...) {
        vfs_file_close(vf);
        APP_CTX_ERROR(app, "plugin_swf_file_open: new tu file fail!");
        return NULL;
    }
}
