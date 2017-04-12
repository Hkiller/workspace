#ifndef DROW_RENDERPP_REPO_REPONSTORY_H
#define DROW_RENDERPP_REPO_REPONSTORY_H
#include "cpepp/nm/Object.hpp"
#include "gdpp/app/System.hpp"
#include "System.hpp"

namespace Drow { namespace Repo {

class Reponstory : public Cpe::Nm::Object {
public:
    virtual ~Reponstory();

    static Reponstory & instance(Gd::App::Application & app);
    static Reponstory & install(Gd::App::Application & app);

    static cpe_hash_string_t NAME;
};

}}

#endif
