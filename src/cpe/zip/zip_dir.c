#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/zip/zip_file.h"
#include "zip_internal_types.h"

static
enum dir_visit_next_op
cpe_unzip_context_search_i(
    cpe_unzip_file_visitor_t visitor, void * ctx,
    cpe_unzip_dir_t d, int maxLevel,
    error_monitor_t em,
    mem_buffer_t buffer)
{
    cpe_unzip_dir_t child_dir;
    cpe_unzip_file_t child_file;
    char * path;
    size_t bufSize;
    enum dir_visit_next_op nextOp;

    if (maxLevel == 0) return dir_visit_next_go;

    path = (char *)mem_buffer_make_continuous(buffer, 0);
    bufSize = mem_buffer_size(buffer);

    nextOp = dir_visit_next_go;

    TAILQ_FOREACH(child_file, &d->m_child_files, m_next_file) {
        if (nextOp != dir_visit_next_go) break;

        if (mem_buffer_strcat(buffer, "/") != 0 || mem_buffer_strcat(buffer, child_file->m_name) != 0) break;

        if (visitor->on_file) {
            nextOp = visitor->on_file(
                (const char *)mem_buffer_make_continuous(buffer, 0),
                child_file,
                ctx);

            if (nextOp == dir_visit_next_exit) break;
        }

        /*restore curent path*/
        mem_buffer_set_size(buffer, bufSize);
        path = (char *)mem_buffer_make_continuous(buffer, 0);
        if (path == NULL) {
            CPE_ERROR(em, "no memory for asset search path");
            nextOp = dir_visit_next_exit;
            break;
        }
        path[bufSize - 1] = 0;
    }

    TAILQ_FOREACH(child_dir, &d->m_child_dirs, m_next_dir) {
        if (nextOp != dir_visit_next_go) break;

        if (mem_buffer_strcat(buffer, "/") != 0 || mem_buffer_strcat(buffer, child_dir->m_name) != 0) break;

        if (visitor->on_dir_enter) {
            nextOp = visitor->on_dir_enter(
                (const char *)mem_buffer_make_continuous(buffer, 0),
                child_dir,
                ctx);

            if (nextOp == dir_visit_next_exit) break;
        }
        else {
            nextOp = dir_visit_next_go;
        }

        if (nextOp == dir_visit_next_go) {
            nextOp = cpe_unzip_context_search_i(
                visitor, ctx,
                child_dir, maxLevel > 0 ? maxLevel - 1 : maxLevel,
                em, buffer);

            if (nextOp == dir_visit_next_exit) break;
        }

        if (visitor->on_dir_leave) {
            nextOp = visitor->on_dir_leave(
                (const char *)mem_buffer_make_continuous(buffer, 0),
                child_dir,
                ctx);

            if (nextOp == dir_visit_next_exit) break;
        }

        mem_buffer_set_size(buffer, bufSize);
        path = (char *)mem_buffer_make_continuous(buffer, 0);
        if (path == NULL) {
            CPE_ERROR(em, "no memory for asset search path");
            nextOp = dir_visit_next_exit;
            break;
        }
        path[bufSize - 1] = 0;
    }

    return nextOp == dir_visit_next_exit ?  dir_visit_next_exit : dir_visit_next_go;
}

void cpe_unzip_dir_search(
    cpe_unzip_file_visitor_t visitor, void * ctx,
    cpe_unzip_dir_t d, int maxLevel,
    error_monitor_t em, mem_allocrator_t talloc)
{
    struct mem_buffer buffer;

    mem_buffer_init(&buffer, talloc);

    cpe_unzip_dir_path(&buffer, d);

    cpe_unzip_context_search_i(visitor, ctx, d, maxLevel, em, &buffer);

    mem_buffer_clear(&buffer);
}

