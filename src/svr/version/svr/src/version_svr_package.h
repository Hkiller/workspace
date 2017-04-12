#ifndef SVR_VERSION_SVR_PACKAGE_H
#define SVR_VERSION_SVR_PACKAGE_H
#include "version_svr_version.h"

struct version_svr_package {
    version_svr_version_t m_version;
    TAILQ_ENTRY(version_svr_package) m_next;
    char m_chanel[16];
    uint8_t m_device_category;
    SVR_VERSION_PACKAGE m_data;
};

version_svr_package_t version_svr_package_create(version_svr_version_t version, const char * chanel, uint8_t device_category, SVR_VERSION_PACKAGE const * data);
void version_svr_package_free(version_svr_package_t package);

version_svr_package_t version_svr_package_find(version_svr_version_t version, const char * chanel, uint8_t device_category);

#endif
