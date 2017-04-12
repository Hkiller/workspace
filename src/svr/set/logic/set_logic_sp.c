#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_data.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dp/dp_responser.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_require.h"
#include "usf/logic_use/logic_require_queue.h"
#include "svr/set/share/set_pkg.h"
#include "svr/set/stub/set_svr_stub.h"
#include "svr/set/stub/set_svr_svr_info.h"
#include "svr/set/logic/set_logic_sp.h"
#include "set_logic_sp_ops.h"
#include "protocol/set/logic/set_logic_sp_data.h"

static void set_logic_sp_clear(nm_node_t node);
extern char g_metalib_set_logic_data_meta[];

struct nm_node_type s_nm_node_type_set_logic_sp = {
    "svr_set_logic_sp",
    set_logic_sp_clear
};

set_logic_sp_t
set_logic_sp_create(
    gd_app_context_t app,
    const char * name,
    logic_manage_t logic_mgr,
    set_svr_stub_t stub,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct set_logic_sp * mgr;
    nm_node_t mgr_node;

    assert(app);

    mgr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct set_logic_sp));
    if (mgr_node == NULL) return NULL;

    mgr = (set_logic_sp_t)nm_node_data(mgr_node);

    mgr->m_app = app;
    mgr->m_alloc = alloc;
    mgr->m_em = em;
    mgr->m_debug = 0;
    mgr->m_stub = stub;
    mgr->m_outgoing_dispatch_to = NULL;
    mgr->m_incoming_recv_at = NULL;

    mgr->m_sp_data_meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_set_logic_data_meta, "set_logic_sp_data");
    assert(mgr->m_sp_data_meta);

    mgr->m_require_queue = logic_require_queue_create(app, alloc, em, name, logic_mgr, 0);
    if (mgr->m_require_queue == NULL) {
        nm_node_free(mgr_node);
        return NULL;
    }

    nm_node_set_type(mgr_node, &s_nm_node_type_set_logic_sp);

    return mgr;
}

static void set_logic_sp_clear(nm_node_t node) {
    set_logic_sp_t mgr;
    mgr = (set_logic_sp_t)nm_node_data(node);

    if (mgr->m_outgoing_dispatch_to) {
        mem_free(mgr->m_alloc, mgr->m_outgoing_dispatch_to);
        mgr->m_outgoing_dispatch_to = NULL;
    }

    if (mgr->m_incoming_recv_at) {
        dp_rsp_free(mgr->m_incoming_recv_at);
        mgr->m_incoming_recv_at = NULL;
    }

    logic_require_queue_free(mgr->m_require_queue);
    mgr->m_require_queue = NULL;
}

gd_app_context_t set_logic_sp_app(set_logic_sp_t mgr) {
    return mgr->m_app;
}

void set_logic_sp_free(set_logic_sp_t mgr) {
    nm_node_t mgr_node;
    assert(mgr);

    mgr_node = nm_node_from_data(mgr);
    if (nm_node_type(mgr_node) != &s_nm_node_type_set_logic_sp) return;
    nm_node_free(mgr_node);
}

set_logic_sp_t
set_logic_sp_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_logic_sp) return NULL;
    return (set_logic_sp_t)nm_node_data(node);
}

set_logic_sp_t
set_logic_sp_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    if (name == NULL) name = "set_logic_sp";

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_set_logic_sp) return NULL;
    return (set_logic_sp_t)nm_node_data(node);
}

const char * set_logic_sp_name(set_logic_sp_t mgr) {
    return nm_node_name(nm_node_from_data(mgr));
}

cpe_hash_string_t
set_logic_sp_name_hs(set_logic_sp_t mgr) {
    return nm_node_name_hs(nm_node_from_data(mgr));
}

set_svr_stub_t set_logic_sp_stub(set_logic_sp_t mgr) {
    return mgr->m_stub;
}

int set_logic_sp_set_outgoing_dispatch_to(set_logic_sp_t sp, const char * outgoing_dispatch_to) {
    cpe_hash_string_t new_outgoing_dispatch_to = cpe_hs_create(sp->m_alloc, outgoing_dispatch_to);
    if (new_outgoing_dispatch_to == NULL) return -1;

    if (sp->m_outgoing_dispatch_to) mem_free(sp->m_alloc, sp->m_outgoing_dispatch_to);
    sp->m_outgoing_dispatch_to = new_outgoing_dispatch_to;

    return 0;
}

cpe_hash_string_t set_logic_sp_outgoing_dispatch_to(set_logic_sp_t sp) {
    return sp->m_outgoing_dispatch_to;
}

static int set_logic_sp_incoming_recv(dp_req_t req, void * ctx, error_monitor_t em) {
    set_logic_sp_t sp = ctx;
    dp_req_t pkg_head = set_pkg_head_find(req);
    set_svr_svr_info_t svr_type;
    logic_require_t require;
    uint32_t cmd;
    void * data_buf;
    LPDRMETA data_meta;
    size_t data_size;
    logic_data_t data;
    SET_LOGIC_SP_DATA * carry_data;

    if (pkg_head == NULL) {
        CPE_ERROR(
            sp->m_em, "%s: receive response of ???: no pkg head!",
            set_logic_sp_name(sp));
        return -1;
    }

    if (set_pkg_sn(pkg_head) == 0) {
        if (sp->m_debug) {
            CPE_INFO(
                sp->m_em, "%s: receive response of %d: ignore for no sn!",
                set_logic_sp_name(sp), set_pkg_sn(pkg_head));
        }
        return 0;
    }

    if (set_pkg_category(pkg_head) != set_pkg_response) {
        CPE_ERROR(
            sp->m_em, "%s: receive response of %d: pkg is not response, category=%d!",
            set_logic_sp_name(sp), set_pkg_sn(pkg_head), set_pkg_category(pkg_head));
        return -1;
    }

    svr_type = set_svr_svr_info_find(sp->m_stub, set_pkg_from_svr_type(pkg_head));
    if (svr_type == NULL) {
        CPE_ERROR(
            sp->m_em, "%s: receive response of %d: svr type of %d not exist!",
            set_logic_sp_name(sp), set_pkg_sn(pkg_head), set_pkg_from_svr_type(pkg_head));
        return -1;
    }

    require = logic_require_queue_remove_get(sp->m_require_queue, set_pkg_sn(pkg_head), NULL, NULL);
    if (require == NULL) {
        CPE_ERROR(
            sp->m_em, "%s: receive response of %d: require not exist, ignore!",
            set_logic_sp_name(sp), set_pkg_sn(pkg_head));
        return -1;
    }

    if (set_svr_stub_read_data(sp->m_stub, svr_type, req, &cmd, &data_meta, &data_buf, &data_size) != 0) {
        CPE_ERROR(
            sp->m_em, "%s: receive response of %d: read response data fail!",
            set_logic_sp_name(sp), set_pkg_sn(pkg_head));
        return -1;
    }

    /*创建携带数据 */
    data = logic_require_data_get_or_create(require, sp->m_sp_data_meta, sizeof(SET_LOGIC_SP_DATA));
    if (data == NULL) {
        CPE_ERROR(
            sp->m_em, "%s: receive response of %d: create carry data fail!",
            set_logic_sp_name(sp), set_pkg_sn(pkg_head));
        logic_require_set_error(require);
        return -1;
    }
    carry_data = (SET_LOGIC_SP_DATA *)logic_data_data(data);
    carry_data->from_svr_type = set_pkg_from_svr_type(pkg_head);
    carry_data->from_svr_id = set_pkg_from_svr_id(pkg_head);

    if (set_svr_svr_info_error_pkg_meta(svr_type) && set_svr_svr_info_error_pkg_cmd(svr_type) == cmd) {
        LPDRMETAENTRY error_entry = set_svr_svr_info_error_pkg_errno_entry(svr_type);
        int32_t err = -1;
        if (dr_entry_try_read_int32(
                &err, ((const char*)data_buf) + dr_entry_data_start_pos(error_entry, 0),
                error_entry, sp->m_em)
            != 0)
        {
            CPE_ERROR(
                sp->m_em, "%s: receive response of %d: read errno from error response fail!",
                set_logic_sp_name(sp), set_pkg_sn(pkg_head));
            logic_require_set_error(require);
        }
        else {
            logic_require_set_error_ex(require, err);
        }
    }
    else if (data_meta) {
        data = logic_require_data_get_or_create(require, data_meta, data_size);
        if (data == NULL) {
            CPE_ERROR(
                sp->m_em, "%s: receive response of %d: create data fail!",
                set_logic_sp_name(sp), set_pkg_sn(pkg_head));
            logic_require_set_error(require);
            return -1;
        }
        memcpy(logic_data_data(data), data_buf, data_size);

        logic_require_set_done(require);
    }
    else {
        logic_require_set_done(require);
    }

    return 0;
}

int set_logic_sp_set_incoming_recv_at(set_logic_sp_t sp, const char * incoming_recv_at) {
    char name_buf[128];

    snprintf(name_buf, sizeof(name_buf), "%s.incoming-recv-rsp", set_logic_sp_name(sp));

    if (sp->m_incoming_recv_at) dp_rsp_free(sp->m_incoming_recv_at);

    sp->m_incoming_recv_at = dp_rsp_create(gd_app_dp_mgr(sp->m_app), name_buf);
    if (sp->m_incoming_recv_at == NULL) return -1;

    dp_rsp_set_processor(sp->m_incoming_recv_at, set_logic_sp_incoming_recv, sp);

    if (dp_rsp_bind_string(sp->m_incoming_recv_at, incoming_recv_at, sp->m_em) != 0) {
        CPE_ERROR(
            sp->m_em, "%s: set incoming_recv_at: bind to %s fail!",
            set_logic_sp_name(sp), incoming_recv_at);
        dp_rsp_free(sp->m_incoming_recv_at);
        sp->m_incoming_recv_at = NULL;
        return -1;
    }

    return 0;
}

int set_logic_sp_send_pkg(set_logic_sp_t sp, dp_req_t pkg, logic_require_t require) {
    dp_req_t pkg_head;
    int r;

    pkg_head = set_pkg_head_find(pkg);
    if (pkg_head == NULL) {
        CPE_ERROR(sp->m_em, "%s: send_pkg: find pkg_head!", set_logic_sp_name(sp));
        return -1;
    }

    if (require) {
        set_pkg_set_sn(pkg_head, logic_require_id(require));
        if (logic_require_queue_add(sp->m_require_queue, logic_require_id(require), NULL, 0) != 0) {
            CPE_ERROR(sp->m_em, "%s: send_req_data: add require fail!", set_logic_sp_name(sp));
            return -1;
        }
    }
    else {
        set_pkg_set_sn(pkg_head, 0);
    }

    r = set_svr_stub_send_pkg(sp->m_stub, pkg);
    if (r != 0) {
        CPE_ERROR(sp->m_em, "%s: send_pkg: send data fail!", set_logic_sp_name(sp));
        if (require) logic_require_queue_remove(sp->m_require_queue, logic_require_id(require), NULL, NULL);
        return r;
    }

    return r;
}

int set_logic_sp_send_req_data(
    set_logic_sp_t sp, uint16_t to_svr_type, uint16_t to_svr_id,
    LPDRMETA meta, void const * data, size_t data_size,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require)
{
    int r;
    uint32_t sn = 0;

    if (require) {
        sn = logic_require_id(require);
        if (logic_require_queue_add(sp->m_require_queue, sn, NULL, 0) != 0) {
            CPE_ERROR(sp->m_em, "%s: send_req_data: add require fail!", set_logic_sp_name(sp));
            return -1;
        }
    }

    r = set_svr_stub_send_req_data(sp->m_stub, to_svr_type, to_svr_id, sn, data, data_size, meta, carry_data, carry_data_size);
    if (r != 0) {
        CPE_ERROR(sp->m_em, "%s: send_req_data: send data fail!", set_logic_sp_name(sp));
        if (require) {
            logic_require_queue_remove(sp->m_require_queue, sn, NULL, NULL);
        }
        return r;
    }

    return r;
}

int set_logic_sp_send_req_pkg(
    set_logic_sp_t sp, uint16_t to_svr_type, uint16_t to_svr_id,
    dp_req_t pkg,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require)
{
    int r;
    uint32_t sn = 0;

    if (require) {
        sn = logic_require_id(require);
        if (logic_require_queue_add(sp->m_require_queue, sn, NULL, 0) != 0) {
            CPE_ERROR(sp->m_em, "%s: send_req_pkg: add require fail!", set_logic_sp_name(sp));
            return -1;
        }
    }

    r = set_svr_stub_send_req_pkg(sp->m_stub, to_svr_type, to_svr_id, sn, pkg, carry_data, carry_data_size);
    if (r != 0) {
        CPE_ERROR(sp->m_em, "%s: send_req_pkg: send pkg fail!", set_logic_sp_name(sp));
        if (require) {
            logic_require_queue_remove(sp->m_require_queue, sn, NULL, NULL);
        }
        return r;
    }

    return r;
}

int set_logic_sp_send_req_cmd(
    set_logic_sp_t sp, uint16_t to_svr_type, uint16_t to_svr_id,
    uint32_t cmd,
    void const * carry_data, size_t carry_data_size,
    logic_require_t require)
{
    int r;
    uint32_t sn = 0;

    if (require) {
        sn = logic_require_id(require);
        if (logic_require_queue_add(sp->m_require_queue, sn, NULL, 0) != 0) {
            CPE_ERROR(sp->m_em, "%s: send_req_cmd: add require fail!", set_logic_sp_name(sp));
            return -1;
        }
    }

    r = set_svr_stub_send_req_cmd(sp->m_stub, to_svr_type, to_svr_id, sn, cmd, carry_data, carry_data_size);
    if (r != 0) {
        CPE_ERROR(sp->m_em, "%s: send_req_cmd: send data fail!", set_logic_sp_name(sp));
        if (require) logic_require_queue_remove(sp->m_require_queue, sn, NULL, NULL);
        return r;
    }

    return r;
}

int set_logic_sp_response_from_svr_type(logic_require_t require, uint16_t * svr_type) {
    logic_data_t data;

    assert(require);

    data = logic_require_data_find(require, "set_logic_sp_data");
    if (data == NULL) return -1;

    if (svr_type) *svr_type = ((SET_LOGIC_SP_DATA *)logic_data_data(data))->from_svr_type;
    return 0;
}

int set_logic_sp_response_from_svr_id(logic_require_t require, uint16_t * svr_id) {
    logic_data_t data;

    assert(require);

    data = logic_require_data_find(require, "set_logic_sp_data");
    if (data == NULL) return -1;

    if (svr_id) *svr_id = ((SET_LOGIC_SP_DATA *)logic_data_data(data))->from_svr_id;
    return 0;
}
