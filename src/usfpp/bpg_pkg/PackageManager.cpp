#include "cpepp/dr/Meta.hpp"
#include "cpepp/dr/MetaLib.hpp"
#include "gdpp/app/Log.hpp"
#include "usf/bpg_pkg/bpg_pkg.h"
#include "usfpp/bpg_pkg/PackageManager.hpp"

namespace Usf { namespace Bpg {

Cpe::Dr::MetaLib const & PackageManager::dataMetaLib(void) const {
    LPDRMETALIB metalib = bpg_pkg_manage_data_metalib(*this);
    if (metalib == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "bpg_pkg_manage %s: no data metalib",
            name().c_str());
    }

    return Cpe::Dr::MetaLib::_cast(metalib);
}

dp_req_t PackageManager::createPackage(size_t capacity) {
    dp_req_t pkg = bpg_pkg_create_with_body(*this, capacity);
    if (pkg == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "bpg_pkg_manage %s: crate pkg fail, data-capacity="FMT_SIZE_T"!",
            name().c_str(), capacity);
    }
    return pkg;
}

PackageManager & PackageManager::_cast(bpg_pkg_manage_t pkg_manage) {
    if (pkg_manage == NULL) {
        throw ::std::runtime_error("Usf::Bpg::PackageManager::_cast: input pkg_manage is NULL!");
    }

    return *(PackageManager*)pkg_manage;
}

PackageManager & PackageManager::instance(gd_app_context_t app, const char * name) {
    bpg_pkg_manage_t pkg_manage = bpg_pkg_manage_find_nc(app, name);
    if (pkg_manage == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "bpg_pkg_manage %s not exist!", name ? name : "default");
    }

    return *(PackageManager*)pkg_manage;
}

uint32_t PackageManager::cmdFromMetaName(const char * metaName) const {
    uint32_t r;

    if (bpg_pkg_find_cmd_from_meta_name(&r, *this, metaName) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "bpg_pkg_manage %s: find cmd of meta %s fail!", name().c_str(), metaName);
    }

    return r;
}

bool PackageManager::isSupportCmd(const char * metaName) const {
    uint32_t r;

    return bpg_pkg_find_cmd_from_meta_name(&r, *this, metaName) == 0;
}

}}
