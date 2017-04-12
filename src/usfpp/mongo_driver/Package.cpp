#include "gdpp/app/Log.hpp"
#include "usfpp/logic/LogicOpRequire.hpp"
#include "usfpp/mongo_driver/Package.hpp"

namespace Usf { namespace Mongo {

Package & Package::_cast(mongo_pkg_t pkg) {
    if (pkg == NULL) {
        throw ::std::runtime_error("Usf::Mongo::Package::_cast: input pkg is NULL!");
    }

    return *(Package*)pkg;
}

Package & Package::_cast(dp_req_t req) {
    if (req == NULL) {
        throw ::std::runtime_error("Usf::Mongo::Package::_cast: input req is NULL!");
    }

    mongo_pkg_t pkg = mongo_pkg_from_dp_req(req);
    if (pkg == NULL) {
        throw ::std::runtime_error("Usf::Mongo::Package::_cast: cast dp_req to pkg fail!");
    }

    return *(Package*)pkg;
}

void Package::docAppend(LPDRMETA meta, void const * data, size_t size) {
    if (mongo_pkg_doc_append(*this, meta, data, size) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: apend doc fail!");
    }
}

void Package::docOpen(void) {
    if (mongo_pkg_doc_open(*this) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: open doc fail!");
    }
}

void Package::docClose(void) {
    if (mongo_pkg_doc_close(*this) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: open doc fail!");
    }
}

void Package::appendInt32(const char * name, const int32_t i) {
    if (mongo_pkg_append_int32(*this, name, i) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append int32 fail!");
    }
}

void Package::appendInt64(const char * name, int64_t i) {
    if (mongo_pkg_append_int64(*this, name, i) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append int64 fail!");
    }
}

void Package::appendDouble(const char * name, const double d) {
    if (mongo_pkg_append_double(*this, name, d) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append double fail!");
    }
}

void Package::appendString(const char * name, const char *str) {
    if (mongo_pkg_append_string(*this, name, str) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append string fail!");
    }
}

void Package::appendString(const char * name, const char *str, int len) {
    if (mongo_pkg_append_string_n(*this, name, str, len) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append string_n fail!");
    }
}

void Package::appendSymbol(const char * name, const char *str) {
    if (mongo_pkg_append_symbol(*this, name, str) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append symbol fail!");
    }
}

void Package::appendSymbol(const char * name, const char *str, int len) {
    if (mongo_pkg_append_symbol_n(*this, name, str, len) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append symbol_n fail!");
    }
}

void Package::appendCode(const char * name, const char *str) {
    if (mongo_pkg_append_code(*this, name, str) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append code fail!");
    }
}

void Package::appendCode(const char * name, const char *str, int len) {
    if (mongo_pkg_append_code_n(*this, name, str, len) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append code_n fail!");
    }
}

void Package::appendBinary(const char * name, char type, const char *str, int len) {
    if (mongo_pkg_append_binary(*this, name, type, str, len) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append binary fail!");
    }
}

void Package::appendBool(const char *name, const bool v) {
    if (mongo_pkg_append_bool(*this, name, v) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append bool fail!");
    }
}

void Package::appendNull(const char *name) {
    if (mongo_pkg_append_null(*this, name) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append null fail!");
    }
}

void Package::appendUndefined(const char *name) {
    if (mongo_pkg_append_undefined(*this, name) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append undefined fail!");
    }
}

void Package::appendRegex(const char *name, const char *pattern, const char *opts) {
    if (mongo_pkg_append_regex(*this, name, pattern, opts) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append regex fail!");
    }
}

void Package::appendTimestamp(const char *name, int time, int increment) {
    if (mongo_pkg_append_timestamp(*this, name, time, increment) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append timestamp fail!");
    }
}

void Package::appendObjectStart(const char * name) {
    if (mongo_pkg_append_start_object(*this, name) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append object start fail!");
    }
}

void Package::appendObject(const char * name, LPDRMETA meta, void const * data, size_t size) {
    if (mongo_pkg_append_object(*this, name, meta, data, size) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append object fail!");
    }
}

void Package::appendObjectFinish() {
    if (mongo_pkg_append_finish_object(*this) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append object finish fail!");
    }
}

void Package::appendArrayStart(const char * name) {
    if (mongo_pkg_append_start_array(*this, name) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append array start fail!");
    }
}

void Package::appendArrayFinish() {
    if (mongo_pkg_append_finish_array(*this) != 0) {
        throw ::std::runtime_error("Usf::Mongo::Package: append array finish fail!");
    }
}

}}
