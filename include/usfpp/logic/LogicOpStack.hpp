#ifndef USFPP_LOGIC_STACK_H
#define USFPP_LOGIC_STACK_H
#include "cpepp/utils/ClassCategory.hpp"
#include "usf/logic/logic_data.h"
#include "usf/logic/logic_stack.h"
#include "LogicOpData.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Logic {

class LogicOpStackNode : public Cpe::Utils::SimulateObject {
public:
    operator logic_stack_node_t () const { return (logic_stack_node_t)this; }

    LogicOpExecutor & executor(void) { return *(LogicOpExecutor*)logic_stack_node_executor(*this); }
    LogicOpExecutor const & executor(void) const { return *(LogicOpExecutor*)logic_stack_node_executor(*this); }

    LogicOpContext & context(void) { return *(LogicOpContext*)logic_stack_node_context(*this); }
    LogicOpContext const & context(void) const { return *(LogicOpContext*)logic_stack_node_context(*this); }

    LogicOpData & data(const char * name);
    LogicOpData const & data(const char * name) const;

    LogicOpRequire & createRequire(const char * name);

    LogicOpData * findData(const char * name) { return (LogicOpData *)logic_stack_data_find(*this, name); }
    LogicOpData const * findData(const char * name) const { return (LogicOpData *)logic_stack_data_find(*this, name); }

    LogicOpData & checkCreateData(LPDRMETA meta, size_t capacity = 0);
    LogicOpData & copy(logic_data_t input);

    template<typename T>
    LogicOpData & copy(T const & data) {
        LogicOpData & r = checkCreateData(Cpe::Dr::MetaTraits<T>::META, Cpe::Dr::MetaTraits<T>::data_size(data));
        memcpy(r.data(), &data, Cpe::Dr::MetaTraits<T>::data_size(data));
        return r;
    }

    bool deleteData(const char * name)  {
        if (logic_data_t r = logic_stack_data_find(*this, name)) {
            logic_data_free(r);
            return true;
        }
        else {
            return false;
        }
    }

    template<typename T>
    T & data(const char * name = Cpe::Dr::MetaTraits<T>::NAME) { return data(name).as<T>(); }

    template<typename T>
    T const & data(const char * name = Cpe::Dr::MetaTraits<T>::NAME) const { return data(name).as<T>(); }

    template<typename T>
    T & checkCreateData(size_t capacity = 0, LPDRMETA meta = Cpe::Dr::MetaTraits<T>::META) {
        return checkCreateData(meta, capacity).as<T>();
    }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
