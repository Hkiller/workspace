#ifndef GDPP_APP_TESTENV_WITHAPPEX_H
#define GDPP_APP_TESTENV_WITHAPPEX_H
#include "cpe/utils/hash_string.h"
#include "gd/app/tests-env/with_app.hpp"
#include "cpepp/utils/System.hpp"
#include "cpepp/nm/Object.hpp"
#include "cpepp/nm/Manager.hpp"
#include "../Application.hpp"

namespace Gd { namespace App { namespace testenv {

class with_app : public gd::app::testenv::with_app {
public:
    with_app();

    void SetUp();
    void TearDown();

    Application & t_app_ex(void) { return Application::_cast(t_app()); }

    Cpe::Nm::Manager & t_nm_ex(void) { return t_app_ex().nmManager(); }

    void t_app_install_random(const char * name = 0);
    void t_app_install_random(Cpe::Utils::Random & random, const char * name = 0);

    template<typename T>
    T & namedObject(const char * name) {
        return dynamic_cast<T &>(t_nm_ex().objectNc(name));
    }
        
    template<typename T>
    T & installNamedObject(void) {
        return * new (gd_app_nm_mgr(t_app()), cpe_hs_data(T::NAME))
            T;
    }

    template<typename T, typename Arg1>
    T & installNamedObject(Arg1 arg1) {
        return * new (gd_app_nm_mgr(t_app()), cpe_hs_data(T::NAME))
            T(arg1);
    }

    template<typename T, typename Arg1, typename Arg2>
    T & installNamedObject(Arg1 arg1, Arg2 arg2) {
        return * new (gd_app_nm_mgr(t_app()), cpe_hs_data(T::NAME))
            T(arg1, arg2);
    }
        
    template<typename T, typename Arg1, typename Arg2, typename Arg3>
    T & installNamedObject(Arg1 arg1, Arg2 arg2, Arg3 arg3) {
        return * new (gd_app_nm_mgr(t_app()), cpe_hs_data(T::NAME))
            T(arg1, arg2, arg3);
    }

    template<typename T, typename Arg1, typename Arg2, typename Arg3
        , typename Arg4>
    T & installNamedObject(Arg1 arg1, Arg2 arg2, Arg3 arg3, Arg4 arg4) {
        return * new (gd_app_nm_mgr(t_app()), cpe_hs_data(T::NAME))
            T(arg1, arg2, arg3, arg4);
    }

    template<typename T>
    void uninstallNamedObject(void) {
        t_app_ex().nmManager().removeObject(T::NAME);
    }
};

}}}

#endif
