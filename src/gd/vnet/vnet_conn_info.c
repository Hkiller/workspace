#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/dp/dp_request.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "gd/app/app_context.h"
#include "gd/vnet/vnet_conn_info.h"
#include "vnet_internal_types.h"
#include "protocol/vnet/vnet_data.h"

extern char g_metalib_vnet_metalib[];

dp_req_t vnet_create_conn_info(gd_app_context_t app) {
    dp_req_t req;
    LPDRMETA meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_vnet_metalib, req_type_vnet_conn_info);
    assert(meta);
    req = dp_req_create(gd_app_dp_mgr(app), sizeof(req_type_vnet_conn_info));
    if (req == NULL) return NULL;

    dp_req_set_meta(req, meta);
    dp_req_set_size(req, sizeof(req_type_vnet_conn_info));
    bzero(dp_req_data(req), dp_req_capacity(req));

    return req;
}

int64_t vnet_conn_info_conn_id(dp_req_t data) {
    return ((VNET_CONN_INFO*)dp_req_data(data))->connection_id;
}

void vnet_conn_info_set_conn_id(dp_req_t data, int64_t conn_id) {
    ((VNET_CONN_INFO*)dp_req_data(data))->connection_id = conn_id;
}

dp_req_t vnet_conn_info_check_or_create(dp_req_t target) {
    dp_req_t req = dp_req_child_find(target, req_type_vnet_conn_info);

    if (req == NULL) {
        LPDRMETA meta = dr_lib_find_meta_by_name((LPDRMETALIB)g_metalib_vnet_metalib, req_type_vnet_conn_info);
        assert(meta);
        req = dp_req_create(dp_req_mgr(target), sizeof(req_type_vnet_conn_info));
        if (req == NULL) return NULL;

        dp_req_set_meta(req, meta);
        dp_req_set_size(req, sizeof(req_type_vnet_conn_info));
        bzero(dp_req_data(req), dp_req_capacity(req));

        dp_req_add_to_parent(req, target);
    }

    return req;
}

dp_req_t vnet_conn_info_find(dp_req_t target) {
    return dp_req_child_find(target, req_type_vnet_conn_info);
}

const char * req_type_vnet_conn_info = "vnet_conn_info";
