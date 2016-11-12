#ifndef GD_APP_LOG_H
#define GD_APP_LOG_H
#include <stdio.h>
#include "app_basic.h"

#ifdef _MSC_VER

#define _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, level, no, format, ...)    \
    if (ctx) {                                                          \
    _CPE_DO_ERROR_NOTIFY(                                           \
    gd_app_em(ctx), level, no, format, __VA_ARGS__);                 \
    } else printf(format, __VA_ARGS__)                                       \

#define _GD_APP_DO_ERROR_NOTIFY(level, no, format, ...)             \
    _GD_APP_CTX_DO_ERROR_NOTIFY(gd_app_ins(), level, no, format, __VA_ARGS__)

/*use for log with app*/
#define APP_CTX_INFO(ctx, format, ...)                      \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_INFO, 0, format, __VA_ARGS__)

#define APP_CTX_WARNING(ctx, format, ...)                            \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_WARNING, -1, format, __VA_ARGS__)

#define APP_CTX_ERROR(ctx, format, ...)                              \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_ERROR, -1, format, __VA_ARGS__)

#define APP_CTX_WARNING_EX(monitor, en, format, ...)                    \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_WARNING, en, format, __VA_ARGS__)

#define APP_CTX_ERROR_EX(monitor, en, format, ...)                      \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_ERROR, en, format, __VA_ARGS__)

/*use for log with global app*/
#define APP_INFO(format, ...)                           \
    _GD_APP_DO_ERROR_NOTIFY(CPE_EL_INFO, 0, format, __VA_ARGS__)

#define APP_WARNING(format, ...)                            \
    _GD_APP_DO_ERROR_NOTIFY(CPE_EL_WARNING, -1, format, __VA_ARGS__)

#define APP_ERROR(format, ...)                              \
    _GD_APP_DO_ERROR_NOTIFY(CPE_EL_ERROR, -1, format, __VA_ARGS__)

#define APP_WARNING_EX(monitor, en, format, ...)                    \
    _GD_APP_DO_ERROR_NOTIFY(monitor, CPE_EL_WARNING, en, format, __VA_ARGS__)

#define APP_ERROR_EX(monitor, en, format, ...)                      \
    _GD_APP_DO_ERROR_NOTIFY(monitor, CPE_EL_ERROR, en, format, __VA_ARGS__)
#else

#define _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, level, no, format, args...)    \
    if (ctx) {                                                          \
        _CPE_DO_ERROR_NOTIFY(                                           \
            gd_app_em(ctx), level, no, format, ##args);                 \
    } else printf(format, ##args)                                       \

#define _GD_APP_DO_ERROR_NOTIFY(level, no, format, args...)             \
    _GD_APP_CTX_DO_ERROR_NOTIFY(gd_app_ins(), level, no, format, ##args)

/*use for log with app*/
#define APP_CTX_INFO(ctx, format, args...)                      \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_INFO, 0, format, ##args)

#define APP_CTX_WARNING(ctx, format, args...)                            \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_WARNING, -1, format, ##args)

#define APP_CTX_ERROR(ctx, format, args...)                              \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_ERROR, -1, format, ##args)

#define APP_CTX_WARNING_EX(monitor, en, format, args...)                    \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_WARNING, en, format, ##args)

#define APP_CTX_ERROR_EX(monitor, en, format, args...)                      \
    _GD_APP_CTX_DO_ERROR_NOTIFY(ctx, CPE_EL_ERROR, en, format, ##args)

/*use for log with global app*/
#define APP_INFO(format, args...)                           \
    _GD_APP_DO_ERROR_NOTIFY(CPE_EL_INFO, 0, format, ##args)

#define APP_WARNING(format, args...)                            \
    _GD_APP_DO_ERROR_NOTIFY(CPE_EL_WARNING, -1, format, ##args)

#define APP_ERROR(format, args...)                              \
    _GD_APP_DO_ERROR_NOTIFY(CPE_EL_ERROR, -1, format, ##args)

#define APP_WARNING_EX(monitor, en, format, args...)                    \
    _GD_APP_DO_ERROR_NOTIFY(monitor, CPE_EL_WARNING, en, format, ##args)

#define APP_ERROR_EX(monitor, en, format, args...)                      \
    _GD_APP_DO_ERROR_NOTIFY(monitor, CPE_EL_ERROR, en, format, ##args)

#endif

#endif

