#include <assert.h>
#include <string.h>
#include "file_internal.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"

ssize_t file_write_from_buf(const char * file, const void * buf, size_t size, error_monitor_t em) {
    ssize_t totalSize;
    FILE * fp;
    
    fp = file_stream_open(file, "wb", em);
    if (fp == NULL) return -1;

    totalSize = file_stream_write_from_buf(fp, buf, size, em);

    file_stream_close(fp, em);

    return totalSize;
}

ssize_t file_write_from_str(const char * file, const char * str, error_monitor_t em) {
    return file_write_from_buf(file, str, strlen(str), em);
}

ssize_t file_write_from_stream(const char * file, read_stream_t stream, error_monitor_t em) {
    ssize_t totalSize;
    FILE * fp;

    fp = file_stream_open(file, "wb", em);
    if (fp == NULL) return -1;

    totalSize = file_stream_write_from_stream(fp, stream, em);

    file_stream_close(fp, em);

    return totalSize;
}

ssize_t file_append_from_buf(const char * file, const void * buf, size_t size, error_monitor_t em) {
    ssize_t totalSize;
    FILE * fp;

    fp = file_stream_open(file, "ab", em);
    if (fp == NULL) return -1;

    totalSize = file_stream_write_from_buf(fp, buf, size, em);

    file_stream_close(fp, em);

    return totalSize;
}

ssize_t file_append_from_str(const char * file, const char * str, error_monitor_t em) {
    return file_append_from_buf(file, str, strlen(str), em);
}

ssize_t file_append_from_stream(const char * file, read_stream_t stream, error_monitor_t em) {
    FILE * fp;
    ssize_t totalSize;

    fp = file_stream_open(file, "ab", em);
    if (fp == NULL) return -1;

    totalSize = file_stream_write_from_stream(fp, stream, em);

    file_stream_close(fp, em);

    return totalSize;
}

ssize_t file_load_to_buf(char * buf, size_t buf_size, const char * file, error_monitor_t em) {
    struct write_stream_mem stream = CPE_WRITE_STREAM_MEM_INITIALIZER(buf, buf_size);
    return file_load_to_stream((write_stream_t)&stream, file, em);
}

ssize_t file_load_to_buffer(mem_buffer_t buffer, const char * file, error_monitor_t em) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    return file_load_to_stream((write_stream_t)&stream, file, em);
}

ssize_t file_load_to_stream(write_stream_t stream, const char * file, error_monitor_t em) {
    FILE * fp;
    ssize_t totalSize;

    fp = file_stream_open(file, "rb", em);
    if (fp == NULL) return -1;

    totalSize = file_stream_load_to_stream(stream, fp, em);

    if (!feof(fp)) {
        totalSize = -1;
    }

    file_stream_close(fp, em);
    return totalSize;
}

ssize_t file_stream_write_from_buf(FILE * fp, const void * buf, size_t size, error_monitor_t em) {
    ssize_t totalSize;
    size_t writeSize;

    totalSize = 0;
    while((writeSize = fwrite(buf, 1, size, fp)) > 0) {
        size -= writeSize;
        totalSize += writeSize;
    }

    if (ferror(fp)) {
        totalSize = -1;
    }

    return totalSize;
}

ssize_t file_stream_write_from_buffer(FILE * fp, mem_buffer_t buffer, error_monitor_t em) {
    ssize_t totalSize;
    size_t writeSize;
    struct mem_buffer_trunk * trunk;

    totalSize = 0;

    for(trunk = mem_buffer_trunk_first(buffer);
        trunk;
        trunk = mem_buffer_trunk_next(trunk))
    {
        char * buf = mem_trunk_data(trunk);
        size_t size = mem_trunk_size(trunk);
        while((writeSize = fwrite(buf, 1, size, fp)) > 0) {
            size -= writeSize;
            totalSize += writeSize;
        }
    }

    if (ferror(fp)) {
        totalSize = -1;
    }

    return totalSize;
}

ssize_t file_stream_write_from_str(FILE * fp, const char * str, error_monitor_t em) {
    return file_stream_write_from_buf(fp, str, strlen(str), em);
}

ssize_t file_stream_write_from_stream(FILE * fp, read_stream_t stream, error_monitor_t em) {
    ssize_t totalSize;
    size_t writeSize;
    size_t writeOkSize;
    size_t size;
    char buf[128];

    totalSize = 0;
    while((size = stream_read(stream, buf, CPE_ARRAY_SIZE(buf))) > 0) {
        writeOkSize = 0;
        while(size > writeOkSize
              && (writeSize = fwrite(buf + writeOkSize, 1, size - writeOkSize, fp)) > 0)
        {
            writeOkSize += writeSize;
        }

        totalSize += writeOkSize;

        if (writeOkSize < size) break;
    }

    if (ferror(fp)) {
        totalSize = -1;
    }

    return totalSize;
}

ssize_t file_stream_load_to_buf(char * buf, size_t size, FILE * fp, error_monitor_t em) {
    struct write_stream_mem stream = CPE_WRITE_STREAM_MEM_INITIALIZER(buf, size);
    return file_stream_load_to_stream((write_stream_t)&stream, fp, em);
}

ssize_t file_stream_load_to_buffer(mem_buffer_t buffer, FILE * fp, error_monitor_t em) {
    struct write_stream_buffer stream = CPE_WRITE_STREAM_BUFFER_INITIALIZER(buffer);
    return file_stream_load_to_stream((write_stream_t)&stream, fp, em);
}

ssize_t file_stream_load_to_stream(write_stream_t stream, FILE * fp, error_monitor_t em) {
    size_t writeSize;
    size_t writeOkSize;
    size_t size;
    ssize_t totalSize;

    size_t exTotalSize;
    char buf[128];

    exTotalSize = 0;
    totalSize = 0;
    while((size = fread(buf, 1, 128, fp)) > 0) {
        exTotalSize += size;
        writeOkSize = 0;
        while(size > writeOkSize
              && (writeSize = stream_write(stream, buf + writeOkSize, size - writeOkSize)) > 0)
        {
            writeOkSize += writeSize;
        }

        totalSize += writeOkSize;
        if (writeOkSize < size)
        {
            break;
        }
    }

    if (ferror(fp)) {
        totalSize = -1;
    }

    return totalSize;
}

int file_exist(const char * path, error_monitor_t em) {
    struct stat buffer;
    int status;
    status = inode_stat_by_path(path, &buffer, ENOENT, em);
    if (status != 0) {
        return 0;
    }

    return S_ISREG(buffer.st_mode);
}

ssize_t file_size(const char * path, error_monitor_t em) {
    struct stat buffer;
    int status;
    status = inode_stat_by_path(path, &buffer, 0, em);
    if (status != 0) {
        return -1;
    }

    if (!S_ISREG(buffer.st_mode)) {
        CPE_ERROR(em, "%s is not file.", path);
        return -1;
    }

    return (ssize_t)buffer.st_size;
}

ssize_t file_stream_size(FILE * fp, error_monitor_t em) {
    struct stat buffer;
    int status;
    status = inode_stat_by_fileno(fileno(fp), &buffer, 0, em);
    if (status != 0) {
        return -1;
    }

    return (ssize_t)buffer.st_size;
}

const char * dir_name(const char * input, mem_buffer_t tbuf) {
    return dir_name_ex(input, 1, tbuf);
}

const char * dir_name_ex(const char * input, int level, mem_buffer_t tbuf) {
    int len;
    assert(level > 0);

    if (input == NULL) return NULL;

    len = (int)strlen(input);

    while(len > 0) {
        char c = input[--len];
        if (c == '/' || c == '\\') {
            --level;
            if (level == 0) {
                char * r = (char *)mem_buffer_alloc(tbuf, len + 1);
                if (len > 1) { memcpy(r, input, len); }
                r[len] = 0; 
                return r;
            }
        }
    }

    return NULL;
}

const char * file_name_suffix(const char * input) {
    int len;

    if (input == NULL) return NULL;

    len = (int)strlen(input);

    while(len > 0) {
        char c = input[--len];
        if (c == '.') return input + len + 1;
        if (c == '/' || c == '\\') return "";
    }

    return input;
}

int file_name_rename_suffix(char * input, size_t input_capacity, const char * postfix) {
    size_t postfix_len = strlen(postfix);
    char * p = strrchr(input, '.');
    if (p == NULL) return -1;

    if ((p - input) + 1 + postfix_len + 1 > input_capacity) return -1;

    memcpy(p + 1, postfix, postfix_len);

    return 0;
}

const char * file_name_no_dir(const char * input) {
    int len;

    if (input == NULL) return NULL;

    len = (int)strlen(input);

    while(len > 0) {
        char c = input[--len];
        if (c == '/' || c == '\\') return input + len + 1;
    }

    return input;
}

int file_name_normalize(char * input) {
    char * sep;
    char * sep_last = input;
    size_t left_len = strlen(input);
    
    while((sep = strchr(sep_last, '/'))) {
        size_t remove_len;
        size_t move_len;
        sep_last = sep + 1;
        while(*sep_last == '/') sep_last++;

        remove_len = (sep_last - sep) - 1;
        if (remove_len == 0) continue;

        move_len = left_len - (sep + 1 - input) - remove_len;
        if (move_len > 0) {
            memmove(sep + 1, sep_last, move_len);
        }
        sep[1 + move_len] = 0;
        left_len -= remove_len;
    }

    if (left_len > 0) {
        if (input[left_len - 1] == '/') {
            input[left_len - 1] = 0;
            left_len--;
        }
    }
    
    return 0;
}

const char *
file_name_base(const char * input, mem_buffer_t tbuf) {
    int len;
    int endPos;
    int beginPos;
    int pos;

    if (input == NULL) return NULL;

    len = (int)strlen(input);
    
    endPos = len + 1;
    beginPos = 0;

    pos = len + 1;
    while(pos > 0) {
        char c = input[--pos];
        if (c == '.') {
            if (endPos == len + 1) {
                endPos = pos;
            }
        }

        if (c == '/' || c == '\\') {
            beginPos = pos + 1;
            break;
        }
    }

    if (endPos == len + 1) {
        return input + beginPos;
    }
    else {
        char * r;
        int resultLen = endPos - beginPos;
        if (resultLen <= 0) return "";

        assert(beginPos < endPos);
        r = (char *)mem_buffer_alloc(tbuf, resultLen + 1);
        memcpy(r, input + beginPos, resultLen);
        r[resultLen] = 0;
        return r;
    }
}

const char * file_name_append_base(mem_buffer_t tbuf, const char * input) {
    if (mem_buffer_size(tbuf) == 0) {
        return file_name_base(input, tbuf);
    }
    else {
        mem_buffer_set_size(tbuf, mem_buffer_size(tbuf) - 1);
        return file_name_base(input, tbuf);
    }
}

int file_copy(const char * output, const char * input, mode_t mode, error_monitor_t em) {
    ssize_t totalSize;
    size_t writeSize;
    size_t writeOkSize;
    size_t size;
    FILE * fp_i;
    FILE * fp_o;
    char buf[512];

    fp_i = file_stream_open(input, "rb", em);
    if (fp_i == NULL) return -1;

    fp_o = file_stream_open(output, "wb", em);
    if (fp_o == NULL) {
        file_stream_close(fp_i, em);
        return -1;
    }

    totalSize = 0;
    while((size = fread(buf, 1, sizeof(buf), fp_i)) > 0) {
        writeOkSize = 0;
        while(size > writeOkSize
              && (writeSize = fwrite(buf + writeOkSize, 1, size - writeOkSize, fp_o)) > 0)
        {
            writeOkSize += writeSize;
        }

        totalSize += writeOkSize;

        if (writeOkSize < size) break;
    }

    if (ferror(fp_o)) {
        totalSize = -1;
    }

    file_stream_close(fp_i, em);
    file_stream_close(fp_o, em);
    
    return (int)totalSize;
}
