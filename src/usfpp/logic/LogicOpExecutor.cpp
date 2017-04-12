#include "usfpp/logic/LogicOpExecutor.hpp"
#include "usfpp/logic/LogicOpContext.hpp"

namespace Usf { namespace Logic {

LogicOpExecutor::LogicOpExecutor(logic_executor_t executor)
    : m_executor(executor)
{
}

LogicOpExecutor::~LogicOpExecutor() {
    if (m_executor) {
        logic_executor_free(m_executor);
        m_executor = 0;
    }
}

void LogicOpExecutor::reset(logic_executor_t executor) {
    if (m_executor) {
        logic_executor_free(m_executor);
    }
    m_executor = executor;
}

LogicOpExecutor &
LogicOpExecutor::operator= (logic_executor_t executor) {
    reset(executor);
    return *this;
}

logic_executor_t LogicOpExecutor::retrieve(void) {
    logic_executor_t r = m_executor;
    m_executor = 0;
    return r;
}

void LogicOpExecutor::execute_at(LogicOpContext & context) const {
    context.execute(m_executor);
}

}}
