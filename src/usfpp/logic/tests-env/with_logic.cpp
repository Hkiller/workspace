#include "gd/app/tests-env/with_app.hpp"
#include "usfpp/logic/LogicOp.hpp"
#include "usfpp/logic/tests-env/with_logic.hpp"

namespace Usf { namespace Logic { namespace testenv {

with_logic::with_logic() {
}

void with_logic::SetUp() {
    Base::SetUp();
}

void with_logic::TearDown() {
    Base::TearDown();
}

void with_logic::t_logic_op_regist(const char * name, const char * group_name) {
    logic_executor_type_group_t group = t_logic_executor_type_group(group_name);
    ASSERT_TRUE(group) << "logic executor type group " << group_name << " not exist!";

    LogicOp::get(envOf<gd::app::testenv::with_app>().t_app(), name).regist_to(group);
}

}}}
