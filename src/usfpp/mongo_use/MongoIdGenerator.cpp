#include "gdpp/app/Log.hpp"
#include "usfpp/mongo_use/MongoIdGenerator.hpp"

namespace Usf { namespace Mongo {

MongoIdGenerator & MongoIdGenerator::instance(gd_app_context_t app, cpe_hash_string_t name) {
    mongo_id_generator_t id_generator = mongo_id_generator_find(app, name);
    if (id_generator == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "mongo_id_generator %s not exist!", cpe_hs_data(name));
    }

    return *(MongoIdGenerator*)id_generator;
}

MongoIdGenerator & MongoIdGenerator::instance(gd_app_context_t app, const char * name) {
    mongo_id_generator_t id_generator = mongo_id_generator_find_nc(app, name);
    if (id_generator == NULL) {
        APP_CTX_THROW_EXCEPTION(
            app,
            ::std::runtime_error,
            "mongo_id_generator %s not exist!", name);
    }

    return *(MongoIdGenerator*)id_generator;
}

}}
