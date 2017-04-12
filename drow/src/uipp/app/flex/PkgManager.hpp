#ifndef UIPP_APP_ENV_FLEX_PKG_MANAGER_H
#define UIPP_APP_ENV_FLEX_PKG_MANAGER_H
#include "System.hpp"

class PkgManager {
public:
    virtual int addPkg(const char * name) = 0;
    virtual uint32_t loadingPkgCount(void) const = 0;
    virtual int install(void) = 0;
    virtual void uninstall(void) = 0;
    virtual ~PkgManager();

    static PkgManager * create(gd_app_context_t app);
};

#endif
