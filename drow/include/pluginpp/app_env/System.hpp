#ifndef DROW_PLUGINPP_APPSVR_SYSTEM_H
#define DROW_PLUGINPP_APPSVR_SYSTEM_H
#include "gdpp/app/System.hpp"
#include "cpepp/dr/System.hpp"
#include "plugin/app_env/plugin_app_env_types.h"

namespace Drow { namespace AppEnv {

class AppEnvService;
class AppEnvProcessor;
class AppEnvProcessorBase;

typedef void (AppEnvProcessor::*AppEnvProcessFun)(uint32_t id, int rv, Cpe::Dr::Data const * data);
typedef int (AppEnvProcessor::*AppEnvMonitorFun)(Cpe::Dr::ConstData const * data);

}}

#endif
