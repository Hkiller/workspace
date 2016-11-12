#ifndef GDPP_APP_RESPONSER_H
#define GDPP_APP_RESPONSER_H
#include "cpe/dp/dp_responser.h"
#include "Log.hpp"
#include "System.hpp"

namespace Gd { namespace App {

class ReqResponser {
public:
    virtual int process(Cpe::Dp::Request & req, error_monitor_t em) = 0;
    virtual ~ReqResponser();

    static int _process(dp_req_t req, void * ctx, error_monitor_t em);
    static dp_rsp_type_t _type;
};

}}

#define GD_DP_RESPONSER_REG(__rsp_name, __rsp_type)         \
    extern "C"                                              \
    int rsp_ ## __rsp_name ## _init(                        \
        dp_rsp_t rsp, gd_app_context_t context,          \
        gd_app_module_t module,                             \
        cfg_t cfg)                                          \
    {                                                       \
        try {                                               \
            ::Gd::App::ReqResponser * processor =           \
                new __rsp_type(                             \
                    *((::Gd::App::Application *)context),   \
                    *((::Gd::App::Module*)module),          \
                    *((::Cpe::Cfg::Node*)cfg));             \
            if (processor == NULL) {return -1;}             \
                                                            \
            dp_rsp_set_processor(                        \
                rsp,                                        \
                &::Gd::App::ReqResponser::_process,         \
                processor);                                 \
                                                            \
            dp_rsp_set_type(                             \
                rsp,                                        \
                ::Gd::App::ReqResponser::_type);            \
                                                            \
            return 0;                                       \
        }                                                   \
        APP_CTX_CATCH_EXCEPTION(                            \
            context, "create rsp " #__rsp_name ":");        \
        return -1;                                          \
    }

#endif


