#ifndef _WIN32
#include <assert.h>
#include <inttypes.h>
#include <sys/mman.h> 
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "cpe/utils/rwpipe_mmap.h"

rwpipe_t rwpipe_mmap_init(const char * path, int * r_fd, uint32_t capacity, int force_new, error_monitor_t em) {
    int fd;
    int is_new;
    void * buf;
    rwpipe_t q;

    fd = open(path, O_RDWR);
    if (fd == -1) {
        if (errno != ENOENT) {
            CPE_ERROR(em, "rwpipe_mmap_init: open file %s fail, errno=%d (%s)\n", path, errno, strerror(errno));
            return NULL;
        }
        else {
            is_new = 1;
            fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if (fd == -1) {
                CPE_ERROR(em, "rwpipe_mmap_init: create file %s fail, errno=%d (%s)\n", path, errno, strerror(errno));
                return NULL;
            }

            if (ftruncate(fd, capacity) == -1) {
                CPE_ERROR(em, "rwpipe_mmap_init: trunk new file %s to size %d fail, errno=%d (%s)\n", path, capacity, errno, strerror(errno));
                close(fd);
                return NULL;
            }
        }
    }
    else {
        struct stat statbuf;
        if (fstat(fd, &statbuf) == -1) {
            CPE_ERROR(em, "rwpipe_mmap_init: state file %s fail, errno=%d (%s)\n", path, errno, strerror(errno));
            close(fd);
            return NULL;
        }

        if (force_new) {
            is_new = 1;
        }
        else {
            is_new = 0;
        }

        if (statbuf.st_size != capacity) {
            if (ftruncate(fd, capacity) == -1) {
                CPE_ERROR(em, "rwpipe_mmap_init: trunk exist file %s to size %d fail, errno=%d (%s)\n", path, capacity, errno, strerror(errno));
                close(fd);
                return NULL;
            }
        }
    }

    buf = mmap(NULL, capacity, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);
    if (buf == MAP_FAILED) {
        CPE_ERROR(em, "rwpipe_mmap_init: mmap fail, errno=%d (%s)\n", errno, strerror(errno));
        close(fd);
        return NULL;
    }

    if (is_new) {
        q = rwpipe_init(buf, capacity);
    }
    else {
        q = rwpipe_attach(buf);

        if (rwpipe_total_capacity(q) != capacity) {
            CPE_ERROR(em, "rwpipe_mmap_init: mmap capacity mismatch, re-init!\n");
            q = rwpipe_init(buf, capacity);
        }
    }

    if (r_fd) *r_fd = fd;
    return q;
}

rwpipe_t rwpipe_mmap_attach(const char * path, int * r_fd, error_monitor_t em) {
    int fd;
    void * buf;
    rwpipe_t q;
    struct stat statbuf;

    fd = open(path, O_RDWR);
    if (fd == -1) {
        CPE_ERROR(em, "rwpipe_mmap_init: stat file %s fail, errno=%d (%s)\n", path, errno, strerror(errno));
        return NULL;
    }

    if (fstat(fd, &statbuf) == -1) {
        CPE_ERROR(em, "rwpipe_mmap_init: state file %s fail, errno=%d (%s)\n", path, errno, strerror(errno));
        close(fd);
        return NULL;
    }

    buf = mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, fd, 0);
    if (buf == NULL) {
        CPE_ERROR(em, "rwpipe_mmap_init: mmap fail, errno=%d (%s)\n", errno, strerror(errno));
        close(fd);
        return NULL;
    }

    q = rwpipe_attach(buf);

    if (r_fd) *r_fd = fd;
    return q;
}
#endif
