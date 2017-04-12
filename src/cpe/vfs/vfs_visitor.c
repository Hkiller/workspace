#include "cpe/pal/pal_string.h"
#include "cpe/utils/buffer.h"
#include "cpe/vfs/vfs_visitor.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_entry_info.h"
#include "vfs_manage_i.h"

enum vfs_visitor_next_op
vfs_dir_search_i(vfs_mgr_t vfs, vfs_visitor_t visitor, void * ctx, int max_level, mem_buffer_t buffer) {
    vfs_dir_t dirp;
    struct vfs_entry_info_it entry_info_it;
    vfs_entry_info_t entry_info;
    char * path;
    size_t buf_size;
    enum vfs_visitor_next_op next_op;

    if (max_level == 0) return vfs_visitor_next_go;

    path = (char *)mem_buffer_make_continuous(buffer, 0);
    buf_size = mem_buffer_size(buffer);

    dirp = vfs_dir_open(vfs, path);
    if (dirp == NULL) return vfs_visitor_next_go;

    next_op = vfs_visitor_next_go;

    vfs_dir_entries(dirp, &entry_info_it);
    
    while((entry_info = vfs_entry_info_it_next(&entry_info_it))) {
        if (strcmp(entry_info->m_name, ".") == 0 || strcmp(entry_info->m_name, "..") == 0) continue;

        if (mem_buffer_strcat(buffer, "/") != 0 || mem_buffer_strcat(buffer, entry_info->m_name) != 0) break;

        if (entry_info->m_type == vfs_entry_dir) {
            if (visitor->on_dir_enter) {
                next_op = visitor->on_dir_enter(
                    vfs,
                    (const char *)mem_buffer_make_continuous(buffer, 0),
                    entry_info->m_name,
                    ctx);

                if (next_op == vfs_visitor_next_exit) break;
            }
            else {
                next_op = vfs_visitor_next_go;
            }

            if (next_op == vfs_visitor_next_go) {
                next_op = vfs_dir_search_i(
                    vfs,
                    visitor, ctx,
                    max_level > 0 ? max_level - 1 : max_level,
                    buffer);

                if (next_op == vfs_visitor_next_exit) break;
            }

            if (visitor->on_dir_leave) {
                next_op = visitor->on_dir_leave(
                    vfs,
                    (const char *)mem_buffer_make_continuous(buffer, 0),
                    entry_info->m_name,
                    ctx);

                if (next_op == vfs_visitor_next_exit) break;
            }
        }
        else if (entry_info->m_type == vfs_entry_file) {
            if (visitor->on_file) {
                next_op = visitor->on_file(
                    vfs,
                    (const char *)mem_buffer_make_continuous(buffer, 0),
                    entry_info->m_name,
                    ctx);

                if (next_op == vfs_visitor_next_exit) break;
            }
        }

        /*restore curent path*/
        mem_buffer_set_size(buffer, buf_size);
        path = (char *)mem_buffer_make_continuous(buffer, 0);
        if (path == NULL) {
            CPE_ERROR(vfs->m_em, "no memory for dir search path");
            next_op = vfs_visitor_next_exit;
            break;
        }
        path[buf_size - 1] = 0;

    }

    /*clear resources*/
    vfs_dir_close(dirp);

    return next_op == vfs_visitor_next_exit ?  vfs_visitor_next_exit : vfs_visitor_next_go;
}

void vfs_search_dir(vfs_mgr_t vfs, vfs_visitor_t visitor, void * ctx, const char * path, int max_level) {
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, vfs->m_alloc);

    mem_buffer_strcat(&buffer, path);

    vfs_dir_search_i(vfs, visitor, ctx, max_level, &buffer);

    mem_buffer_clear(&buffer);
}
