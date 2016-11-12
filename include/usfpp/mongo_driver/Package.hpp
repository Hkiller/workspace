#ifndef USFPP_MONGO_PACKAGE_H
#define USFPP_MONGO_PACKAGE_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/dr/System.hpp"
#include "usf/mongo_driver/mongo_pkg.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Usf { namespace Mongo {

class Package : public Cpe::Utils::SimulateObject {
public:
    operator mongo_pkg_t() const { return (mongo_pkg_t)this; }

    void init(void) { mongo_pkg_init(*this); }

    mongo_db_op_t op(void) const { return mongo_pkg_op(*this); }
    void setOp(mongo_db_op_t op) { mongo_pkg_set_op(*this, op); }

    uint32_t id(void) const { return mongo_pkg_id(*this); }
    void setId(uint32_t id) { mongo_pkg_set_id(*this, id); }

    const char * db(void) const { return mongo_pkg_db(*this); }
    void setDb(const char * db) { mongo_pkg_set_db(*this, db); }

    const char * collection(void) const { return mongo_pkg_collection(*this); }
    void setCollection(const char * collection) { mongo_pkg_set_collection(*this, collection); }

    /*doc op*/
    void docAppend(LPDRMETA meta, void const * data, size_t size);

    template<typename T>
    void docAppend(T const & data) {
        docAppend(Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    void docOpen(void);
    void docClose(void);
    bool docIsClosed(void) const { return mongo_pkg_doc_is_closed(*this) ? true : false; }
    int docCount(void) const { return mongo_pkg_doc_count(*this); }

    /*basic data op*/
    void appendInt32(const char * name, const int32_t i);
    void appendInt64(const char * name, int64_t i);
    void appendDouble(const char * name, const double d);
    void appendString(const char * name, const char *str);
    void appendString(const char * name, const char *str, int len);
    void appendSymbol(const char * name, const char *str);
    void appendSymbol(const char * name, const char *str, int len);
    void appendCode(const char * name, const char *str);
    void appendCode(const char * name, const char *str, int len);
    void appendBinary(const char * name, char type, const char *str, int len);
    void appendBool(const char *name, const bool v);
    void appendNull(const char *name);
    void appendUndefined(const char *name);
    void appendRegex(const char *name, const char *pattern, const char *opts);
    void appendTimestamp(const char *name, int time, int increment);
    void appendObjectStart(const char * name);
    void appendObject(const char * name, LPDRMETA meta, void const * data, size_t size);
    void appendObjectFinish();
    void appendArrayStart(const char * name);
    void appendArrayFinish();

    template<typename T>
    void appendObject(const char * name, T const & data) {
        appendObject(name, Cpe::Dr::MetaTraits<T>::META, &data, Cpe::Dr::MetaTraits<T>::data_size(data));
    }

    /*query pkg operations*/
    int32_t queryFlags(void) const { return mongo_pkg_query_flags(*this); }
    void querySetFlag(mongo_pro_flags_query_t flag) { mongo_pkg_query_set_flag(*this, flag); }
    void queryUnSetFlag(mongo_pro_flags_query_t flag) { mongo_pkg_query_unset_flag(*this, flag); }
    int32_t queryNumberToSkip(void) const { return mongo_pkg_query_number_to_skip(*this); }
    void querySetNumberToSkip(int32_t number_to_skip) { mongo_pkg_query_set_number_to_skip(*this, number_to_skip); }
    int32_t queryNumberToReturn(void) const { return mongo_pkg_query_number_to_return(*this); }
    void querySetNumberToReturn(int32_t number_to_return) { mongo_pkg_query_set_number_to_return(*this, number_to_return); }

    /*other op*/
    const char * dump_data(mem_buffer_t buffer) const { return mongo_pkg_dump(*this, buffer, 0); }

    static Package & _cast(mongo_pkg_t pkg);
    static Package & _cast(dp_req_t req);
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
