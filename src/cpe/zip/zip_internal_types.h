#ifndef CPE_ZIP_INTERNAL_TYPES_H_INCLEDED
#define CPE_ZIP_INTERNAL_TYPES_H_INCLEDED
#include "cpe/pal/pal_queue.h"
#include "cpe/utils/memory.h"
#include "cpe/zip/zip_types.h"
#include "unzip.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef TAILQ_HEAD(cpe_unzip_dir_list, cpe_unzip_dir) cpe_unzip_dir_list_t;
typedef TAILQ_HEAD(cpe_unzip_file_list, cpe_unzip_file) cpe_unzip_file_list_t;

struct cpe_unzip_context {
    mem_allocrator_t m_alloc;
    unzFile m_zip_file;
    unz_global_info64 m_global_info;
    cpe_unzip_dir_t m_root;
};

struct cpe_unzip_dir {
    cpe_unzip_context_t m_context;
    cpe_unzip_dir_t m_parent_dir;
    TAILQ_ENTRY(cpe_unzip_dir) m_next_dir;
    cpe_unzip_dir_list_t m_child_dirs;
    cpe_unzip_file_list_t m_child_files;
    char m_name[64];
};

struct cpe_unzip_file {
    cpe_unzip_context_t m_context;
    cpe_unzip_dir_t m_parent_dir;
    TAILQ_ENTRY(cpe_unzip_file) m_next_file;
    char m_name[64];
    unz_file_info64 m_file_info;
};

#ifdef __cplusplus
}
#endif

#endif
