#include <assert.h>
#include "cpe/utils/buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/net/net_chanel.h"
#include "cpe/net/net_endpoint.h"
#include "cpe/dp/dp_manage.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/vnet/vnet_conn_info.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_net/bpg_net_agent.h"
#include "bpg_net_internal_ops.h"

static void bpg_net_agent_on_read(bpg_net_agent_t agent, net_ep_t ep) {
    dp_req_t req_buf;
    dp_req_t conn_info;

    if(agent->m_debug >= 2) {
        CPE_INFO(
            agent->m_em, "%s: ep %d: on read",
            bpg_net_agent_name(agent), (int)net_ep_id(ep));
    }

    req_buf = bpg_net_agent_req_buf(agent);
    if (req_buf == NULL) {
        CPE_ERROR(
            agent->m_em, "%s: ep %d: get req buf fail!",
            bpg_net_agent_name(agent), (int)net_ep_id(ep));
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
                agent->m_em, "%s: ep %d: peek data fail, size=%d!",
                bpg_net_agent_name(agent), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }

        input_size = buf_size;

        bpg_pkg_init(req_buf);

        cvt_result =
            bpg_pkg_decode(
                req_buf,
                buf, &input_size, agent->m_em, agent->m_debug >= 2 ? 1 : 0);
        if (cvt_result == dr_cvt_result_not_enough_input) {
            if(agent->m_debug) {
                CPE_ERROR(
                    agent->m_em, "%s: ep %d: not enough data, input size is %d!",
                bpg_net_agent_name(agent), (int)net_ep_id(ep), (int)buf_size);
            }
            break;
        }
        else if (cvt_result != dr_cvt_result_success) {
            CPE_ERROR(
                agent->m_em, "%s: ep %d: decode package fail, input size is %d!",
                bpg_net_agent_name(agent), (int)net_ep_id(ep), (int)buf_size);
            net_ep_close(ep);
            break;
        }
        net_ep_erase(ep, input_size);

        if(agent->m_debug >= 2) {
            CPE_INFO(
                agent->m_em, "%s: ep %d: decode one package, buf-origin-size=%d left-size=%d!",
                bpg_net_agent_name(agent), (int)net_ep_id(ep), (int)buf_size, (int)net_ep_size(ep));
        }

        if (agent->m_debug) {
            CPE_ERROR(
                agent->m_em,
                "%s: ep %d: <== recv on request\n%s",
                bpg_net_agent_name(agent), (int)net_ep_id(ep),
                bpg_pkg_dump(req_buf, &agent->m_dump_buffer));
        }

        if ((conn_info = vnet_conn_info_check_or_create(req_buf)) == NULL) {
            CPE_ERROR(
                agent->m_em, "%s: ep %d: set connection info error!",
                bpg_net_agent_name(agent), (int)net_ep_id(ep));
            bpg_pkg_set_errno(req_buf, -1);
            bpg_net_agent_reply(req_buf, agent, agent->m_em);
        }
        else {
            vnet_conn_info_set_conn_id(conn_info, net_ep_id(ep));
            if (agent->m_dispatch_to) {
                if (dp_dispatch_by_string(agent->m_dispatch_to, dp_req_mgr(req_buf), req_buf, agent->m_em) != 0) {
                    CPE_ERROR(
                        agent->m_em, "%s: ep %d: dispatch to %s error!",
                        bpg_net_agent_name(agent), (int)net_ep_id(ep), cpe_hs_data(agent->m_dispatch_to));

                    bpg_pkg_set_errno(req_buf, -1);
                    bpg_net_agent_reply(req_buf, agent, agent->m_em);
                }
            }
            else {
                if (dp_dispatch_by_numeric(bpg_pkg_cmd(req_buf), dp_req_mgr(req_buf), req_buf, agent->m_em) != 0) {
                    CPE_ERROR(
                        agent->m_em, "%s: ep %d: dispatch cmd %d error!",
                        bpg_net_agent_name(agent), (int)net_ep_id(ep), bpg_pkg_cmd(req_buf));

                    bpg_pkg_set_errno(req_buf, -1);
                    bpg_net_agent_reply(req_buf, agent, agent->m_em);
                }
            }
        }
    }
}

static void bpg_net_agent_on_open(bpg_net_agent_t agent, net_ep_t ep) {
    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: ep %d: on open",
            bpg_net_agent_name(agent), (int)net_ep_id(ep));
    }
}

static void bpg_net_agent_on_close(bpg_net_agent_t agent, net_ep_t ep, net_ep_event_t event) {
    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: ep %d: on close, event=%d",
            bpg_net_agent_name(agent), (int)net_ep_id(ep), event);
    }

    net_ep_free(ep);
}

static void bpg_net_agent_process(net_ep_t ep, void * ctx, net_ep_event_t event) {
    bpg_net_agent_t agent = (bpg_net_agent_t)ctx;

    assert(agent);

    switch(event) {
    case net_ep_event_read:
        bpg_net_agent_on_read(agent, ep);
        break;
    case net_ep_event_open:
        bpg_net_agent_on_open(agent, ep);
        break;
    default:
        bpg_net_agent_on_close(agent, ep, event);
        break;
    }
}

static void bpg_net_agent_free_chanel_buf(net_chanel_t chanel, void * ctx) {
    bpg_net_agent_t agent = (bpg_net_agent_t)ctx;

    assert(agent);

    mem_free(agent->m_alloc, net_chanel_queue_buf(chanel));
}


void bpg_net_agent_accept(net_listener_t listener, net_ep_t ep, void * ctx) {
    bpg_net_agent_t agent = (bpg_net_agent_t)ctx;
    void * buf_r = NULL;
    void * buf_w = NULL;
    net_chanel_t chanel_r = NULL;
    net_chanel_t chanel_w = NULL;

    assert(agent);

    buf_r = mem_alloc(agent->m_alloc, agent->m_read_chanel_size);
    buf_w = mem_alloc(agent->m_alloc, agent->m_write_chanel_size);
    if (buf_r == NULL || buf_w == NULL) goto ACCEPT_ERROR;

    chanel_r = net_chanel_queue_create(net_ep_mgr(ep), buf_r, agent->m_read_chanel_size);
    if (chanel_r == NULL) goto ACCEPT_ERROR;
    net_chanel_queue_set_close(chanel_r, bpg_net_agent_free_chanel_buf, agent);
    buf_r = NULL;

    chanel_w = net_chanel_queue_create(net_ep_mgr(ep), buf_w, agent->m_write_chanel_size);
    if (chanel_w == NULL) goto ACCEPT_ERROR;
    net_chanel_queue_set_close(chanel_w, bpg_net_agent_free_chanel_buf, agent);
    buf_w = NULL;

    net_ep_set_chanel_r(ep, chanel_r);
    chanel_r = NULL;

    net_ep_set_chanel_w(ep, chanel_w);
    chanel_w = NULL;

    net_ep_set_processor(ep, bpg_net_agent_process, agent);
    net_ep_set_timeout(ep, agent->m_conn_timeout);

    if(agent->m_debug) {
        CPE_INFO(
            agent->m_em, "%s: ep %d: accept success!",
            bpg_net_agent_name(agent), (int)net_ep_id(ep));
    }

    return;
ACCEPT_ERROR:
    if (buf_r) mem_free(agent->m_alloc, buf_r);
    if (buf_w) mem_free(agent->m_alloc, buf_w);
    if (chanel_r) net_chanel_free(chanel_r);
    if (chanel_w) net_chanel_free(chanel_w);
    net_ep_close(ep);

    CPE_ERROR(
        agent->m_em, "%s: ep %d: accept fail!",
        bpg_net_agent_name(agent), (int)net_ep_id(ep));
}

