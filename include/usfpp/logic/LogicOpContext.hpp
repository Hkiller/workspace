#ifndef USFPP_LOGIC_OPCONTEXT_H
#define USFPP_LOGIC_OPCONTEXT_H
#include "cpe/utils/hash_string.h"
#include "cpepp/utils/System.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/logic/logic_context.h"
#include "usf/logic/logic_data.h"
#include "LogicOpData.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Logic {

class LogicOpContext : public Cpe::Utils::SimulateObject  {
public:
    operator logic_context_t () const { return (logic_context_t)this; }

    LogicOpContextID id(void) const { return logic_context_id(*this); }

    logic_context_state_t state(void) const { return logic_context_state(*this); }

    size_t contextCapacity(void) const { return logic_context_capacity(*this); }
    void * contextData(void) { return logic_context_data(*this); }
    const void * contextData(void) const { return logic_context_data(*this); }

    uint32_t flags(void) const { return logic_context_flags(*this); }
    void setFlags(uint32_t flag) { logic_context_flags_set(*this, flag); }
    void enableFlag(logic_context_flag_t flag) { logic_context_flag_enable(*this, flag); }
    void disableFlag(logic_context_flag_t flag) { logic_context_flag_disable(*this, flag); }
    bool isFlagEnable(logic_context_flag_t flag) const { return logic_context_flag_is_enable(*this, flag) ? true : false; }

    bool debug(void) const  { return logic_context_flag_is_enable(*this, logic_context_flag_debug) ? true : false; }
    void setErrorNo(int32_t err) { logic_context_errno_set(*this, err); }
    int32_t errorNo(void) { return logic_context_errno(*this); }

    LogicOpManager & mgr(void) { return *(LogicOpManager*)logic_context_mgr(*this); }
    LogicOpManager const & mgr(void) const { return *(LogicOpManager*)logic_context_mgr(*this); }

    Cpe::Utils::Random & random(cpe_hash_string_t name = 0);

    void bind(logic_executor_t executor);
    void execute(void) { logic_context_execute(*this); }
    void execute(logic_executor_t execute) { bind(execute); logic_context_execute(*this); }

    void cancel(void) { logic_context_cancel(*this); }
    void timeout(void) { logic_context_timeout(*this); }

    bool timeoutIsStart(void) const { return logic_context_timeout_is_start(*this) ? true : false; }
    void timeoutStart(tl_time_span_t timeout_ms);
    void timeoutStop(void) { logic_context_timeout_stop(*this); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(logic_context_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(logic_context_app(*this)); }

    LogicOpData & data(const char * name);
    LogicOpData const & data(const char * name) const;

    LogicOpData * findData(const char * name) { return (LogicOpData *)logic_context_data_find(*this, name); }
    LogicOpData const * findData(const char * name) const { return (LogicOpData *)logic_context_data_find(*this, name); }
    LogicOpData & checkCreateData(LPDRMETA meta, size_t capacity = 0);
    LogicOpData & copy(logic_data_t input);

    LogicOpData & copy(LogicOpData const & input) { return copy((logic_data_t)input); }

    template<typename T>
    LogicOpData & copy(T const & data) {
        LogicOpData & r = checkCreateData(Cpe::Dr::MetaTraits<T>::META, Cpe::Dr::MetaTraits<T>::data_size(data));
        memcpy(r.data(), &data, Cpe::Dr::MetaTraits<T>::data_size(data));
        return r;
    }

    bool deleteData(const char * name)  {
        if (logic_data_t r = logic_context_data_find(*this, name)) {
            logic_data_free(r);
            return true;
        }
        else {
            return false;
        }
    }

    template<typename T>
    T & contextData(void) { return *(T*)contextData(); }

    template<typename T>
    T const & contextData(void) const { return *(T const *)contextData(); }

    template<typename T>
    T & data(const char * name = Cpe::Dr::MetaTraits<T>::NAME) { return data(name).as<T>(); }

    template<typename T>
    T const & data(const char * name = Cpe::Dr::MetaTraits<T>::NAME) const { return data(name).as<T>(); }

    template<typename T>
    T * findData(const char * name = Cpe::Dr::MetaTraits<T>::NAME) { 
        LogicOpData * r = findData(name);
        return r ? &r->as<T>() : NULL;
    }

    template<typename T>
    T const * findData(const char * name = Cpe::Dr::MetaTraits<T>::NAME) const { 
        LogicOpData const * r = findData(name);
        return r ? &r->as<T>() : NULL;
    }

    template<typename T>
    bool deleteData(void) { return deleteData(Cpe::Dr::MetaTraits<T>::NAME); }

    template<typename T>
    T & checkCreateData(size_t capacity = 0, LPDRMETA meta = Cpe::Dr::MetaTraits<T>::META) {
        return checkCreateData(meta, capacity).as<T>();
    }

    void dump_data(cfg_t cfg) const { logic_context_data_dump_to_cfg(*this, cfg); }

    void destory(void) { logic_context_free(*this); }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
