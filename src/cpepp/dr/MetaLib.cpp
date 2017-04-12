#include <limits>
#include <stdexcept>
#include <sstream>
#include "cpepp/utils/MemBuffer.hpp"
#include "cpepp/utils/ErrorCollector.hpp"
#include "cpe/utils/file.h"
#include "cpe/dr/dr_metalib_xml.h"
#include "cpepp/dr/MetaLib.hpp"

namespace Cpe { namespace Dr {

Meta const & MetaLib::meta(const char * name) const {
    Meta const * r = findMeta(name);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "meta-lib " << this->name() << ": get meta (name=" << name << ") fail!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

Meta const & MetaLib::meta(int id) const {
    Meta const * r = findMeta(id);
    if (r == NULL) {
        ::std::ostringstream os;
        os << "meta-lib " << this->name() << ": get meta (id=" << id << ") fail!";
        throw ::std::runtime_error(os.str());
    }
    return *r;
}

MetaLib const &
MetaLib::_cast(const void * p, size_t size) {
    LPDRMETALIB lib = dr_lib_attach(p, size);
    if (lib == NULL) {
        throw ::std::runtime_error("cast to MetaLib: attach fail!"); 
    }

    return _cast(lib);
}

MetaLib const & MetaLib::_cast(LPDRMETALIB ml) {
    if (ml == NULL) {
        throw ::std::runtime_error("cast to MetaLib: input is NULL!"); 
    }
    return *(MetaLib const *)ml;
}

MetaLib const &
MetaLib::_load_from_bin_file(const char * file, Utils::MemBuffer & buf) {
    Utils::ErrorCollector ec;

    buf.clear();

    ssize_t loadSize = file_load_to_buffer(buf, file, ec);
    if (loadSize < 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }

    return _cast(buf.make_continuous(), buf.size());
}

#ifndef CPE_DR_NO_XML
MetaLib const &
MetaLib::_load_from_xml_file(const char * file, Utils::MemBuffer & buf, uint8_t dft_align) {
    Utils::ErrorCollector ec;
    Utils::MemBuffer xmlBuf(NULL);

    if (file_load_to_buffer(xmlBuf, file, ec) < 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }

    buf.clear();
    if (dr_create_lib_from_xml_ex(buf, (char*)xmlBuf.make_continuous(), (int)xmlBuf.size(), dft_align, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }
            
    return _cast(buf.make_continuous(), buf.size());
}

MetaLib const &
MetaLib::_load_from_xml(const char * xml, Utils::MemBuffer & buf, uint8_t dft_align) {
    Utils::ErrorCollector ec;

    if (dr_create_lib_from_xml_ex(buf, xml, (int)strlen(xml), dft_align, ec) != 0) {
        ec.checkThrowWithMsg< ::std::runtime_error>();
    }

    return _cast(buf.make_continuous(), buf.size());
}
#endif
}}
