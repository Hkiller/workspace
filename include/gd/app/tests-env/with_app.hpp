#ifndef GD_APP_TESTENV_WITHAPP_H
#define GD_APP_TESTENV_WITHAPP_H
#include "cpe/utils/tests-env/test-env.hpp"
#include "../app.h"

namespace gd { namespace app { namespace testenv {

class with_app : public ::testenv::env<> {
public:
    with_app();

    void SetUp();
    void TearDown();

    gd_app_context_t t_app(void);

    void t_app_create(size_t capacity, int argc, char ** argv);
    void t_app_free(void);

    gd_app_module_t
    t_app_install_module(
        const char * name,
        const char * type_name,
        const char * libName,
        const char * cfg);

    gd_app_module_t
    t_app_install_module(
        const char * name,
        const char * type_name,
        const char * cfg);

    void t_app_init_module_type(
        const char * name,
        gd_app_module_app_init app_init,
        gd_app_module_app_fini app_fini,
        gd_app_module_global_init global_init = NULL,
        gd_app_module_global_fini global_fini = NULL);
    void t_app_fini_module_type(const char * name);

    int t_app_uninstall_module(const char * name);
    void t_app_uninstall_modules_by_type(const char * name);

    int t_app_install_rsps(
        gd_app_module_t module,
        const char * cfg);

    int t_app_install_rsps(
        const char * moduleName,
        const char * cfg);

    dp_mgr_t t_dp(void);
    nm_mgr_t t_nm(void);
    net_mgr_t t_net(void);

    gd_app_module_t t_app_find_module(const char * moduleName);

    void t_app_set_timer_source_last_event(void);
    int t_app_tick(int count = -1);
    void t_app_net_run(void);
private:
    gd_app_context_t m_app;
};

}}}

#endif
