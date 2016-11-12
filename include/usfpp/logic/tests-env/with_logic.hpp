#ifndef USFPP_LOGIC_TESTENV_WITH_LOGIC_H
#define USFPP_LOGIC_TESTENV_WITH_LOGIC_H
#include "usf/logic/tests-env/with_logic.hpp"
#include "../LogicOpManager.hpp"

namespace Usf { namespace Logic { namespace testenv {

class with_logic : public usf::logic::testenv::with_logic {
    typedef usf::logic::testenv::with_logic Base;
public:
    with_logic();

    void SetUp();
    void TearDown();

    void t_logic_op_regist(const char * name, const char * group_name = 0);

    LogicOpManager & t_logic_manage_ex(const char * name = NULL) { return *(LogicOpManager*)t_logic_manage(name); }
};

}}}

#endif
