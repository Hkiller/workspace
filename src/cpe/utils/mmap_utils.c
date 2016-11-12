#include "cpe/pal/pal_fcntl.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_unistd.h"
#include "cpe/utils/mmap_utils.h"
#include "cpe/utils/file.h"
#include "file_internal.h"

#if defined _WIN32

void * mmap_file_load(const char * file, const char * mod, size_t * size, error_monitor_t em) {
    FILE * fp;
    ssize_t buf_size;
	ssize_t read_size;
	void * r;

    fp = file_stream_open(file, "rb", em);
    if (fp == NULL) {
        CPE_ERROR(
            em, "mmap_file_load: open file %s fail, error=%d (%s)",
            file, errno, strerror(errno));
        return NULL;
    }

    buf_size = file_stream_size(fp, em);
    if (buf_size < 0) {
        CPE_ERROR(
            em, "mmap_file_load: read file %s size, error=%d (%s)",
            file, errno, strerror(errno));
        file_stream_close(fp, em);
        return NULL;
    }

    r = malloc(buf_size);
    if (r == NULL) {
        CPE_ERROR(
            em, "mmap_file_load: malloc file buff, size=%d, error=%d (%s)",
            (int)buf_size, errno, strerror(errno));
        file_stream_close(fp, em);
        return NULL;
    }

	read_size = file_stream_load_to_buf(r, buf_size, fp, em);
    if (read_size != buf_size) {
        CPE_ERROR(
            em, "mmap_file_load: read file %s fail, error=%d (%s)",
            file, errno, strerror(errno));
        file_stream_close(fp, em);
		free(r);
        return NULL;
    };

    file_stream_close(fp, em);

    if (size) *size = buf_size;

    return r;
}

void mmap_unload(void * p, size_t size) {
    free(p);
}

#else
#include <sys/mman.h>

void * mmap_file_load(const char * file, const char * mod, size_t * size, error_monitor_t em) {
    int fd;
    int mode = O_BINARY;
    char c;
    void * r;
    struct stat state_buf;
    const char * p = mod;
    int prot = 0;

    for(c = *p; c; p++, c=*p) {
        if (c == 'r') {
            mode |= O_RDONLY;
            prot |= PROT_READ;
        }
        else if (c == 'w') {
            mode |= O_WRONLY;
            prot |= PROT_WRITE;
        }
        else {
            CPE_ERROR(em, "mmap_file_load: mod %s format error!", mod);
            return NULL;
        }
    }

    fd = open(file, mode, FILE_DEFAULT_MODE); 
    if (fd < 0) {
        CPE_ERROR(
            em, "mmap_file_load: open file %s fail, error=%d (%s)",
            file, errno, strerror(errno));
        return NULL;
    }

    if (fstat(fd, &state_buf) != 0) {
        CPE_ERROR(
            em, "mmap_file_load: state file %s fail, error=%d (%s)",
            file, errno, strerror(errno));
        close(fd);
        return NULL;
    }

    r = mmap(NULL, state_buf.st_size, prot, MAP_FILE | MAP_PRIVATE, fd, 0);
    if ((ptr_int_t)r == -1) {
        CPE_ERROR(
            em, "mmap_file_load: mmap file %s fail, size=%d, error=%d (%s)",
            file, (int)state_buf.st_size, errno, strerror(errno));
        close(fd);
        return NULL;
    }

    close(fd);

    if (size) *size = state_buf.st_size;

    return r;
}

void mmap_unload(void * p, size_t size) {
    munmap(p, size);
}

#endif


