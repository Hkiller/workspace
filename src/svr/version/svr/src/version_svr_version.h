#ifndef SVR_VERSION_SVR_VERSION_H
#define SVR_VERSION_SVR_VERSION_H
#include "version_svr.h"

struct version_svr_version {
    version_svr_t m_svr;
    TAILQ_ENTRY(version_svr_version) m_next;
    struct cpe_hash_entry m_hh;
    const char * m_name;
    uint8_t m_update_strategy;
    version_svr_package_list_t m_packages;
};

version_svr_version_t version_svr_version_create(version_svr_t svr, const char * name, uint8_t update_strategy);
void version_svr_version_free(version_svr_version_t version);

version_svr_version_t version_svr_version_find(version_svr_t svr, const char * str_version);

uint32_t version_svr_version_hash(version_svr_version_t o);
int version_svr_version_eq(version_svr_version_t l, version_svr_version_t r);

#endif
