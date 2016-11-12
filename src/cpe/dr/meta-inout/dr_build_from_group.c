#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/file.h"
#include "cpe/vfs/vfs_file.h"
#include "cpe/vfs/vfs_dir.h"
#include "cpe/vfs/vfs_visitor.h"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_build.h"
#include "cpe/dr/dr_metalib_builder.h"

static int dr_metalib_build_parse_group(
    dr_metalib_builder_t builder,
    vfs_mgr_t vfs, const char * group_root, const char * group,
    error_monitor_t em);

static int dr_metalib_build_search_dir(
    dr_metalib_builder_t builder,
    vfs_mgr_t vfs, const char * group_root,
    error_monitor_t em);

static LPDRMETALIB dr_metalib_build_alloc(
    mem_allocrator_t alloc, error_monitor_t em, dr_metalib_builder_t builder);

LPDRMETALIB
dr_metalib_build_from_group(mem_allocrator_t alloc, error_monitor_t em, vfs_mgr_t vfs, const char * group_root, const char * group) {
    dr_metalib_builder_t builder;
    LPDRMETALIB r_lib;
    
    builder = dr_metalib_builder_create(NULL, em);
    if (builder == NULL) {
        CPE_ERROR(em, "create metalib builder fail!");
        return NULL;
    }

    if (dr_metalib_build_parse_group(builder, vfs, group_root, group, em) != 0) return NULL;

    r_lib = dr_metalib_build_alloc(alloc, em, builder);

    dr_metalib_builder_free(builder);
    
    return r_lib;
}

LPDRMETALIB dr_metalib_build_from_dir(
    mem_allocrator_t alloc, error_monitor_t em, vfs_mgr_t vfs, const char * root)
{
    dr_metalib_builder_t builder;
    LPDRMETALIB r_lib;

    builder = dr_metalib_builder_create(NULL, em);
    if (builder == NULL) {
        CPE_ERROR(em, "create metalib builder fail!");
        return NULL;
    }

    if (dr_metalib_build_search_dir(builder, vfs, root, em) != 0) return NULL;
    
    r_lib = dr_metalib_build_alloc(alloc, em, builder);

    dr_metalib_builder_free(builder);
    
    return r_lib;
}

static int dr_metalib_build_parse_group(
    dr_metalib_builder_t builder,
    vfs_mgr_t vfs, const char * group_root, const char * group,
    error_monitor_t em)
{
    char path_buf[256];
    size_t path_len = 0;
    vfs_file_t group_file;
    ssize_t file_len, readed_len;
    
    path_len = strlen(group_root);
    if (path_len + 5 > sizeof(path_buf)) {
        CPE_ERROR(em, "group input %s is too long!", group);
		return -1;
    }

    snprintf(path_buf, sizeof(path_buf), "%s", group_root);
    if (path_buf[path_len - 1] != '/') {
        ++path_len;
        path_buf[path_len - 1] = '/';
        path_buf[path_len] = 0;
    }

    group_file = vfs_file_open(vfs, group, "r");
    if (group_file == NULL) {
        CPE_ERROR(em, "group input %s not exist!", group);
		return -1;
    }

    file_len = 0;
    while((readed_len = vfs_file_read(group_file, path_buf + path_len, sizeof(path_buf) - path_len - 1))) {
        char * p;
        size_t used_len;
        
        if (readed_len < 0) {
            CPE_ERROR(em, "read group def from %s fail!", group);
            vfs_file_close(group_file);
			return -1;
        }
        if (readed_len == 0 && file_len == 0) break;
        
        file_len += readed_len;

        path_buf[path_len + file_len] = 0;
        
        if ((p = strchr(path_buf + path_len, '\n'))) {
            char * p2;

            if ((p2 = strchr(path_buf + path_len, '\r'))) {
                if (p2 < p) {
                    *p2 = 0;
                    used_len = (p - path_buf) - path_len;
                }
                else {
                    *p = 0;
                    used_len = (p2 - path_buf) - path_len;
                }
            }
            else {
                *p = 0;
                used_len = (p - path_buf) - path_len;
            }
        }
        else if ((p = strchr(path_buf + path_len, '\r'))) {
            *p = 0;
            used_len = (p - path_buf) - path_len;
        }
        else {
            used_len = strlen(path_buf + path_len);
        }
        
        if (vfs_file_exist(vfs, path_buf)) {
            dr_metalib_builder_add_file(builder, NULL, path_buf);
        }
        else {
            CPE_ERROR(em, "input %s not exist!", path_buf);
            vfs_file_close(group_file);
			return -1;
        }

        memmove(path_buf + path_len, path_buf + path_len + used_len, file_len - used_len);
    }

    vfs_file_close(group_file);
	return 0;
}

vfs_visitor_next_op_t
dr_metalib_build_accept_input_file(vfs_mgr_t mgr, const char * full, const char * f, void * ctx) {
    if (strcmp(file_name_suffix(f), "xml") == 0) {
        dr_metalib_builder_add_file((dr_metalib_builder_t)ctx, NULL, full);
    }
    return vfs_visitor_next_go;
}

static int dr_metalib_build_search_dir(
    dr_metalib_builder_t builder,
    vfs_mgr_t vfs, const char * group_root,
    error_monitor_t em)
{
    struct vfs_visitor visitor = { NULL, NULL, dr_metalib_build_accept_input_file };
    
    vfs_search_dir(vfs, &visitor, builder, group_root, -1);

    return 0;
}

static LPDRMETALIB dr_metalib_build_alloc(mem_allocrator_t alloc, error_monitor_t em, dr_metalib_builder_t builder) {
    struct mem_buffer buffer;
    LPDRMETALIB tmp_lib;
    LPDRMETALIB r_lib;
    dr_metalib_builder_analize(builder);

    if (dr_inbuild_tsort(dr_metalib_bilder_lib(builder), em) != 0) {
        CPE_ERROR(em, "dr_metalib_build_alloc: tsort fail!");
        return NULL;
    }
    
    mem_buffer_init(&buffer, NULL);
    if (dr_inbuild_build_lib(&buffer, dr_metalib_bilder_lib(builder), em) != 0) {
        mem_buffer_clear(&buffer);
        return NULL;
    }

    tmp_lib = (LPDRMETALIB)mem_buffer_make_continuous(&buffer, 0);
    assert(tmp_lib);

    r_lib = mem_alloc(alloc, dr_lib_size(tmp_lib));
    if (r_lib == NULL) {
        CPE_ERROR(em, "dr_metalib_build_alloc: alloc lib buf(size=%d) fail!", (int)dr_lib_size(tmp_lib));
        mem_buffer_clear(&buffer);
        return NULL;
    }

    memcpy(r_lib, tmp_lib, dr_lib_size(tmp_lib));

    return r_lib;
}
