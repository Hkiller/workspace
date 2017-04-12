#include <assert.h>
#include <errno.h>
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_stream.h"
#include "render/model/ui_data_mgr.h"
#include "render/model/ui_data_language.h"
#include "render/model/ui_data_src_rw.h"
#include "ui_data_src_i.h"

int ui_data_src_save_to_file(
    ui_data_src_t src, const char * root, const char * postfix,
    ui_data_src_do_save_fun_t save_fun, void * save_fun_ctx,
    error_monitor_t em)
{
    ui_data_mgr_t mgr = src->m_mgr;
    char path_buff[512];
    struct write_stream_mem path_stream = CPE_WRITE_STREAM_MEM_INITIALIZER(path_buff, sizeof(path_buff));
    int path_len;
    vfs_file_t fp = NULL;
    int rv = -1;
    ui_data_language_t language;
    
    if (root == NULL) {
        root = ui_data_src_data(ui_data_mgr_src_root(mgr));
    }

    /*构造路径 */
    if (root[0]) {
        stream_printf((write_stream_t)&path_stream, root);
        if (root[strlen(root) - 1] != '/') {
            stream_printf((write_stream_t)&path_stream, "/");
        }
    }
    ui_data_src_path_print((write_stream_t)&path_stream, ui_data_src_parent(src));
    stream_putc((write_stream_t)&path_stream, 0);

    if (vfs_dir_mk_recursion(gd_app_vfs_mgr(mgr->m_app), path_buff) != 0) {
        CPE_ERROR(
            em, "save %s: make dir '%s' fail, errno=%d (%s)!", 
            ui_data_src_path_dump(&mgr->m_dump_buffer, src), path_buff, errno, (const char*)strerror(errno));
        goto COMPLETE;
    }

    /*构造文件路径 */
    path_len = strlen(path_buff);
    path_len += snprintf(path_buff + path_len, sizeof(path_buff) - path_len, "/%s", ui_data_src_data(src));
    if ((language = ui_data_src_language(src))) {
        path_len += snprintf(path_buff + path_len, sizeof(path_buff) - path_len, "_%s", ui_data_language_name(language));
    }
    path_len += snprintf(path_buff + path_len, sizeof(path_buff) - path_len, ".%s", postfix);

    fp = vfs_file_open(gd_app_vfs_mgr(mgr->m_app), path_buff, "wb");
    if (fp == NULL) {
        CPE_ERROR(
            em, "save %s: open file '%s' fail, errno=%d (%s)!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src), path_buff, errno, (const char*)strerror(errno));
        goto COMPLETE;
    }

    /*写入文件 */
    if (save_fun(save_fun_ctx, src, fp, em) != 0) {
        CPE_ERROR(
            em, "save %s: do save fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        goto COMPLETE;
    }

    rv = 0;
    
COMPLETE:
    if (fp) vfs_file_close(fp);

    return rv;
}

int ui_data_src_load_from_file(
    ui_data_src_t src, const char * postfix,
    ui_data_src_do_load_fun_t load_fun, void * load_fun_ctx,
    error_monitor_t em)
{
    ui_data_mgr_t mgr = src->m_mgr;
    char path_buff[512];
    struct write_stream_mem path_stream = CPE_WRITE_STREAM_MEM_INITIALIZER(path_buff, sizeof(path_buff));
    vfs_file_t fp = NULL;
    int rv = -1;
    const char * root = ui_data_src_data(ui_data_mgr_src_root(mgr));

    /*构造路径 */
    if (root[0]) {
        stream_printf((write_stream_t)&path_stream, root);
        if (root[strlen(root) - 1] != '/') {
            stream_printf((write_stream_t)&path_stream, "/");
        }
    }
    ui_data_src_path_print((write_stream_t)&path_stream, src);
    stream_printf((write_stream_t)&path_stream, ".%s", postfix);
    stream_putc((write_stream_t)&path_stream, 0);

    fp = vfs_file_open(gd_app_vfs_mgr(mgr->m_app), path_buff, "rb");
    if (fp == NULL) {
        CPE_ERROR(
            em, "load %s: open file %s fail, errno=%d (%s)!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src), path_buff, errno, (const char*)strerror(errno));
        goto COMPLETE;
    }

    /*读取文件 */
    if (load_fun(load_fun_ctx, src, fp, em) != 0) {
        CPE_ERROR(
            em, "load %s: do load fail!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src));
        goto COMPLETE;
    }

    rv = 0;

COMPLETE:
    if (fp) vfs_file_close(fp);

    return rv;
}

int ui_data_src_remove_file(ui_data_src_t src, const char * root, const char * postfix, error_monitor_t em) {
    ui_data_mgr_t mgr = src->m_mgr;
    char path_buff[512];
    struct write_stream_mem path_stream = CPE_WRITE_STREAM_MEM_INITIALIZER(path_buff, sizeof(path_buff));

    stream_printf((write_stream_t)&path_stream, root);
    if (root[strlen(root) - 1] != '/') {
        stream_printf((write_stream_t)&path_stream, "/");
    }
    ui_data_src_path_print((write_stream_t)&path_stream, src);
    stream_printf((write_stream_t)&path_stream, ".%s", postfix);
    stream_putc((write_stream_t)&path_stream, 0);

    if (file_rm(path_buff, em) != 0) {
        CPE_ERROR(
            em, "remove %s: rm file %s fail, errno=%d (%s)!",
            ui_data_src_path_dump(&mgr->m_dump_buffer, src),
            path_buff, errno, (const char*)strerror(errno));
        return -1;
    }

    return 0;
}

