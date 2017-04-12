#include <limits>
#include "cpepp/nm/Manager.hpp"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/app/Module.hpp"
#include "gdpp/app/Random.hpp"

namespace Gd { namespace App {

class RandomImpl : public Random {
public:
    virtual uint32_t generate(uint32_t max) { return cpe_rand_dft(max); }
};

Random & Random::instance(Application & app, cpe_hash_string_t name) {
    Random * r =
        dynamic_cast<Random *>(
            &app.nmManager().object(name));
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(app, ::std::runtime_error, "Random cast fail!");
    }

    return *r;
}

static cpe_hash_string_buf s_Event_DEFAULT_NAME_buf = CPE_HS_BUF_MAKE("AppRandom");
cpe_hash_string_t Random::DEFAULT_NAME = (cpe_hash_string_t)s_Event_DEFAULT_NAME_buf;

}}

extern "C"
EXPORT_DIRECTIVE
int AppRandom_app_init(Gd::App::Application & app, Gd::App::Module & module, Cpe::Cfg::Node & cfg) {
    try {
        return (new (app.nmManager(), module.name()) Gd::App::RandomImpl)
            == NULL
            ? -1
            : 0;
    }
    APP_CTX_CATCH_EXCEPTION(app, "create AppRandom: ");
    return -1;
}

extern "C"
EXPORT_DIRECTIVE
void AppRandom_app_fini(Gd::App::Application & app, Gd::App::Module & module) {
    app.nmManager().removeObject(module.name());
}
