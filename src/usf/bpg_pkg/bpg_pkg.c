#include <assert.h>
#include "cpe/pal/pal_strings.h"
#include "cpe/pal/pal_platform.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpe/dp/dp_request.h"
#include "gd/app/app_context.h"
#include "gd/dr_store/dr_ref.h"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usf/bpg_pkg/bpg_pkg_data.h"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "protocol/base/base_package.h"
#include "bpg_pkg_internal_types.h"

bpg_pkg_t bpg_pkg_create(bpg_pkg_manage_t mgr) {
    dp_req_t dp_req;
    bpg_pkg_t bpg_pkg;

    assert(mgr);

    dp_req = dp_req_create(gd_app_dp_mgr(mgr->m_app), sizeof(struct bpg_pkg));
    if (dp_req == NULL) return NULL;

    dp_req_set_type(dp_req, req_type_bpg_pkg);

    bzero(dp_req_data(dp_req), dp_req_capacity(dp_req));

    bpg_pkg = (bpg_pkg_t)dp_req_data(dp_req);
    bpg_pkg->m_mgr = mgr;
    bpg_pkg->m_dp_req = dp_req;

    return bpg_pkg;
}

dp_req_t bpg_pkg_create_with_body(bpg_pkg_manage_t mgr, size_t capacity) {
    dp_req_t body;
    bpg_pkg_t bpg_pkg;
    LPDRMETALIB metalib;
    LPDRMETA meta;

    assert(mgr);

    metalib = dr_ref_lib(mgr->m_metalib_basepkg_ref);
    meta = metalib ? dr_lib_find_meta_by_name(metalib, "basepkg") : NULL;
    if (meta == NULL) return NULL; 

    body = dp_req_create(gd_app_dp_mgr(mgr->m_app), capacity);
    if (body == NULL) return NULL;

    dp_req_set_meta(body, meta);

    bpg_pkg = bpg_pkg_create(mgr);
    if (bpg_pkg == NULL) {
        dp_req_free(body);
        return NULL;
    }

    bpg_pkg_init(body);
    
    dp_req_add_to_parent(bpg_pkg_to_dp_req(bpg_pkg), body);

    return body;
}

dp_req_t bpg_pkg_create_with_body_by_data_capacity(bpg_pkg_manage_t mgr, size_t capacity) {
    return bpg_pkg_create_with_body(mgr, sizeof(BASEPKG_HEAD) + capacity);
}

void bpg_pkg_free(bpg_pkg_t req) {
    dp_req_free(req->m_dp_req);
}

bpg_pkg_manage_t bpg_pkg_mgr(bpg_pkg_t req) {
    return req->m_mgr;
}

size_t bpg_pkg_pkg_data_capacity(bpg_pkg_t req) {
    return dp_req_capacity(req->m_dp_req) - sizeof(struct bpg_pkg);
}

bpg_pkg_t bpg_pkg_find(dp_req_t req) {
    dp_req_t pkg = dp_req_child_find(req, req_type_bpg_pkg);
    return pkg ? (bpg_pkg_t)dp_req_data(pkg) : NULL;
}

dp_req_t bpg_pkg_to_dp_req(bpg_pkg_t pkg) {
    return pkg->m_dp_req;
}

dr_cvt_t bpg_pkg_data_cvt(bpg_pkg_t pkg) {
    return pkg->m_mgr->m_data_cvt;
}

dr_cvt_t bpg_pkg_base_cvt(bpg_pkg_t pkg) {
    return pkg->m_mgr->m_base_cvt;
}

LPDRMETA bpg_pkg_base_meta(bpg_pkg_t pkg) {
    LPDRMETALIB metalib;
    metalib = dr_ref_lib(pkg->m_mgr->m_metalib_basepkg_ref);
    return metalib ? dr_lib_find_meta_by_name(metalib, "basepkg") : NULL;
}

const char * req_type_bpg_pkg = "bpg_pkg";

