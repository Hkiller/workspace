#include <assert.h>
#include "cpe/pal/pal_platform.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "svr/set/share/set_utils.h"
#include "svr/set/stub/set_svr_stub_buff.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "set_svr_stub_internal_ops.h"

struct set_svr_stub_buff_head {
    uint16_t m_magic;
    uint16_t m_version;
    uint8_t m_init;
    uint8_t m_shm_key;
    uint16_t m_data_start;
    uint64_t m_capacity;
    char m_name[64];
};

#define SET_SVR_STUB_BUFF_HEAD_MAGIC ((uint16_t)0x412d3a24u)

#define STUB_BUFF_HEAD(__buf) ((struct set_svr_stub_buff_head *)(__buf)->m_buff)

static void set_svr_stub_buff_destory(set_svr_stub_t stub, set_svr_stub_buff_t buff);
static void set_svr_stub_buff_free(set_svr_stub_t stub, set_svr_stub_buff_t buff);
static int8_t set_svr_stub_buff_find_shm_key(set_svr_stub_t stub);

set_svr_stub_buff_t set_svr_stub_buff_check_create(set_svr_stub_t stub, const char * name, uint64_t capacity) {
    set_svr_stub_buff_t buff;
    struct set_svr_stub_buff_head * head;
    size_t head_size;
    int8_t shm_key = 0;

    buff = set_svr_stub_buff_find(stub, name);
    if (buff) {
        if (STUB_BUFF_HEAD(buff)->m_capacity != capacity) {
            CPE_INFO(
                stub->m_em, "%s: buff_check_create: capacity "FMT_UINT64_T" and "FMT_UINT64_T" mismatch, destory!",
                set_svr_stub_name(stub), STUB_BUFF_HEAD(buff)->m_capacity, capacity);
            set_svr_stub_buff_destory(stub, buff);
            buff = NULL;
        }
    }

    if (buff) return buff;

    buff = mem_alloc(stub->m_alloc, sizeof(struct set_svr_stub_buff));
    if (buff == NULL) {
        CPE_ERROR(stub->m_em, "%s: buff_check_create: alloc buff fail!", set_svr_stub_name(stub));
        return NULL;
    }

    head_size = sizeof(struct set_svr_stub_buff_head);
    CPE_PAL_ALIGN_DFT(head_size);

    if (stub->m_use_shm) {
        int h;

        buff->m_buff_type = set_svr_stub_buff_type_shm;

        shm_key = set_svr_stub_buff_find_shm_key(stub);
        if (shm_key < 0) {
            CPE_ERROR(stub->m_em, "%s: buff_check_create: find shm key char fail!", set_svr_stub_name(stub));
            mem_free(stub->m_alloc, buff);
            return NULL;
        }

        buff->m_shm_id = set_shm_key_get(set_svr_svr_info_svr_type_id(stub->m_svr_type), stub->m_svr_id, shm_key);
        if (buff->m_shm_id == -1) {
            CPE_ERROR(
                stub->m_em, "%s: buff_check_create: gen shm key fail, error=%d (%s)!",
                set_svr_stub_name(stub), errno, strerror(errno));
            mem_free(stub->m_alloc, buff);
            return NULL;
        }

    SHM_CREATE:
        h = cpe_shm_create(buff->m_shm_id, head_size + capacity, 0600);
        if (h == -1) {
            if (errno == EEXIST) {
                h = cpe_shm_get(buff->m_shm_id);
                if (h == -1) {
                    CPE_ERROR(
                        stub->m_em, "%s: buff_check_create: shmid=%d already exist, get for remove fail, errno=%d (%s)",
                        set_svr_stub_name(stub), buff->m_shm_id, errno, strerror(errno));
                    mem_free(stub->m_alloc, buff);
                    return NULL;
                }

                if (cpe_shm_rm(h) == -1) {
                    CPE_ERROR(
                        stub->m_em, "%s: buff_check_create: shmid=%d already exist, remove fail, errno=%d (%s)",
                        set_svr_stub_name(stub), buff->m_shm_id, errno, strerror(errno));
                    mem_free(stub->m_alloc, buff);
                    return NULL;
                }
                else {
                    CPE_ERROR(
                        stub->m_em, "%s: buff_check_create: shmid=%d already exist, remove and try again",
                        set_svr_stub_name(stub), buff->m_shm_id);
                    goto SHM_CREATE;
                }
            }
            else {
                CPE_ERROR(
                    stub->m_em, "%s: buff_check_create: create shm fai, shmid=%d, capacity=%.2fmb, errno=%d (%s)",
                    set_svr_stub_name(stub), buff->m_shm_id, (head_size + capacity) / 1024.0 / 1024.0, errno, strerror(errno));
                mem_free(stub->m_alloc, buff);
                return NULL;
            }
        }

        buff->m_buff = cpe_shm_attach(h, NULL, 0);
        if (buff->m_buff == NULL) {
            CPE_ERROR(
                stub->m_em, "%s: buff_check_create: attach shm fail, error=%d (%s)!",
                set_svr_stub_name(stub), errno, strerror(errno));
            cpe_shm_rm(h);
            mem_free(stub->m_alloc, buff);
            return NULL;
        }
    }
    else {
        buff->m_buff_type = set_svr_stub_buff_type_mem;
        buff->m_shm_id = -1;
        buff->m_buff = mem_alloc(stub->m_alloc, head_size + capacity);
        if (buff->m_buff == NULL) {
            CPE_ERROR(
                stub->m_em, "%s: buff_check_create: alloc data buff fail, size="FMT_UINT64_T"!",
                set_svr_stub_name(stub), (head_size + capacity));
            mem_free(stub->m_alloc, buff);
            return NULL;
        }

    }
    TAILQ_INSERT_TAIL(&stub->m_buffs, buff, m_next);

    head = STUB_BUFF_HEAD(buff);
    head->m_magic = SET_SVR_STUB_BUFF_HEAD_MAGIC;
    head->m_version = 1;
    head->m_init = 0;
    head->m_shm_key = shm_key;
    head->m_data_start = head_size;
    head->m_capacity = capacity;
    cpe_str_dup(head->m_name, sizeof(head->m_name), name);

    if (stub->m_use_shm) {
        set_svr_stub_write_pidfile(stub);
    }

    if (stub->m_debug) {
        switch(buff->m_buff_type) {
        case set_svr_stub_buff_type_shm:
            CPE_INFO(
                stub->m_em, "%s: buff_check_create: buff %s create at shm success, shmid=%d, capacity=%.2fm!",
                set_svr_stub_name(stub), head->m_name, buff->m_shm_id, head->m_capacity / 1024.0 / 1024.0);
            break;
        case set_svr_stub_buff_type_mem:
            CPE_INFO(
                stub->m_em, "%s: buff_check_create: buff %s create at mem success, capacity=%.2fm!",
                set_svr_stub_name(stub), head->m_name, head->m_capacity / 1024.0 / 1024.0);
            break;
        }
    }

    return buff;
}

set_svr_stub_buff_t set_svr_stub_buff_shm_attach(set_svr_stub_t stub, int shmid) {
    int h;
    set_svr_stub_buff_t buff;
    struct set_svr_stub_buff_head * head;

    h = cpe_shm_get(shmid);
    if (h == -1) {
        CPE_ERROR(
            stub->m_em, "%s: buff_shm_attach: get shm fai, shmid=%d, errno=%d (%s)",
            set_svr_stub_name(stub), shmid, errno, strerror(errno));
        return NULL;
    }

    buff = mem_alloc(stub->m_alloc, sizeof(struct set_svr_stub_buff));
    if (buff == NULL) {
        CPE_ERROR(stub->m_em, "%s: buff_shm_attach: alloc buff fail!", set_svr_stub_name(stub));
        return NULL;
    }

    buff->m_buff = cpe_shm_attach(h, NULL, 0);
    if (buff->m_buff == NULL) {
        CPE_ERROR(
            stub->m_em, "%s: buff_shm_attach: attach shm fail, error=%d (%s)!",
            set_svr_stub_name(stub), errno, strerror(errno));
        mem_free(stub->m_alloc, buff);
        return NULL;
    }

    buff->m_buff_type = set_svr_stub_buff_type_shm;
    buff->m_shm_id = shmid;

    head = STUB_BUFF_HEAD(buff);

    if (head->m_magic != SET_SVR_STUB_BUFF_HEAD_MAGIC) {
        CPE_ERROR(stub->m_em, "%s: buff_shm_attach: shm buff magic mismatch!", set_svr_stub_name(stub));
        cpe_shm_detach(buff->m_buff);
        mem_free(stub->m_alloc, buff);
        return NULL;
    }

    if (head->m_version != 1) {
        CPE_ERROR(stub->m_em, "%s: buff_shm_attach: shm buff version mismatch!", set_svr_stub_name(stub));
        cpe_shm_detach(buff->m_buff);
        mem_free(stub->m_alloc, buff);
        return NULL;
    }

    TAILQ_INSERT_TAIL(&stub->m_buffs, buff, m_next);

    if (stub->m_debug) {
        CPE_INFO(
            stub->m_em, "%s: buff_shm_attach: attach buff %s success, shmid=%d, capacity=%.2fm!",
            set_svr_stub_name(stub), head->m_name, shmid, head->m_capacity / 1024.0 / 1024.0);
    }

    return buff;
}

set_svr_stub_buff_t set_svr_stub_buff_shm_save(set_svr_stub_t stub, int shmid) {
    set_svr_stub_buff_t buff;

    buff = mem_alloc(stub->m_alloc, sizeof(struct set_svr_stub_buff));
    if (buff == NULL) {
        CPE_ERROR(stub->m_em, "%s: buff_shm_attach: alloc buff fail!", set_svr_stub_name(stub));
        return NULL;
    }

    buff->m_buff = NULL;
    buff->m_buff_type = set_svr_stub_buff_type_shm;
    buff->m_shm_id = shmid;

    if (stub->m_debug) {
        CPE_INFO(
            stub->m_em, "%s: buff_shm_attach: save shm buff success, shmid=%d!",
            set_svr_stub_name(stub), shmid);
    }

    return buff;
}

set_svr_stub_buff_t set_svr_stub_buff_find(set_svr_stub_t stub, const char * name) {
    set_svr_stub_buff_t buff;
    TAILQ_FOREACH(buff, &stub->m_buffs, m_next) {
        if (STUB_BUFF_HEAD(buff) && strcmp(STUB_BUFF_HEAD(buff)->m_name, name) == 0) {
            return buff;
        }
    }

    return NULL;
}

uint8_t set_svr_stub_buff_is_init(set_svr_stub_buff_t buff) {
    assert(STUB_BUFF_HEAD(buff));
    return STUB_BUFF_HEAD(buff)->m_init;
}

void set_svr_stub_buff_set_init(set_svr_stub_buff_t buff, uint8_t is_init) {
    assert(STUB_BUFF_HEAD(buff));
    STUB_BUFF_HEAD(buff)->m_init = is_init;
}

uint64_t set_svr_stub_buff_capacity(set_svr_stub_buff_t buff) {
    assert(STUB_BUFF_HEAD(buff));
    return STUB_BUFF_HEAD(buff)->m_capacity;
}

void * set_svr_stub_buff_data(set_svr_stub_buff_t buff) {
    assert(STUB_BUFF_HEAD(buff));
    return ((char*)buff->m_buff) + STUB_BUFF_HEAD(buff)->m_data_start;
}

void set_svr_stub_buff_free_all(set_svr_stub_t stub) {
    while(!TAILQ_EMPTY(&stub->m_buffs)) {
        set_svr_stub_buff_free(stub, TAILQ_FIRST(&stub->m_buffs));
    }
}

static void set_svr_stub_buff_destory(set_svr_stub_t stub, set_svr_stub_buff_t buff) {
    TAILQ_REMOVE(&stub->m_buffs, buff, m_next);

    switch(buff->m_buff_type) {
    case set_svr_stub_buff_type_mem:
        assert(buff->m_buff);
        mem_free(stub->m_alloc, buff->m_buff);
        break;
    case set_svr_stub_buff_type_shm: {
        int h;

        if (buff->m_buff) {
            if (cpe_shm_detach(buff->m_buff) != 0) {
                CPE_ERROR(
                    stub->m_em, "%s: buff %s destory:  shm detach fail, error=%d (%s)",
                    set_svr_stub_name(stub), STUB_BUFF_HEAD(buff)->m_name, errno, strerror(errno));
            }
        }

        h = cpe_shm_get(buff->m_shm_id);
        if (h == -1) {
            CPE_ERROR(
                stub->m_em, "%s: buff (detached) destory: shm get (id=%d) fail, errno=%d (%s)",
                set_svr_stub_name(stub), buff->m_shm_id, errno, strerror(errno));
            break;
        }
        
        if (cpe_shm_rm(h) != 0) {
            CPE_ERROR(
                stub->m_em, "%s: buff (detached) destory:  shm rm (id=%d, h=%d) fail, error=%d (%s)",
                set_svr_stub_name(stub), buff->m_shm_id, h, errno, strerror(errno));
        }

        break;
    }
    default:
        CPE_ERROR(stub->m_em, "%s: buff_free_all: unknown buff type %d", set_svr_stub_name(stub), buff->m_buff_type);
        break;
    }

    mem_free(stub->m_alloc, buff);
}

static void set_svr_stub_buff_free(set_svr_stub_t stub, set_svr_stub_buff_t buff) {
    TAILQ_REMOVE(&stub->m_buffs, buff, m_next);

    switch(buff->m_buff_type) {
    case set_svr_stub_buff_type_mem:
        assert(buff->m_buff);
        mem_free(stub->m_alloc, buff->m_buff);
        break;
    case set_svr_stub_buff_type_shm:
        if (buff->m_buff) {
            if (cpe_shm_detach(buff->m_buff) != 0) {
                CPE_ERROR(
                    stub->m_em, "%s: buff %s destory:  shm detach fail, error=%d (%s)",
                    set_svr_stub_name(stub), STUB_BUFF_HEAD(buff)->m_name, errno, strerror(errno));
            }
        }
        break;
    default:
        CPE_ERROR(stub->m_em, "%s: buff_free_all: unknown buff type %d", set_svr_stub_name(stub), buff->m_buff_type);
        break;
    }

    mem_free(stub->m_alloc, buff);
}

static int8_t set_svr_stub_buff_find_shm_key(set_svr_stub_t stub) {
    int8_t key;
    for(key = 'l'; key <= 'z'; ++key) {
        set_svr_stub_buff_t buff;
        int found = 0;
        TAILQ_FOREACH(buff, &stub->m_buffs, m_next) {
            assert(STUB_BUFF_HEAD(buff));
            if (STUB_BUFF_HEAD(buff)->m_shm_key == key) {
                found = 1;
                break;
            }
        }

        if (!found) return key;
    }

    return -1;
}
