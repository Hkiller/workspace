#ifndef CPE_UTILS_SERVICE_H
#define CPE_UTILS_SERVICE_H
#include "cpe/utils/error.h"

#ifdef __cplusplus
extern "C" {
#endif

void cpe_daemonize(error_monitor_t em);
int cpe_check_and_write_pid(const char * pidfile, error_monitor_t em);
int cpe_check_and_remove_pid(const char * pidfile, error_monitor_t em);    
int cpe_kill_by_pidfile(const char * pidfile, int sig, error_monitor_t em);

#ifdef __cplusplus
}
#endif

#endif
