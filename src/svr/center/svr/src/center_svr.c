#include <assert.h>
#include "cpe/pal/pal_external.h"
#include "cpe/pal/pal_socket.h"
#include "cpe/pal/pal_stdio.h"
#include "cpe/pal/pal_shm.h"
#include "cpe/tl/tl_manage.h"
#include "cpe/nm/nm_manage.h"
#include "cpe/nm/nm_read.h"
#include "cpe/net/net_manage.h"
#include "cpe/aom/aom_obj_mgr.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dr/dr_metalib_cmp.h"
#include "gd/app/app_module.h"
#include "gd/app/app_context.h"
#include "center_svr.h"
#include "center_svr_type.h"
#include "center_svr_ins_proxy.h"
#include "center_svr_set_proxy.h"

extern void center_svr_listener_cb(EV_P_ ev_io *w, int revents);
extern char g_metalib_svr_center_pro[];
static void center_svr_clear(nm_node_t node);

struct nm_node_type s_nm_node_type_center_svr = {
    "svr_center_svr",
    center_svr_clear
};

center_svr_t
center_svr_create(
    gd_app_context_t app,
    const char * name,
    mem_allocrator_t alloc,
    error_monitor_t em)
{
    struct center_svr * svr;
    nm_node_t svr_node;

    assert(app);

    svr_node = nm_group_create(gd_app_nm_mgr(app), name, sizeof(struct center_svr));
    if (svr_node == NULL) return NULL;

    svr = (center_svr_t)nm_node_data(svr_node);

    svr->m_app = app;
    svr->m_alloc = alloc;
    svr->m_em = em;
    svr->m_debug = 0;
    svr->m_ev_loop = net_mgr_ev_loop(gd_app_net_mgr(app));
    svr->m_conn_timeout_ms = 500 * 1000;
    svr->m_read_block_size = 2048;
    svr->m_client_data_mgr = NULL;
    svr->m_process_count_per_tick = 10;
    svr->m_max_pkg_size = 1024 * 1024 * 5;
    svr->m_set_offline_timeout = 60;

    svr->m_ringbuf = NULL;
    svr->m_fd = -1;
    svr->m_watcher.data = svr;

    svr->m_record_meta =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_center_pro, "svr_center_cli_record");
    if (svr->m_record_meta == NULL) {
        CPE_ERROR(em, "%s: create find record meta fail!", name);
        nm_node_free(svr_node);
        return NULL;
    }

    svr->m_pkg_meta =
        dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_svr_center_pro, "svr_center_pkg");
    if (svr->m_pkg_meta == NULL) {
        CPE_ERROR(em, "%s: create find pkg meta fail!", name);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_svr_types,
            alloc,
            (cpe_hash_fun_t) center_svr_type_hash,
            (cpe_hash_eq_t) center_svr_type_eq,
            CPE_HASH_OBJ2ENTRY(center_svr_type, m_hh),
            -1) != 0)
    {
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_ins_proxies,
            alloc,
            (cpe_hash_fun_t) center_svr_ins_proxy_hash,
            (cpe_hash_eq_t) center_svr_ins_proxy_eq,
            CPE_HASH_OBJ2ENTRY(center_svr_ins_proxy, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_svr_types);
        nm_node_free(svr_node);
        return NULL;
    }

    if (cpe_hash_table_init(
            &svr->m_set_proxies,
            alloc,
            (cpe_hash_fun_t) center_svr_set_proxy_hash,
            (cpe_hash_eq_t) center_svr_set_proxy_eq,
            CPE_HASH_OBJ2ENTRY(center_svr_set_proxy, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_ins_proxies);
        cpe_hash_table_fini(&svr->m_svr_types);
        nm_node_free(svr_node);
        return NULL;
    }
    
    if (cpe_hash_table_init(
            &svr->m_conns,
            alloc,
            (cpe_hash_fun_t) center_svr_conn_hash,
            (cpe_hash_eq_t) center_svr_conn_eq,
            CPE_HASH_OBJ2ENTRY(center_svr_conn, m_hh),
            -1) != 0)
    {
        cpe_hash_table_fini(&svr->m_set_proxies);
        cpe_hash_table_fini(&svr->m_ins_proxies);
        cpe_hash_table_fini(&svr->m_svr_types);
        nm_node_free(svr_node);
        return NULL;
    }

    if (gd_app_tick_add(app, center_svr_tick, svr, 0) != 0) {
        CPE_ERROR(em, "%s: create: add tick fail!", name);
        cpe_hash_table_fini(&svr->m_set_proxies);
        cpe_hash_table_fini(&svr->m_ins_proxies);
        cpe_hash_table_fini(&svr->m_svr_types);
        cpe_hash_table_fini(&svr->m_conns);
        nm_node_free(svr_node);
        return NULL;
    }
    
    mem_buffer_init(&svr->m_mem_data_buf, svr->m_alloc);
    mem_buffer_init(&svr->m_outgoing_pkg_buf, svr->m_alloc);
    mem_buffer_init(&svr->m_dump_buffer, svr->m_alloc);

    nm_node_set_type(svr_node, &s_nm_node_type_center_svr);

    return svr;
}

static void center_svr_clear(nm_node_t node) {
    center_svr_t svr;
    svr = (center_svr_t)nm_node_data(node);

    center_svr_stop(svr);

    gd_app_tick_remove(svr->m_app, center_svr_tick, svr);

    /*清理连接 */
    center_svr_conn_free_all(svr);
    assert(cpe_hash_table_count(&svr->m_conns) == 0);

    /*清理客户端数据缓存 */
    center_svr_type_free_all(svr);
    center_svr_set_proxy_free_all(svr);
    center_svr_ins_proxy_free_all(svr);

    assert(cpe_hash_table_count(&svr->m_svr_types) == 0);
    assert(cpe_hash_table_count(&svr->m_ins_proxies) == 0);

    cpe_hash_table_fini(&svr->m_conns);
    cpe_hash_table_fini(&svr->m_set_proxies);
    cpe_hash_table_fini(&svr->m_ins_proxies);
    cpe_hash_table_fini(&svr->m_svr_types);

    if (svr->m_client_data_mgr) {
        aom_obj_mgr_free(svr->m_client_data_mgr);
        svr->m_client_data_mgr = NULL;
    }

    mem_buffer_clear(&svr->m_mem_data_buf);
    mem_buffer_clear(&svr->m_outgoing_pkg_buf);
    mem_buffer_clear(&svr->m_dump_buffer);

    if (svr->m_ringbuf) {
        ringbuffer_delete(svr->m_ringbuf);
        svr->m_ringbuf = NULL;
    }
}

int center_svr_start(center_svr_t svr, const char * ip, uint16_t port) {
    struct sockaddr_in addr;

    svr->m_fd = cpe_sock_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (svr->m_fd == -1) {
        CPE_ERROR(svr->m_em, "%s: socket call fail, errno=%d (%s)!", center_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }

    if (cpe_sock_set_reuseaddr(svr->m_fd, 1) != 0) {
        CPE_ERROR(svr->m_em, "%s: set sock reuseaddr fail, errno=%d (%s)!", center_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        return -1;
    }


    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = strcmp(ip, "") == 0 ? INADDR_ANY : inet_addr(ip);
    if(cpe_bind(svr->m_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        CPE_ERROR(svr->m_em, "%s: bind error, errno=%d (%s)", center_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        cpe_sock_close(svr->m_fd);
        svr->m_fd = -1;
        return -1;
    }

    if (cpe_listen(svr->m_fd, 512) != 0) {
        CPE_ERROR(svr->m_em, "%s: listen error, errno=%d (%s)", center_svr_name(svr), cpe_sock_errno(), cpe_sock_errstr(cpe_sock_errno()));
        cpe_sock_close(svr->m_fd);
        svr->m_fd = -1;
        return -1;
    }

    svr->m_watcher.data = svr;
    ev_io_init(&svr->m_watcher, center_svr_listener_cb, svr->m_fd, EV_READ);
    ev_io_start(svr->m_ev_loop, &svr->m_watcher);

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: listen start", center_svr_name(svr));
    }

    return 0;
}

void center_svr_stop(center_svr_t svr) {
    if (svr->m_fd < 0) return;

    ev_io_stop(svr->m_ev_loop, &svr->m_watcher);
    cpe_sock_close(svr->m_fd);
    svr->m_fd = -1;

    if (svr->m_debug) {
        CPE_INFO(svr->m_em, "%s: listen stop", center_svr_name(svr));
    }
}

static void center_svr_build_datas_from_aom(center_svr_t svr) {
    struct aom_obj_it it;
    SVR_CENTER_CLI_RECORD * record;

    aom_objs(svr->m_client_data_mgr, &it);

    record = aom_obj_it_next(&it);
    while(record) {
        SVR_CENTER_CLI_RECORD * next = aom_obj_it_next(&it);
        center_svr_type_t svr_type;
        center_svr_set_proxy_t set;

        svr_type = center_svr_type_find(svr, record->svr_type);
        if (svr_type == NULL) {
            CPE_INFO(
                svr->m_em, "%s: build datas from aom: svr_type %d not exist, release record!",
                center_svr_name(svr), record->svr_type);
            aom_obj_free(svr->m_client_data_mgr, record);
            record = next;
            continue;
        }

        set = center_svr_set_proxy_find(svr, record->set.id);
        if (set == NULL) {
            set = center_svr_set_proxy_create(svr, &record->set);
            if (set == NULL) {
                CPE_INFO(
                    svr->m_em, "%s: build datas from aom: svr %d create fail!",
                    center_svr_name(svr), record->set.id);
                aom_obj_free(svr->m_client_data_mgr, record);
                record = next;
                continue;
            }
        }
        
        if (center_svr_ins_proxy_create(svr, svr_type, set, record) == NULL) {
            CPE_INFO(
                svr->m_em, "%s: build datas from aom: create data fail, release record!",
                center_svr_name(svr));
            aom_obj_free(svr->m_client_data_mgr, record);
            record = next;
            continue;
        }

        record = next;
    }
}

int center_svr_init_clients_from_shm(center_svr_t svr, int shm_key) {
    cpe_shm_id_t shmid;
    cpe_shmid_ds shm_info;
    void * data;

    if (svr->m_client_data_mgr) {
        aom_obj_mgr_free(svr->m_client_data_mgr);
        svr->m_client_data_mgr = NULL;
    }

    shmid = cpe_shm_get(shm_key);
    if (shmid == -1) {
        CPE_ERROR(svr->m_em, "%s: init from shm %d: shm_get fail!", center_svr_name(svr), shm_key);
        return -1;
    }

    if (cpe_shm_ds_get(shmid, &shm_info) != 0) {
        CPE_ERROR(svr->m_em, "%s: init from shm %d: shm_ds_get fail!", center_svr_name(svr), shm_key);
        return -1;
    }

    data = cpe_shm_attach(shmid, NULL, 0);
    if (data == NULL) {
        CPE_ERROR(svr->m_em, "%s: init from shm %d: shm_attach fail!", center_svr_name(svr), shm_key);
        return -1;
    }

    svr->m_client_data_mgr = aom_obj_mgr_create(svr->m_alloc, data, shm_info.shm_segsz, svr->m_em);
    if (svr->m_client_data_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: init from shm %d: create aom_obj_mgr fail!", center_svr_name(svr), shm_key);
        cpe_shm_detach(data);
        return -1;
    }

    if (!dr_meta_compatible(svr->m_record_meta, aom_obj_mgr_meta(svr->m_client_data_mgr))) {
        aom_obj_mgr_free(svr->m_client_data_mgr);
        svr->m_client_data_mgr = NULL;
        cpe_shm_detach(data);
        CPE_ERROR(svr->m_em, "%s: init from shm %d: aom grp meta not compatable!", center_svr_name(svr), shm_key);
        return -1;
    }

    center_svr_build_datas_from_aom(svr);

    return 0;
}

int center_svr_init_clients_from_mem(center_svr_t svr, size_t capacity) {
    void * buf;

    if (svr->m_client_data_mgr) {
        center_svr_ins_proxy_free_all(svr);
        assert(cpe_hash_table_count(&svr->m_svr_types) == 0);
        assert(cpe_hash_table_count(&svr->m_ins_proxies) == 0);
        aom_obj_mgr_free(svr->m_client_data_mgr);
        svr->m_client_data_mgr = NULL;
    }

    if (mem_buffer_set_size(&svr->m_mem_data_buf, capacity) != 0
        || (buf = mem_buffer_make_continuous(&svr->m_mem_data_buf, 0)) == NULL)
    {
        CPE_ERROR(svr->m_em, "%s: init from mem: alloc buff fail, capacity=%d!", center_svr_name(svr), (int)capacity);
        return -1;
    }

    if (aom_obj_mgr_buf_init(svr->m_record_meta, buf, capacity, svr->m_em) != 0) {
        CPE_ERROR(svr->m_em, "%s: init from mem: aom_obj_mgr_buf_init fail!", center_svr_name(svr));
        return -1;
    }

    svr->m_client_data_mgr = aom_obj_mgr_create(svr->m_alloc, buf, capacity, svr->m_em);
    if (svr->m_client_data_mgr == NULL) {
        CPE_ERROR(svr->m_em, "%s: init from mem: create aom_obj_mgr fail!", center_svr_name(svr));
        return -1;
    }

    center_svr_build_datas_from_aom(svr);

    return 0;
}

gd_app_context_t center_svr_app(center_svr_t svr) {
    return svr->m_app;
}

void center_svr_free(center_svr_t svr) {
    nm_node_t svr_node;
    assert(svr);

    svr_node = nm_node_from_data(svr);
    if (nm_node_type(svr_node) != &s_nm_node_type_center_svr) return;
    nm_node_free(svr_node);
}

center_svr_t
center_svr_find(gd_app_context_t app, cpe_hash_string_t name) {
    nm_node_t node;

    node = nm_mgr_find_node(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_center_svr) return NULL;
    return (center_svr_t)nm_node_data(node);
}

center_svr_t
center_svr_find_nc(gd_app_context_t app, const char * name) {
    nm_node_t node;

    node = nm_mgr_find_node_nc(gd_app_nm_mgr(app), name);
    if (node == NULL || nm_node_type(node) != &s_nm_node_type_center_svr) return NULL;
    return (center_svr_t)nm_node_data(node);
}

const char * center_svr_name(center_svr_t svr) {
    return nm_node_name(nm_node_from_data(svr));
}

cpe_hash_string_t
center_svr_name_hs(center_svr_t svr) {
    return nm_node_name_hs(nm_node_from_data(svr));
}

uint32_t center_svr_cur_time(center_svr_t svr) {
    return tl_manage_time_sec(gd_app_tl_mgr(svr->m_app));
}

int center_svr_set_ringbuf_size(center_svr_t svr, size_t capacity) {
    assert(svr->m_ringbuf == NULL);
    svr->m_ringbuf = ringbuffer_new(capacity);
    if (svr->m_ringbuf == NULL) return -1;
    return 0;
}

SVR_CENTER_PKG *
center_svr_get_res_pkg_buff(center_svr_t svr, SVR_CENTER_PKG * req, size_t capacity) {
    SVR_CENTER_PKG * res;

    if (mem_buffer_size(&svr->m_outgoing_pkg_buf) < capacity) {
        if (mem_buffer_set_size(&svr->m_outgoing_pkg_buf, capacity) != 0) {
            CPE_ERROR(
                svr->m_em, "%s: create pkg buf for data size %d fail",
                center_svr_name(svr), (int)capacity);
            return NULL;
        }
    }

    res = mem_buffer_make_continuous(&svr->m_outgoing_pkg_buf, 0);
    bzero(res, capacity);

    if (req) {
        res->cmd = req->cmd + 1;
    }

    return res;
}

