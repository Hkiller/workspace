#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/stream.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_connector.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/fsm/fsm_def.h"
#include "cpe/fsm/fsm_ins.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "gd/dr_cvt/dr_cvt.h"
#include "set_svr_center.h"
#include "set_svr_center_fsm.h"

extern char g_metalib_svr_center_pro[];
static fsm_def_machine_t
set_svr_center_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em);

set_svr_center_t set_svr_center_create(set_svr_t svr) {
    set_svr_center_t center;

    center = mem_alloc(svr->m_alloc, sizeof(struct set_svr_center));
    if (center == NULL) {
        CPE_ERROR(svr->m_em, "%s: center: alloc fail!", set_svr_name(svr));
        return NULL;
    }

    center->m_svr = svr;

    center->m_conn_id = ++svr->m_max_conn_id;
    center->m_read_block_size = 2048;
    center->m_max_pkg_size = 1024 * 1024 * 5;
    center->m_reconnect_span_ms = 3 * 1000;
    center->m_update_span_s = 60;

    center->m_ip[0] = 0;
    center->m_port = 0;

    center->m_fsm_timer_id = GD_TIMER_ID_INVALID;

    center->m_rb = NULL;
    center->m_wb = NULL;
    center->m_tb = NULL;
    center->m_fd = -1;
    center->m_watcher.data = center;

    center->m_pkg_meta =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_center_pro, "svr_center_pkg");
    if (center->m_pkg_meta == NULL) {
        CPE_ERROR(svr->m_em, "%s: center: create find pkg meta fail!", set_svr_name(svr));
        mem_free(svr->m_alloc, center);
        return NULL;
    }

    center->m_fsm_def = set_svr_center_create_fsm_def(set_svr_name(svr), svr->m_alloc, svr->m_em);
    if (center->m_fsm_def == NULL) {
        CPE_ERROR(svr->m_em, "%s: center: create fsm def fail!", set_svr_name(svr));
        mem_free(svr->m_alloc, center);
        return NULL;
    }

    if (fsm_machine_init(&center->m_fsm, center->m_fsm_def, "disable", center, 1) != 0) {
        CPE_ERROR(svr->m_em, "%s: center: init fsm fail!", set_svr_name(svr));
        fsm_def_machine_free(center->m_fsm_def);
        mem_free(svr->m_alloc, center);
        return NULL;
    }

    mem_buffer_init(&center->m_outgoing_pkg_buf, svr->m_alloc);

    return center;
}

void set_svr_center_free(set_svr_center_t center) {
    set_svr_t svr = center->m_svr;

    fsm_machine_fini(&center->m_fsm);
    assert(center->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    set_svr_center_disconnect(center);
    assert(center->m_fd == -1);

    fsm_def_machine_free(center->m_fsm_def);
    center->m_fsm_def = NULL;
    
    if (center->m_rb) {
        ringbuffer_free(center->m_svr->m_ringbuf, center->m_rb);
        center->m_rb = NULL;
    }

    if (center->m_wb) {
        ringbuffer_free(center->m_svr->m_ringbuf, center->m_wb);
        center->m_wb = NULL;
    }

    if (center->m_tb) {
        ringbuffer_free(center->m_svr->m_ringbuf, center->m_tb);
        center->m_tb = NULL;
    }

    mem_buffer_clear(&center->m_outgoing_pkg_buf);

    mem_free(svr->m_alloc, center);
}

int set_svr_center_set_svr(set_svr_center_t center, const char * ip, short port) {
    int need_start = 0;

    if (fsm_machine_curent_state(&center->m_fsm) != set_svr_center_state_disable) {
        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_stop);
        need_start = 1;
    }

    assert(fsm_machine_curent_state(&center->m_fsm) == set_svr_center_state_disable);
    assert(center->m_fd == -1);

    cpe_str_dup(center->m_ip, sizeof(center->m_ip), ip);
    center->m_port = port;

    if (need_start) {
        set_svr_center_apply_evt(center, set_svr_center_fsm_evt_start);
    }

    return 0;
}

int set_svr_center_set_reconnect_span_ms(set_svr_center_t center, uint32_t span_ms) {
    center->m_reconnect_span_ms = span_ms;
    return 0;
}

static void set_svr_center_dump_event(write_stream_t s, fsm_def_machine_t m, void * input_event) {
    struct set_svr_center_fsm_evt * evt = input_event;
    switch(evt->m_type) {
    case set_svr_center_fsm_evt_pkg:
        stream_printf(s, "package: ");
        break;
    case set_svr_center_fsm_evt_start:
        stream_printf(s, "start");
        break;
    case set_svr_center_fsm_evt_stop:
        stream_printf(s, "stop");
        break;
    case set_svr_center_fsm_evt_timeout:
        stream_printf(s, "timeout");
        break;
    case set_svr_center_fsm_evt_connected:
        stream_printf(s, "connected");
        break;
    case set_svr_center_fsm_evt_disconnected:
        stream_printf(s, "disconnected");
        break;
    case set_svr_center_fsm_evt_wb_update:
        stream_printf(s, "wb_update");
        break;
    }
}

fsm_def_machine_t
set_svr_center_create_fsm_def(const char * name, mem_allocrator_t alloc, error_monitor_t em) {
    fsm_def_machine_t fsm_def = fsm_def_machine_create(name, alloc, em);
    if (fsm_def == NULL) {
        CPE_ERROR(em, "set_svr_create_fsm_def: create fsm def fail!");
        return NULL;
    }

    fsm_def_machine_set_evt_dumper(fsm_def, set_svr_center_dump_event);

    if (set_svr_center_fsm_create_disable(fsm_def, em) != 0
        || set_svr_center_fsm_create_connecting(fsm_def, em) != 0
        || set_svr_center_fsm_create_disconnected(fsm_def, em) != 0
        || set_svr_center_fsm_create_idle(fsm_def, em) != 0
        || set_svr_center_fsm_create_join(fsm_def, em) != 0
        )
    {
        CPE_ERROR(em, "set_svr_create_fsm_def: init fsm fail!");
        fsm_def_machine_free(fsm_def);
        return NULL;
    }

    return fsm_def;
}

void set_svr_center_disconnect(set_svr_center_t center) {
    if (center->m_fd == -1) return;

    ev_io_stop(center->m_svr->m_ev_loop, &center->m_watcher);
    cpe_sock_close(center->m_fd);
    center->m_fd = -1;
}

void set_svr_center_apply_evt(set_svr_center_t center, enum set_svr_center_fsm_evt_type type) {
    struct set_svr_center_fsm_evt evt;
    evt.m_type = type;
    evt.m_pkg = NULL;
    fsm_machine_apply_event(&center->m_fsm, &evt);
}

void set_svr_center_apply_pkg(set_svr_center_t center, SVR_CENTER_PKG * pkg) {
    struct set_svr_center_fsm_evt evt;
    evt.m_type = set_svr_center_fsm_evt_pkg;
    evt.m_pkg = pkg;
    fsm_machine_apply_event(&center->m_fsm, &evt);
}

void set_svr_center_state_timeout(void * ctx, gd_timer_id_t timer_id, void * arg) {
    set_svr_center_t center = ctx;
    assert(center->m_fsm_timer_id == timer_id);
    set_svr_center_apply_evt(center, set_svr_center_fsm_evt_timeout);
}

int set_svr_center_start_state_timer(set_svr_center_t center, tl_time_span_t span) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(center->m_svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(
            center->m_svr->m_em, "%s: start state timer: get default timer manager fail!",
            set_svr_name(center->m_svr));
        return -1;
    }

    assert(center->m_fsm_timer_id == GD_TIMER_ID_INVALID);

    if (gd_timer_mgr_regist_timer(timer_mgr, &center->m_fsm_timer_id, set_svr_center_state_timeout, center, NULL, NULL, span, span, -1) != 0) {
        assert(center->m_fsm_timer_id == GD_TIMER_ID_INVALID);
        CPE_ERROR(center->m_svr->m_em, "%s: start state timer: regist timer fail!", set_svr_name(center->m_svr));
        return -1;
    }

    assert(center->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    return 0;
}

void set_svr_center_stop_state_timer(set_svr_center_t center) {
    gd_timer_mgr_t timer_mgr = gd_timer_mgr_default(center->m_svr->m_app);
    if (timer_mgr == NULL) {
        CPE_ERROR(center->m_svr->m_em, "%s: start state timer: get default timer manager fail!", set_svr_name(center->m_svr));
        return;
    }

    assert(center->m_fsm_timer_id != GD_TIMER_ID_INVALID);
    gd_timer_mgr_unregist_timer_by_id(timer_mgr, center->m_fsm_timer_id);
    center->m_fsm_timer_id = GD_TIMER_ID_INVALID;
}

void set_svr_center_link_node_r(set_svr_center_t center, ringbuffer_block_t blk) {
    if (center->m_rb) {
		ringbuffer_link(center->m_svr->m_ringbuf, center->m_rb , blk);
	}
    else {
		blk->id = center->m_conn_id;
		center->m_rb = blk;
	}
}

void set_svr_center_link_node_w(set_svr_center_t center, ringbuffer_block_t blk) {
    if (center->m_wb) {
		ringbuffer_link(center->m_svr->m_ringbuf, center->m_wb , blk);
	}
    else {
		blk->id = center->m_conn_id;
		center->m_wb = blk;
	}
}

void set_svr_center_start_watch(set_svr_center_t center) {
    assert(center->m_fd != -1);
    ev_io_init(&center->m_watcher, set_svr_center_rw_cb, center->m_fd, center->m_wb ? EV_READ | EV_WRITE : EV_READ);
    ev_io_start(center->m_svr->m_ev_loop, &center->m_watcher);
}

SVR_CENTER_PKG * set_svr_center_get_pkg_buff(set_svr_center_t center, size_t capacity) {
    set_svr_t svr = center->m_svr;
    SVR_CENTER_PKG * res;

    if (mem_buffer_size(&center->m_outgoing_pkg_buf) < capacity) {
        if (mem_buffer_set_size(&center->m_outgoing_pkg_buf, capacity) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: center: create pkg buf for data size %d fail",
                set_svr_name(svr), (int)capacity);
            return NULL;
        }
    }

    res = mem_buffer_make_continuous(&center->m_outgoing_pkg_buf, 0);
    bzero(res, capacity);

    return res;
}
