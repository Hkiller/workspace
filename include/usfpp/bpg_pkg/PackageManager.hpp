#ifndef USFPP_BPG_PKG_PACKAGEMANAGER_H
#define USFPP_BPG_PKG_PACKAGEMANAGER_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/CString.hpp"
#include "gdpp/app/Application.hpp"
#include "usf/bpg_pkg/bpg_pkg_manage.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Bpg {

class PackageManager : public Cpe::Utils::SimulateObject {
public:
    operator bpg_pkg_manage_t() const { return (bpg_pkg_manage_t)this; }

    Cpe::Utils::CString const & name(void) const { return Cpe::Utils::CString::_cast(bpg_pkg_manage_name(*this)); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(bpg_pkg_manage_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(bpg_pkg_manage_app(*this)); }

    Cpe::Dr::MetaLib const & dataMetaLib(void) const;
    Cpe::Dr::Meta const * findCmdMeta(uint32_t cmd) const { return (Cpe::Dr::Meta const *)bpg_pkg_manage_find_meta_by_cmd(*this, cmd); }

    uint32_t cmdFromMetaName(const char * metaName) const;
    bool isSupportCmd(const char * metaName) const;

    dp_req_t createPackage(size_t capacity);

    static PackageManager & _cast(bpg_pkg_manage_t pkg_mgr);
    static PackageManager & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
