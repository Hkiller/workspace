#ifndef DROW_RENDERPP_ENV_ENV_H
#define DROW_RENDERPP_ENV_ENV_H
#include "cpepp/nm/Object.hpp"
#include "gdpp/app/System.hpp"
#include "System.hpp"

namespace Drow { namespace Env {

class Env : public Cpe::Nm::Object {
public:
    virtual ~Env();

    static Env & instance(Gd::App::Application & app);

    static cpe_hash_string_t NAME;
};

}}

#endif
