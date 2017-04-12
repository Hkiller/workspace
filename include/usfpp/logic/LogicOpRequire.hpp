#ifndef USFPP_LOGIC_OPREQUIRE_H
#define USFPP_LOGIC_OPREQUIRE_H
#include "cpe/utils/hash_string.h"
#include "cpepp/utils/System.hpp"
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/logic/logic_require.h"
#include "usf/logic/logic_data.h"
#include "LogicOpData.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Logic {

class LogicOpRequire : public Cpe::Utils::SimulateObject  {
public:
    operator logic_require_t () const { return (logic_require_t)this; }

    logic_require_id_t id(void) const { return logic_require_id(*this); }

    logic_require_state_t state(void) const { return logic_require_state(*this); }

	Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(logic_require_name(*this)); }

    LogicOpContext & context(void) { return *(LogicOpContext*)logic_require_context(*this); }
    LogicOpContext const & context(void) const { return *(LogicOpContext*)logic_require_context(*this); }

    void cancel(void) { logic_require_cancel(*this); }
    int32_t error(void) const { return logic_require_error(*this); }
    void setError(int32_t err = -1) { logic_require_set_error_ex(*this, err); }
    void setDone(void) { logic_require_set_done(*this); }

    bool timeoutIsStart(void) const { return logic_require_timeout_is_start(*this) ? true : false; }
    void timeoutStart(tl_time_span_t timeout_ms);
    void timeoutStop(void) { logic_require_timeout_stop(*this); }

    LogicOpData & data(const char * name);
    LogicOpData const & data(const char * name) const;

    LogicOpData * findData(const char * name) { return (LogicOpData *)logic_require_data_find(*this, name); }
    LogicOpData const * findData(const char * name) const { return (LogicOpData *)logic_require_data_find(*this, name); }
    LogicOpData & checkCreateData(LPDRMETA meta, size_t capacity = 0);
    LogicOpData & copy(logic_data_t input);

    template<typename T>
    LogicOpData & copy(T const & data) {
        LogicOpData & r = checkCreateData(Cpe::Dr::MetaTraits<T>::META, Cpe::Dr::MetaTraits<T>::data_size(data));
        memcpy(r.data(), &data, Cpe::Dr::MetaTraits<T>::data_size(data));
        return r;
    }

    bool deleteData(const char * name)  {
        if (logic_data_t r = logic_require_data_find(*this, name)) {
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

    void dump_data(cfg_t cfg) const { logic_require_data_dump_to_cfg(*this, cfg); }

    void destory(void) { logic_require_free(*this); }
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
