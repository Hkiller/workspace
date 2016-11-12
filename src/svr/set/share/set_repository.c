#include "cpe/pal/pal_unistd.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/utils/file.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream_mem.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/cfg/cfg_read.h"
#include "gd/app/app_context.h"
#include "svr/set/share/set_chanel.h"
#include "svr/set/share/set_repository.h"
#include "protocol/svr/set/set_share_chanel.h"

set_chanel_t set_chanel_shm_attach(int shmid, error_monitor_t em) {
    int h;
    SVR_SET_CHANEL * chanel;

    h = cpe_shm_get(shmid);
    if (h == -1) {
        CPE_ERROR(em, "set_chanel_shm_attach: get shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    chanel = shmat(h, NULL, 0);
    if (chanel == NULL) {
        CPE_ERROR(em, "set_chanel_shm_attach: attach shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    if (chanel->magic != 132523426) {
        CPE_ERROR(em, "set_chanel_shm_attach: magic mismatch!");
        shmdt(chanel);
        return NULL;
    }

    return (set_chanel_t)chanel;
}

set_chanel_t set_chanel_shm_init(int shmid, uint32_t w_capacity, uint32_t r_capacity, error_monitor_t em) {
    int h;
    int is_new;
    SVR_SET_CHANEL * chanel;
    uint32_t capacity = sizeof(SVR_SET_CHANEL) + w_capacity + r_capacity;

TRY_AGAIN:
    is_new = 0;

    h = cpe_shm_get(shmid);
    if (h == -1) {
        if (errno != ENOENT) {
            CPE_ERROR(em, "set_chanel_shm_init: get shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
            return NULL;
        }
        else {
            is_new = 1;
            h = cpe_shm_create(shmid, capacity, 0600);
            if (h == -1) {
                CPE_ERROR(em, "set_chanel_shm_init: init shm (id=%d, size=%d) fail, errno=%d (%s)\n", shmid, (int)capacity, errno, strerror(errno));
                return NULL;
            }
        }
    }

    chanel = shmat(h, NULL, 0);
    if (chanel == NULL || ((int)(ptr_int_t)chanel) == -1) {
        CPE_ERROR(em, "set_chanel_shm_init: attach shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
        return NULL;
    }

    if (is_new) {
        chanel->magic = 132523426;
        chanel->r.capacity = r_capacity;
        chanel->r.flags = 0;
        chanel->r.begin = sizeof(SVR_SET_CHANEL);
        chanel->r.rp = 0;
        chanel->r.wp = 0;

        chanel->w.capacity = w_capacity;
        chanel->w.flags = 0;
        chanel->w.begin = sizeof(SVR_SET_CHANEL) + r_capacity;
        chanel->w.rp = 0;
        chanel->w.wp = 0;
    }
    else {
        if (chanel->r.capacity != r_capacity || chanel->w.capacity != w_capacity) {
            CPE_ERROR(em, "set_chanel_shm_init: shm capacity mismatch, delete!\n");

            if (shmdt(chanel) != 0) {
                CPE_ERROR(em, "set_chanel_shm_init: detach shm %p (id=%d) fail, errno=%d (%s)\n", chanel, shmid, errno, strerror(errno));
                return NULL;
            }

            if (shmctl(h, IPC_RMID, NULL) == -1) {
                CPE_ERROR(em, "set_chanel_shm_init: remove shm (id=%d) fail, errno=%d (%s)\n", shmid, errno, strerror(errno));
                return NULL;
            }

            goto TRY_AGAIN;
        }
    }

    return (set_chanel_t)chanel;
}

void set_repository_chanel_detach(set_chanel_t chanel, error_monitor_t em) {
    if (shmdt(chanel) != 0) {
        CPE_ERROR(em, "set_repository_chanel_detach: shmdt fail, errno=%d (%s)\n", errno, strerror(errno));
    }
}
