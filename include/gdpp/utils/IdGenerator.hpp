#ifndef GDPP_UTILS_IDGENERATOR_H
#define GDPP_UTILS_IDGENERATOR_H
#include "cpepp/utils/ClassCategory.hpp"
#include "gdpp/app/Application.hpp"
#include "gd/utils/id_generator.h"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Gd { namespace Utils {

class IdGenerator : public Cpe::Utils::SimulateObject  {
public:
    operator gd_id_generator_t () const { return (gd_id_generator_t)this; }

    uint64_t generate(const char * id_name);
    uint64_t tryGenerate(const char * id_name);

    const char * name(void) const { return gd_id_generator_name(*this); }

    Gd::App::Application & app(void) { return Gd::App::Application::_cast(gd_id_generator_app(*this)); }
    Gd::App::Application const & app(void) const { return Gd::App::Application::_cast(gd_id_generator_app(*this)); }

    static IdGenerator & instance(gd_app_context_t app, cpe_hash_string_t name);
    static IdGenerator & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
