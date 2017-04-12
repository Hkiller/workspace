#include "pluginpp/app_env/AppEnvProcessorBase.hpp"

namespace Drow { namespace AppEnv {

AppEnvProcessor::~AppEnvProcessor() {
}

AppEnvProcessorBase::AppEnvProcessorBase(AppEnvService & service)
    : m_service(service)
{
}

AppEnvProcessorBase::~AppEnvProcessorBase() {
    plugin_app_env_clear_requests_by_ctx(m_service, static_cast<AppEnvProcessor*>(this));
}

}}

