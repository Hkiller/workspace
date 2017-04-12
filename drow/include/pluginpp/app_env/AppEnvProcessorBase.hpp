#ifndef DROW_PLUGINPP_PROCESSOR_BASE_SYSTEM_H
#define DROW_PLUGINPP_PROCESSOR_BASE_SYSTEM_H
#include "cpepp/utils/TypeUtils.hpp"
#include "cpepp/dr/Data.hpp"
#include "AppEnvProcessor.hpp"
#include "AppEnvService.hpp"

namespace Drow { namespace AppEnv {

class AppEnvProcessorBase : public AppEnvProcessor {
public:
	AppEnvProcessorBase(AppEnvService & service);
	~AppEnvProcessorBase();

    template<typename ProcessorT, typename DataT>
    uint32_t executeLocalRequest(
        DataT const & req,
        ProcessorT & processor, void (ProcessorT::*fun)(uint32_t id, int rv, Cpe::Dr::Data const * data))
    {
        return m_service.execute(req, processor, fun);
    }

    template<typename ResT, typename ReqT>
    ResT * executeLocalRequest(ReqT const & req, mem_buffer_t buffer) {
        return m_service.execute<ResT, ReqT>(req, buffer);
    }
    
    template<typename DataT>
    void postLocalRequest(DataT const & req) {
        return m_service.post(req);
    }
    
    void cancelRequest(uint32_t id) { m_service.cancelRequest(id); }

    template<typename ProcessorT>
    void addLocalMonitor(
        const char * meta_name, int (ProcessorT::*fun)(Cpe::Dr::Data const * data))
    {
        ProcessorT & processor = Cpe::Utils::calc_cast<ProcessorT, AppEnvProcessorBase>(*this);
        return m_service.addMonitor(meta_name, processor, fun);
    }
    
    template<typename ProcessorT, typename DataT>
    struct MonitorFunTraits {
        typedef int (ProcessorT::*fun_t)(DataT const & data);
        static fun_t m_fun;
    };

    template<typename ProcessorT, typename DataT>
    void addLocalMonitor(int (ProcessorT::*fun)(DataT const & data)) {
        MonitorFunTraits<ProcessorT, DataT>::m_fun = fun;
        return m_service.addMonitor(dr_meta_name(Cpe::Dr::MetaTraits<DataT>::META), *this, &ProcessorT::template _call_monitor<ProcessorT, DataT>);
    }

    template<typename ProcessorT, typename DataT>
    int _call_monitor(Cpe::Dr::ConstData const * data) {
        if (data) {
            ProcessorT & processor = Cpe::Utils::calc_cast<ProcessorT, AppEnvProcessorBase>(*this);
            return (processor.*MonitorFunTraits<ProcessorT, DataT>::m_fun)(data->as<DataT>());
        }
        return 0;
    }

private:	
	AppEnvService & m_service;
};

template<typename ProcessorT, typename DataT>
typename AppEnvProcessorBase::MonitorFunTraits<ProcessorT, DataT>::fun_t
AppEnvProcessorBase::MonitorFunTraits<ProcessorT, DataT>::m_fun = NULL;

}}

#endif
