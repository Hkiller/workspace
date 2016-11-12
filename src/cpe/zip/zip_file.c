#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_strings.h"
#include "cpe/utils/string_utils.h"
#include "cpe/zip/zip_file.h"
#include "zip_internal_types.h"

static void cpe_unzip_dir_free(cpe_unzip_dir_t d);
static void cpe_unzip_file_free(cpe_unzip_file_t f);
static int cpe_unzip_context_build(cpe_unzip_context_t context, error_monitor_t em);
static int cpe_unzip_file_open(cpe_unzip_file_t f, error_monitor_t em);
static void cpe_unzip_dir_build_path(mem_buffer_t buffer, cpe_unzip_dir_t d);

cpe_unzip_context_t
cpe_unzip_context_create(const char * path, mem_allocrator_t alloc, error_monitor_t em) {
    cpe_unzip_context_t r;
    int rv;

    r = (cpe_unzip_context_t)mem_alloc(alloc, sizeof(struct cpe_unzip_context));
    if (r == NULL) {
        CPE_ERROR(em, "cpe_unzip_context_create: malloc fail!");
        return NULL;
    }

    r->m_zip_file = cpe_unzOpen64(path);
    if (r == NULL) {
        CPE_ERROR(em, "cpe_unzip_context_create: open zip file %s fail!", path);
        mem_free(alloc, r);
        return NULL;
    }

    rv = cpe_unzGetGlobalInfo64(r->m_zip_file, &r->m_global_info);
    if (rv != UNZ_OK) {
        CPE_ERROR(em, "cpe_unzip_context_create: zip file %s: get global info fail!", path);
        cpe_unzClose(r->m_zip_file);
        mem_free(alloc, r);
        return NULL;
    }

    r->m_alloc = alloc;
    r->m_root = NULL;

    if (cpe_unzip_context_build(r, em) != 0) {
        CPE_ERROR(em, "cpe_unzip_context_create: zip file %s: build zip file info fail!", path);
        cpe_unzip_context_free(r);
        return NULL;
    }

    return r;
}

void cpe_unzip_context_free(cpe_unzip_context_t unzc) {
    if (unzc->m_root) cpe_unzip_dir_free(unzc->m_root);
    cpe_unzClose(unzc->m_zip_file);
    mem_free(unzc->m_alloc, unzc);
}

cpe_unzip_file_t cpe_unzip_file_find(cpe_unzip_context_t context, const char * file, error_monitor_t em) {
    const char * start;
    const char * end;
    cpe_unzip_dir_t cur_dir;
    cpe_unzip_file_t f;

    assert(context);
    assert(context->m_root);

    cur_dir = context->m_root;

    start = file;
    while((end = strchr(start, '/'))) {
        cpe_unzip_dir_t c;

        TAILQ_FOREACH(c, &cur_dir->m_child_dirs, m_next_dir) {
            size_t len = end - start;
            if (memcmp(c->m_name, start, len) == 0 && c->m_name[len] == 0) {
                break;
            }
        }

        if (c == TAILQ_END(&cur_dir->m_child_dirs)) return NULL;
        
        cur_dir = c;
        start = end + 1;
    }

    TAILQ_FOREACH(f, &cur_dir->m_child_files, m_next_file) {
        if (strcmp(f->m_name, start) == 0) return f;
    }

    return NULL;
}

const char * cpe_unzip_file_name(cpe_unzip_file_t zf) {
    return zf->m_name;
}

const char * cpe_unzip_file_path(mem_buffer_t buffer, cpe_unzip_file_t zf) {
    if (zf->m_parent_dir) {
        cpe_unzip_dir_build_path(buffer, zf->m_parent_dir);
        mem_buffer_strcat(buffer, "/");
    }

    mem_buffer_strcat(buffer, zf->m_name);

    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

cpe_unzip_dir_t cpe_unzip_dir_find(cpe_unzip_context_t context, const char * path, error_monitor_t em) {
    const char * start;
    const char * end;
    cpe_unzip_dir_t cur_dir;
    cpe_unzip_dir_t f;

    assert(context);
    assert(context->m_root);

    cur_dir = context->m_root;

    start = path;
    while((end = strchr(start, '/'))) {
        cpe_unzip_dir_t c;

        TAILQ_FOREACH(c, &cur_dir->m_child_dirs, m_next_dir) {
            size_t len = end - start;
            if (memcmp(c->m_name, start, len) == 0 && c->m_name[len] == 0) {
                break;
            }
        }

        if (c == TAILQ_END(&cur_dir->m_child_dirs)) return NULL;
        
        cur_dir = c;
        start = end + 1;
    }

    TAILQ_FOREACH(f, &cur_dir->m_child_dirs, m_next_dir) {
        if (strcmp(f->m_name, start) == 0) return f;
    }

    return NULL;
}

const char * cpe_unzip_dir_name(cpe_unzip_dir_t d) {
    return d->m_name;
}

static void cpe_unzip_dir_build_path(mem_buffer_t buffer, cpe_unzip_dir_t d) {
    if (d->m_parent_dir != d->m_context->m_root) {
        cpe_unzip_dir_build_path(buffer, d->m_parent_dir);
        mem_buffer_strcat(buffer, "/");
    }

    mem_buffer_strcat(buffer, d->m_name);
}

const char * cpe_unzip_dir_path(mem_buffer_t buffer, cpe_unzip_dir_t d) {
    cpe_unzip_dir_build_path(buffer, d);
    return (const char *)mem_buffer_make_continuous(buffer, 0);
}

ssize_t cpe_unzip_file_load_to_buffer(mem_buffer_t buffer, cpe_unzip_file_t zf, error_monitor_t em) {
    ssize_t len;
    size_t start;
    char * buf;
    int r;

    assert(buffer);

    len = zf->m_file_info.uncompressed_size;
    start = mem_buffer_size(buffer);
    mem_buffer_set_size(buffer, start + len);
    buf = mem_buffer_make_continuous(buffer, 0);

    if (buf == NULL) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buffer: alloc buff fail!");
        return -1;
    }

    if (cpe_unzip_file_open(zf, em) != 0) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buffer: open zip file fail!");
        return -1;
    }

    mem_buffer_set_size(buffer, len + start);

    assert(buf == mem_buffer_make_continuous(buffer, 0));

    r = cpe_unzReadCurrentFile(zf->m_context->m_zip_file, buf + start, (unsigned)len);
    assert(r == len);
    
    return len;
}

ssize_t cpe_unzip_file_load_to_buf(char * buf, size_t size, cpe_unzip_file_t zf, error_monitor_t em) {
    ssize_t read_size;

    if (cpe_unzip_file_open(zf, em) != 0) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: open zip file fail!");
        return -1;
    }

    read_size = cpe_unzReadCurrentFile(zf->m_context->m_zip_file, buf, (unsigned)size);
    if (read_size < 0) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: read fail!");
        cpe_unzCloseCurrentFile(zf->m_context->m_zip_file);
        return -1;
    }

    cpe_unzCloseCurrentFile(zf->m_context->m_zip_file);
    return read_size;
}

static int cpe_unzip_file_open(cpe_unzip_file_t zf, error_monitor_t em) {
    struct mem_buffer name_buffer;
    const char * path;
    int rv;

    mem_buffer_init(&name_buffer, zf->m_context->m_alloc);

    path = cpe_unzip_file_path(&name_buffer, zf);
    if (path == NULL) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: make path fail!");
        mem_buffer_clear(&name_buffer);
        return -1;
    }

    rv = cpe_unzLocateFile(zf->m_context->m_zip_file, path, 0);
    if (rv != UNZ_OK) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: zip file %s: locate file fail!", path);
        mem_buffer_clear(&name_buffer);
        return -1;
    }

    rv = cpe_unzOpenCurrentFile(zf->m_context->m_zip_file);
    if (rv != UNZ_OK) {
        CPE_ERROR(em, "cpe_unzip_file_load_to_buf: zip file %s: open zip fail!", path);
        mem_buffer_clear(&name_buffer);
        return -1;
    }

    mem_buffer_clear(&name_buffer);
    return 0;
}

static void cpe_unzip_file_free(cpe_unzip_file_t f) {
    assert(f);
    assert(f->m_parent_dir);

    TAILQ_REMOVE(&f->m_parent_dir->m_child_files, f, m_next_file);

    mem_free(f->m_context->m_alloc, f);
}

static void cpe_unzip_dir_free(cpe_unzip_dir_t d) {
    while(!TAILQ_EMPTY(&d->m_child_files)) {
        cpe_unzip_file_free(TAILQ_FIRST(&d->m_child_files));
    }

    while(!TAILQ_EMPTY(&d->m_child_dirs)) {
        cpe_unzip_dir_free(TAILQ_FIRST(&d->m_child_dirs));
    }

    if (d->m_parent_dir) {
        TAILQ_REMOVE(&d->m_parent_dir->m_child_dirs, d, m_next_dir);
    }
    
    mem_free(d->m_context->m_alloc, d);
}

static cpe_unzip_file_t cpe_unzip_file_create(cpe_unzip_dir_t parent, const char * name, unz_file_info64 * file_info, error_monitor_t em) {
    cpe_unzip_file_t r;

    r = mem_alloc(parent->m_context->m_alloc, sizeof(struct cpe_unzip_file));
    if (r == NULL) {
        CPE_ERROR(em, "cpe_unzip_file_create: malloc fail");
        return NULL;
    }

    r->m_context = parent->m_context;
    r->m_parent_dir = parent;
    cpe_str_dup(r->m_name, sizeof(r->m_name), name);
    memcpy(&r->m_file_info, file_info, sizeof(r->m_file_info));

    TAILQ_INSERT_TAIL(&parent->m_child_files, r, m_next_file);

    return r;
}

static cpe_unzip_dir_t cpe_unzip_dir_create(cpe_unzip_context_t context, cpe_unzip_dir_t parent, const char * name, error_monitor_t em) {
    cpe_unzip_dir_t r;

    r = mem_alloc(context->m_alloc, sizeof(struct cpe_unzip_dir));
    if (r == NULL) {
        CPE_ERROR(em, "cpe_unzip_dir_create: malloc fail");
        return NULL;
    }

    r->m_context = context;
    r->m_parent_dir = parent;
    TAILQ_INIT(&r->m_child_dirs);
    TAILQ_INIT(&r->m_child_files);
    cpe_str_dup(r->m_name, sizeof(r->m_name), name);

    if (parent) {
        TAILQ_INSERT_TAIL(&parent->m_child_dirs, r, m_next_dir);
    }

    return r;
}

static cpe_unzip_dir_t cpe_unzip_dir_find_or_create(cpe_unzip_dir_t parent, const char * name, error_monitor_t em) {
    cpe_unzip_dir_t r;

    TAILQ_FOREACH(r, &parent->m_child_dirs, m_next_dir) {
        if (strcmp(r->m_name, name) == 0) return r;
    }

    return cpe_unzip_dir_create(parent->m_context, parent, name, em);
}

static int cpe_unzip_context_build(cpe_unzip_context_t context, error_monitor_t em) {
    uLong i;
    int err;

    if (context->m_root != NULL) return 0;

    context->m_root = cpe_unzip_dir_create(context, NULL, "", em);
    if (context->m_root == NULL) {
        CPE_ERROR(em, "cpe_unzip_context_build: create root dir fail");
        return -1;
    }

    err = cpe_unzGoToFirstFile(context->m_zip_file);
    if (err != UNZ_OK) {
        CPE_ERROR(em, "cpe_unzip_context_build: unzGoToFirstFile error, error=%d", err);
        return -1;
    }

    for (i = 0; i < context->m_global_info.number_entry; i++) {
        char filename_inzip[256];
        unz_file_info64 file_info;
        cpe_unzip_dir_t cur_dir;
        char * start;
        char * end;

        err = cpe_unzGetCurrentFileInfo64(context->m_zip_file, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
        if (err != UNZ_OK) {
            CPE_ERROR(em, "cpe_unzip_context_build: unzGetCurrentFileInfo error, error=%d", err);
            return -1;
        }

        cur_dir = context->m_root;
        start = filename_inzip;
        while((end = strchr(start, '/'))) {
            *end = 0;
            cur_dir = cpe_unzip_dir_find_or_create(cur_dir, start, em);
            if (cur_dir == NULL) {
                CPE_ERROR(em, "cpe_unzip_context_build: create dir fail");
                return -1;
            }
            start = end + 1;
        }

        if (cpe_unzip_file_create(cur_dir, start, &file_info, em) == NULL) {
            CPE_ERROR(em, "cpe_unzip_context_build: create file error, error=%d", err);
            return -1;
        }

        if ((i + 1) < context->m_global_info.number_entry) {
            err = cpe_unzGoToNextFile(context->m_zip_file);
            if (err != UNZ_OK) {
                CPE_ERROR(em, "cpe_unzip_context_build: unzGoToNextFile error, error=%d",err);
                return -1;
            }
        }
    }

    return 0;
}

