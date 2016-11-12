#ifndef CPEPP_DR_METALIB_H
#define CPEPP_DR_METALIB_H
#include "cpepp/utils/ClassCategory.hpp"
#include "cpepp/utils/System.hpp"
#include "cpepp/utils/CString.hpp"
#include "cpe/dr/dr_metalib_init.h"
#include "cpe/dr/dr_metalib_manage.h"
#include "System.hpp"

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning(disable:4624)
#endif

namespace Cpe { namespace Dr {

class MetaLib : public Cpe::Utils::SimulateObject {
public:
    operator LPDRMETALIB (void) const { return (LPDRMETALIB)this; }

    size_t size(void) const { return dr_lib_size(*this); }
    Utils::CString const & name(void) const { return Utils::CString::_cast(dr_lib_name(*this)); }
    int version(void) const { return dr_lib_version(*this); }
    int buildVersion(void) const { return dr_lib_build_version(*this); }

    Meta const * findMeta(const char * name) const { return (Meta const *)dr_lib_find_meta_by_name(*this, name); }
    Meta const & meta(const char * name) const;

    Meta const * findMeta(int id) const  { return (Meta const *)dr_lib_find_meta_by_id(*this, id); }
    Meta const & meta(int id) const;

    static MetaLib const & _cast(void const * p, size_t size);
    static MetaLib const & _cast(LPDRMETALIB ml);

    static MetaLib const & _load_from_bin_file(const char * file, Utils::MemBuffer & buf);

#ifndef CPE_DR_NO_XML
    static MetaLib const & _load_from_xml_file(const char * file, Utils::MemBuffer & buf, uint8_t dft_align = 0);
    static MetaLib const & _load_from_xml(const char * xml, Utils::MemBuffer & buf, uint8_t dft_align = 0);
#endif
};

}}

#ifdef _MSC_VER
# pragma warning(pop)
#endif

#endif
