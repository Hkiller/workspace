#include <assert.h>
#include "cpe/utils/md5.h"
#include "vfs_file_i.h"

int vfs_file_calc_md5(vfs_file_t file, cpe_md5_value_t md5) {
    struct cpe_md5_ctx ctx;
    char buf[256];
    ssize_t size;

    cpe_md5_ctx_init(&ctx);

    while((size = vfs_file_read(file, buf, sizeof(buf)))) {
        if (size < 0) return -1;

        if (size == 0) break;
        
        cpe_md5_ctx_update(&ctx, buf, (size_t)size);
    }

    cpe_md5_ctx_final(&ctx);

    *md5 = ctx.value;

    return 0;
}

int vfs_file_calc_md5_by_path(vfs_mgr_t mgr, const char * path, cpe_md5_value_t md5) {
    vfs_file_t file;
    int rv;
    
    file = vfs_file_open(mgr, path, "rb");
    if (file == NULL) return -1;

    rv = vfs_file_calc_md5(file, md5);
    
    vfs_file_close(file);

    return rv;
}

uint8_t vfs_file_check_md5_str(vfs_mgr_t mgr, const char * path, const char * str_md5) {
    struct cpe_md5_value md5;
    if (cpe_md5_parse(&md5, str_md5) != 0) return 0;

    return vfs_file_check_md5(mgr, path, &md5);
}

uint8_t vfs_file_check_md5(vfs_mgr_t mgr, const char * path, cpe_md5_value_t md5) {
    struct cpe_md5_value file_md5;

    if (vfs_file_calc_md5_by_path(mgr, path, &file_md5) != 0) return 0;

    return cpe_md5_cmp(&file_md5, md5) == 0 ? 1 : 0;
}
