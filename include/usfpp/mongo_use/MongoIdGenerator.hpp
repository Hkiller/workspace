#ifndef USFPP_MONGO_USE_ID_GENERATOR_H
#define USFPP_MONGO_USE_ID_GENERATOR_H
#include "gdpp/utils/IdGenerator.hpp"
#include "usf/mongo_use/id_generator.h"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Mongo {

class MongoIdGenerator : public Gd::Utils::IdGenerator  {
public:
    operator mongo_id_generator_t() const { return (mongo_id_generator_t)this; }

    static MongoIdGenerator & instance(gd_app_context_t app, cpe_hash_string_t name);
    static MongoIdGenerator & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
