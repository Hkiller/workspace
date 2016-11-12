#ifndef GD_APP_LIBRARY_H
#define GD_APP_LIBRARY_H
#include "app_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void * gd_app_lib_sym(gd_app_lib_t lib, const char * symName, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
