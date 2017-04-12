#include "usf/logic_use/logic_op_async.h"
#include "usf/logic/logic_executor_type.h"
#include "DynDataTest.hpp"

DynDataTest::DynDataTest()
    : m_context(NULL)
{
}

void DynDataTest::SetUp() {
    Base::SetUp();
    t_logic_executor_type_group();
    m_context = t_logic_context_create();
}

void DynDataTest::TearDown() {
    if (m_context) {
        logic_context_free(m_context);
        m_context = NULL;
    }

    Base::TearDown();
}
