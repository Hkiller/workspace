#ifndef GDPP_APP_APPLICATION_H
#define GDPP_APP_APPLICATION_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gd/app/app_context.h"
#include "gd/app/app_tl.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)

#endif
namespace Gd { namespace App {

class Application : public Cpe::Utils::SimulateObject {
public:
    operator gd_app_context_t () const { return (gd_app_context_t)(void*)this; }

    int argc(void) const { return gd_app_argc(*this); }
    char ** argv(void) const { return gd_app_argv(*this); }

    const char * root(void) const { return gd_app_root(*this); }

    mem_allocrator_t allocrator(void) { return gd_app_alloc(*this); }
    error_monitor_t em(void) const { return gd_app_em(*this); }
    error_monitor_t em(const char * name) const { return gd_app_named_em(*this, name); }

    Cpe::Cfg::Node & cfg(void) { return *(Cpe::Cfg::Node *)gd_app_cfg(*this); }
    Cpe::Cfg::Node const & cfg(void) const { return *(Cpe::Cfg::Node *)gd_app_cfg(*this); }

    Cpe::Dp::Manager & dpManager(void) { return *((Cpe::Dp::Manager *)gd_app_dp_mgr(*this)); }
    Cpe::Dp::Manager const & dpManager(void) const { return *((Cpe::Dp::Manager *)gd_app_dp_mgr(*this)); }

    Cpe::Nm::Manager & nmManager(void) { return *((Cpe::Nm::Manager *)gd_app_nm_mgr(*this)); }
    Cpe::Nm::Manager const & nmManager(void) const { return *((Cpe::Nm::Manager *)gd_app_nm_mgr(*this)); }

    Cpe::Tl::Manager & tlManager(void) { return *(Cpe::Tl::Manager *)gd_app_tl_mgr(*this); }
    Cpe::Tl::Manager const & tlManager(void) const { return *(Cpe::Tl::Manager *)gd_app_tl_mgr(*this); }
    Cpe::Tl::Manager * findTlManager(const char * name) { return (Cpe::Tl::Manager *)app_tl_manage_find(*this, name); }
    Cpe::Tl::Manager const * findTlManager(const char * name) const { return (Cpe::Tl::Manager *)app_tl_manage_find(*this, name); }
    Cpe::Tl::Manager & tlManager(const char * name);
    Cpe::Tl::Manager const & tlManager(const char * name) const;

    Cpe::Nm::Object const * findObject(cpe_hash_string_t name) const;
    Cpe::Nm::Object * findObject(cpe_hash_string_t name);
    Cpe::Nm::Object const & object(cpe_hash_string_t name) const;
    Cpe::Nm::Object & object(cpe_hash_string_t name);

    Cpe::Nm::Object const * findObjectNc(const char * name) const;
    Cpe::Nm::Object * findObjectNc(const char * name);
    Cpe::Nm::Object const & objectNc(const char * name) const;
    Cpe::Nm::Object & objectNc(const char * name);
    
    mem_buffer_t tmpBuffer(void) { return gd_app_tmp_buffer(*this); }
    
    void tick(float delta_s) { gd_app_tick(*this, delta_s); }

    static Application & instance(void);
    static Application & _cast(gd_app_context_t ctx);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif

