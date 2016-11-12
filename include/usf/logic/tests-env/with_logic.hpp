#ifndef USF_LOGIC_TESTENV_WITH_LOGIC_H
#define USF_LOGIC_TESTENV_WITH_LOGIC_H
#include "cpe/dr/dr_types.h"
#include "../logic_manage.h"
#include "../logic_executor.h"
#include "../logic_context.h"
#include "cpe/utils/tests-env/test-env.hpp"

namespace usf { namespace logic { namespace testenv {

class with_logic : public ::testenv::env<> {
public:
    with_logic();

    void SetUp();
    void TearDown();

    logic_manage_t t_logic_manage(const char * name = NULL);

    logic_executor_mgr_t t_logic_executor_mgr_create(const char * name);
    logic_executor_mgr_t t_logic_executor_mgr_find(const char * name);

    logic_context_t t_logic_context_create(
        size_t capacity = 0, logic_require_id_t id = INVALID_LOGIC_CONTEXT_ID);

    logic_context_t t_logic_context_create(
        const char * cfg, LPDRMETALIB metalib,
        size_t capacity = 0, logic_require_id_t id = INVALID_LOGIC_CONTEXT_ID);

    logic_context_t t_logic_context_create(
        cfg_t cfg, LPDRMETALIB metalib,
        size_t capacity = 0, logic_require_id_t id = INVALID_LOGIC_CONTEXT_ID);

    logic_context_t t_logic_context_find(logic_context_id_t id);
    logic_context_t t_logic_context(logic_context_id_t id);

    void t_logic_context_install_data(logic_context_t context, const char * cfg, LPDRMETALIB metalib);
    void t_logic_context_install_data(logic_context_t context, cfg_t cfg, LPDRMETALIB metalib);

    void t_logic_execute(logic_context_t context, logic_executor_t executor);

    logic_executor_type_group_t t_logic_executor_type_group(const char * group_name = NULL);

    logic_executor_t t_logic_executor_build(cfg_t cfg, const char * group_name = 0, error_monitor_t em = 0);
    logic_executor_t t_logic_executor_build(const char * cfg, const char * group_name = 0, error_monitor_t em = 0);

    logic_executor_t t_logic_executor_action_create(const char * name, cfg_t args = 0, const char * group_name = 0);
    logic_executor_t t_logic_executor_action_create(const char * name, const char * args, const char * group_name = 0);

    const char * t_logic_executor_dump(logic_executor_t executor);
};

}}}

#endif
