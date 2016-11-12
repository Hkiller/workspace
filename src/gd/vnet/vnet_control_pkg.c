#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/vnet/vnet_control_pkg.h"
#include "vnet_internal_types.h"

vnet_control_pkg_t
vnet_control_pkg_create(gd_app_context_t app, size_t pkg_capacity) {
    dp_req_t dp_req;
    vnet_control_pkg_t pkg;

    dp_req = dp_req_create(gd_app_dp_mgr(app), sizeof(struct vnet_control_pkg) + pkg_capacity);
    if (dp_req == NULL) return NULL;

    dp_req_set_type(dp_req, req_type_vnet_control_pkg);

    pkg = (vnet_control_pkg_t)dp_req_data(dp_req);

    pkg->m_app = app;
    pkg->m_dp_req = dp_req;
    pkg->m_cmd = 0;
    pkg->m_connector_id = 0;
    vnet_control_pkg_init(pkg);

    return pkg;
}

void vnet_control_pkg_free(vnet_control_pkg_t req) {
    dp_req_free(req->m_dp_req);
}

dp_req_t vnet_control_pkg_to_dp_req(vnet_control_pkg_t req) {
    return req->m_dp_req;
}

vnet_control_pkg_t vnet_control_pkg_from_dp_req(dp_req_t req) {
    if (!dp_req_is_type(req, req_type_vnet_control_pkg)) return NULL;
    return (vnet_control_pkg_t)dp_req_data(req);
}


gd_app_context_t vnet_control_pkg_app(vnet_control_pkg_t req) {
    return req->m_app;
}

void vnet_control_pkg_init(vnet_control_pkg_t pkg) {
    pkg->m_data_meta = NULL;
    dp_req_set_size(pkg->m_dp_req, sizeof(struct vnet_control_pkg));
}

uint32_t vnet_control_pkg_cmd(vnet_control_pkg_t pkg) {
    return pkg->m_cmd;
}

void vnet_control_pkg_set_cmd(vnet_control_pkg_t pkg, uint32_t cmd) {
    pkg->m_cmd = cmd;
}

uint32_t vnet_control_pkg_connection_id(vnet_control_pkg_t pkg) {
    return pkg->m_connector_id;
}

void vnet_control_pkg_set_connection_id(vnet_control_pkg_t pkg, uint32_t connection_id) {
    pkg->m_connector_id = connection_id;
}

LPDRMETA vnet_control_pkg_data_meta(vnet_control_pkg_t pkg) {
    return pkg->m_data_meta;
}

void * vnet_control_pkg_data_buf(vnet_control_pkg_t pkg) {
    return (pkg + 1);
}

size_t vnet_control_pkg_data_size(vnet_control_pkg_t pkg) {
    return dp_req_size(pkg->m_dp_req) - sizeof(struct vnet_control_pkg);
}

size_t vnet_control_pkg_data_capacity(vnet_control_pkg_t pkg) {
    return dp_req_capacity(pkg->m_dp_req) - sizeof(struct vnet_control_pkg);
}

int vnet_control_pkg_set_data(vnet_control_pkg_t pkg, LPDRMETA meta, void const * data, size_t size) {
    size_t total_size = size + sizeof(struct vnet_control_pkg);
    if (total_size > dp_req_capacity(pkg->m_dp_req)) {
        CPE_ERROR(gd_app_em(pkg->m_app), "vnet_control_pkg_set_data: size overflow"); 
        return -1;
    }

    dp_req_set_size(pkg->m_dp_req, total_size);
    pkg->m_data_meta = meta;

    memcpy(pkg + 1, data, size);

    return 0;
}

void * vnet_control_pkg_alloc_data(vnet_control_pkg_t pkg, LPDRMETA meta, size_t size) {
    size_t total_size = size + sizeof(struct vnet_control_pkg);
    if (total_size > dp_req_capacity(pkg->m_dp_req)) {
        CPE_ERROR(gd_app_em(pkg->m_app), "vnet_control_pkg_alloc_data: size overflow"); 
        return NULL;
    }

    dp_req_set_size(pkg->m_dp_req, total_size);
    pkg->m_data_meta = meta;

    return pkg + 1;
}

const char * req_type_vnet_control_pkg = "vnet_control_pkg";

