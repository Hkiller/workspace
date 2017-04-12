#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/file.h"
#include "gd/app/app_context.h"
#include "gd/dr_store/dr_store.h"
#include "gd/dr_store/dr_store_manage.h"
#include "dr_store_internal_ops.h"

static void dr_store_loader_lib_destory(LPDRMETALIB lib, void * ctx) {
    mem_free((mem_allocrator_t)ctx, lib);
}

int dr_store_manage_load_from_bin(dr_store_manage_t mgr, const char * libname, const char * path) {
    ssize_t buf_size, load_size;
    char * buf;
    FILE * fp;
    struct mem_buffer buffer;
    const char * file_path;

    mem_buffer_init(&buffer, mgr->m_alloc);

    if (gd_app_root(mgr->m_app)) {
        mem_buffer_strcat(&buffer, gd_app_root(mgr->m_app));
        mem_buffer_strcat(&buffer, "/");
        mem_buffer_strcat(&buffer, path);
        file_path = mem_buffer_make_continuous(&buffer, 0);
    }
    else {
        file_path = path;
    }

    fp = file_stream_open(file_path, "rb", mgr->m_em);
    if (fp == NULL) {
        CPE_ERROR(mgr->m_em, "%s: read load-from-bin %s: open file error!", dr_store_manage_name(mgr), file_path);
        mem_buffer_clear(&buffer);
        return -1;
    }

    buf_size = file_stream_size(fp, mgr->m_em);
    if (buf_size <= 0) {
        CPE_ERROR(mgr->m_em, "%s: read load-from-bin %s: read size error!", dr_store_manage_name(mgr), file_path);
        file_stream_close(fp, mgr->m_em);
        mem_buffer_clear(&buffer);
        return -1;
    }

    buf = mem_alloc(mgr->m_alloc, buf_size);
    if (buf == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: load from bin %s: alloc buf to store metalib fail, size is %d!",
            dr_store_manage_name(mgr), file_path, (int)buf_size);
        file_stream_close(fp, mgr->m_em);
        mem_buffer_clear(&buffer);
        return -1;
    }

    load_size = file_stream_load_to_buf(buf, buf_size, fp, mgr->m_em);
    if (load_size != buf_size) {
        CPE_ERROR(
            mgr->m_em, "%s: load from bin %s: load data to buf fail, reque %d, but load %d!",
            dr_store_manage_name(mgr), file_path, (int)buf_size, (int)load_size);
        file_stream_close(fp, mgr->m_em);
        mem_buffer_clear(&buffer);
        return -1;
    }

    if (dr_store_add_lib(mgr, libname, (LPDRMETALIB)buf, dr_store_loader_lib_destory, mgr->m_alloc) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: load from bin %s: load data to buf fail, reque %d, but load %d!",
            dr_store_manage_name(mgr), file_path, (int)buf_size, (int)load_size);
        file_stream_close(fp, mgr->m_em);
        mem_buffer_clear(&buffer);
        return -1;
    }

    file_stream_close(fp, mgr->m_em);
    mem_buffer_clear(&buffer);

    return 0;
}
