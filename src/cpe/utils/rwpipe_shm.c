#include <assert.h>
#include "cpe/pal/pal_shm.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_string.h"
#include "cpe/utils/rwpipe_shm.h"

#if ! defined _WIN32 && ! defined ANDROID

rwpipe_t rwpipe_shm_init_by_path(const char * key_file, uint32_t capacity, int force_new, error_monitor_t em) {
    int shmid = cpe_shm_key_gen(key_file, 'a');
    if (shmid == -1) {
        CPE_ERROR(em, "rwpipe_shm_init_by_path: shm_key_gen at %s fail!", key_file);
        return NULL;
    }
    return rwpipe_shm_init(shmid, capacity, force_new, em);
}

rwpipe_t rwpipe_shm_attach_by_path(const char * key_file, error_monitor_t em) {
    int shmid = cpe_shm_key_gen(key_file, 'a');
    if (shmid == -1) {
        CPE_ERROR(em, "rwpipe_shm_attach_by_path: shm_key_gen at %s fail!", key_file);
        return NULL;
    }
    return rwpipe_shm_attach(shmid, em);
}

rwpipe_t rwpipe_shm_init(int shmid, uint32_t capacity, int force_new, error_monitor_t em) {
    int h;
    int is_new;
    void * buf;
    rwpipe_t q;

TRY_AGAIN:
    h = cpe_shm_get(shmid);
    if (h == -1) {
        if (errno != ENOENT) {
            CPE_ERROR(em, "rwpipe_shm_init: get shm (key=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
            return NULL;
        }
        else {
            is_new = 1;
            h = cpe_shm_create(shmid, capacity, 0600);
            if (h == -1) {
                CPE_ERROR(em, "rwpipe_shm_init: init shm (key=%d, size=%d) fail, errno=%d (%s)\n", shmid, (int)capacity, errno, strerror(errno));
                return NULL;
            }
        }
    }
    else {
        if (force_new) {
            cpe_shm_rm(h);

            is_new = 1;
            h = cpe_shm_create(shmid, capacity, 0600);
            if (h == -1) {
                CPE_ERROR(em, "rwpipe_shm_init: init shm (key=%d, size=%d) fail, errno=%d (%s)\n", shmid, (int)capacity, errno, strerror(errno));
                return NULL;
            }
        }
        else {
            is_new = 0;
        }
    }

    buf = shmat(h, NULL, 0);
    if (buf == NULL) {
        CPE_ERROR(em, "rwpipe_shm_init: attach shm (key=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    if (is_new) {
        q = rwpipe_init(buf, capacity);
    }
    else {
        q = rwpipe_attach(buf);

        if (rwpipe_total_capacity(q) != capacity) {
            CPE_ERROR(em, "rwpipe_shm_init: shm capacity mismatch, delete!\n");
            goto DELETE_AND_TRY_AGAIN;
        }
    }

    return q;

DELETE_AND_TRY_AGAIN:
    if (shmctl(h, IPC_RMID, NULL) == -1) {
        CPE_ERROR(em, "rwpipe_shm_init: remove shm (key=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    goto TRY_AGAIN;
}

rwpipe_t rwpipe_shm_attach(int shmid, error_monitor_t em) {
    int h;
    void * buf;
    rwpipe_t q;

    h = shmget(shmid, 0, 066);
    if (h == -1) {
        CPE_ERROR(em, "rwpipe_shm_attach: get shm (key=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    buf = shmat(h, NULL, 0);
    if (buf == NULL) {
        CPE_ERROR(em, "rwpipe_shm_attach: attach shm (key=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    q = rwpipe_attach(buf);
    if (q == NULL) {
        CPE_ERROR(em, "rwpipe_shm_attach: attach fail!\n");
        return NULL;
    }

    return q;
}

#endif
