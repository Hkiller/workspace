#include <assert.h>
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_net/bpg_net_client.h"
#include "bpg_net_internal_ops.h"

static void bpg_net_client_on_read(bpg_net_client_t client, net_ep_t ep) {
    dp_req_t req_buf;

    if(client->m_debug >= 2) {
        CPE_INFO(
            client->m_em, "%s: ep %d: on read",
            bpg_net_client_name(client), (int)net_ep_id(ep));
    }

    req_buf = bpg_net_client_req_buf(client);
    if (req_buf == NULL) {
        CPE_ERROR(
            client->m_em, "%s: ep %d: get req buf fail!",
            bpg_net_client_name(client), (int)net_ep_id(ep));
        net_ep_close(ep);
        return;
    }

    while(1) {
        char * buf;
        size_t buf_size;
        size_t input_size;
        dr_cvt_result_t cvt_result;

        buf_size = net_ep_size(ep);
        if (buf_size <= 0) break;

        buf = net_ep_peek(ep, NULL, buf_size);
        if (buf == NULL) {
            CPE_ERROR(
                client->m_em, "%s: ep %d: peek data fail, size=%d!",
                bpg_net_client_name(client), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }

        input_size = buf_size;

        cvt_result =
            bpg_pkg_decode(
                req_buf,
                buf, &input_size, client->m_em, client->m_debug >= 2 ? 1 : 0);
        if (cvt_result == dr_cvt_result_not_enough_input) {
            if(client->m_debug) {
                CPE_ERROR(
                    client->m_em, "%s: ep %d: not enough data, input size is %d!",
                bpg_net_client_name(client), (int)net_ep_id(ep), (int)buf_size);
            }
            break;
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                client->m_em, "%s: ep %d: decode package fail, input size is %d!",
                bpg_net_client_name(client), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }
        net_ep_erase(ep, input_size);

        if(client->m_debug >= 2) {
            CPE_INFO(
                client->m_em, "%s: ep %d: decode one package, buf-origin-size=%d left-size=%d!",
                bpg_net_client_name(client), (int)net_ep_id(ep), (int)buf_size, (int)net_ep_size(ep));
        }

        if (client->m_debug) {
            CPE_ERROR(
                client->m_em,
                "%s: ep %d: <== recv one request!\n%s",
                bpg_net_client_name(client), (int)net_ep_id(ep),
                bpg_pkg_dump(req_buf, &client->m_dump_buffer));
        }

        if (bpg_pkg_sn(req_buf) != INVALID_LOGIC_REQUIRE_ID) {
            if (bpg_net_client_remove_require_id(client, bpg_pkg_sn(req_buf)) != 0) {
                CPE_INFO(
                    client->m_em, "%s: ep %d: remove require id fail!",
                    bpg_net_client_name(client), (int)net_ep_id(ep));
            }
        }

        if (bpg_pkg_dsp_dispatch(client->m_rsp_dsp, req_buf, client->m_em) != 0) {
            CPE_ERROR(
                client->m_em, "%s: ep %d: dispatch cmd %d error!",
                bpg_net_client_name(client), (int)net_ep_id(ep), bpg_pkg_cmd(req_buf));
        }
    }
}

static void bpg_net_client_on_open(bpg_net_client_t client, net_ep_t ep) {
    if(client->m_debug) {
        CPE_INFO(
            client->m_em, "%s: ep %d: on open",
            bpg_net_client_name(client), (int)net_ep_id(ep));
    }

}

static void bpg_net_client_on_close(bpg_net_client_t client, net_ep_t ep, net_ep_event_t event) {
    if(client->m_debug) {
        CPE_INFO(
            client->m_em, "%s: ep %d: on close, event=%d",
            bpg_net_client_name(client), (int)net_ep_id(ep), event);
    }

    bpg_net_client_notify_all_require_disconnect(client);
}

static void bpg_net_client_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
    bpg_net_client_t client = (bpg_net_client_t)ctx;

    assert(client);

    switch(event) {
    case net_ep_event_read:
        bpg_net_client_on_read(client, ep);
        break;
    case net_ep_event_open:
        bpg_net_client_on_open(client, ep);
        break;
    default:
        bpg_net_client_on_close(client, ep, event);
        break;
    }
}

static void bpg_net_client_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    bpg_net_client_t client = (bpg_net_client_t)ctx;

    assert(client);

    mem_free(client->m_alloc, net_chanel_queue_buf(chanel));
}

int bpg_net_client_ep_init(bpg_net_client_t client, net_ep_t ep, size_t read_chanel_size, size_t write_chanel_size) {
    void * buf_r = NULL;
    void * buf_w = NULL;
    net_chanel_t chanel_r = NULL;
    net_chanel_t chanel_w = NULL;

    assert(client);

    buf_r = mem_alloc(client->m_alloc, read_chanel_size);
    buf_w = mem_alloc(client->m_alloc, write_chanel_size);
    if (buf_r == NULL || buf_w == NULL) goto EP_INIT_ERROR;

    chanel_r = net_chanel_queue_create(net_ep_mgr(ep), buf_r, read_chanel_size);
    if (chanel_r == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_r, bpg_net_client_free_chanel_buf, client);
    buf_r = NULL;

    chanel_w = net_chanel_queue_create(net_ep_mgr(ep), buf_w, write_chanel_size);
    if (chanel_w == NULL) goto EP_INIT_ERROR;
    net_chanel_queue_set_close(chanel_w, bpg_net_client_free_chanel_buf, client);
    buf_w = NULL;

    net_ep_set_chanel_r(ep, chanel_r);
    chanel_r = NULL;

    net_ep_set_chanel_w(ep, chanel_w);
    chanel_w = NULL;

    net_ep_set_processor(ep, bpg_net_client_process, client);

    if(client->m_debug) {
        CPE_INFO(
            client->m_em, "%s: ep %d: init success!",
            bpg_net_client_name(client), (int)net_ep_id(ep));
    }

    return 0;
EP_INIT_ERROR:
    if (buf_r) mem_free(client->m_alloc, buf_r);
    if (buf_w) mem_free(client->m_alloc, buf_w);
    if (chanel_r) net_chanel_free(chanel_r);
    if (chanel_w) net_chanel_free(chanel_w);
    net_ep_close(ep);

    CPE_ERROR(
        client->m_em, "%s: ep %d: init fail!",
        bpg_net_client_name(client), (int)net_ep_id(ep));

    return -1;
}

