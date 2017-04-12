#include "cpe/utils/error.h"
#include "cpe/dp/dp_request.h"
#include "gd/vnet/vnet_control_pkg.h"
#include "gd/vnet/vnet_conn_info.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_dsp.h"
#include "usf/bpg_bind/bpg_bind_manage.h"
#include "bpg_bind_internal_ops.h"

int bpg_bind_manage_outgoing_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_bind_manage_t mgr = (bpg_bind_manage_t)ctx;
    bpg_pkg_t pkg;

    pkg = bpg_pkg_find(req);
    if (pkg == NULL) {
        if (bpg_pkg_dsp_pass(mgr->m_outgoing_send_to, req, em) != 0) {
            CPE_ERROR(
                em, "%s: outgoing: forward %s pkg fail!",
                bpg_bind_manage_name(mgr), dp_req_type(req));
            return -1;
        }
        else {
            if (mgr->m_debug) {
                CPE_INFO (
                em, "%s: outgoing: forward %s pkg success!",
                bpg_bind_manage_name(mgr), dp_req_type(req));
            }

            return 0;
        }
    }

    return bpg_pkg_dsp_dispatch(mgr->m_outgoing_send_to, req, em);
}

static void bpg_bind_manage_kickoff_old(
    bpg_bind_manage_t mgr,
    uint32_t new_client_id,
    uint32_t old_connection_id,
    uint32_t old_client_id,
    error_monitor_t em)
{
    dp_req_t kickoff_pkg;
    vnet_control_pkg_t control_pkg;

    kickoff_pkg = bpg_bind_manage_data_pkg(mgr);
    if (kickoff_pkg == NULL) {
        CPE_ERROR(
            mgr->m_em, "%s: kickoff_old: get data pkg fail!",
            bpg_bind_manage_name(mgr));
    }
    else {
        if (mgr->m_cmd_kickoff > 0){
            dp_req_t vnet_conn_info;

            bpg_pkg_init(kickoff_pkg);
            bpg_pkg_set_cmd(kickoff_pkg, mgr->m_cmd_kickoff);
            bpg_pkg_set_client_id(kickoff_pkg, old_client_id);

            vnet_conn_info = vnet_conn_info_check_or_create(kickoff_pkg);
            if (vnet_conn_info == NULL) {
                CPE_ERROR(mgr->m_em, "%s: kickoff_old: check_create_connection_info fail!", bpg_bind_manage_name(mgr));
            }
            else {
                vnet_conn_info_set_conn_id(vnet_conn_info, old_connection_id);

                if (bpg_pkg_dsp_dispatch(mgr->m_outgoing_send_to, kickoff_pkg, em) != 0) {
                    CPE_ERROR(
                        mgr->m_em, "%s: kickoff_old: send kickof pkg fail, client-id=%d, connection-id=%d!",
                        bpg_bind_manage_name(mgr), new_client_id, old_connection_id);
                }
                else {
                    if (mgr->m_debug) {
                        CPE_INFO(
                            mgr->m_em, "%s: kickoff_old: send kickof pkg success, client-id=%d, connection-id=%d!",
                            bpg_bind_manage_name(mgr), new_client_id, old_connection_id);
                    }
                }
            }
        }
    }

    control_pkg = bpg_bind_manage_control_pkg(mgr);
    if (kickoff_pkg == NULL) {
        CPE_ERROR(mgr->m_em, "bpg_bind_manage_kickoff_old: send control pkg fail!");
    }
    else {
        vnet_control_pkg_init(control_pkg);
        vnet_control_pkg_set_cmd(control_pkg, vnet_control_op_disconnect);
        vnet_control_pkg_set_connection_id(control_pkg, old_connection_id);
        if (bpg_pkg_dsp_pass(mgr->m_outgoing_send_to, vnet_control_pkg_to_dp_req(control_pkg), em) != 0) {
            CPE_ERROR(
                mgr->m_em, "%s: kickoff_old: send control pkg fail, client-id=%d, connection-id=%d!",
                bpg_bind_manage_name(mgr), new_client_id, old_connection_id);
        }
        else {
            if (mgr->m_debug) {
                CPE_INFO(
                    mgr->m_em, "%s: kickoff_old: send control pkg success, client-id=%d, connection-id=%d!",
                    bpg_bind_manage_name(mgr), new_client_id, old_connection_id);
            }
        }
    }
}

int bpg_bind_manage_incoming_rsp(dp_req_t req, void * ctx, error_monitor_t em) {
    bpg_bind_manage_t mgr = (bpg_bind_manage_t)ctx;
    struct bpg_bind_binding * found_binding;
    uint32_t connection_id;
    uint32_t client_id;
    dp_req_t vnet_conn_info;

    if (bpg_pkg_find(req) == NULL) {
        if (bpg_pkg_dsp_pass(mgr->m_incoming_send_to, req, em) != 0) {
            CPE_ERROR(
                em, "%s: incoming: forward %s pkg fail!",
                bpg_bind_manage_name(mgr), dp_req_type(req));
            return -1;
        }
        else {
            if (mgr->m_debug) {
                CPE_INFO (
                em, "%s: incoming: forward %s pkg success!",
                bpg_bind_manage_name(mgr), dp_req_type(req));
            }
            return 0;
        }
    }

    if ((vnet_conn_info = vnet_conn_info_find(req)) == NULL) {
        if (mgr->m_debug) {
            CPE_INFO(em, "%s: incoming: incoming pkg no conn_info!", bpg_bind_manage_name(mgr));
        }

        if (bpg_pkg_dsp_dispatch(mgr->m_incoming_send_to, req, em) != 0) {
            CPE_ERROR(
                mgr->m_em, "%s: dispatch cmd %d error!",
                bpg_bind_manage_name(mgr), bpg_pkg_cmd(req));
            return -1;
        }

        return 0;
    }

    connection_id = vnet_conn_info_conn_id(vnet_conn_info);
    client_id = bpg_pkg_client_id(req);

    if (connection_id == BPG_INVALID_CONNECTION_ID) { 
        CPE_ERROR(
            mgr->m_em, "%s: dispatch cmd %d: connection id invalid!",
            bpg_bind_manage_name(mgr), bpg_pkg_cmd(req));
        return -1;
    }

    if (client_id) {
        found_binding = bpg_bind_binding_find_by_client_id(mgr, client_id);
        if (found_binding) {
            if (found_binding->m_connection_id != connection_id) {
                bpg_bind_manage_kickoff_old(
                    mgr, client_id, found_binding->m_connection_id, found_binding->m_client_id, mgr->m_em);
                bpg_bind_binding_free(mgr, found_binding);
            }
        }
    }

    found_binding = bpg_bind_binding_find_by_connection_id(mgr, connection_id); 
    if (found_binding) {
        if (found_binding->m_client_id != client_id) {
            bpg_bind_binding_free(mgr, found_binding);
            found_binding = NULL;
        }
    }

    if (found_binding == NULL && client_id != 0) {
        if (bpg_bind_binding_create(mgr, client_id, connection_id) != 0) { 
            CPE_ERROR( 
                mgr->m_em, "%s: ep %d: binding: create binding fail!", 
                bpg_bind_manage_name(mgr), (int)connection_id); 
        } 
    }

    if (bpg_pkg_dsp_dispatch(mgr->m_incoming_send_to, req, em) != 0) {
        CPE_ERROR(
            mgr->m_em, "%s: dispatch cmd %d error!",
            bpg_bind_manage_name(mgr), bpg_pkg_cmd(req));
        return -1;
    }

    if (mgr->m_debug) {
        CPE_INFO(
            mgr->m_em, "%s: dispatch cmd %d success, connection-id=%d, client-id=%d!",
            bpg_bind_manage_name(mgr), bpg_pkg_cmd(req), connection_id, client_id);
    }

    return 0;
}
