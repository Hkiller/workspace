#ifndef CPE_ZIP_FILE_H_INCLEDED
#define CPE_ZIP_FILE_H_INCLEDED
#include "cpe/utils/file.h"
#include "cpe/utils/error.h"
#include "cpe/utils/memory.h"
#include "zip_types.h"

#ifdef __cplusplus
extern "C" {
#endif

cpe_unzip_context_t cpe_unzip_context_create(const char * path, mem_allocrator_t alloc, error_monitor_t em);
void cpe_unzip_context_free(cpe_unzip_context_t unzc);

typedef struct cpe_unzip_file_visitor {
    /*ignore: ignore this dir, but leave will be still called*/
    /*exit: exit direct*/
    dir_visit_next_op_t (*on_dir_enter)(const char * full, cpe_unzip_dir_t d, void * ctx);
    /*exit: exit direct*/
    dir_visit_next_op_t (*on_dir_leave)(const char * full, cpe_unzip_dir_t d, void * ctx);
    /*exit: exit direct*/
    dir_visit_next_op_t (*on_file)(const char * full, cpe_unzip_file_t f, void * ctx);
} * cpe_unzip_file_visitor_t;

void cpe_unzip_dir_search(
    cpe_unzip_file_visitor_t visitor, void * ctx,
    cpe_unzip_dir_t d, int maxLevel,
    error_monitor_t em, mem_allocrator_t talloc);

cpe_unzip_file_t cpe_unzip_file_find(cpe_unzip_context_t context, const char * path, error_monitor_t em);
const char * cpe_unzip_file_name(cpe_unzip_file_t zf);
const char * cpe_unzip_file_path(mem_buffer_t buffer, cpe_unzip_file_t zf);

cpe_unzip_dir_t cpe_unzip_dir_find(cpe_unzip_context_t context, const char * path, error_monitor_t em);
const char * cpe_unzip_dir_name(cpe_unzip_dir_t d);
const char * cpe_unzip_dir_path(mem_buffer_t buffer, cpe_unzip_dir_t d);

ssize_t cpe_unzip_file_load_to_buffer(mem_buffer_t buffer, cpe_unzip_file_t zf, error_monitor_t em);
ssize_t cpe_unzip_file_load_to_buf(char * buf, size_t size, cpe_unzip_file_t zf, error_monitor_t em);


#ifdef __cplusplus
}
#endif

#endif
