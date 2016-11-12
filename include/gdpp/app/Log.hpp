#ifndef GDPP_LOG_H
#define GDPP_LOG_H
#include <stdexcept>
#include "cpe/pal/pal_stdio.h"
#include "gd/app/app_log.h"
#include "System.hpp"

#ifdef _MSC_VER  /******************* for vc */

#define APP_THROW_EXCEPTION(__e, __format, ...)         \
    do {                                                \
        char __buf[1024];                               \
        snprintf(__buf, 1023, __format, __VA_ARGS__);   \
        APP_ERROR("%s", __buf);                         \
        throw __e( __buf );                             \
    } while(0)

#define APP_CTX_THROW_EXCEPTION(__app, __e, __format, ...)  \
    do {                                                    \
        char __buf[1024];                                   \
        snprintf(__buf, 1023, __format, __VA_ARGS__);       \
        APP_CTX_ERROR(__app, "%s", __buf);                  \
        throw __e( __buf );                                 \
    } while(0)

#define APP_CTX_CATCH_EXCEPTION_RETHROW(__app, __format, ...)  \
    catch(::std::exception const & __e) {                          \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, __VA_ARGS__);                \
        APP_CTX_ERROR(                                          \
            __app,                                              \
            "%s catch exception %s", __buf, __e.what()); \
        throw;                                                  \
    }                                                           \
    catch(...) {                                                \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, __VA_ARGS__);                \
        APP_CTX_ERROR(                                          \
            __app,                                              \
            "%s catch unknown exception", __buf);        \
        throw;                                                  \
    }

#define APP_CTX_CATCH_EXCEPTION(__app, __format, ...)       \
    catch(::std::exception const & __e) {                       \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, __VA_ARGS__);                \
        APP_CTX_ERROR(                                          \
            __app,                                              \
            "%s catch exception %s", __buf, __e.what()); \
    }                                                           \
    catch(...) {                                                \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, __VA_ARGS__);                \
        APP_CTX_ERROR(                                          \
            __app,                                              \
            "%s catch unknown exception", __buf);        \
    }

#define APP_CATCH_EXCEPTION_RETHROW(__format, ...)          \
    APP_CTX_CATCH_EXCEPTION_RETHROW(gd_app_ins(), __format, __VA_ARGS__)

#define APP_CATCH_EXCEPTION( __format, ...)       \
    APP_CTX_CATCH_EXCEPTION(gd_app_ins(), __format, __VA_ARGS__)


#else /******************* for other(gcc) */

#define APP_THROW_EXCEPTION(__e, __format, args...) \
    APP_CTX_THROW_EXCEPTION(gd_app_ins(), __e, __format, ##args)

#define APP_CTX_THROW_EXCEPTION(__app, __e, __format, args...)  \
    do {                                                        \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, ##args);                \
        APP_CTX_ERROR(__app, "%s", __buf);                      \
        throw __e( __buf );                                     \
    } while(0)

#define APP_CTX_CATCH_EXCEPTION_RETHROW(__app, __format, args...)  \
    catch(::std::exception const & __e) {                          \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, ##args);                \
        APP_CTX_ERROR(                                          \
            __app,                                              \
            "%s catch exception %s", __buf, __e.what()); \
        throw;                                                  \
    }                                                           \
    catch(...) {                                                \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, ##args);                \
        APP_CTX_ERROR(                                          \
            __app,                                              \
            "%s catch unknown exception", __buf);        \
        throw;                                                  \
    }

#define APP_CTX_CATCH_EXCEPTION(__app, __format, args...)       \
    catch(::std::exception const & __e) {                       \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, ##args);                \
        APP_CTX_ERROR(                                          \
            __app,                                              \
            "%s catch exception %s", __buf, __e.what()); \
    }                                                           \
    catch(...) {                                                \
        char __buf[1024];                                       \
        snprintf(__buf, 1023, __format, ##args);                \
        APP_CTX_ERROR(                                          \
            __app,                                              \
            "%s catch unknown exception", __buf);        \
    }

#define APP_CATCH_EXCEPTION_RETHROW(__format, args...)          \
    APP_CTX_CATCH_EXCEPTION_RETHROW(gd_app_ins(), __format, ##args)

#define APP_CATCH_EXCEPTION( __format, args...)       \
    APP_CTX_CATCH_EXCEPTION(gd_app_ins(), __format, ##args)

#endif

#endif


