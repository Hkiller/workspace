#include "cpe/utils/string_utils.h"
#include "version_svr_package.h"

version_svr_package_t
version_svr_package_create(version_svr_version_t version, const char * chanel, uint8_t device_category, SVR_VERSION_PACKAGE const * data) {
    version_svr_t svr = version->m_svr;
    version_svr_package_t package;

    package = mem_alloc(svr->m_alloc, sizeof(struct version_svr_package));
    if (package == NULL) {
        CPE_ERROR(svr->m_em, "package of version %s create: alloc fail!", version->m_name);
        return NULL;
    }

    cpe_str_dup(package->m_chanel, sizeof(package->m_chanel), chanel);
    package->m_device_category = device_category;
    package->m_data = *data;

    TAILQ_INSERT_TAIL(&version->m_packages, package, m_next);
    
    return package;
}

void version_svr_package_free(version_svr_package_t package) {
    version_svr_version_t version = package->m_version;
    version_svr_t svr = version->m_svr;

    TAILQ_REMOVE(&version->m_packages, package, m_next);

    mem_free(svr->m_alloc, package);
}

version_svr_package_t version_svr_package_find(version_svr_version_t version, const char * chanel, uint8_t device_category) {
    version_svr_package_t package;

    TAILQ_FOREACH(package, &version->m_packages, m_next) {
        if (package->m_device_category == device_category && strcmp(package->m_chanel, chanel) == 0) return package;
    }

    return NULL;
}
