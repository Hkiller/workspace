#include <assert.h>
#include "cpe/dr/dr_ctypes_op.h"
#include "cpe/cfg/cfg_read.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "bpg_pkg_internal_types.h"

static uint32_t bpg_pkg_dsp_node_hash(const struct bpg_pkg_dsp_node * node) {
    return node->m_cmd;
}

static int bpg_pkg_dsp_node_cmp(const struct bpg_pkg_dsp_node * l, const struct bpg_pkg_dsp_node * r) {
    return l->m_cmd == r->m_cmd;
}

struct bpg_pkg_dsp_node *
bpg_pkg_dsp_node_create_to_string(mem_allocrator_t alloc, int32_t rsp_cmd, const char * str) {
    struct bpg_pkg_dsp_node * node = mem_alloc(alloc, sizeof(struct bpg_pkg_dsp_node));
    if (node == NULL) return NULL;

    node->m_type = bpg_pkg_dsp_to_str;
    node->m_cmd = rsp_cmd;
    node->m_target.m_to_str = cpe_hs_create(alloc, str);

    if (node->m_target.m_to_str == NULL) {
        mem_free(alloc, node);
        return NULL;
    }

    cpe_hash_entry_init(&node->m_hh);

    return node;
}

struct bpg_pkg_dsp_node *
bpg_pkg_dsp_node_create_to_cmd(mem_allocrator_t alloc, int32_t rsp_cmd, uint32_t cmd) {
    struct bpg_pkg_dsp_node * node = mem_alloc(alloc, sizeof(struct bpg_pkg_dsp_node));
    if (node == NULL) return NULL;

    node->m_type = bpg_pkg_dsp_to_cmd;
    node->m_cmd = rsp_cmd;
    node->m_target.m_to_cmd = rsp_cmd;
    cpe_hash_entry_init(&node->m_hh);

    return node;
}

static void bpg_pkg_dsp_node_free(mem_allocrator_t alloc, struct bpg_pkg_dsp_node * node) {
    switch(node->m_type) {
    case bpg_pkg_dsp_to_cmd:
        break;
    case bpg_pkg_dsp_to_str:
        mem_free(alloc, (void *)node->m_target.m_to_str);
        break;
    }

    mem_free(alloc, node);
}

static int bpg_pkg_dsp_node_dispatch(struct bpg_pkg_dsp_node * node, dp_req_t body, error_monitor_t em) {
    switch(node->m_type) {
    case bpg_pkg_dsp_to_cmd:
        return dp_dispatch_by_numeric(
            node->m_target.m_to_cmd, dp_req_mgr(body), body, em) == 0
            ? 0
            : -1;
    case bpg_pkg_dsp_to_str:
        return dp_dispatch_by_string(
            node->m_target.m_to_str, dp_req_mgr(body), body, em) == 0
            ? 0
            : -1;
    }

    CPE_ERROR(em, "bpg_pkg_dsp: unknown dispatch type %d", node->m_type);

    return -1;
}

bpg_pkg_dsp_t bpg_pkg_dsp_create(mem_allocrator_t alloc) {
    bpg_pkg_dsp_t dsp;

    dsp = mem_alloc(alloc, sizeof(struct bpg_pkg_dsp));
    if (dsp == NULL) return NULL;

    dsp->m_alloc = alloc;
    dsp->m_dft_dsp = NULL;

    if (cpe_hash_table_init(
            &dsp->m_cmd_dsp,
            alloc,
            (cpe_hash_fun_t) bpg_pkg_dsp_node_hash,
            (cpe_hash_eq_t) bpg_pkg_dsp_node_cmp,
            CPE_HASH_OBJ2ENTRY(bpg_pkg_dsp_node, m_hh),
            -1) != 0)
    {
        mem_free(alloc, dsp);
        return NULL;
    }

    return dsp;
}

void bpg_pkg_dsp_free(bpg_pkg_dsp_t dsp) {
    struct cpe_hash_it it;
    struct bpg_pkg_dsp_node * node;

    cpe_hash_it_init(&it, &dsp->m_cmd_dsp);

    node = cpe_hash_it_next(&it);
    while(node) {
        struct bpg_pkg_dsp_node * next = cpe_hash_it_next(&it);
        bpg_pkg_dsp_node_free(dsp->m_alloc, node);
        node = next;
    }

    if (dsp->m_dft_dsp) {
        bpg_pkg_dsp_node_free(dsp->m_alloc, dsp->m_dft_dsp);
    }

    cpe_hash_table_fini(&dsp->m_cmd_dsp);

    mem_free(dsp->m_alloc, dsp);
}

int bpg_pkg_dsp_dispatch(bpg_pkg_dsp_t dsp, dp_req_t body, error_monitor_t em) {
    if (cpe_hash_table_count(&dsp->m_cmd_dsp) > 0) {
        struct bpg_pkg_dsp_node key;
        struct bpg_pkg_dsp_node * node;

        key.m_cmd = bpg_pkg_cmd(body);
        if ((node = (struct bpg_pkg_dsp_node *)cpe_hash_table_find(&dsp->m_cmd_dsp, &key)))
            return bpg_pkg_dsp_node_dispatch(node, body, em);
    }

    if (dsp->m_dft_dsp) 
        return bpg_pkg_dsp_node_dispatch(dsp->m_dft_dsp, body, em);

    return dp_dispatch_by_numeric(
        bpg_pkg_cmd(body), dp_req_mgr(body), body, em) == 0
        ? 0
        : -1;
}

int bpg_pkg_dsp_pass(bpg_pkg_dsp_t dsp, dp_req_t req, error_monitor_t em) {
    if (dsp->m_dft_dsp == NULL) {
        CPE_ERROR(em, "bpg_pkg_dsp_pass: no default dsp!");
        return -1;
    }

    switch(dsp->m_dft_dsp->m_type) {
    case bpg_pkg_dsp_to_cmd:
        return dp_dispatch_by_numeric(
            dsp->m_dft_dsp->m_target.m_to_cmd, dp_req_mgr(req), req, em) == 0
            ? 0
            : -1;
    case bpg_pkg_dsp_to_str:
        return dp_dispatch_by_string(
            dsp->m_dft_dsp->m_target.m_to_str, dp_req_mgr(req), req, em) == 0
            ? 0
            : -1;
    default:
        CPE_ERROR(em, "bpg_pkg_dsp_pass: unknown dispatch type %d", dsp->m_dft_dsp->m_type);
        return -1;
    }
}

struct bpg_pkg_dsp_node *
bpg_pkg_dsp_create_node_from_cfg(bpg_pkg_dsp_t dsp, const char * str_cmd, int32_t cmd, cfg_t cfg, error_monitor_t em) {
    switch(cfg_type(cfg)) {
    case CPE_CFG_TYPE_INT8:
    case CPE_CFG_TYPE_UINT8:
    case CPE_CFG_TYPE_INT16:
    case CPE_CFG_TYPE_UINT16:
    case CPE_CFG_TYPE_INT32:
    case CPE_CFG_TYPE_UINT32:
    case CPE_CFG_TYPE_INT64:
    case CPE_CFG_TYPE_UINT64: {
        struct bpg_pkg_dsp_node * node;
        uint32_t to_cmd;

        if (cfg_try_as_uint32(cfg, &to_cmd) != 0) {
            CPE_ERROR(em, "bpg_pgk_dsp_load: %s: to numeric, read cmd fail!", str_cmd);
            return NULL;
        }

        node = bpg_pkg_dsp_node_create_to_cmd(dsp->m_alloc, cmd, to_cmd);
        if (node == NULL) {
            CPE_ERROR(em, "bpg_pgk_dsp_load:  %s: to numeric %u: fail!", str_cmd, to_cmd);
            return NULL;
        }

        return node;
    }
    case CPE_CFG_TYPE_STRING: {
        struct bpg_pkg_dsp_node * node;
        const char * to_str;

        to_str = (const char *)cfg_data(cfg);
        assert(to_str);

        node = bpg_pkg_dsp_node_create_to_string(dsp->m_alloc, cmd, to_str);
        if (node == NULL) {
            CPE_ERROR(em, "bpg_pgk_dsp_load:  %s: to string %s: fail!", str_cmd, to_str);
            return NULL;
        }

        return node;
    }
    default:
        CPE_ERROR(em, "bpg_pgk_dsp_load: %s: not support load from cfg type %d!", str_cmd, cfg_type(cfg));
        return NULL;
    }

    return NULL;
}

int bpg_pkg_dsp_load(bpg_pkg_dsp_t dsp, cfg_t cfg, error_monitor_t em) {
    if (cfg == NULL) return 0;

    if (cfg_type(cfg) == CPE_CFG_TYPE_STRUCT) {
        struct cfg_it it;
        cfg_t child_cfg;

        cfg_it_init(&it, cfg);

        while((child_cfg = cfg_it_next(&it))) {
            struct bpg_pkg_dsp_node * child_node;
            const char * str_cmd;
            int32_t cmd;

            str_cmd = (const char *)cfg_data(child_cfg);
            assert(str_cmd);

            if (dr_ctype_set_from_string(&cmd, CPE_DR_TYPE_INT32, str_cmd, em) != 0) {
                CPE_ERROR(em, "bpg_pgk_dsp_load: %s: read cmd fail!", str_cmd);
                return -1;
            }

            child_node = bpg_pkg_dsp_create_node_from_cfg(dsp, str_cmd, cmd, child_cfg, em);
            if (child_node == NULL) return -1;

            if (cpe_hash_table_insert_unique(&dsp->m_cmd_dsp, child_node) != 0) {
                CPE_ERROR(em, "bpg_pgk_dsp_load: %s: already exist!", str_cmd);
                bpg_pkg_dsp_node_free(dsp->m_alloc, child_node);
                return -1;
            }
        }

        return 0;
    }
    else {
        struct bpg_pkg_dsp_node * node;

        node = bpg_pkg_dsp_create_node_from_cfg(dsp, "default", -1, cfg, em);
        if (node == NULL) return -1;

        if (dsp->m_dft_dsp) bpg_pkg_dsp_node_free(dsp->m_alloc, dsp->m_dft_dsp);
        dsp->m_dft_dsp = node;
        return 0;
    }
}

