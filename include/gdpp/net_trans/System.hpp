#ifndef GDPP_NETTRANS_SYSTEM_H
#define GDPP_NETTRANS_SYSTEM_H
#include "gdpp/app/System.hpp"
#include "gd/net_trans/net_trans_types.h"

namespace Gd { namespace NetTrans {

class NetTransMgr;
class NetTransTask;
class NetTransProcessor;
class NetTransProcessorBase;

typedef void (NetTransProcessor::*NetTransProcessFun)(NetTransTask & task);
typedef void (NetTransProcessor::*NetTransProgressFun)(NetTransTask & task, double dltotal, double dlnow);
typedef void (NetTransProcessor::*NetTransWriteFun)(NetTransTask & task, void * data, size_t data_size);

}}

#endif
