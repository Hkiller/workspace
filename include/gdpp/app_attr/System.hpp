#ifndef GDPP_APP_ATTR_SYSTEM_H
#define GDPP_APP_ATTR_SYSTEM_H
#include "gd/app_attr/app_attr_types.h"

namespace Gd { namespace AppAttr {

class AppAttrManager;
class AppAttrRequest;
class AppAttrFormula;
class AppAttrFormulaIt;
class AppAttrProcessor;
class AppAttrProcessorBase;

typedef void (AppAttrProcessor::*AppAttrProcessFun)(AppAttrRequest & request);

}}

#endif
