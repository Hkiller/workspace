#ifndef USFPP_LOGIC_LOGICOPEXECUTOR_H
#define USFPP_LOGIC_LOGICOPEXECUTOR_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/logic/logic_executor.h"
#include "System.hpp"

namespace Usf { namespace Logic {

class LogicOpExecutor : public Cpe::Utils::Noncopyable {
public:
    LogicOpExecutor(logic_executor_t executor = 0);
    ~LogicOpExecutor();

    LogicOpExecutor & operator= (logic_executor_t executor);

    void reset(logic_executor_t executor);
    logic_executor_t retrieve(void);

    operator logic_executor_t() const { return m_executor; }
    logic_executor_t get(void) { return m_executor; }

    void execute_at(LogicOpContext & context) const;

private:
    logic_executor_t m_executor;
};

}}

#endif
