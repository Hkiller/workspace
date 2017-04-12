#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/utils/string_utils.h"
#include "ui_data_src_i.h"

ui_data_src_t ui_data_src_create_file(ui_data_mgr_t mgr, ui_data_src_type_t type, const char * full_file) {
    char name_buf[128];
    size_t root_len = strlen(mgr->m_src_root->m_data);
    ui_data_src_t parent;
    ui_data_src_t src;
    const char * sep;
    char * p;

    if (root_len > 0) {
        if (memcmp(mgr->m_src_root->m_data, full_file, root_len) != 0 || full_file[root_len] != '/') {
            CPE_ERROR(mgr->m_em, "ui_data_src: file %s not in root %s!", full_file, mgr->m_src_root->m_data);
            return NULL;
        }

        full_file += root_len + 1;
    }
    else {
        if (full_file[0] == '/') {
            CPE_ERROR(mgr->m_em, "ui_data_src: file %s should relative!", full_file);
            return NULL;
        }
    }

    parent = mgr->m_src_root;

    while((sep = strchr(full_file, '/'))) {
        ui_data_src_t child;
        size_t name_len = sep - full_file;

        if (name_len >= CPE_ARRAY_SIZE(name_buf)) {
            CPE_ERROR(mgr->m_em, "ui_data_src: name %s too long!, size=%d", full_file, (int)name_len);
            return NULL;
        }

        memcpy(name_buf, full_file, name_len);
        name_buf[name_len] = 0;

        child = ui_data_src_child_find(parent, name_buf, ui_data_src_type_dir);
        if (child == NULL) {
            child = ui_data_src_create_i(mgr, parent, ui_data_src_type_dir, name_buf);
            if (child == NULL) {
                CPE_ERROR(
                    mgr->m_em, "ui_data_src: create child %s at %s fail",
                    name_buf, ui_data_src_path_dump(&mgr->m_dump_buffer, parent));
                return NULL;
            }
        }

        parent = child;
        full_file = sep + 1;
    }

    cpe_str_dup(name_buf, sizeof(name_buf), full_file);
    if ((p = strrchr(name_buf, '.'))) *p = 0;

    src = ui_data_src_child_find(parent, name_buf, type);
    if (src) {
        assert(0);
        CPE_ERROR(
            mgr->m_em, "ui_data_src: src %s already exist in %s",
            name_buf, ui_data_src_path_dump(&mgr->m_dump_buffer, parent));
        return NULL;
    }
    
    src = ui_data_src_create_i(mgr, parent, type, name_buf);
    if (src == NULL) {
        CPE_ERROR(
            mgr->m_em, "ui_data_src: create src %s at %s fail",
            name_buf, ui_data_src_path_dump(&mgr->m_dump_buffer, parent));
        return NULL;
    }

    return src;
}

ui_data_src_t ui_data_src_create_relative(ui_data_mgr_t mgr, ui_data_src_type_t type, const char * path) {
    char name_buf[128];
    ui_data_src_t parent;
    ui_data_src_t src;
    const char * sep;

    if (path[0] == '/') path++;

    parent = mgr->m_src_root;

    while((sep = strchr(path, '/'))) {
        ui_data_src_t child;
        size_t name_len = sep - path;

        if (*(sep + 1) == 0) break;

        if (name_len >= CPE_ARRAY_SIZE(name_buf)) {
            CPE_ERROR(mgr->m_em, "ui_data_src: name %s too long!, size=%d", path, (int)name_len);
            return NULL;
        }

        memcpy(name_buf, path, name_len);
        name_buf[name_len] = 0;

        child = ui_data_src_child_find(parent, name_buf, ui_data_src_type_dir);
        if (child == NULL) {
            child = ui_data_src_create_i(mgr, parent, ui_data_src_type_dir, name_buf);
            if (child == NULL) {
                CPE_ERROR(
                    mgr->m_em, "ui_data_src: create child %s at %s fail",
                    name_buf, ui_data_src_path_dump(&mgr->m_dump_buffer, parent));
                return NULL;
            }
        }

        parent = child;
        path = sep + 1;
    }

    src = ui_data_src_child_find(parent, path, type);
    if (src) {
        CPE_ERROR(
            mgr->m_em, "ui_data_src: src %s %s already exist in %s",
            ui_data_src_type_name(type), path, ui_data_src_path_dump(&mgr->m_dump_buffer, parent));
        return NULL;
    }
    
    src = ui_data_src_create_i(mgr, parent, type, path);
    if (src == NULL) {
        CPE_ERROR(
            mgr->m_em, "ui_data_src: create src %s at %s fail",
            path, ui_data_src_path_dump(&mgr->m_dump_buffer, parent));
        return NULL;
    }

    return src;
}
