#include "cpe/pal/pal_shm.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/stream.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/aom/aom_shm.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "aom_internal_types.h"

#ifndef _MSC_VER

int aom_shm_init(
    LPDRMETA meta,
    int shm_key, int shm_size, int force, error_monitor_t em)
{
    cpe_shm_id_t shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        if (cpe_shm_errno() != ENOENT) {
            CPE_ERROR(
                em, "shm init: get shm (key=%d) fail, errno=%d (%s)",
                shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
            return -1;
        }        
    }
    else {
        if (force) {
            if (cpe_shm_rm(shmid) == -1) {
                CPE_ERROR(
                    em, "shm init: delete shm (id=%d, key=%d) fail, errno=%d (%s)",
                    shmid, shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
                return -1;
            }
            shmid = -1;
        }
        else {
            CPE_INFO(em, "shm init: shm (key=%d) already exist, reuse!", shm_key);
        }
    }

    if (shmid == -1) {
        shmid = cpe_shm_create(shm_key, shm_size, 0600);
        if (shmid == -1) {
            CPE_ERROR(
                em, "shm init: create shm (key=%d, size=%d) fail, errno=%d (%s)",
                shm_key, shm_size, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
            return -1;
        }
    }

    void * data = cpe_shm_attach(shmid, NULL, 0);
    if (data == NULL) {
        CPE_ERROR(
            em, "shm init: attach shm (key=%d, size=%d) fail, errno=%d (%s)",
            shm_key, shm_size, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    if (aom_obj_mgr_buf_init(meta, data, shm_size, em) != 0) {
        if (cpe_shm_rm(shmid) == -1) {
            CPE_ERROR(
                em, "shm init: delete shm (id=%d, key=%d) fail, errno=%d (%s)",
                shmid, shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        }
        cpe_shm_detach(data);
        return -1;
    }

    CPE_INFO(
        em, "shm init: init shm (id=%d, key=%d) success, size=%d",
        shmid, shm_key, shm_size);

    cpe_shm_detach(data);
    return 0;
}

int aom_shm_info(int shm_key, write_stream_t stream, int ident, error_monitor_t em) {
    cpe_shm_id_t shmid;
    aom_obj_mgr_t aom_obj_mgr;

    shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        CPE_ERROR(
            em, "shm : get shm (key=%d) fail, errno=%d (%s)",
            shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    cpe_shmid_ds shm_info;
    if (cpe_shm_ds_get(shmid, &shm_info) != 0) {
        CPE_ERROR(
            em, "shm : get shm info (key=%d) fail, errno=%d (%s)",
            shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    stream_putc_count(stream, ' ', ident << 2);
    stream_printf(
        stream, "shm info: id=%d, key=%d, size="FMT_SIZE_T", nattach=%d\n",
        shmid, shm_key, shm_info.shm_segsz, shm_info.shm_nattch);
    stream_printf(stream, "\n");

    void * data = cpe_shm_attach(shmid, NULL, 0);
    if (data == NULL) {
        CPE_ERROR(
            em, "shm : attach shm fail, errno=%d (%s)",
            cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    aom_obj_mgr = aom_obj_mgr_create(NULL, data, shm_info.shm_segsz, em);
    if (aom_obj_mgr == NULL) {
        CPE_ERROR(em, "shm : create aom_obj_mgr fail!");
        cpe_shm_detach(data);
        return -1;
    }

    aom_obj_mgr_info(aom_obj_mgr, stream, ident);

    aom_obj_mgr_free(aom_obj_mgr);
    cpe_shm_detach(data);
    return 0;
}

int aom_shm_rm(int shm_key, error_monitor_t em) {
    cpe_shm_id_t shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        if (cpe_shm_errno() == ENOENT) {
            CPE_INFO(
                em, "shm rm: shm(key=%d) not exist, skip.",
                shm_key);
            return 0;
        }
        else {
            CPE_ERROR(
                em, "shm rm: get shm (key=%d) fail, errno=%d (%s)",
                shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
            return -1;
        }        
    }

    if (cpe_shm_rm(shmid) == -1) {
        CPE_ERROR(
            em, "shm rm: delete shm (id=%d, key=%d) fail, errno=%d (%s)",
            shmid, shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    CPE_INFO(
        em, "shm rm: shm(key=%d) remove success.",
        shm_key);

    return 0;
}

int aom_shm_dump(int shm_key, write_stream_t stream, int ident, error_monitor_t em) {
    cpe_shm_id_t shmid;
    aom_obj_mgr_t aom_obj_mgr;
    cpe_shmid_ds shm_info;
    void * data;

    shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        CPE_ERROR(
            em, "shm : get shm (key=%d) fail, errno=%d (%s)",
            shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    if (cpe_shm_ds_get(shmid, &shm_info) != 0) {
        CPE_ERROR(
            em, "shm : get shm info (key=%d) fail, errno=%d (%s)",
            shm_key, cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    data = cpe_shm_attach(shmid, NULL, 0);
    if (data == NULL) {
        CPE_ERROR(
            em, "shm : attach shm fail, errno=%d (%s)",
            cpe_shm_errno(), cpe_shm_errstr(cpe_shm_errno()));
        return -1;
    }

    aom_obj_mgr = aom_obj_mgr_create(NULL, data, shm_info.shm_segsz, em);
    if (aom_obj_mgr == NULL) {
        CPE_ERROR(em, "shm : create aom_obj_mgr fail!");
        cpe_shm_detach(data);
        return -1;
    }

    aom_obj_mgr_free(aom_obj_mgr);
    cpe_shm_detach(data);
    return 0;
}

#endif
