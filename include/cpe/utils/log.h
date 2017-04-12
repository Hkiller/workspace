#ifndef CPE_UTILS_LOG_H_
#define CPE_UTILS_LOG_H_
#include "log4c.h"
#ifdef _MSC_VER

#define LOG_ERROR(category, msg, ...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_ERROR, msg, __VA_ARGS__); \
    }

#define LOG_WARN(category, msg, ...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_WARN, msg, __VA_ARGS__); \
    } 
 
#define LOG_INFO(category, msg, ...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL); \
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_INFO, msg, __VA_ARGS__); \
    }

#define LOG_DEBUG(category, msg, ...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL); \
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_DEBUG, msg, __VA_ARGS__); \
    }
 
#define LOG_TRACE(category, msg, ...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_TRACE, msg, __VA_ARGS__); \
    }
#else
#define LOG_ERROR(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_ERROR, msg, ##args); \
    }

#define LOG_WARN(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_WARN, msg, ##args); \
    } 

#define LOG_INFO(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL); \
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_INFO, msg, ##args); \
    }

#define LOG_DEBUG(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL); \
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_DEBUG, msg, ##args); \
    }

#define LOG_TRACE(category, msg, args...) \
    { \
    const log4c_location_info_t locinfo = LOG4C_LOCATION_INFO_INITIALIZER(NULL);\
    log4c_category_log_locinfo(mycat, &locinfo, LOG4C_PRIORITY_TRACE, msg, ##args); \
    }
#endif
#endif
