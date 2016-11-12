#include <limits>
#include "gd/app/app_context.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/app/Application.hpp"
#include "gdpp/utils/IdGenerator.hpp"

namespace Gd { namespace Utils {

gd_id_t IdGenerator::generate(const char * id_name) {
    gd_id_t r;
    if (gd_id_generator_generate(&r, *this, id_name) != 0) {
        APP_CTX_THROW_EXCEPTION(
            app(),
            ::std::runtime_error,
            "IdGeneratro %s generate id fail!", name());
    }
    //printf("gd_id: %d\n", r);
    return r;
}

gd_id_t IdGenerator::tryGenerate(const char * id_name) {
    gd_id_t r;
    if (gd_id_generator_generate(&r, *this, id_name) != 0) {
        return 0;
    }
    else {
        return r;
    }
}

IdGenerator & IdGenerator::instance(gd_app_context_t app, cpe_hash_string_t name) {
    gd_id_generator_t r = gd_id_generator_find(app, name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "IdGenerator %s not exist!", cpe_hs_data(name));
    }

    return *(IdGenerator*)r;
}

IdGenerator & IdGenerator::instance(gd_app_context_t app, const char * name) {
    gd_id_generator_t r = gd_id_generator_find_nc(app, name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "IdGeneratro %s not exist!", name);
    }

    return *(IdGenerator*)r;
}

}}
