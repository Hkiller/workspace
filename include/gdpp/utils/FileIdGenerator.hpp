#ifndef GDPP_UTILS_FILE_IDGENERATOR_H
#define GDPP_UTILS_FILE_IDGENERATOR_H
#include "gd/utils/id_generator_file.h"
#include "IdGenerator.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Gd { namespace Utils {

class FileIdGenerator : public IdGenerator  {
public:
    operator gd_id_file_generator_t () const { return (gd_id_file_generator_t)this; }

    static FileIdGenerator & instance(gd_app_context_t app, cpe_hash_string_t name);
    static FileIdGenerator & instance(gd_app_context_t app, const char * name);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
