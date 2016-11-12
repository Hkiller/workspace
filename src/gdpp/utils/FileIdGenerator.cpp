#include "gd/app/app_context.h"
#include "gdpp/app/Log.hpp"
#include "gdpp/utils/FileIdGenerator.hpp"

namespace Gd { namespace Utils {

FileIdGenerator & FileIdGenerator::instance(gd_app_context_t app, cpe_hash_string_t name) {
    gd_id_file_generator_t r = gd_id_file_generator_find(app, name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "IdGeneratro %s not exist!", cpe_hs_data(name));
    }

    return *(FileIdGenerator*)r;
}

FileIdGenerator & FileIdGenerator::instance(gd_app_context_t app, const char * name) {
    gd_id_file_generator_t r = gd_id_file_generator_find_nc(app, name);
    if (r == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "IdGeneratro %s not exist!", name);
    }

    return *(FileIdGenerator*)r;
}

}}
