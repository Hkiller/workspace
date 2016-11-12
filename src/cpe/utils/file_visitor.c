#include <string.h>
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/file.h"
#include "cpe/utils/buffer.h"
#include "file_internal.h"

enum dir_visit_next_op
dir_search_i(
    dir_visitor_t visitor, void * ctx,
    int maxLevel,
    error_monitor_t em,
    mem_buffer_t buffer)
{
    DIR * dirp;
    struct dirent dbuf;
    struct dirent * dp;
    char * path;
    int rv;
    size_t bufSize;
    enum dir_visit_next_op nextOp;

    if (maxLevel == 0) return dir_visit_next_go;

    path = (char *)mem_buffer_make_continuous(buffer, 0);
    bufSize = mem_buffer_size(buffer);

    dirp = dir_open(path, 0, em);
    if (dirp == NULL) return dir_visit_next_go;

    nextOp = dir_visit_next_go;

    while((rv = readdir_r(dirp, &dbuf, &dp)) == 0 && dp) {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;

        if (mem_buffer_strcat(buffer, "/") != 0 || mem_buffer_strcat(buffer, dp->d_name) != 0) break;

        if (dp->d_type == DT_DIR) {
            if (visitor->on_dir_enter) {
                nextOp = visitor->on_dir_enter(
                    (const char *)mem_buffer_make_continuous(buffer, 0),
                    dp->d_name,
                    ctx);

                if (nextOp == dir_visit_next_exit) break;
            }
            else {
                nextOp = dir_visit_next_go;
            }

            if (nextOp == dir_visit_next_go) {
                nextOp = dir_search_i(
                    visitor, ctx,
                    maxLevel > 0 ? maxLevel - 1 : maxLevel,
                    em, buffer);

                if (nextOp == dir_visit_next_exit) break;
            }

            if (visitor->on_dir_leave) {
                nextOp = visitor->on_dir_leave(
                    (const char *)mem_buffer_make_continuous(buffer, 0),
                    dp->d_name,
                    ctx);

                if (nextOp == dir_visit_next_exit) break;
            }
        }
        else if (dp->d_type == DT_REG) {
            if (visitor->on_file) {
                nextOp = visitor->on_file(
                    (const char *)mem_buffer_make_continuous(buffer, 0),
                    dp->d_name,
                    ctx);

                if (nextOp == dir_visit_next_exit) break;
            }
        }

        /*restore curent path*/
        mem_buffer_set_size(buffer, bufSize);
        path = (char *)mem_buffer_make_continuous(buffer, 0);
        if (path == NULL) {
            CPE_ERROR_EX(em, ENOMEM, "no memory for dir search path");
            nextOp = dir_visit_next_exit;
            break;
        }
        path[bufSize - 1] = 0;

    }

    /*clear resources*/
    dir_close(dirp, em);

    return nextOp == dir_visit_next_exit ?  dir_visit_next_exit : dir_visit_next_go;
}

void dir_search(
    dir_visitor_t visitor, void * ctx,
    const char * path, int maxLevel,
    error_monitor_t em, mem_allocrator_t talloc)
{
    struct mem_buffer buffer;
    mem_buffer_init(&buffer, talloc);

    mem_buffer_strcat(&buffer, path);

    dir_search_i(visitor, ctx, maxLevel, em, &buffer);

    mem_buffer_clear(&buffer);
}

