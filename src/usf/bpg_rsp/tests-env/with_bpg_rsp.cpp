#include <stdexcept>
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "cpe/utils/tests-env/with_em.hpp"
#include "cpe/cfg/tests-env/with_cfg.hpp"
#include "cpe/nm/nm_manage.h"
#include "gd/app/tests-env/with_app.hpp"
#include "usf/logic/tests-env/with_logic.hpp"
#include "usf/bpg_rsp/tests-env/with_bpg_rsp.hpp"
#include "usf/bpg_pkg/tests-env/with_bpg_pkg.hpp"

namespace usf { namespace bpg { namespace testenv {

with_bpg_rsp::with_bpg_rsp() {
}

void with_bpg_rsp::SetUp() {
}

void with_bpg_rsp::TearDown() {
    nm_mgr_free_nodes_with_type_name(
        envOf<gd::app::testenv::with_app>().t_nm(),
        "usf_bpg_rsp_manage");
}

bpg_rsp_manage_t
with_bpg_rsp::t_bpg_rsp_manage(
    const char * name,
    const char * logic_name,
    const char * executor_mgr_name)
{
    bpg_rsp_manage_t mgr = bpg_rsp_manage_find_nc(envOf<gd::app::testenv::with_app>().t_app(), name);
    if (mgr == NULL) {

        error_monitor_t em = 0;
        if (utils::testenv::with_em * with_em = tryEnvOf<utils::testenv::with_em>()) {
            em = with_em->t_em();
        }

        if (executor_mgr_name == NULL) executor_mgr_name = name;
        EXPECT_TRUE(executor_mgr_name) << "executor_mgr_name is NULL";

        logic_executor_mgr_t executor_mgr = 
            envOf<usf::logic::testenv::with_logic>().t_logic_executor_mgr_find(executor_mgr_name);
        if (executor_mgr == NULL) {
            executor_mgr = envOf<usf::logic::testenv::with_logic>().t_logic_executor_mgr_create(executor_mgr_name);
            EXPECT_TRUE(executor_mgr) << "create executor_mgr " << executor_mgr_name << " fail!";
            return NULL;
        }

        mgr = bpg_rsp_manage_create(
            envOf<gd::app::testenv::with_app>().t_app(),
            name,
            bpg_rsp_manage_dp_scope_global,
            envOf<usf::logic::testenv::with_logic>().t_logic_manage(logic_name),
            executor_mgr,
            envOf<usf::bpg::testenv::with_bpg_pkg >().t_bpg_pkg_manage(),
            em);
    }

    return mgr;
}

logic_context_t
with_bpg_rsp::t_bpg_context_create(dp_req_t pkg, const char * rsp_manage_name) {
    error_monitor_t em = 0;
    if (utils::testenv::with_em * with_em = tryEnvOf<utils::testenv::with_em>()) {
        em = with_em->t_em();
    }

    logic_context_t r =  bpg_rsp_manage_create_context(
        t_bpg_rsp_manage(rsp_manage_name), pkg, em);

    return r;
}

}}}
