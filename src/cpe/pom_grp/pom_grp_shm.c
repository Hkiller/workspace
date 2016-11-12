#include "cpe/pal/pal_shm.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/stream.h"
#include "cpe/pom/pom_manage.h"
#include "cpe/pom_grp/pom_grp_shm.h"
#include "cpe/pom_grp/pom_grp_cfg.h"
#include "cpe/pom_grp/pom_grp_obj_mgr.h"
#include "pom_grp_internal_types.h"

#ifndef _MSC_VER

int pom_grp_shm_init(
    LPDRMETALIB metalib, pom_grp_meta_t grp_meta,
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
            CPE_ERROR(em, "shm init: shm (key=%d) already exist!", shm_key);
            return -1;
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

    if (pom_grp_obj_mgr_buf_init(metalib, grp_meta, data, shm_size, em) != 0) {
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

int pom_grp_shm_info(int shm_key, write_stream_t stream, int ident, error_monitor_t em) {
    cpe_shm_id_t shmid;
    pom_grp_obj_mgr_t pom_grp_obj_mgr;

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

    pom_grp_obj_mgr = pom_grp_obj_mgr_create(NULL, data, shm_info.shm_segsz, em);
    if (pom_grp_obj_mgr == NULL) {
        CPE_ERROR(em, "shm : create pom_grp_obj_mgr fail!");
        cpe_shm_detach(data);
        return -1;
    }

    pom_mgr_dump_page_info(stream, pom_grp_obj_mgr->m_omm, ident);
    pom_mgr_dump_alloc_info(stream, pom_grp_obj_mgr->m_omm, ident);
    pom_grp_obj_mgr_info(pom_grp_obj_mgr, stream, ident);

    pom_grp_obj_mgr_free(pom_grp_obj_mgr);
    cpe_shm_detach(data);
    return 0;
}

int pom_grp_shm_rm(int shm_key, error_monitor_t em) {
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

int pom_grp_shm_dump(int shm_key, write_stream_t stream, int ident, error_monitor_t em) {
    cpe_shm_id_t shmid;
    pom_grp_obj_mgr_t pom_grp_obj_mgr;
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

    pom_grp_obj_mgr = pom_grp_obj_mgr_create(NULL, data, shm_info.shm_segsz, em);
    if (pom_grp_obj_mgr == NULL) {
        CPE_ERROR(em, "shm : create pom_grp_obj_mgr fail!");
        cpe_shm_detach(data);
        return -1;
    }

    if (pom_grp_obj_cfg_dump_all_to_stream(stream, pom_grp_obj_mgr, em) != 0) {
        CPE_ERROR(em, "shm : dump fail!");
    }

    pom_grp_obj_mgr_free(pom_grp_obj_mgr);
    cpe_shm_detach(data);
    return 0;
}

#endif
