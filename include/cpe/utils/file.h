#ifndef CPE_UTILS_FILE_H
#define CPE_UTILS_FILE_H
#include <stdio.h>
#include "error.h"
#include "memory.h"
#include "stream.h"
#include "buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/*file operations*/
ssize_t file_write_from_buf(const char * file, const void * buf, size_t size, error_monitor_t em);
ssize_t file_write_from_str(const char * file, const char * str, error_monitor_t em);
ssize_t file_write_from_stream(const char * file, read_stream_t stream, error_monitor_t em);

ssize_t file_append_from_buf(const char * file, const void * buf, size_t size, error_monitor_t em);
ssize_t file_append_from_str(const char * file, const char * str, error_monitor_t em);
ssize_t file_append_from_stream(const char * file, read_stream_t stream, error_monitor_t em);

ssize_t file_load_to_buf(char * buf, size_t buf_size, const char * file, error_monitor_t em);
ssize_t file_load_to_buffer(mem_buffer_t buffer, const char * file, error_monitor_t em);
ssize_t file_load_to_stream(write_stream_t stream, const char * file, error_monitor_t em);

int file_rm(const char * file, error_monitor_t em);
int file_exist(const char * file, error_monitor_t em);
ssize_t file_size(const char * file, error_monitor_t em);
int file_copy(const char * output, const char * input, mode_t mode, error_monitor_t em);

/*file stream operations*/
FILE * file_stream_open(const char *path, const char *mode, error_monitor_t em);
void file_stream_close(FILE * fp, error_monitor_t em);
ssize_t file_stream_size(FILE * fp, error_monitor_t em);
ssize_t file_stream_write_from_buf(FILE * fp, const void * buf, size_t size, error_monitor_t em);
ssize_t file_stream_write_from_buffer(FILE * fp, mem_buffer_t buffer, error_monitor_t em);
ssize_t file_stream_write_from_str(FILE * fp, const char * str, error_monitor_t em);
ssize_t file_stream_write_from_stream(FILE * fp, read_stream_t stream, error_monitor_t em);
ssize_t file_stream_load_to_buf(char * buf, size_t size, FILE * fp, error_monitor_t em);
ssize_t file_stream_load_to_buffer(mem_buffer_t buffer, FILE * fp, error_monitor_t em);
ssize_t file_stream_load_to_stream(write_stream_t stream, FILE * fp, error_monitor_t em);

/*file name operations*/
const char * dir_name(const char * input, mem_buffer_t tbuf);
const char * dir_name_ex(const char * input, int level, mem_buffer_t tbuf);
const char * file_name_suffix(const char * input);
int file_name_rename_suffix(char * input, size_t input_capacity, const char * postfix);
const char * file_name_base(const char * input, mem_buffer_t tbuf);
const char * file_name_no_dir(const char * input);
const char * file_name_append_base(mem_buffer_t tbuf, const char * input);
int file_name_normalize(char * input);
    
/*dir operations*/
int dir_exist(const char * path, error_monitor_t em);
int dir_is_empty(const char * path, error_monitor_t em);
int dir_rm(const char * path, error_monitor_t em);
int dir_mk(const char * path, mode_t mode, error_monitor_t em);
int dir_mk_recursion(const char * path, mode_t mode, error_monitor_t em, mem_allocrator_t talloc);
int dir_rm_recursion(const char * path, error_monitor_t em, mem_allocrator_t talloc);

/*dir visit*/
typedef enum dir_visit_next_op {
    dir_visit_next_go
    , dir_visit_next_ignore
    , dir_visit_next_exit
} dir_visit_next_op_t;

typedef struct dir_visitor {
    /*ignore: ignore this dir, but leave will be still called*/
    /*exit: exit direct*/
    dir_visit_next_op_t (*on_dir_enter)(const char * full, const char * base, void * ctx);
    /*exit: exit direct*/
    dir_visit_next_op_t (*on_dir_leave)(const char * full, const char * base, void * ctx);
    /*exit: exit direct*/
    dir_visit_next_op_t (*on_file)(const char * full, const char * base, void * ctx);
} * dir_visitor_t;

void dir_search(
    dir_visitor_t visitor, void * ctx,
    const char * path, int maxLevel,
    error_monitor_t em, mem_allocrator_t talloc);

extern int DIR_DEFAULT_MODE;
extern int FILE_DEFAULT_MODE;

#ifdef __cplusplus
}
#endif

#endif
