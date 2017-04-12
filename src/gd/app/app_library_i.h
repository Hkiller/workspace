#ifndef GD_APP_LIBRARY_I_H
#define GD_APP_LIBRARY_I_H
#include "gd/app/app_library.h"
#include "app_internal_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gd_app_lib {
    char * m_name;
    void * m_handler;
    gd_app_module_type_list_t m_modules;
    TAILQ_ENTRY(gd_app_lib) m_next;
};

#ifdef __cplusplus
}
#endif

#endif
