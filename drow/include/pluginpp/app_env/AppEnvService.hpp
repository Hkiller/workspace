#ifndef DROW_PLUGINPP_APPSVR_SERVICE_H
#define DROW_PLUGINPP_APPSVR_SERVICE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpe/dr/dr_metalib_manage.h"
#include "cpepp/dr/Meta.hpp"
#include "gdpp/app/Application.hpp"
#include "plugin/app_env/plugin_app_env_module.h"
#include "plugin/app_env/plugin_app_env_request.h"
#include "System.hpp"
#include "AppEnvProcessor.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
# pragma warning(disable:4407)
#endif

namespace Drow { namespace AppEnv {

class AppEnvService : public Cpe::Utils::SimulateObject {
public:
    operator plugin_app_env_module_t (void) const { return (plugin_app_env_module_t)this; }

    const char * name(void) const { return plugin_app_env_module_name(*this); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(plugin_app_env_module_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(plugin_app_env_module_app(*this)); }

    template<typename ProcessorT, typename DataT>
    uint32_t execute(
        DataT const & req,
        ProcessorT & processor, void (ProcessorT::*fun)(uint32_t id, int rv, Cpe::Dr::Data const * data))
    {
#ifdef _MSC_VER
        return this->execute(
            Cpe::Dr::MetaTraits<DataT>::META, &req, Cpe::Dr::MetaTraits<DataT>::data_size(req),
            processor, static_cast<AppEnvProcessFun>(fun) , *((AppEnvProcessor*)((void*)&processor)));
#else
        return this->execute(
            Cpe::Dr::MetaTraits<DataT>::META, &req, Cpe::Dr::MetaTraits<DataT>::data_size(req),
            static_cast<AppEnvProcessor&>(processor), static_cast<AppEnvProcessFun>(fun));
#endif
    }

    /*VC编译器处理成员函数地址时有错误，没有生成垫片函数，所以为了正确调用函数指针，必须直接把传入T类型的对象绑定在调用的对象上
      所以传入的realResponser为真实的Responser地址，而useResponser是T的this地址，用于调用函数的
     */
	uint32_t execute(
        LPDRMETA req_meta, void const * req_data, size_t req_data_size,
        AppEnvProcessor& realResponser, AppEnvProcessFun fun
#ifdef _MSC_VER
        , AppEnvProcessor& useResponser
#endif
        );

    template<typename ResT, typename ReqT>
    ResT * execute(ReqT const & req, mem_buffer_t buffer) {
        dr_data_t data =
            execute(
                Cpe::Dr::MetaTraits<ReqT>::META, &req, Cpe::Dr::MetaTraits<ReqT>::data_size(req),
                buffer);
        if (data == NULL) return NULL;

        return (ResT*)data->m_data;
    }
    
	dr_data_t execute(
        LPDRMETA req_meta, void const * req_data, size_t req_data_size, mem_buffer_t buffer, LPDRMETA res_meta = NULL);
    
    template<typename T>
    void post(T const & t = T()) {
        post(Cpe::Dr::MetaTraits<T>::META, &t, Cpe::Dr::MetaTraits<T>::data_size(t));
    }

    void post(LPDRMETA meta, void const * data, size_t size) {
        plugin_app_env_post_request(*this, meta, data, size);
    }

    void cancelRequest(uint32_t id) { plugin_app_env_cancel_request_by_id(*this, id); }

	void addMonitor(
        const char * meta_name, AppEnvProcessor& realResponser, AppEnvMonitorFun fun
#ifdef _MSC_VER
        , AppEnvProcessor& useResponser
#endif
        );


    template<typename ProcessorT>
    void addMonitor(
        const char * meta_name, ProcessorT & processor, int (ProcessorT::*fun)(Cpe::Dr::ConstData const * data))
    {
#ifdef _MSC_VER
        return this->addMonitor(
            meta_name, processor, static_cast<AppEnvMonitorFun>(fun), *((AppEnvProcessor*)((void*)&processor)));
#else
        return this->addMonitor(
            meta_name, static_cast<AppEnvProcessor&>(processor), static_cast<AppEnvMonitorFun>(fun));
#endif
    }

    static AppEnvService & instance(gd_app_context_t app, const char * name = NULL);
    static AppEnvService & _cast(plugin_app_env_module_t evm);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
