#ifndef GDPP_APP_RANDOM_H
#define GDPP_APP_RANDOM_H
#include "cpepp/utils/Random.hpp"
#include "cpepp/nm/Object.hpp"
#include "System.hpp"

namespace Gd { namespace App {

class Random : public Cpe::Nm::Object, public Cpe::Utils::Random {
public:
    static cpe_hash_string_t DEFAULT_NAME;

    static Random & instance(Application & app, cpe_hash_string_t name = DEFAULT_NAME);
};

}}

#endif

