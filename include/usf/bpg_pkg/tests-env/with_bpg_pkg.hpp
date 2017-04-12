#ifndef USF_BPG_PKG_TESTENV_WITH_BPG_PKG_H
#define USF_BPG_PKG_TESTENV_WITH_BPG_PKG_H
#include "../bpg_pkg_manage.h"
#include "../bpg_pkg.h"
#include "../bpg_pkg_data.h"
#include "cpe/utils/tests-env/test-env.hpp"

namespace usf { namespace bpg { namespace testenv {

class with_bpg_pkg : public ::testenv::env<> {
public:
    with_bpg_pkg();

    void SetUp();
    void TearDown();

    bpg_pkg_manage_t t_bpg_pkg_manage(const char * name = NULL);

    void t_bpg_pkg_manage_set_cvt(
        const char * data_cvt,
        const char * base_cvt,
        const char * mgr_name = NULL);

    void t_bpg_pkg_manage_set_model(
        const char * model,
        uint8_t dft_align = 0,
        const char * mgr_name = NULL);

    void t_bpg_pkg_manage_add_cmd(uint32_t cmd, const char * meta_name, const char * mgr_name = NULL);
    void t_bpg_pkg_manage_add_cmd_by_meta(const char * cmd_meta_name, const char * mgr_name = NULL);

    dp_req_t t_bpg_pkg_create(
        size_t capacity = 1024,
        const char * mgr_name = NULL);

    const char * t_bpg_pkg_dump(dp_req_t body);

    dp_req_t t_bpg_pkg_build(const char * cfg, const char * mgr_name = NULL);
    dp_req_t t_bpg_pkg_build(cfg_t cfg, const char * mgr_name = NULL);

    void t_bpg_pkg_init(dp_req_t pkg, const char * cfg);
    void t_bpg_pkg_init(dp_req_t pkg, cfg_t cfg);
};

}}}

#endif
